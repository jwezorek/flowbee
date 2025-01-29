#pragma once

#include <functional>
#include "types.hpp"
#include "canvas.hpp"
#include "paint_particle.hpp"

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    using mix_fn = std::function<paint_particle(const paint_particle&, double, double)>;

    struct brush {
        double radius;
        paint_particle paint;
        bool mix;
        mix_fn from_brush_fn;
        mix_fn from_canv_fn;
    };

    brush create_mixing_brush(double radius);
    brush create_simple_brush(const paint_particle& p, double radius,
        double from_canvas, double from_brush
    );

    void apply_brush(canvas& canv, brush& brush, const point& loc, double t, int antialias);

}