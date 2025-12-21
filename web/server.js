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
    // Parse port from URL: /proxy/9005
    const match = req.url.match(/\/proxy\/(\d+)/);
    if (!match) {
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

    // TCP -> WS
    tcpClient.on('data', (data) => {
        if (ws.readyState === WebSocket.OPEN) {
            ws.send(data.toString());
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

    // WS -> TCP
    ws.on('message', (message) => {
        tcpClient.write(message + '\n');
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
