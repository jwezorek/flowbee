#pragma once

#include <functional>
#include "types.hpp"
#include "canvas.hpp"
#include "paint.hpp"

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    using mix_fn = std::function<paint(const paint&, double, double)>;

    struct brush {
        double radius;
        paint paint;
        bool mix;
        mix_fn from_brush_fn;
        mix_fn from_canv_fn;
    };

    brush create_mixing_brush(double radius);
    brush create_simple_brush(const paint& p, double volume, double radius,
        double from_canvas, double from_brush
    );

    void apply_brush(canvas& canv, brush& brush, const point& loc, double t, int antialias);

}