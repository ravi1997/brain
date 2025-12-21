import { BrainClient } from '../core/client.js';

export const SystemFeature = {
    init() {
        // Simple Rolling Chart Class
        class RollingChart {
            constructor(canvasId, color) {
                this.cvs = document.getElementById(canvasId);
                if (!this.cvs) return;
                this.ctx = this.cvs.getContext('2d');
                this.data = new Array(50).fill(0);
                this.color = color;
            }
            push(val) {
                this.data.push(val);
                this.data.shift();
                this.draw();
            }
            draw() {
                const { width, height } = this.cvs;
                this.ctx.clearRect(0, 0, width, height);

                this.ctx.beginPath();
                this.ctx.moveTo(0, height);
                for (let i = 0; i < this.data.length; i++) {
                    const x = (i / (this.data.length - 1)) * width;
                    const y = height - (this.data[i] * height);
                    this.ctx.lineTo(x, y);
                }
                this.ctx.strokeStyle = this.color;
                this.ctx.lineWidth = 2;
                this.ctx.stroke();

                // Fill
                this.ctx.lineTo(width, height);
                this.ctx.lineTo(0, height);
                this.ctx.fillStyle = this.color.replace(')', ', 0.2)').replace('rgb', 'rgba');
                this.ctx.fill();
            }
        }

        const cpuChart = new RollingChart('chart-cpu', 'rgb(0, 243, 255)');
        const memChart = new RollingChart('chart-mem', 'rgb(189, 147, 249)');

        BrainClient.subscribe('extra', (data) => {
            try {
                const stats = JSON.parse(data);
                if (document.getElementById('sys-cpu')) document.getElementById('sys-cpu').textContent = stats.cpu + '%';
                if (document.getElementById('sys-mem')) document.getElementById('sys-mem').textContent = (stats.memory_usage / 1024).toFixed(1) + 'KB';
                if (document.getElementById('sys-syn')) document.getElementById('sys-syn').textContent = stats.synapses;

                // Update Charts
                if (cpuChart) cpuChart.push(stats.cpu / 100);
                if (memChart) memChart.push(stats.memory_usage / (1024 * 1024));
            } catch (e) { }
        });
    }
};
