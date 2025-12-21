import { BrainClient } from '../core/client.js';

export const NeuralFeature = {
    init() {
        // Initialize Canvas
        const cvs = document.getElementById('neural-canvas');
        if (!cvs) return;

        // Resize
        cvs.width = cvs.offsetWidth;
        cvs.height = cvs.offsetHeight;
        const ctx = cvs.getContext('2d');

        // Create Mock Nodes
        const nodes = [];
        for (let i = 0; i < 30; i++) {
            nodes.push({
                x: Math.random() * cvs.width,
                y: Math.random() * cvs.height,
                vx: (Math.random() - 0.5) * 0.5,
                vy: (Math.random() - 0.5) * 0.5,
                active: 0
            });
        }

        this.running = true;

        // Animation Loop
        const draw = () => {
            if (!this.running) return;
            if (!document.getElementById('neural-canvas')) return; // Stop if view changed

            ctx.fillStyle = 'rgba(0,0,0,0.1)'; // Trail effect
            ctx.fillRect(0, 0, cvs.width, cvs.height);

            // Update & Draw Nodes
            nodes.forEach(n => {
                n.x += n.vx; n.y += n.vy;
                if (n.x < 0 || n.x > cvs.width) n.vx *= -1;
                if (n.y < 0 || n.y > cvs.height) n.vy *= -1;
                n.active *= 0.95; // Decay

                ctx.beginPath();
                ctx.arc(n.x, n.y, 3 + n.active * 5, 0, Math.PI * 2);
                ctx.fillStyle = `rgba(0, 243, 255, ${0.3 + n.active})`;
                ctx.fill();
            });

            // Draw Connections
            ctx.strokeStyle = 'rgba(0, 243, 255, 0.1)';
            for (let i = 0; i < nodes.length; i++) {
                for (let j = i + 1; j < nodes.length; j++) {
                    const d = Math.hypot(nodes[i].x - nodes[j].x, nodes[i].y - nodes[j].y);
                    if (d < 100) {
                        ctx.beginPath();
                        ctx.moveTo(nodes[i].x, nodes[i].y);
                        ctx.lineTo(nodes[j].x, nodes[j].y);
                        ctx.stroke();
                    }
                }
            }

            requestAnimationFrame(draw);
        };
        draw();

        BrainClient.subscribe('thoughts', (data) => {
            this.appendLog('neural-stream', data);
            // "Pulse" random nodes
            for (let i = 0; i < 5; i++) {
                nodes[Math.floor(Math.random() * nodes.length)].active = 1.0;
            }
        });
        BrainClient.subscribe('research', (data) => {
            this.appendLog('research-stream', data);
        });
    },

    cleanup() {
        this.running = false;
    },

    appendLog(id, text) {
        const el = document.getElementById(id);
        if (!el) return;
        const div = document.createElement('div');
        div.textContent = text;
        div.style.color = '#00f3ff';
        div.style.marginBottom = '5px';
        div.style.fontFamily = 'monospace';
        el.appendChild(div);
        el.scrollTop = el.scrollHeight;
    }
};
