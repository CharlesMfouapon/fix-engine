# FIX Protocol Engine

Zero-copy FIX protocol parser and session layer. Compile-time message validation, wire-speed throughput, OWASP-hardened fuzzing harness.

[![C++ CI](https://github.com/CharlesMfouapon/fix-engine/actions/workflows/ci.yml/badge.svg)](https://github.com/YOUR_USERNAME/fix-engine/actions/workflows/ci.yml)
[![C++20](https://img.shields.io/badge/C++-20-blue?logo=c%2B%2B)](https://en.cppreference.com)
[![Fuzzing](https://img.shields.io/badge/Fuzzing-LibFuzzer-purple)](fuzz/)
[![Apache 2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE)

**Zero-copy FIX protocol engine.** Parses at wire speed. Validates at compile time. Hardened against malformed input.

## Performance
- **1M+ messages/sec** on single core
- **<100ns parsing latency** per message
- **Zero heap allocations** in parse path

## Supported Messages
- Logon/Logout/Heartbeat (session layer)
- NewOrderSingle (D)
- ExecutionReport (8)
- CancelRequest/CancelReject

## Security
- [x] Checksum validation before field parsing
- [x] Sequence number gap detection
- [x] LibFuzzer harness for malformed input
- [x] ASan/UBSan builds in CI

## Quick Start
```bash
git clone https://github.com/CharlesMfouapon/fix-engine.git
cd fix-engine
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./fix_tests
./fix_bench
```

Example

```cpp
#include "fix/fix_session.hpp"

// Build a NewOrderSingle
auto msg = fix::FixSession::buildNewOrderSingle(
    "CL-001", "AAPL", fix::Side::Buy, 150.25, 1000
);

// Parse incoming message
fix::FixMessage parsed(msg);
auto symbol = parsed.get<fix::Tag::Symbol>();  // "AAPL"
auto price  = parsed.get<fix::Tag::Price>();   // 150.25
```
Integration with Order Book

This engine is designed to feed the [limit-order-book](https://github.com/CharlesMfouapon/limit-order-book) engine. Together they form:

* FIX Engine → Network ingress, session management, protocol compliance
* Order Book → Core matching, price-time priority, trade generation
* Immutable Ledger → Settlement, audit trail, cryptographic verification

Read [ARCHITECTURE.md](ARCHITECTURE.md) for design rationale.
