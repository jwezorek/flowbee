#pragma once

#include "matrix.hpp"
#include "vec2.hpp"

namespace flo {

    struct rgb_color {
        uint8_t red, green, blue;
    };

    using scalar_field = matrix<double>;

    struct vector_field {
        scalar_field x;
        scalar_field y;
    };

    using image = matrix<uint32_t>;

    using point = vec2<double>;
    using coords = vec2<int>;

    struct rect {
        coords min;
        coords max;
    };
}