# Infrastructure Specifications

## Vector Search De-risking (Items 313, 314)
**Recall Optimization**:
Lock-free indexing structures enable concurrent insertion/lookup without global mutexes.
**Throughput Enhancement**:
SIMD distance calculations (AVX2) process 8 vectors simultaneously.

## Curiosity Drive gRPC (Item 316)
**Protocol**:
Streaming RPC for continuous interest updates.
**Message Format**:
```protobuf
message InterestUpdate {
  string topic = 1;
  float level = 2;
}
```

## Redis Stability (Item 321)
**Connection Pooling**:
TBB-based connection manager maintains 10 persistent connections.
**Retry Logic**:
Exponential backoff with jitter for failed operations.

## Memory Store SIMD (Item 317)
**Vectorized Search**:
AVX2 enables parallel keyword matching across memory chunks.
**Performance**:
~4x speedup vs. scalar search on large memory sets.

## Rate Limiter UX (Item 318)
**Client Integration**:
`RateLimiterClient.ts` provides browser-side throttling.
**Server Sync**:
WebSocket messages include rate limit headers for graceful degradation.
