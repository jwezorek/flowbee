#pragma once

#include <vector>
#include <cstdint>
#include "types.hpp"
#include "third-party/mixbox.h"
#include <unordered_map>

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    struct pigment {
        mixbox_latent impl;

        bool operator==(const pigment& p) const;
    };

    pigment operator+(const pigment& lhs, const pigment& rhs);
    pigment operator*(double k, const pigment& rhs);

    struct hash_pigment {
        size_t operator()(const pigment& p) const;
    };

    template <typename T>
    using pigment_map = std::unordered_map<pigment, T, hash_pigment>;

    pigment rgb_to_pigment(const rgb_color& rgb);
    pigment rgb_to_pigment(uint8_t r, uint8_t g, uint8_t b);

    void pigment_to_rgb_values(const pigment& p, uint8_t& r, uint8_t& g, uint8_t& b);
    rgb_color pigment_to_rgb(const pigment& p);

    pigment mix_pigments(const pigment& a, double a_vol, const pigment& b, double b_vol);
    pigment mix_paint(const pigment_map<double>& pigments);

    using paint = std::vector<double>;
    paint operator*(double k, const paint& paint);
}