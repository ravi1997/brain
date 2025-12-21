import { BrainClient } from '../core/client.js';

export const CognitionFeature = {
    init: () => {
        const stream = document.getElementById('cognition-stream');
        const entityCloud = document.getElementById('entity-cloud');
        const memoryFeed = document.getElementById('memory-feed');

        // Listen for 'chat' to extract entities (simulation for now, or real if backend sends JSON)
        BrainClient.subscribe('chat', (data) => {
            // For now, we visualize the chat flow here too
            addStreamItem(stream, 'input', data);

            // Heuristic visualization (since we don't have a dedicated 'entity' event port yet)
            // In a real scenario, the brain would emit specific entity events.
            // We can try to parse logs if they contain "[Entity]"
        });

        BrainClient.subscribe('logs', (data) => {
            // Look for specific log patterns we added in brain.cpp
            // output: "[Memory]: Recalled fact about '...'"

            if (data.includes("[Memory]: Recalled")) {
                addMemoryItem(memoryFeed, data);
                addStreamItem(stream, 'memory', "Associative Memory Triggered");
            }

            // "Extracted entities: ..." - We didn't add this log explicitly in brain.cpp yet,
            // but we can infer or add it later.
        });
    },
    destroy: () => {
        // Cleanup if needed
    }
};

function addStreamItem(container, type, text) {
    if (!container) return;
    const div = document.createElement('div');
    div.className = `stream-item ${type}`;
    div.innerHTML = `<span class="timestamp">${new Date().toLocaleTimeString()}</span> <span class="content">${text}</span>`;
    container.prepend(div);
    if (container.children.length > 50) container.lastChild.remove();
}

function addMemoryItem(container, text) {
    if (!container) return;
    const div = document.createElement('div');
    div.className = 'memory-card';
    div.innerText = text;
    container.prepend(div);
    if (container.children.length > 10) container.lastChild.remove();
}
