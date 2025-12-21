import { BrainClient } from '../core/client.js';

export const AdminFeature = {
    init() {
        window.sendAdmin = (cmd) => {
            BrainClient.send('admin', cmd);
            const log = document.getElementById('admin-log');
            if (log) log.textContent = `> Sent command: ${cmd}...`;
        };

        BrainClient.subscribe('admin', (data) => {
            const log = document.getElementById('admin-log');
            if (log) log.textContent = `Response: ${data}`;
        });
    }
};
