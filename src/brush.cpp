#include "brush.hpp"
#include "canvas.hpp"

/*------------------------------------------------------------------------------------------------*/

namespace {

}

flo::brush flo::create_mixing_brush(double radius) {

    return brush{
        .radius = radius,
        .volume = 0.0,
        .paint = {},
        .mix = true,
        .from_brush_fn = {},
        .from_canv_fn = {}
    };

}

flo::brush flo::create_simple_brush(const paint& p, double volume, double radius)
{
    return brush();
}

void flo::apply_brush(canvas& canv, brush& brush, const point& loc, double t, int aa_level) {

    if (brush.mix) {
        mix(canv, loc, brush.radius, aa_level);
    }

    //TODO

}


