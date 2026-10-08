#include "pricing_engine.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

static void near(double a, double b, double eps) { assert(std::abs(a - b) <= eps); }

int main() {
    const Option call{100, 100, 0.05, 0.20, 1, OptionType::Call};
    const Option put{100, 100, 0.05, 0.20, 1, OptionType::Put};
    const BlackScholesPricer bs;

    near(bs.price(call), 10.4505835722, 1e-7);
    near(bs.price(put), 5.5735260223, 1e-7);
    near(bs.price(call) - bs.price(put), call.spot - call.strike * std::exp(-call.rate * call.maturity), 1e-8);
    near(BinomialPricer(1200).price(call), bs.price(call), 0.05);
    near(FiniteDifferencePricer(250, 5000).price(call), bs.price(call), 0.10);
    near(MonteCarloPricer(2'000'000, 7, 4).price(call), bs.price(call), 0.08);

    const Greeks g = bs.greeks(call);
    near(g.delta, 0.6368306512, 1e-7);
    near(g.gamma, 0.0187620173, 1e-7);
    near(g.vega, 37.5240346917, 1e-6);
    near(g.rho, 53.2324815454, 1e-6);

    const double iv = implied_volatility(call, bs.price(call));
    near(iv, 0.20, 1e-8);

    bool threw = false;
    try { Option bad{-1, 100, 0.05, 0.2, 1, OptionType::Call}; bs.price(bad); }
    catch (const std::invalid_argument&) { threw = true; }
    assert(threw);

    std::cout << "All pricing, Greeks, solver, convergence and validation tests passed.\n";
}
