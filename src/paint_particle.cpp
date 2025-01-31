#include "paint_particle.hpp"
namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/
namespace {
    void normalize(std::vector<double>& vec) {
        auto sum = r::fold_left(vec, 0.0, std::plus<>());
        if (sum == 0.0) {
            return;
        }
        for (auto& v : vec) {
            v /= sum;
        }
    }
}

void flo::paint_particle::fix_sign() {
    if (volume_ < 0) {
        volume_ *= -1.0;
        for (auto& v : mixture_) {
            v *= -1.0;
        }
    }
}

flo::paint_particle::paint_particle(double volume, const std::vector<double>& mixture) :
    volume_(volume), mixture_(mixture)
{
    fix_sign();
}

double flo::paint_particle::volume() const
{
    return volume_;
}

const std::vector<double>& flo::paint_particle::mixture() const
{
    return mixture_;
}

void flo::paint_particle::normalize() {
    volume_ = 1.0;
}


flo::paint_particle flo::operator*(double k, const paint_particle& p) {
    auto prod = paint_particle{ k * p.volume(), p.mixture() };
    prod.fix_sign();
    return prod;
}

flo::paint_particle& flo::operator+=(flo::paint_particle& paint_lhs, const flo::paint_particle& paint_rhs) {
    auto new_mixture = rv::zip(paint_lhs.mixture(), paint_rhs.mixture()) |
        rv::transform(
            [&](const auto& pair) {
                const auto& [lhs, rhs] = pair;
                return paint_lhs.volume() * lhs + paint_rhs.volume() * rhs;
            }
        ) | r::to<std::vector>();
    auto new_volume = r::fold_left(new_mixture, 0.0, std::plus<>());
    ::normalize(new_mixture);
    paint_lhs = paint_particle{
        new_volume,
        new_mixture
    };
    paint_lhs.fix_sign();

    return paint_lhs;
}

flo::paint_particle& flo::operator-=(paint_particle& lhs, const paint_particle& rhs) {
    return lhs += (-1.0) * rhs;
}

flo::paint_particle flo::operator+(const paint_particle& lhs, const paint_particle& rhs) {
    auto sum = lhs;
    sum += rhs;
    return sum;
}

flo::paint_particle flo::operator-(const paint_particle& lhs, const paint_particle& rhs) {
    auto difference = lhs;
    difference -= rhs;
    return difference;
}

flo::paint_particle flo::normalize(const paint_particle& p) {
    return { 1.0, p.mixture() };
}

flo::paint_particle flo::make_one_color_paint(int palette_sz, int color_index, double volume) {
    auto mixture = std::vector<double>(palette_sz, 0.0);
    mixture[color_index] = 1.0;
    return { volume, mixture };
}

std::string flo::display(const paint_particle& p)
{
    auto mixture_str = rv::join_with(
        p.mixture() | std::views::transform(
            [](double v) { return std::format("{:.4g}", v); }
        ), ", ") | r::to<std::string>();
    return std::format("[ {} : {}]", p.volume(), mixture_str);
}
