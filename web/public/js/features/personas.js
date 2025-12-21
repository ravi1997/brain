import { BrainClient } from '../core/client.js';

export const PersonasFeature = {
    init() {
        // Init Sliders
        const sliders = ['curiosity', 'playfulness', 'friendliness', 'formality', 'positivity'];
        sliders.forEach(key => {
            const el = document.getElementById(`p-${key}`);
            if (!el) return;

            // On Drag
            el.addEventListener('input', (e) => {
                document.getElementById(`v-${key}`).textContent = e.target.value;
            });
            // On Replace (Commit)
            el.addEventListener('change', (e) => {
                const val = parseFloat(e.target.value);
                BrainClient.send('control', JSON.stringify({ [key]: val }));
            });
        });

        // Listen to State Updates
        BrainClient.subscribe('control', (json) => {
            try {
                const state = JSON.parse(json);
                const p = state.personality;
                const e = state.emotions;

                // Update Sliders if not dragging
                sliders.forEach(key => {
                    const el = document.getElementById(`p-${key}`);
                    if (el && document.activeElement !== el && p[key] !== undefined) {
                        el.value = p[key];
                        document.getElementById(`v-${key}`).textContent = p[key];
                    }
                });

                // Update Emotion Bars
                if (e) {
                    if (e.happiness) document.querySelector('#bar-happiness').style.width = (e.happiness * 100) + '%';
                    if (e.anger) document.querySelector('#bar-anger').style.width = (e.anger * 100) + '%';
                    if (e.sadness) document.querySelector('#bar-sadness').style.width = (e.sadness * 100) + '%';
                    if (e.energy) document.querySelector('#bar-energy').style.width = (e.energy * 100) + '%';
                }
            } catch (e) { }
        });

        // Global Inject Function
        window.injectEmotion = (type, amount) => {
            BrainClient.send('control', JSON.stringify({ [type]: 0.9 }));
        };
    }
};
