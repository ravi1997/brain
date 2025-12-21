import { BrainClient } from '../core/client.js';

export const LibraryFeature = {
    init() {
        BrainClient.subscribe('research', (data) => {
            const el = document.getElementById('library-content');
            if (el) {
                const div = document.createElement('div');
                div.className = 'task-item'; // reuse style
                div.textContent = data;
                el.prepend(div);
            }
        });
    }
};
