import { BrainClient } from '../core/client.js';

export const TerminalFeature = {
    init() {
        const inp = document.getElementById('term-in');

        BrainClient.subscribe('chat', (data) => {
            if (data.startsWith('Brain:')) {
                this.appendLog('term-out', data, 'brain');
            } else {
                this.appendLog('term-out', data, 'user');
            }
        });

        if (inp) {
            inp.addEventListener('keypress', (e) => {
                if (e.key === 'Enter') {
                    const txt = inp.value;
                    BrainClient.send('chat', txt);
                    this.appendLog('term-out', `root@brain:~# ${txt}`, 'user');
                    inp.value = '';
                }
            });
            inp.focus();
        }
    },

    appendLog(id, text, type = '') {
        const el = document.getElementById(id);
        if (!el) return;
        const div = document.createElement('div');
        div.className = `term-msg ${type}`;
        div.textContent = text;
        el.appendChild(div);
        el.scrollTop = el.scrollHeight;
    }
};
