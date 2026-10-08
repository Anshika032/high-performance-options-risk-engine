#include "pricing_engine.hpp"
#include <iomanip>
#include <iostream>

int main() {
    const Option option{100, 100, 0.05, 0.20, 1.0, OptionType::Call};
    const BlackScholesPricer bs;
    const BinomialPricer binomial(1500);
    const FiniteDifferencePricer fdm(300, 6000);
    const MonteCarloPricer mc1(2'000'000, 42, 1);
    const MonteCarloPricer mc4(2'000'000, 42, 4);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "method,price,time_us\n";
    auto emit = [](const char* name, PricingResult r) { std::cout << name << ',' << r.price << ',' << r.elapsed_us << '\n'; };
    emit("black_scholes", timed_price(option, bs));
    emit("binomial", timed_price(option, binomial));
    emit("finite_difference", timed_price(option, fdm));
    emit("monte_carlo_1_thread", timed_price(option, mc1));
    emit("monte_carlo_4_threads", timed_price(option, mc4));
}
