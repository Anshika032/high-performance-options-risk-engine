# Architecture

```text
CLI / benchmark
      |
      v
  Option value object
      |
      +-------------------+-------------------+-------------------+
      |                   |                   |                   |
      v                   v                   v                   v
Black-Scholes         Binomial             FDM              Monte Carlo
      |                   |                   |                   |
      +-------------------+-------------------+-------------------+
                              |
                              v
                       Greeks / IV Solver
```

## Engineering choices

- **C++20:** object-oriented interfaces with value semantics and standard-library concurrency.
- **Performance:** `-O3`, contiguous `std::vector` storage, no dynamic allocation inside the inner Monte Carlo payoff loop, and optional multi-threaded Monte Carlo.
- **Correctness:** deterministic seeds, cross-method numerical checks, put-call parity, Greeks checks, implied-volatility round trip, and input validation.
- **Debuggability:** dedicated `make debug` target with AddressSanitizer and UndefinedBehaviorSanitizer.
- **Extensibility:** each pricing model implements a small, focused interface so additional models can be introduced without changing the CLI or tests.
