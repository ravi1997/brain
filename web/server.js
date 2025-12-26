console.log("SERVER STARTING...");
const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const net = require('net');
const path = require('path');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

// Serve static UI
app.use(express.static(path.join(__dirname, 'public')));

// WebSocket Proxy Logic
// Client connects to ws://host:5000/proxy/9005 -> Node connects to brain:9005
wss.on('connection', (ws, req) => {
    console.log(`[Proxy] Incoming connection on ${req.url}`);

    // Parse port from URL: /proxy/9005
    const match = req.url.match(/\/proxy\/(\d+)/);
    if (!match) {
        console.log(`[Proxy] No port match for ${req.url}, closing connection.`);
        ws.close();
        return;
    }

    const targetPort = parseInt(match[1]);
    const targetHost = 'brain'; // Docker service name

    console.log(`[Proxy] New Client connecting to ${targetHost}:${targetPort}`);

    const tcpClient = new net.Socket();

    tcpClient.connect(targetPort, targetHost, () => {
        console.log(`[Proxy] Connected to ${targetHost}:${targetPort}`);
    });

    // TCP -> WS: Wrap raw text in JSON
    tcpClient.on('data', (data) => {
        if (ws.readyState === WebSocket.OPEN) {
            const lines = data.toString().split('\n');
            lines.forEach(line => {
                if (line.trim()) {
                    ws.send(JSON.stringify({ type: 'log', payload: line.trim() }));
                }
            });
        }
    });

    tcpClient.on('error', (err) => {
        console.error(`[Proxy] TCP Error on ${targetPort}:`, err.message);
        ws.close();
    });

    tcpClient.on('close', () => {
        console.log(`[Proxy] TCP Connection closed on ${targetPort}`);
        ws.close();
    });

    // WS -> TCP: Parse JSON and send raw text
    ws.on('message', (data) => {
        try {
            const msg = JSON.parse(data);
            if (msg.type === 'input') {
                console.log(`[Proxy] Forwarding input to brain: ${msg.payload}`);
                tcpClient.write(msg.payload + '\n');
            }
        } catch (e) {
            console.log(`[Proxy] Forwarding raw input to brain: ${data}`);
            tcpClient.write(data + '\n');
        }
    });

    ws.on('close', () => {
        console.log(`[Proxy] WS Client disconnected from ${targetPort}`);
        tcpClient.destroy();
    });
});

const PORT = 5000;
server.listen(PORT, () => {
    console.log(`Dashboard running on http://localhost:${PORT}`);
});
