#include "brush.hpp"
#include "canvas.hpp"

/*------------------------------------------------------------------------------------------------*/

namespace {

}

flo::brush flo::create_mixing_brush(const point& loc, double radius) {

    return brush{
        .loc = loc,
        .radius = radius,
        .volume = 0.0,
        .paint = {},
        .update_radius = {},
        .update_volume = {},
        .update_canvas = {}
    };

}

void flo::paint(canvas& canv, brush& brush, double t, int aa_level) {

    mix(canv, brush.loc, brush.radius, aa_level);
    
    if (brush.paint.volume == 0.0) {
        return;
    }

    //TODO

}


