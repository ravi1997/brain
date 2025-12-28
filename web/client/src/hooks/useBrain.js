import { useState, useEffect, useCallback } from 'react';

export const useBrain = () => {
    const [status, setStatus] = useState('disconnected');
    const [messages, setMessages] = useState([]);
    const [brainData, setBrainData] = useState({
        needs: { energy: 100, happiness: 50, boredom: 0 },
        thought: "Initializing...",
        input: ""
    });
    const [neuralEvents, setNeuralEvents] = useState([]);
    const [ws, setWs] = useState(null);

    // Connection management with auto-reconnect
    useEffect(() => {
        let socket;
        let reconnectTimeout;
        let reconnectDelay = 1000;

        const connect = () => {
            setStatus('connecting');
            socket = new WebSocket('ws://' + window.location.host + '/proxy/9001');

            socket.onopen = () => {
                console.log("Connected to Brain Dashboard");
                setStatus('connected');
                reconnectDelay = 1000; // Reset delay on success
            };

            socket.onmessage = (event) => {
                try {
                    const msg = JSON.parse(event.data);
                    if (msg.type === 'log') {
                        setMessages(prev => [...prev.slice(-49), { type: 'log', text: msg.payload }]);
                    } else if (msg.type === 'thought') {
                        setBrainData(prev => ({ ...prev, thought: msg.payload }));
                        setMessages(prev => [...prev.slice(-49), { type: 'thought', text: msg.payload }]);
                    } else if (msg.type === 'chat') {
                        setMessages(prev => [...prev.slice(-49), { type: 'chat', text: msg.payload }]);
                    } else if (msg.type === 'state') {
                        setBrainData(prev => ({ ...prev, ...msg.payload }));
                    } else if (msg.type === 'neural_event') {
                        setNeuralEvents(prev => [...prev.slice(-19), {
                            id: Date.now() + Math.random(),
                            type: msg.event_type,
                            data: msg.data,
                            time: new Date().toLocaleTimeString()
                        }]);
                    }
                } catch (e) {
                    console.error("Parse error", e);
                }
            };

            socket.onclose = () => {
                setStatus('disconnected');
                console.log(`Socket closed. Reconnecting in ${reconnectDelay}ms...`);
                reconnectTimeout = setTimeout(() => {
                    reconnectDelay = Math.min(reconnectDelay * 2, 30000); // Max 30s
                    connect();
                }, reconnectDelay);
            };

            setWs(socket);
        };

        connect();

        return () => {
            if (socket) socket.close();
            if (reconnectTimeout) clearTimeout(reconnectTimeout);
        };
    }, []);

    const sendMessage = useCallback((text) => {
        setMessages(prev => [...prev.slice(-49), { type: 'user', text: text }]);
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify({ type: 'input', payload: text }));
        }
    }, [ws]);

    return { status, messages, brainData, neuralEvents, sendMessage };
};

// Item 43: React Hook for Emotion
export const useEmotion = (wsClient) => {
    const [emotion, setEmotion] = useState({ happiness: 0.5, energy: 1.0 });

    useEffect(() => {
        if (!wsClient) return;
        // Mock sub
    }, [wsClient]);

    return emotion;
};
