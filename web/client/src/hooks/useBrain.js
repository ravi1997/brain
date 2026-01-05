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
    const [pendingCommands] = useState(new Map());

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
                    } else if (msg.type === 'cognitive_test_result') {
                        const { id, payload } = msg;
                        if (pendingCommands.has(id)) {
                            const { resolve } = pendingCommands.get(id);
                            resolve(payload);
                            pendingCommands.delete(id);
                        }
                    } else if (msg.type === 'error') {
                        console.error("Backend error:", msg.payload);
                    }
                } catch (e) {
                    console.error("Parse error", e);
                }
            };

            socket.onclose = () => {
                setStatus('disconnected');
                console.log(`Socket closed. Reconnecting in ${reconnectDelay}ms...`);

                // Reject all pending commands on disconnect
                pendingCommands.forEach(({ reject }) => reject(new Error('Connection closed')));
                pendingCommands.clear();

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
    }, [pendingCommands]);

    const sendMessage = useCallback((text) => {
        setMessages(prev => [...prev.slice(-49), { type: 'user', text: text }]);
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify({ type: 'input', payload: text }));
        }
    }, [ws]);

    const sendCommand = useCallback((command) => {
        return new Promise((resolve, reject) => {
            if (ws && ws.readyState === WebSocket.OPEN) {
                const commandId = Date.now() + Math.floor(Math.random() * 1000);
                pendingCommands.set(commandId, { resolve, reject });
                ws.send(JSON.stringify({ ...command, id: commandId }));

                // Set a timeout for the command
                setTimeout(() => {
                    if (pendingCommands.has(commandId)) {
                        pendingCommands.delete(commandId);
                        reject(new Error('Command timeout'));
                    }
                }, 10000); // 10s timeout
            } else {
                reject(new Error('WebSocket not connected'));
            }
        });
    }, [ws, pendingCommands]);

    return {
        connected: status === 'connected',
        status,
        messages,
        brainData,
        neuralEvents,
        sendMessage,
        sendCommand
    };
};

// Hook for specific port communication (Logs, Chat, Admin, etc.)
export const usePort = (port) => {
    const [messages, setMessages] = useState([]);
    const [status, setStatus] = useState('disconnected');
    const [ws, setWs] = useState(null);

    useEffect(() => {
        if (!port) return;

        let socket;
        const connect = () => {
            setStatus('connecting');
            socket = new WebSocket(`ws://${window.location.host}/proxy/${port}`);

            socket.onopen = () => {
                setStatus('connected');
                console.log(`Connected to Port ${port}`);
            };

            socket.onmessage = (event) => {
                let text = event.data;
                try {
                    const msg = JSON.parse(event.data);
                    if (msg.payload) text = msg.payload;
                } catch (e) {
                    // Fallback to raw text
                }
                setMessages(prev => [...prev.slice(-99), {
                    id: Date.now() + Math.random(),
                    text: typeof text === 'string' ? text : JSON.stringify(text),
                    time: new Date().toLocaleTimeString()
                }]);
            };

            socket.onclose = () => {
                setStatus('disconnected');
            };

            setWs(socket);
        };

        connect();

        return () => {
            if (socket) socket.close();
        };
    }, [port]);

    const send = useCallback((text) => {
        if (ws && ws.readyState === WebSocket.OPEN) {
            // Forward as input to the specific port
            ws.send(JSON.stringify({ type: 'input', payload: text }));
        }
    }, [ws]);

    return { status, messages, send };
};
