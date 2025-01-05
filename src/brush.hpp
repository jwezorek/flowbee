#pragma once

#include <functional>
#include "types.hpp"
#include "matrix.hpp"

namespace flo {

    using mix_fn = std::function<double(double)>;

    struct brush {
        double radius;
        double volume;
        paint_particle paint;
        mix_fn update_radius;
        mix_fn update_volume;
        mix_fn update_canvas;
    };


}