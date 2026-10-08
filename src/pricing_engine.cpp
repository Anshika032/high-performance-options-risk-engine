#include "pricing_engine.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <future>
#include <numeric>
#include <random>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {
constexpr double PI = 3.1415926535897932384626433832795;

double normal_pdf(double x) { return std::exp(-0.5 * x * x) / std::sqrt(2.0 * PI); }
double normal_cdf(double x) { return 0.5 * std::erfc(-x / std::sqrt(2.0)); }

double payoff(double spot, const Option& o) { return intrinsic_value(o, spot); }

void validate_market_price(const Option& o, double market_price) {
    o.validate();
    if (!std::isfinite(market_price) || market_price < 0.0) throw std::invalid_argument("invalid market price");
}
}

double BlackScholesPricer::price(const Option& o) const {
    o.validate();
    if (o.maturity == 0.0) return payoff(o.spot, o);
    if (o.volatility == 0.0) return std::exp(-o.rate * o.maturity) * payoff(o.spot * std::exp(o.rate * o.maturity), o);
    const double sqrt_t = std::sqrt(o.maturity);
    const double d1 = (std::log(o.spot / o.strike) + (o.rate + 0.5 * o.volatility * o.volatility) * o.maturity) /
                      (o.volatility * sqrt_t);
    const double d2 = d1 - o.volatility * sqrt_t;
    if (o.type == OptionType::Call)
        return o.spot * normal_cdf(d1) - o.strike * std::exp(-o.rate * o.maturity) * normal_cdf(d2);
    return o.strike * std::exp(-o.rate * o.maturity) * normal_cdf(-d2) - o.spot * normal_cdf(-d1);
}

Greeks BlackScholesPricer::greeks(const Option& o) const {
    o.validate();
    if (o.maturity <= 0.0 || o.volatility <= 0.0) return {};
    const double sqrt_t = std::sqrt(o.maturity);
    const double vsqrt = o.volatility * sqrt_t;
    const double d1 = (std::log(o.spot / o.strike) + (o.rate + 0.5 * o.volatility * o.volatility) * o.maturity) / vsqrt;
    const double d2 = d1 - vsqrt;
    const double discount = std::exp(-o.rate * o.maturity);
    Greeks g;
    g.delta = o.type == OptionType::Call ? normal_cdf(d1) : normal_cdf(d1) - 1.0;
    g.gamma = normal_pdf(d1) / (o.spot * vsqrt);
    g.vega = o.spot * normal_pdf(d1) * sqrt_t;
    g.theta = -(o.spot * normal_pdf(d1) * o.volatility) / (2.0 * sqrt_t);
    if (o.type == OptionType::Call)
        g.theta -= o.rate * o.strike * discount * normal_cdf(d2);
    else
        g.theta += o.rate * o.strike * discount * normal_cdf(-d2);
    g.rho = o.type == OptionType::Call ? o.strike * o.maturity * discount * normal_cdf(d2)
                                       : -o.strike * o.maturity * discount * normal_cdf(-d2);
    return g;
}

double BinomialPricer::price(const Option& o) const {
    o.validate();
    if (o.maturity == 0.0) return payoff(o.spot, o);
    const std::size_t n = std::max<std::size_t>(1, steps_);
    const double dt = o.maturity / static_cast<double>(n);
    const double u = std::exp(o.volatility * std::sqrt(dt));
    const double d = 1.0 / u;
    if (std::abs(u - d) < 1e-14) return BlackScholesPricer{}.price(o);
    const double disc = std::exp(-o.rate * dt);
    const double p = (std::exp(o.rate * dt) - d) / (u - d);
    if (p < 0.0 || p > 1.0) throw std::runtime_error("invalid risk-neutral probability; reduce binomial step size");
    std::vector<double> values(n + 1);
    double s = o.spot * std::pow(d, static_cast<double>(n));
    const double ratio = u / d;
    for (std::size_t j = 0; j <= n; ++j) { values[j] = payoff(s, o); s *= ratio; }
    for (std::size_t i = n; i-- > 0;) for (std::size_t j = 0; j <= i; ++j)
        values[j] = disc * (p * values[j + 1] + (1.0 - p) * values[j]);
    return values[0];
}

