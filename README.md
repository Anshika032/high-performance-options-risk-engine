# High-Performance C++ Options & Risk Engine

A from-scratch C++20 quantitative-finance engine designed as a compact demonstration of **performance-oriented software engineering, numerical correctness, automated testing, and market-pricing fundamentals**.

## Why this project

The project is intentionally built around engineering concerns relevant to low-latency financial software:

- clean object-oriented interfaces
- deterministic and reproducible numerical tests
- compiler optimization and cache-friendly contiguous storage
- single-thread vs multi-thread performance comparison
- automated regression testing
- explicit validation and exception handling
- debug builds with AddressSanitizer/UndefinedBehaviorSanitizer
- benchmark output that can be reproduced on the user's machine

## Implemented models

| Component | Purpose |
|---|---|
| Black-Scholes | Closed-form European call/put valuation |
| Binomial Tree | Recombining lattice valuation |
| Explicit FDM | Numerical solution of the Black-Scholes PDE |
| Monte Carlo | Risk-neutral GBM simulation |
| Greeks | Delta, Gamma, Vega, Theta, Rho |
| Implied Volatility | Bisection solver against Black-Scholes |
| Parallel Monte Carlo | Multi-threaded pricing benchmark |

## Build

Requirements: GCC/Clang with C++20 and either `make` or CMake 3.16+.

```bash
make
./build/options_engine
```

Custom contract:

```bash
./build/options_engine 100 100 0.05 0.20 1 call
```

Arguments:

```text
spot strike risk_free_rate volatility maturity call|put
```

## CMake

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

GitHub Actions runs a release build/test job plus an AddressSanitizer/UndefinedBehaviorSanitizer job on every push and pull request.

## Automated tests

```bash
make test
```

The test suite covers:

- analytical Black-Scholes values
- put-call parity
- binomial convergence
- finite-difference agreement
- deterministic Monte Carlo tolerance
- Greeks
- implied-volatility round trip
- invalid-input handling

## Performance benchmark

```bash
make benchmark
```

Output is CSV-like and includes runtime in microseconds. Run it on the target machine rather than copying benchmark numbers between systems because CPU, compiler, OS scheduling and thermal conditions affect results.

A useful experiment is to compare:

```text
Monte Carlo — 1 thread
        vs
Monte Carlo — 4 threads
```

This provides a concrete example of trading-off throughput, parallelism and implementation complexity.

## Debugging / memory safety

```bash
make debug
./build/options_engine_debug
```

The debug target enables AddressSanitizer and UndefinedBehaviorSanitizer.

## Engineering structure

```text
high-performance-options-risk-engine/
├── include/
│   ├── option.hpp
│   └── pricing_engine.hpp
├── src/
│   ├── main.cpp
│   └── pricing_engine.cpp
├── tests/
│   └── test_pricing.cpp
├── benchmarks/
│   └── benchmark.cpp
├── docs/
│   └── ARCHITECTURE.md
├── Makefile
├── LICENSE
└── .gitignore
```

## Example output

Exact timing varies by machine. The program reports the actual values measured locally:

```text
Method               Price          Runtime
Black-Scholes        ...            ... us
Binomial             ...            ... us
Finite Difference    ...            ... us
Monte Carlo (1T)     ...            ... us
Monte Carlo (4T)     ...            ... us
```

## What I would extend next

- AVX2/AVX-512 vectorized Monte Carlo path generation behind compile-time feature detection
- variance reduction (antithetic variates)
- batch pricing for thousands of contracts
- lock-free market-data ingestion
- latency histograms (p50/p95/p99)
- CI with GCC + Clang and sanitizer jobs

## Attribution

The source code in this repository is an independent implementation. It uses classical mathematical models from quantitative-finance literature and is not copied from another repository.
