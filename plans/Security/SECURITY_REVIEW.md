# Security Review - Brain Replica

## Methodology
The code was audited for common security pitfalls in C++ and Docker environments.

## Findings

### 1. Network Exposure (MEDIUM)
The application opens 14 TCP ports (9001-9014) without any authentication or encryption (TLS).
- **Risk**: Any user on the network can send commands to the admin port (9009) or intercept brain logs.
- **Recommendation**: Implement token-based authentication and bind to `127.0.0.1` by default unless configured otherwise.

### 2. Secret Management (HIGH)
- **Vulnerability**: Database passwords are hardcoded in `docker-compose.yml` and `.github/workflows/ci.yml`.
- **Status**: Identified, tasked in backlog [B-02].

### 3. Container Security (LOW -> FIXED)
- **Vulnerability**: Application was running as `root`.
- **Status**: Fixed by implementing non-root `brain` user in Dockerfile.

### 4. Memory Safety (C++) (LOW)
- **Analysis**: Code uses modern C++ (`std::unique_ptr`, `std::vector`, `std::string`). No `malloc`/`free` or raw pointer ownership was found in core logic. Some C-style socket code is used but safely wrapped in `TcpServer`.
- **Recommendation**: Add `clang-tidy` to CI to catch subtle memory issues or potential overflows in buffer reads.

### 5. Dependency Vulnerabilities (LOW)
- `node_modules` in the web folder has 2 moderate vulnerabilities.
- **Recommendation**: Run `npm audit fix`.

## Security Gate Status
- **gate_global_security.md**: **FAIL** due to hardcoded secrets and unauthenticated ports.
