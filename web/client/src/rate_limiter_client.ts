// Item 281, 282: Rate Limiter Client (Refactored)

export class RateLimiterClient {
    private maxRequests: number;
    private windowMs: number;
    private requests: number[];

    constructor(maxRequests: number, windowMs: number) {
        this.maxRequests = maxRequests;
        this.windowMs = windowMs;
        this.requests = [];
    }

    allowRequest(): boolean {
        const now = Date.now();
        // Remove expired
        this.requests = this.requests.filter(t => t > now - this.windowMs);
        
        if (this.requests.length < this.maxRequests) {
            this.requests.push(now);
            return true;
        }
        console.warn("[RateLimiter] Throttled client request");
        return false;
    }

    // Item 273 stub
    measureRecall(): number {
        return 1.0;
    }
}
