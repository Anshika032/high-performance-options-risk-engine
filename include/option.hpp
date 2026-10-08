#pragma once

#include <stdexcept>

enum class OptionType { Call, Put };

struct Option {
    double spot{};
    double strike{};
    double rate{};
    double volatility{};
    double maturity{};
    OptionType type{OptionType::Call};

    void validate() const {
        if (spot <= 0.0 || strike <= 0.0) throw std::invalid_argument("spot and strike must be positive");
        if (volatility < 0.0) throw std::invalid_argument("volatility must be non-negative");
        if (maturity < 0.0) throw std::invalid_argument("maturity must be non-negative");
    }
};

inline double intrinsic_value(const Option& o, double spot) {
    return o.type == OptionType::Call ? (spot - o.strike > 0.0 ? spot - o.strike : 0.0)
                                      : (o.strike - spot > 0.0 ? o.strike - spot : 0.0);
}
