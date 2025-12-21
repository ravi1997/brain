import { BrainClient } from '../core/client.js';

export const TasksFeature = {
    init() {
        BrainClient.subscribe('tasks', (json) => {
            try {
                const data = JSON.parse(json);

                // Render Pending
                const pendEl = document.getElementById('col-pending');
                if (pendEl) {
                    pendEl.innerHTML = data.pending.map(t => this.renderTaskCard(t)).join('');
                }

                // Render Active
                const actEl = document.getElementById('col-active');
                if (actEl) {
                    actEl.innerHTML = data.active ? this.renderTaskCard(data.active, true) : '';
                }

                // Render History
                const histEl = document.getElementById('col-history');
                if (histEl) {
                    histEl.innerHTML = data.history.map(t => this.renderTaskCard(t)).join('');
                }
            } catch (e) { }
        });
    },

    renderTaskCard(t, isActive = false) {
        const pClass = t.priority >= 2 ? 'priority-high' : 'priority-low';
        return `
            <div class="task-item ${pClass}">
                <div class="meta">#${t.id} | P${t.priority}</div>
                <div class="desc">${t.desc}</div>
                ${isActive ? '<div style="margin-top:5px; color:#00f3ff; font-size:0.7rem;">// EXECUTING</div>' : ''}
            </div>
        `;
    }
};
