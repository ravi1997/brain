import { BrainClient } from './core/client.js';
import { Router } from './core/router.js';

// Feature Imports
import { DashboardFeature } from './features/dashboard.js';
import { TasksFeature } from './features/tasks.js';
import { NeuralFeature } from './features/neural.js';
import { PersonasFeature } from './features/personas.js';
import { LibraryFeature } from './features/library.js';
import { SystemFeature } from './features/system.js';
import { AdminFeature } from './features/admin.js';
import { TerminalFeature } from './features/terminal.js';
import { CognitionFeature } from './features/cognition.js';

// Route Configuration
const routes = {
    'dashboard': DashboardFeature,
    'tasks': TasksFeature,
    'neural': NeuralFeature,
    'personas': PersonasFeature,
    'library': LibraryFeature,
    'system': SystemFeature,
    'admin': AdminFeature,
    'terminal': TerminalFeature,
    'cognition': CognitionFeature
};

// Boot
window.addEventListener('DOMContentLoaded', () => {
    console.log("[Boot] Cortex OS v2.1 Modular");

    // 1. Connect Data Layer
    // We connect to all ports initially to ensure background data flow (e.g. logs ticker)
    // even if the view isn't active.
    const ports = {
        9002: 'emotions',
        9010: 'tasks',
        9006: 'thoughts',
        9007: 'research',
        9005: 'chat',
        9003: 'logs',
        9004: 'errors',
        9008: 'system',
        9009: 'admin',
        9011: 'control'
    };

    Object.entries(ports).forEach(([port, type]) => {
        BrainClient.connect(port, (data) => {
            // Global Ticker Handler
            if (type === 'logs' || type === 'chat') {
                const ticker = document.getElementById('ticker-text');
                if (ticker) ticker.textContent = `[${type.toUpperCase()}] ${data.substring(0, 80)}`;
            }
            // Emit to specific listeners
            BrainClient.emit(type, data);
        });
    });

    // 2. Init Router
    const router = new Router(routes);

    // 3. Clock
    setInterval(() => {
        const clk = document.getElementById('clk');
        if (clk) clk.textContent = new Date().toLocaleTimeString();
    }, 1000);
});
