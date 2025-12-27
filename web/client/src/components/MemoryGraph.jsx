import React, { useEffect, useState, useRef } from 'react';

const MemoryGraph = () => {
    const [nodes, setNodes] = useState([]);
    const [links, setLinks] = useState([]);
    const canvasRef = useRef(null);
    const requestRef = useRef();
    const dataRef = useRef({ nodes: [], links: [] });
    const draggedNodeRef = useRef(null);

    useEffect(() => {
        const socket = new WebSocket('ws://' + window.location.host + '/proxy/9012');
        socket.onmessage = (event) => {
            try {
                const msg = JSON.parse(event.data);
                if (msg.type === 'graph') {
                    updateGraphData(msg.nodes, msg.links);
                }
            } catch (e) {
                console.error("Graph parse error", e);
            }
        };
        return () => {
            socket.close();
            cancelAnimationFrame(requestRef.current);
        };
    }, []);

    const updateGraphData = (newNodes, newLinks) => {
        const currentNodes = dataRef.current.nodes;
        const nodeMap = new Map(currentNodes.map(n => [n.id, n]));

        const processedNodes = newNodes.map(nd => {
            const existing = nodeMap.get(nd.id);
            return {
                ...nd,
                x: existing ? existing.x : Math.random() * 800,
                y: existing ? existing.y : Math.random() * 600,
                vx: existing ? existing.vx : 0,
                vy: existing ? existing.vy : 0
            };
        });

        dataRef.current = { nodes: processedNodes, links: newLinks };
    };

    const animate = () => {
        const { nodes, links } = dataRef.current;
        if (nodes.length === 0) {
            requestRef.current = requestAnimationFrame(animate);
            return;
        }

        // Simple Force Simulation
        const k = 0.05; // Force constant
        const friction = 0.9;
        const center = { x: 400, y: 300 };

        // 1. Repulsion between all nodes
        for (let i = 0; i < nodes.length; i++) {
            for (let j = i + 1; j < nodes.length; j++) {
                const dx = nodes[i].x - nodes[j].x;
                const dy = nodes[i].y - nodes[j].y;
                const distSq = dx * dx + dy * dy + 1;
                const force = 100.0 / distSq;
                const fx = (dx / Math.sqrt(distSq)) * force;
                const fy = (dy / Math.sqrt(distSq)) * force;
                nodes[i].vx += fx;
                nodes[i].vy += fy;
                nodes[j].vx -= fx;
                nodes[j].vy -= fy;
            }
        }

        // 2. Attraction for links
        links.forEach(link => {
            const s = nodes.find(n => n.id === link.source);
            const t = nodes.find(n => n.id === link.target);
            if (!s || !t) return;
            const dx = t.x - s.x;
            const dy = t.y - s.y;
            const dist = Math.sqrt(dx * dx + dy * dy);
            const force = (dist - 100) * 0.01;
            const fx = (dx / dist) * force;
            const fy = (dy / dist) * force;
            s.vx += fx;
            s.vy += fy;
            t.vx -= fx;
            t.vy -= fy;
        });

        // 3. Gravity to center
        // 3. Gravity to center (and integration)
        nodes.forEach(n => {
            if (n === draggedNodeRef.current) return; // Skip physics for dragged node

            n.vx += (center.x - n.x) * 0.005;
            n.vy += (center.y - n.y) * 0.005;
            n.x += n.vx;
            n.y += n.vy;
            n.vx *= friction;
            n.vy *= friction;
        });

        draw();
        requestRef.current = requestAnimationFrame(animate);
    };

    const draw = () => {
        const canvas = canvasRef.current;
        if (!canvas) return;
        const ctx = canvas.getContext('2d');
        const { nodes, links } = dataRef.current;

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        // Draw links
        ctx.strokeStyle = 'rgba(71, 85, 105, 0.4)';
        links.forEach(link => {
            const s = nodes.find(n => n.id === link.source);
            const t = nodes.find(n => n.id === link.target);
            if (!s || !t) return;
            ctx.beginPath();
            ctx.lineWidth = Math.log(link.weight + 1) + 0.5;
            ctx.moveTo(s.x, s.y);
            ctx.lineTo(t.x, t.y);
            ctx.stroke();
        });

        // Draw nodes
        nodes.forEach(n => {
            ctx.fillStyle = '#38bdf8';
            ctx.beginPath();
            ctx.arc(n.x, n.y, 4 + Math.sqrt(n.val), 0, Math.PI * 2);
            ctx.fill();

            ctx.fillStyle = '#94a3b8';
            ctx.font = '10px Inter, sans-serif';
            ctx.fillText(n.id, n.x + 8, n.y + 4);
        });
    };

    useEffect(() => {
        requestRef.current = requestAnimationFrame(animate);
        return () => cancelAnimationFrame(requestRef.current);
    }, []);

    const getPointerPos = (e) => {
        const canvas = canvasRef.current;
        const rect = canvas.getBoundingClientRect();
        const clientX = e.touches ? e.touches[0].clientX : e.clientX;
        const clientY = e.touches ? e.touches[0].clientY : e.clientY;
        // Scale in case canvas is resized via CSS
        const scaleX = canvas.width / rect.width;
        const scaleY = canvas.height / rect.height;
        return {
            x: (clientX - rect.left) * scaleX,
            y: (clientY - rect.top) * scaleY
        };
    };

    const handleStart = (e) => {
        const { x, y } = getPointerPos(e);
        const { nodes } = dataRef.current;
        // Find clicked node
        for (let i = nodes.length - 1; i >= 0; i--) {
            const n = nodes[i];
            const dist = Math.sqrt((n.x - x) ** 2 + (n.y - y) ** 2);
            if (dist < 10 + Math.sqrt(n.val)) { // Hit radius
                draggedNodeRef.current = n;
                n.vx = 0; // Stop momentum
                n.vy = 0;
                break;
            }
        }
    };

    const handleMove = (e) => {
        if (!draggedNodeRef.current) return;
        const { x, y } = getPointerPos(e);
        draggedNodeRef.current.x = x;
        draggedNodeRef.current.y = y;
        e.preventDefault(); // Prevent scrolling on touch
    };

    const handleEnd = () => {
        draggedNodeRef.current = null;
    };

    return (
        <div className="bg-slate-900 p-4 rounded-xl border border-slate-700 h-[600px] overflow-hidden flex flex-col">
            <h3 className="text-slate-400 text-sm font-semibold mb-4 uppercase tracking-wider">Associative Knowledge Graph</h3>
            <canvas 
                ref={canvasRef} 
                width={800} 
                height={600} 
                className="w-full h-full cursor-move touch-none"
                onMouseDown={handleStart}
                onMouseMove={handleMove}
                onMouseUp={handleEnd}
                onMouseLeave={handleEnd}
                onTouchStart={handleStart}
                onTouchMove={handleMove}
                onTouchEnd={handleEnd}
            />
        </div>
    );
};

export default MemoryGraph;
