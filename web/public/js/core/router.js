export class Router {
    constructor(routes) {
        this.routes = routes;
        this.currentModule = null;
        this.viewContainer = document.getElementById('main-view');

        // Handle nav clicks
        document.querySelectorAll('.nav-links li').forEach(el => {
            el.addEventListener('click', (e) => {
                const view = e.currentTarget.getAttribute('data-view');
                if (view) this.navigate(view);
            });
        });

        // Default
        this.navigate('dashboard');
    }

    async navigate(viewName) {
        if (!this.routes[viewName]) return;

        // Update UI Active State
        document.querySelectorAll('.nav-links li').forEach(el => el.classList.remove('active'));
        const activeLink = document.querySelector(`.nav-links li[data-view="${viewName}"]`);
        if (activeLink) activeLink.classList.add('active');

        // Cleanup previous module
        if (this.currentModule && this.currentModule.cleanup) {
            this.currentModule.cleanup();
        }

        // Load HTML
        try {
            const response = await fetch(`./views/${viewName}.html`);
            const html = await response.text();
            this.viewContainer.innerHTML = html;

            // Init new Module
            const module = this.routes[viewName];
            if (module && module.init) {
                module.init();
                this.currentModule = module;
            }
        } catch (e) {
            console.error("Navigation Error:", e);
            this.viewContainer.innerHTML = "<h3>Error loading view.</h3>";
        }
    }
}
