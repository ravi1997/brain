const { test, expect } = require('@playwright/test');

test.describe('Cortex OS Dashboard', () => {
    test.beforeEach(async ({ page }) => {
        // Assuming the docker container is running on port 5000
        await page.goto('http://localhost:5000');
    });

    test('should load the dashboard view by default', async ({ page }) => {
        await expect(page).toHaveTitle(/BRAIN \/\/ COMMAND CENTER/);
        await expect(page.locator('#page-title')).toHaveText('SYSTEM OVERVIEW');

        // Check for dashboard specific cards
        await expect(page.locator('#dash-energy')).toBeVisible();
        await expect(page.locator('text=CURRENT FOCUS')).toBeVisible();
    });

    test('should navigate to Neural Link view', async ({ page }) => {
        // Click on Neural Link in sidebar
        await page.click('li[data-view="neural"]');

        // Verify view container updated
        await expect(page.locator('#neural-canvas')).toBeVisible();
        await expect(page.locator('text=NEURAL ACTIVITY')).toBeVisible();

        // Verify nav active state
        await expect(page.locator('li[data-view="neural"]')).toHaveClass(/active/);
    });

    test('should navigate to System view and show charts', async ({ page }) => {
        await page.click('li[data-view="system"]');

        // Verify charts
        await expect(page.locator('#chart-cpu')).toBeVisible();
        await expect(page.locator('#chart-mem')).toBeVisible();

        // Verify text update (wait briefly for data)
        // We expect some text content, even if 0%
        await expect(page.locator('#sys-cpu')).not.toBeEmpty();
    });

    test('should navigate to Terminal and accept input', async ({ page }) => {
        await page.click('li[data-view="terminal"]');

        await expect(page.locator('#term-in')).toBeVisible();

        // Type a command
        await page.fill('#term-in', 'help');
        await page.press('#term-in', 'Enter');

        // Verify log updated (user command echoed)
        await expect(page.locator('#term-out')).toContainText('root@brain:~# help');
    });
});
