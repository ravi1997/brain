/**
 * CORTEX OS v2.0
 * Modular Client Logic
 */

// --- 1. ROUTER ---
const Router = {
    current: 'dashboard',

    init() {
        document.querySelectorAll('.nav-links li').forEach(li => {
            li.addEventListener('click', () => {
                const view = li.dataset.view;
                this.navigate(view);
            });
        });

        // Load default
        this.navigate('dashboard');
    },

    navigate(viewId) {
        // Update Nav UI
        document.querySelectorAll('.nav-links li').forEach(li => li.classList.remove('active'));
        const nav = document.querySelector(`.nav-links li[data-view="${viewId}"]`);
        if (nav) nav.classList.add('active');

        // Render View
        this.current = viewId;
        const container = document.getElementById('view-container');
        const tmpl = document.getElementById(`tmpl-${viewId}`);
        if (tmpl) {
            container.innerHTML = tmpl.innerHTML;
            document.getElementById('page-title').textContent = nav.querySelector('span').textContent.toUpperCase();

            // Initialize View Logic
            ViewManager.initView(viewId);
        }
    }
};

// --- 2. BRAIN CLIENT (Data Layer) ---
const BrainClient = {
    sockets: {},
    host: window.location.hostname,
    port: window.location.port,
    listeners: [], // { type: 'energy', callback: fn }

    connectAll() {
        this.connect(9002, 'emotions'); // Vital Stats
        this.connect(9010, 'tasks');    // Task Data
        this.connect(9006, 'thoughts'); // Thought Stream
        this.connect(9007, 'research'); // Research
        this.connect(9005, 'chat');     // Chat
        this.connect(9003, 'logs');     // Logs
        this.connect(9004, 'errors');   // Errors
        this.connect(9008, 'system');   // System/Extra
        this.connect(9009, 'admin');    // Admin
        this.connect(9011, 'control');  // Personality Control
    },

    connect(port, type) {
        const ws = new WebSocket(`ws://${this.host}:${this.port}/proxy/${port}`);
        ws.onmessage = (e) => this.handleMessage(type, e.data);
        ws.onclose = () => setTimeout(() => this.connect(port, type), 3000);
        this.sockets[type] = ws;
    },

    handleMessage(type, data) {
        // Global Ticker for everything except raw streams
        if (type === 'logs' || type === 'chat') {
            const ticker = document.getElementById('ticker-text');
            if (ticker) ticker.textContent = `[${type.toUpperCase()}] ${data.substring(0, 80)}`;
        }

        // Notify Listeners
        this.listeners.filter(l => l.type === type).forEach(l => l.callback(data));
    },

    subscribe(type, callback) {
        this.listeners.push({ type, callback });
    },

    send(type, msg) {
        if (this.sockets[type] && this.sockets[type].readyState === WebSocket.OPEN) {
            this.sockets[type].send(msg);
        }
    }
};

// --- 3. VIEW MANAGER ---
const ViewManager = {
    initView(viewId) {
        if (viewId === 'dashboard') this.setupDashboard();
        if (viewId === 'tasks') this.setupTasks();
        if (viewId === 'neural') this.setupNeural();
        if (viewId === 'personas') this.setupPersonas();
        if (viewId === 'library') this.setupLibrary();
        if (viewId === 'system') this.setupSystem();
        if (viewId === 'admin') this.setupAdmin();
        if (viewId === 'terminal') this.setupTerminal();
    },

    setupDashboard() {
        // Subscribe to Emotions
        BrainClient.subscribe('emotions', (data) => {
            // "Energy: 0.95 | Boredom: 0.05 | Happiness: 0.50"
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
    },

    setupTasks() {
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
    },

    setupNeural() {
        BrainClient.subscribe('thoughts', (data) => {
            this.appendLog('neural-stream', data);
        });
        BrainClient.subscribe('research', (data) => {
            this.appendLog('research-stream', data);
        });
    },

    setupPersonas() {
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

        // Listen to State Updates from Brain (via Port 9011)
        BrainClient.subscribe('control', (json) => {
            try {
                const state = JSON.parse(json);
                const p = state.personality;
                const e = state.emotions;

                // Update Sliders if not dragging (to avoid fighting user)
                sliders.forEach(key => {
                    const el = document.getElementById(`p-${key}`);
                    // Only update if not active element to avoid jitter
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
            // We can just set the value directly via the API
            // But we need the current value first? 
            // Simplification: Sending a value to the API sets it. 
            // Ideally we'd have a Delta API, but Set is fine for now; we'll hardcode typical "Dose" values
            // Actually, we can just send "happiness": 1.0 to max it out, or whatever.
            // Let's just set it to a high value to simulate "injection"
            BrainClient.send('control', JSON.stringify({ [type]: 0.9 }));
        };
    },



    setupLibrary() {
        BrainClient.subscribe('research', (data) => {
            const el = document.getElementById('library-content');
            if (el) {
                const div = document.createElement('div');
                div.className = 'task-item'; // reuse style
                div.textContent = data;
                el.prepend(div);
            }
        });
    },

    setupSystem() {
        BrainClient.subscribe('system', (json) => {
            try {
                const data = JSON.parse(json);
                document.getElementById('sys-cpu').textContent = data.cpu + '%';
                document.getElementById('sys-mem').textContent = (data.memory_usage / 1024).toFixed(1) + 'KB';
                document.getElementById('sys-syn').textContent = data.synapses;
            } catch (e) { }
        });
    },

    setupAdmin() {
        window.sendAdmin = (cmd) => {
            BrainClient.send('admin', cmd);
            const log = document.getElementById('admin-log');
            if (log) log.textContent = `> Sent command: ${cmd}...`;
        };

        BrainClient.subscribe('admin', (data) => {
            const log = document.getElementById('admin-log');
            if (log) log.textContent = `Response: ${data}`;
        });
    },

    setupTerminal() {
        const out = document.getElementById('term-out');
        const inp = document.getElementById('term-in');

        // Re-print recent logs if available (simple mock for now, ideally store history)

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

// --- BOOT ---
window.addEventListener('DOMContentLoaded', () => {
    BrainClient.connectAll();
    Router.init();

    // Clock
    setInterval(() => {
        document.getElementById('clk').textContent = new Date().toLocaleTimeString();
    }, 1000);
});
