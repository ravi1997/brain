export const BrainClient = {
    ws: null,
    listeners: {},
    port: 5000, // Proxy Port

    init() {
        // No global connection needed for proxy setup, we connect on demand or just use one
        // tailored approach: The proxy is at 5000, and we use paths /proxy/PORT
        // Actually, the current implementations make new WS connections for EACH port.
        // Let's keep that pattern for now but wrap it cleaner.
    },

    // Connect to a specific Brain port via Proxy
    // returns a WS instance
    connect(brainPort, onMessage) {
        const proto = window.location.protocol === 'https:' ? 'wss' : 'ws';
        const url = `${proto}://${window.location.host}/proxy/${brainPort}`;
        const ws = new WebSocket(url);

        ws.onopen = () => {
            console.log(`[Client] Connected to Port ${brainPort}`);
            document.getElementById('sys-status-light').style.boxShadow = '0 0 8px #00ff9d';
        };

        ws.onmessage = (e) => {
            if (onMessage) onMessage(e.data);
            this.emit(brainPort, e.data); // Also emit to global listeners if any
        };

        ws.onclose = () => {
            console.warn(`[Client] Disconnected from Port ${brainPort}`);
            setTimeout(() => this.connect(brainPort, onMessage), 3000);
        };

        if (!this.connections) this.connections = {};
        this.connections[brainPort] = ws;
        return ws;
    },

    send(brainPort, msg) {
        if (this.connections && this.connections[brainPort]) {
            this.connections[brainPort].send(msg);
        }
    },

    // Simple Event Bus
    subscribe(channel, cb) {
        if (!this.listeners[channel]) this.listeners[channel] = [];
        this.listeners[channel].push(cb);
    },

    emit(channel, data) {
        if (this.listeners[channel]) {
            this.listeners[channel].forEach(cb => cb(data));
        }
    }
};
