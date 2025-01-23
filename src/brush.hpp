#pragma once

#include <functional>
#include "types.hpp"
#include "canvas.hpp"
#include "paint.hpp"

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    using mix_fn = std::function<double(double,double)>;

    struct brush {
        point loc;
        double radius;
        double volume;
        paint_particle paint;
        mix_fn update_radius;
        mix_fn update_volume;
        mix_fn update_canvas;
    };

    brush create_mixing_brush(const point& loc, double radius);

    void paint(canvas& canv, brush& brush, double t, int antialias);

}