double FiniteDifferencePricer::price(const Option& o) const {
    o.validate();
    if (o.maturity == 0.0) return payoff(o.spot, o);
    const std::size_t m = std::max<std::size_t>(50, price_steps_);
    const std::size_t n = std::max<std::size_t>(50, time_steps_);
    const double smax = std::max(2.5 * o.spot, 2.5 * o.strike);
    const double ds = smax / static_cast<double>(m);
    const double dt = o.maturity / static_cast<double>(n);
    std::vector<double> old(m + 1), next(m + 1);
    for (std::size_t j = 0; j <= m; ++j) old[j] = payoff(static_cast<double>(j) * ds, o);
    for (std::size_t step = n; step-- > 0;) {
        const double tau = static_cast<double>(step) * dt;
        old[0] = o.type == OptionType::Call ? 0.0 : o.strike * std::exp(-o.rate * (o.maturity - tau));
        old[m] = o.type == OptionType::Call ? smax - o.strike * std::exp(-o.rate * (o.maturity - tau)) : 0.0;
        next = old;
        for (std::size_t j = 1; j < m; ++j) {
            const double s = static_cast<double>(j) * ds;
            const double a = 0.5 * dt * (o.volatility * o.volatility * s * s / (ds * ds) - o.rate * s / ds);
            const double b = 1.0 - dt * (o.volatility * o.volatility * s * s / (ds * ds) + o.rate);
            const double c = 0.5 * dt * (o.volatility * o.volatility * s * s / (ds * ds) + o.rate * s / ds);
            next[j] = a * old[j - 1] + b * old[j] + c * old[j + 1];
        }
        old.swap(next);
    }
    const double x = o.spot / ds;
    const auto lo = static_cast<std::size_t>(std::clamp(std::floor(x), 0.0, static_cast<double>(m - 1)));
    const double w = x - static_cast<double>(lo);
    return old[lo] * (1.0 - w) + old[lo + 1] * w;
}

double MonteCarloPricer::price(const Option& o) const {
    o.validate();
    if (o.maturity == 0.0) return payoff(o.spot, o);
    const std::size_t paths = std::max<std::size_t>(1'000, paths_);
    const std::size_t thread_count = std::max<std::size_t>(1, std::min(threads_, paths));
    const std::size_t base = paths / thread_count;
    const std::size_t rem = paths % thread_count;
    std::vector<std::future<double>> futures;
    futures.reserve(thread_count);
    for (std::size_t t = 0; t < thread_count; ++t) {
        const std::size_t count = base + (t < rem ? 1 : 0);
        futures.emplace_back(std::async(std::launch::async, [&, t, count] {
            std::mt19937_64 gen(static_cast<std::uint64_t>(seed_) + 0x9e3779b97f4a7c15ULL * t);
            std::normal_distribution<double> normal(0.0, 1.0);
            const double drift = (o.rate - 0.5 * o.volatility * o.volatility) * o.maturity;
            const double diffusion = o.volatility * std::sqrt(o.maturity);
            double sum = 0.0;
            for (std::size_t i = 0; i < count; ++i) {
                const double st = o.spot * std::exp(drift + diffusion * normal(gen));
                sum += payoff(st, o);
            }
            return sum;
        }));
    }
    double sum = 0.0;
    for (auto& f : futures) sum += f.get();
    return std::exp(-o.rate * o.maturity) * sum / static_cast<double>(paths);
}

double implied_volatility(const Option& input, double market_price, double low, double high,
                          double tolerance, int max_iterations) {
    validate_market_price(input, market_price);
    if (low <= 0.0 || high <= low || tolerance <= 0.0 || max_iterations <= 0) throw std::invalid_argument("invalid solver bounds");
    Option o = input;
    const BlackScholesPricer bs;
    auto f = [&](double vol) { o.volatility = vol; return bs.price(o) - market_price; };
    double fl = f(low), fh = f(high);
    if (fl * fh > 0.0) throw std::invalid_argument("market price is outside the supplied volatility bracket");
    for (int i = 0; i < max_iterations; ++i) {
        const double mid = 0.5 * (low + high), fm = f(mid);
        if (std::abs(fm) < tolerance || (high - low) < tolerance) return mid;
        if (fl * fm <= 0.0) { high = mid; fh = fm; } else { low = mid; fl = fm; }
    }
    return 0.5 * (low + high);
}

template <typename Pricer>
PricingResult timed_impl(const Option& o, const Pricer& p) {
    const auto start = std::chrono::steady_clock::now();
    const double value = p.price(o);
    const auto end = std::chrono::steady_clock::now();
    return {value, std::chrono::duration<double, std::micro>(end - start).count()};
}
PricingResult timed_price(const Option& o, const BlackScholesPricer& p) { return timed_impl(o, p); }
PricingResult timed_price(const Option& o, const BinomialPricer& p) { return timed_impl(o, p); }
PricingResult timed_price(const Option& o, const FiniteDifferencePricer& p) { return timed_impl(o, p); }
PricingResult timed_price(const Option& o, const MonteCarloPricer& p) { return timed_impl(o, p); }
