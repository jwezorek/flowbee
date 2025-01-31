#include "brush.hpp"
#include "canvas.hpp"
#include <print>

/*------------------------------------------------------------------------------------------------*/

namespace {

}

flo::brush flo::create_mixing_brush(double radius, int aa_level) {

    return flo::brush{
        .params = {
            .radius = radius,
            .paint = {},
            .mix = true,
            .antialias = aa_level
        },
        .apply = {}
    };

}

flo::brush flo::create_absolute_brush(double radius, const paint_particle& pp, int aa_level, double k) {

    return flo::brush{
        .params = {
            .radius = radius,
            .paint = pp,
            .mix = false,
            .antialias = aa_level
        },
        .apply = [k](canvas& canv, brush_params& bp, const application_params& args) {
            auto brush_rgn_area = brush_region_area(canv.bounds(), args.loc, bp.radius, bp.antialias);
            auto paint_on_canvas = all_paint_in_brush_region(canv, args.loc, bp.radius, bp.antialias);
            paint_on_canvas.normalize();
            auto new_paint = (1.0 - k) * paint_on_canvas + k * bp.paint;
            bp.paint = new_paint;
            if (std::isnan(bp.paint.volume())) {
                int aaa;
                aaa = 5;
                auto test = all_paint_in_brush_region(canv, args.loc, bp.radius, bp.antialias);
            }
            fill(canv, args.loc, bp.radius, bp.antialias, new_paint);
        }
    };

}

void flo::apply_brush(canvas& canv, brush& brush, const point& loc, double t) {
    if (brush.params.mix) {
        mix(canv, loc, brush.params.radius, brush.params.antialias);
    }
    if (!brush.apply) {
        return;
    }
    brush.apply(canv, brush.params, { loc, t });
}

/*
void flo::apply_brush(canvas& canv, brush& brush, const point& loc, double delta_t, int aa_level) {

    if (brush.mix) {
        mix(canv, loc, brush.radius, aa_level);
    }

    if (!brush.from_brush_fn || !brush.from_canv_fn) {
        return;
    }
    auto brush_rgn_area = brush_region_area(canv.bounds(), loc, brush.radius, aa_level);
    auto paint_on_canvas = all_paint_in_brush_region(canv, loc, brush.radius, aa_level);
    auto paint_from_canvas = brush.from_canv_fn(paint_on_canvas, brush_rgn_area, delta_t);
    auto paint_from_brush = brush.from_brush_fn(brush.paint, brush_rgn_area, delta_t);
    auto canvas_delta = paint_from_brush - paint_from_canvas;
    auto brush_delta = paint_from_canvas - paint_from_brush;
    auto canvas_delta_per_pixel = (1.0 / brush_rgn_area) * canvas_delta;

    std::println("curr brush : {}", display(brush.paint));
    std::println("paint from canvas : {}", display(paint_from_canvas));
    std::println("paint from brush : {}", display(paint_from_brush));
    std::println("canvas delta : {}", display(canvas_delta));
    std::println("brush delta : {}", display(brush_delta));
    std::println("canvas delta per pixel : {}", display(canvas_delta_per_pixel));
    std::println("");

    overlay(canv, loc, brush.radius, aa_level, canvas_delta_per_pixel);
    brush.paint += brush_delta;
    //brush.paint = clamp_nonnegative(brush.paint - paint_from_brush + paint_from_canvas);
}
*/
