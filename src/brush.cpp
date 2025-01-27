#include "brush.hpp"
#include "canvas.hpp"

/*------------------------------------------------------------------------------------------------*/

namespace {

}

flo::brush flo::create_mixing_brush(double radius) {

    return brush{
        .radius = radius,
        .paint = {},
        .mix = true,
        .from_brush_fn = {},
        .from_canv_fn = {}
    };

}

flo::brush flo::create_simple_brush(
    const paint& p, double vol, double radius, double from_canvas, double from_brush) {
    return brush{
        .radius = radius,
        .paint = vol * p,
        .mix = false,
        .from_brush_fn = [from_brush](const paint& p, double t)->paint {
            return {};
        },
        .from_canv_fn = [from_canvas](const paint& p, double t)->paint {
            return {};
        }
    };
}

void flo::apply_brush(canvas& canv, brush& brush, const point& loc, double delta_t, int aa_level) {

    if (brush.mix) {
        mix(canv, loc, brush.radius, aa_level);
    }

    if (!brush.from_brush_fn || !brush.from_canv_fn) {
        return;
    }

    auto brush_rgn_area = brush_region_area(canv.bounds(), loc, brush.radius, aa_level);
    auto paint_on_canvas = all_paint_in_brush_region(canv, loc, brush.radius, aa_level);
    auto paint_from_canvas = brush.from_canv_fn(paint_on_canvas, delta_t);
    auto paint_from_brush = brush.from_brush_fn(brush.paint, delta_t);

    auto canvas_delta = paint_on_canvas - paint_from_canvas + paint_from_brush;
    auto canvas_delta_per_pixel = (1.0 / brush_rgn_area) * canvas_delta;
    overlay(canv, loc, brush.radius, aa_level, canvas_delta_per_pixel);

    brush.paint = clamp_nonnegative(brush.paint - paint_from_brush + paint_from_canvas);
}


