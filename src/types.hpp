#pragma once

#include "matrix.hpp"
#include "vec2.hpp"

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    struct rgb_color {
        uint8_t red, green, blue;
    };

    using scalar_field = matrix<double>;

    using image = matrix<uint32_t>;

    using point = vec2<double>;

    struct rect {
        coords min;
        coords max;
    };
}