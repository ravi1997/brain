import { BrainClient } from '../core/client.js';

export const CognitionFeature = {
    canvas: null,
    ctx: null,
    currentEmotions: { happiness: 0.5, energy: 0.5, boredom: 0.0, anger: 0.0, fear: 0.0 },

    init() {
        console.log("[Cognition] Initializing...");

        // 1. Setup Radar Chart
        this.canvas = document.getElementById('emotion-radar');
        if (this.canvas) {
            this.ctx = this.canvas.getContext('2d');
            this.loop();
        }

        // 2. Subscribe to Brain State
        BrainClient.subscribe('emotions', (jsonString) => {
            try {
                // The stream sends the Full JSON state? Or just a log? 
                // Based on `on_emotion_update`, it sends a JSON string.
                const state = JSON.parse(jsonString);
                if (state.emotions) {
                    this.currentEmotions = state.emotions;
                    this.updateValues();
                    this.pulseNode('node-input'); // Just as a heartbeat
                }
            } catch (e) {
                // Might be a log string instead of JSON
                // console.warn("[Cognition] Failed to parse emotion state:", e);
            }
        });

        // 3. Listen for Logs (to visualize decisions/flow)
        BrainClient.subscribe('logs', (msg) => {
            if (msg.includes("[Cognition]")) {
                this.pulseNode('node-decision');
                this.addLog(msg.replace("[Cognition]:", ""));

                if (msg.includes("Research")) this.pulseNode('node-reasoning');
                if (msg.includes("Task")) this.pulseNode('node-planning');
            }
            if (msg.includes("[Reflex]")) {
                this.pulseNode('node-decision');
            }
        });

        BrainClient.subscribe('thoughts', (msg) => {
            this.pulseNode('node-reasoning');
            this.pulseNode('node-output');
        });

        // 4. Initial Draw
        this.drawRadar();
    },

    pulseNode(id) {
        const el = document.getElementById(id);
        if (el) {
            el.classList.add('active');
            setTimeout(() => el.classList.remove('active'), 200);
        }
    },

    addLog(text) {
        const logs = document.getElementById('decision-log');
        if (!logs) return;
        const div = document.createElement('div');
        div.className = 'log-entry';
        div.innerHTML = `<span class="time">[${new Date().toLocaleTimeString()}]</span> ${text}`;
        logs.prepend(div);
        if (logs.children.length > 20) logs.lastChild.remove();
    },

    updateValues() {
        const set = (id, val) => {
            const el = document.getElementById(id);
            if (el) el.textContent = val.toFixed(2);
        };
        set('val-hap', this.currentEmotions.happiness);
        set('val-eng', this.currentEmotions.energy);
        set('val-bor', this.currentEmotions.boredom);
    },

    loop() {
        if (!document.getElementById('emotion-radar')) return; // Stop if view changed
        this.drawRadar();
        requestAnimationFrame(() => this.loop());
    },

    drawRadar() {
        if (!this.ctx) return;
        const ctx = this.ctx;
        const w = this.canvas.width;
        const h = this.canvas.height;
        const cx = w / 2;
        const cy = h / 2;
        const r = Math.min(w, h) / 2 - 20;

        // Clear
        ctx.clearRect(0, 0, w, h);

        // Debug Grid
        ctx.strokeStyle = '#333';
        ctx.beginPath();
        ctx.arc(cx, cy, r, 0, Math.PI * 2);
        ctx.stroke();
        ctx.beginPath();
        ctx.arc(cx, cy, r * 0.5, 0, Math.PI * 2);
        ctx.stroke();

        // Labels & Points
        const keys = ['happiness', 'energy', 'boredom', 'anger', 'fear'];
        // const labels = ['HAP', 'ENG', 'BOR', 'ANG', 'FEA'];
        const step = (Math.PI * 2) / keys.length;

        // Draw Shape
        ctx.beginPath();
        keys.forEach((key, i) => {
            const val = this.currentEmotions[key] || 0;
            const angle = i * step - Math.PI / 2;
            const dist = val * r;
            const x = cx + Math.cos(angle) * dist;
            const y = cy + Math.sin(angle) * dist;
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
        });
        ctx.closePath();
        ctx.fillStyle = 'rgba(0, 243, 255, 0.4)';
        ctx.fill();
        ctx.strokeStyle = '#00f3ff';
        ctx.lineWidth = 2;
        ctx.stroke();

        // Draw Axis
        keys.forEach((key, i) => {
            const angle = i * step - Math.PI / 2;
            ctx.strokeStyle = '#444';
            ctx.beginPath();
            ctx.moveTo(cx, cy);
            ctx.lineTo(cx + Math.cos(angle) * r, cy + Math.sin(angle) * r);
            ctx.stroke();

            // Label
            ctx.fillStyle = '#fff';
            ctx.font = '10px monospace';
            const lx = cx + Math.cos(angle) * (r + 15);
            const ly = cy + Math.sin(angle) * (r + 15);
            ctx.textAlign = 'center';
            ctx.fillText(key.toUpperCase().substring(0, 3), lx, ly);
        });
    }
};
