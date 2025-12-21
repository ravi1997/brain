import { BrainClient } from '../core/client.js';

export const DashboardFeature = {
    init() {
        // Subscribe to Emotions
        BrainClient.subscribe('emotions', (data) => {
            const energy = data.match(/Energy: ([\d.]+)/);
            if (energy && document.getElementById('dash-energy')) {
                document.getElementById('dash-energy').textContent = Math.round(energy[1] * 100) + '%';
            }
            const hap = data.match(/Happiness: ([\d.]+)/);
            if (hap && document.getElementById('dash-happiness')) {
                document.getElementById('dash-happiness').textContent = Math.round(hap[1] * 100) + '%';
            }
            const bor = data.match(/Boredom: ([\d.]+)/);
            if (bor && document.getElementById('dash-boredom')) {
                document.getElementById('dash-boredom').textContent = Math.round(bor[1] * 100) + '%';
            }
        });

        // Subscribe to Tasks for "Current Focus"
        BrainClient.subscribe('tasks', (json) => {
            try {
                const data = JSON.parse(json);
                const focus = document.getElementById('dash-focus');
                if (focus) {
                    focus.textContent = data.active ? data.active.desc.toUpperCase() : "IDLE / WAITING";
                }
            } catch (e) { }
        });
    }
};
