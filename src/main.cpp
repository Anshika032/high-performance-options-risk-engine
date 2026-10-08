#include "pricing_engine.hpp"
#include <iomanip>
#include <iostream>
#include <string>

namespace {
void usage(const char* p) {
    std::cout << "Usage: " << p << " [spot strike rate volatility maturity call|put]\n";
}
void print(const char* name, const PricingResult& r) {
    std::cout << std::left << std::setw(20) << name << std::right << std::setw(14)
              << r.price << "   " << std::setw(12) << r.elapsed_us << " us\n";
}
}

int main(int argc, char** argv) {
    try {
        Option option{100.0, 100.0, 0.05, 0.20, 1.0, OptionType::Call};
        if (argc == 7) {
            option.spot = std::stod(argv[1]); option.strike = std::stod(argv[2]);
            option.rate = std::stod(argv[3]); option.volatility = std::stod(argv[4]);
            option.maturity = std::stod(argv[5]);
            const std::string type = argv[6];
            if (type == "call" || type == "CALL") option.type = OptionType::Call;
            else if (type == "put" || type == "PUT") option.type = OptionType::Put;
            else { usage(argv[0]); return 1; }
        } else if (argc != 1) { usage(argv[0]); return 1; }

        const BlackScholesPricer bs;
        const BinomialPricer binomial(1000);
        const FiniteDifferencePricer fdm(250, 5000);
        const MonteCarloPricer mc_single(1'000'000, 42, 1);
        const MonteCarloPricer mc_parallel(1'000'000, 42, 4);

        std::cout << std::fixed << std::setprecision(8);
        std::cout << "\nHigh-Performance C++ Options & Risk Engine\n";
        std::cout << "==========================================\n";
        std::cout << "Method               Price          Runtime\n";
        print("Black-Scholes", timed_price(option, bs));
        print("Binomial", timed_price(option, binomial));
        print("Finite Difference", timed_price(option, fdm));
        print("Monte Carlo (1T)", timed_price(option, mc_single));
        print("Monte Carlo (4T)", timed_price(option, mc_parallel));

        const Greeks g = bs.greeks(option);
        std::cout << "\nGreeks (Black-Scholes)\n";
        std::cout << "Delta: " << g.delta << "  Gamma: " << g.gamma << "  Vega: " << g.vega
                  << "  Theta: " << g.theta << "  Rho: " << g.rho << "\n";

        const double iv = implied_volatility(option, bs.price(option));
        std::cout << "Implied volatility round-trip: " << iv << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
