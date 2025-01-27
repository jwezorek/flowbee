#include "paint.hpp"
#include "matrix_3d.hpp"
#include <boost/functional/hash.hpp>
#include <stdexcept>
#include <cstring> // For memset
#include <print>
#include <ranges>

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {
    constexpr float k_eps = 0.000005f;
    constexpr float k_mult = 1.0f / k_eps;
}

flo::pigment flo::rgb_to_pigment(uint8_t r, uint8_t g, uint8_t b) {
    pigment p;
    mixbox_rgb_to_latent(r, g, b, p.impl);
    return p;
}

flo::pigment flo::operator+(const pigment& lhs, const pigment& rhs) {
    return {
        lhs.impl[0] + rhs.impl[0],
        lhs.impl[1] + rhs.impl[1],
        lhs.impl[2] + rhs.impl[2],
        lhs.impl[3] + rhs.impl[3],
        lhs.impl[4] + rhs.impl[4],
        lhs.impl[5] + rhs.impl[5],
        lhs.impl[6] + rhs.impl[6]
    };
}

flo::pigment flo::operator*(double k, const pigment& rhs) {
    return {
        static_cast<float>(k) * rhs.impl[0],
        static_cast<float>(k) * rhs.impl[1],
        static_cast<float>(k) * rhs.impl[2],
        static_cast<float>(k) * rhs.impl[3],
        static_cast<float>(k) * rhs.impl[4],
        static_cast<float>(k) * rhs.impl[5],
        static_cast<float>(k) * rhs.impl[6]
    };
}

flo::pigment flo::rgb_to_pigment(const rgb_color& rgb)
{
    return rgb_to_pigment(rgb.red, rgb.green, rgb.blue);
}

void flo::pigment_to_rgb_values(const pigment& p, uint8_t& r, uint8_t& g, uint8_t& b) {
    mixbox_latent_to_rgb(const_cast<float*>(p.impl), &r, &g, &b);
}

flo::rgb_color flo::pigment_to_rgb(const pigment& p) {
    flo::rgb_color rgb;
    pigment_to_rgb_values(p, rgb.red, rgb.green, rgb.blue);
    return rgb;
}

flo::pigment flo::mix_pigments(const pigment& a, double a_vol, const pigment& b, double b_vol) {
    pigment mixed_pigment = a_vol * a + b_vol * b;
    double vol = a_vol + b_vol;
    return (1.0 / vol) * mixed_pigment;
}

flo::pigment flo::mix_paint(const pigment_map<double>& pigments) {
    if (pigments.empty()) {
        throw std::invalid_argument("Cannot mix an empty set of pigments.");
    }

    pigment mixed_pigment;
    std::memset(mixed_pigment.impl, 0, sizeof(mixbox_latent)); // Initialize to zeros
    double total_volume = 0.0f;

    for (const auto& [pigment, volume] : pigments) {
        for (int i = 0; i < MIXBOX_LATENT_SIZE; ++i) {
            mixed_pigment.impl[i] += pigment.impl[i] * volume;
        }
        total_volume += volume;
    }

    // Normalize the mixed pigment by the total volume
    if (total_volume > 0.0f) {
        for (int i = 0; i < MIXBOX_LATENT_SIZE; ++i) {
            mixed_pigment.impl[i] /= total_volume;
        }
    }

    return mixed_pigment;
}

flo::paint flo::operator*(double k, const paint& paint) {
    return paint | rv::transform(
            [k](auto v)->double {
                return k * v;
            }
        ) | r::to<std::vector>();
}

flo::paint& flo::operator+=(flo::paint& lhs, const flo::paint& rhs) {
    for (int i = 0; i < lhs.size(); ++i) {
        lhs[i] += rhs[i];
    }
    return lhs;
}

flo::paint& flo::operator-=(paint& lhs, const paint& rhs) {
    for (int i = 0; i < lhs.size(); ++i) {
        lhs[i] -= rhs[i];
    }
    return lhs;
}

flo::paint flo::operator+(const paint& lhs, const paint& rhs) {
    auto sum = lhs;
    sum += rhs;
    return sum;
}

flo::paint flo::operator-(const paint& lhs, const paint& rhs) {
    auto difference = lhs;
    difference -= rhs;
    return difference;
}

flo::paint& flo::clamp_nonnegative(paint& p)
{
    for (int i = 0; i < p.size(); ++i) {
        p[i] = (p[i] >= 0.0) ? p[i] : 0.0;
    }
    return p;
}

bool flo::pigment::operator==(const pigment& p) const {

    static const auto approx_eql = [](float u, float v)->bool {
        return std::abs(u - v) <= k_eps;
        };

    return
        approx_eql( impl[0], p.impl[0] ) &&
        approx_eql( impl[1], p.impl[1] ) &&
        approx_eql( impl[2], p.impl[2] ) &&
        approx_eql( impl[3], p.impl[3] ) &&
        approx_eql( impl[4], p.impl[4] ) &&
        approx_eql( impl[5], p.impl[5] ) &&
        approx_eql( impl[6], p.impl[6] );
}

size_t flo::hash_pigment::operator()(const pigment& p) const {
    size_t seed = 0;
    for (int i = 0; i < 7; ++i) {
        int64_t val = static_cast<int64_t>(p.impl[i] * k_mult);
        boost::hash_combine(seed, val);
    }
    return seed;
}

flo::paint flo::create_paint(int num_colors) {
    return flo::paint( num_colors, 0.0 );
}

flo::paint flo::make_paint(int num_colors, int index, double val) {
    auto paint = create_paint(num_colors);
    paint[index] = val;
    return paint;
}
