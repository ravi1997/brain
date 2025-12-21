import { useState, useEffect, useCallback } from 'react';

export const useBrain = () => {
    const [status, setStatus] = useState('disconnected');
    const [messages, setMessages] = useState([]);
    const [brainData, setBrainData] = useState({
        needs: { energy: 100, happiness: 50, boredom: 0 },
        thought: "Initializing...",
        input: ""
    });
    const [ws, setWs] = useState(null);

    // Initial connection
    useEffect(() => {
        const socket = new WebSocket('ws://' + window.location.host + '/ws');

        socket.onopen = () => {
            console.log("Connected to Brain");
            setStatus('connected');
        };

        socket.onmessage = (event) => {
            try {
                const msg = JSON.parse(event.data);
                if (msg.type === 'log') {
                    setMessages(prev => [...prev.slice(-49), { type: 'log', text: msg.payload }]);
                } else if (msg.type === 'thought') {
                    setBrainData(prev => ({ ...prev, thought: msg.payload }));
                    setMessages(prev => [...prev.slice(-49), { type: 'thought', text: msg.payload }]);
                } else if (msg.type === 'state') {
                    // Assuming payload is key:value or json
                    // For now handled via logs mostly
                } else if (msg.type === 'chat') {
                    setMessages(prev => [...prev.slice(-49), { type: 'chat', text: msg.payload }]);
                }
            } catch (e) {
                console.error("Parse error", e);
            }
        };

        socket.onclose = () => setStatus('disconnected');

        setWs(socket);

        return () => socket.close();
    }, []);

    const sendMessage = useCallback((text) => {
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify({ type: 'input', payload: text }));
            setMessages(prev => [...prev.slice(-49), { type: 'user', text: text }]);
        }
    }, [ws]);

    return { status, messages, brainData, sendMessage };
};
