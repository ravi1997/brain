# Frontend Specifications

## Neural Visualizer (Item 219)
**Architecture**:
Canvas-based rendering using `requestAnimationFrame`.
**Scalability**:
Uses `WebGPUContext` (Item 253) for rendering >10k nodes if available.
**Precision**:
TypeScript interfaces enforce node/edge data structures (Item 251).

## WebSocket Layer (Item 221)
**Protocol**:
JSON-RPC style messages over `ws://`. 
**Concurrency**:
Handled by the Node.js event loop + async handlers.
**Buffering**:
Implements a client-side queue for retry logic (Item 245).

## React Hooks (Item 205)
**Philosophy**:
Logic is extracted into custom hooks (`useBrain`, `useCuriosity`).
**Rendering**:
Components subscribe to hooks and only re-render on relevant state changes.
