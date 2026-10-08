#pragma once

#include "option.hpp"
#include <cstddef>

struct Greeks {
    double delta{};
    double gamma{};
    double vega{};
    double theta{};
    double rho{};
};

struct PricingResult {
    double price{};
    double elapsed_us{};
};

class BlackScholesPricer {
public:
    double price(const Option& option) const;
    Greeks greeks(const Option& option) const;
};

class BinomialPricer {
public:
    explicit BinomialPricer(std::size_t steps = 500) : steps_(steps) {}
    double price(const Option& option) const;
private:
    std::size_t steps_;
};

class FiniteDifferencePricer {
public:
    FiniteDifferencePricer(std::size_t price_steps = 200, std::size_t time_steps = 4000)
        : price_steps_(price_steps), time_steps_(time_steps) {}
    double price(const Option& option) const;
private:
    std::size_t price_steps_;
    std::size_t time_steps_;
};

class MonteCarloPricer {
public:
    MonteCarloPricer(std::size_t paths = 1'000'000, unsigned seed = 42, std::size_t threads = 1)
        : paths_(paths), seed_(seed), threads_(threads) {}
    double price(const Option& option) const;
private:
    std::size_t paths_;
    unsigned seed_;
    std::size_t threads_;
};

double implied_volatility(const Option& option, double market_price,
                          double low = 1e-6, double high = 5.0,
                          double tolerance = 1e-10, int max_iterations = 100);

PricingResult timed_price(const Option& option, const BlackScholesPricer& pricer);
PricingResult timed_price(const Option& option, const BinomialPricer& pricer);
PricingResult timed_price(const Option& option, const FiniteDifferencePricer& pricer);
PricingResult timed_price(const Option& option, const MonteCarloPricer& pricer);
