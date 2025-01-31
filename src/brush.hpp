#pragma once

#include <functional>
#include "types.hpp"
#include "canvas.hpp"
#include "paint_particle.hpp"

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    struct brush_params {
        double radius;
        paint_particle paint;
        bool mix;
        int antialias;
    };

    struct application_params {
        point loc;
        double time;
    };

    using brush_fn = std::function<void(canvas&, brush_params&, const application_params&)>;

    struct brush {
        brush_params params;
        brush_fn apply;
    };

    brush create_mixing_brush(double radius, int aa_level);
    brush create_absolute_brush(double radius, const paint_particle& pp, int aa_level, double k);

    void apply_brush(canvas& canv, brush& brush, const point& loc, double t);

}