#include "canvas.hpp"
#include "brush.hpp"
#include "util.hpp"
#include <ranges>
#include <functional>
#include <numeric>

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {
    flo::pigment_map<double> paint_in_region(
            const flo::canvas& canv, const flo::point& loc, double radius, int aa_level) {

        flo::pigment_map<double> paint;
        for (const auto& [loc, weight] : flo::brush_region(canv.bounds(), loc, radius, aa_level)) {
            paint[canv[loc].color] += weight * canv[loc].volume;
        }

        return paint;
    }

}

double flo::brush_region_area(const dimensions& dim, const point& loc, double rad, int aa)
{
    return r::fold_left(
        flo::brush_region(dim, loc, rad, aa) | rv::values,
        0.0,
        std::plus<double>()
    );
}

void flo::fill(canvas& canv, const point& loc, double radius, int aa_level,
        const paint_particle& paint) {
    for (const auto& [loc, weight] : flo::brush_region(canv.bounds(), loc, radius, aa_level)) {
        if (weight == 1.0) {
            canv[loc] = paint;
            continue;
        }
        auto c = canv[loc];
        auto new_color = mix_pigments(c.color, c.volume, paint.color, paint.volume * weight);
        auto new_volume = c.volume + paint.volume * weight;
        canv[loc] = { new_color, new_volume };
    }
}

void flo::mix(canvas& canv, const point& loc, double radius, int aa_level) {
    auto paints = paint_in_region(canv, loc, radius, aa_level);
    auto total_volume = r::fold_left(paints | rv::values, 0.0, std::plus<double>());
    auto new_pigment = mix_paint(paints);
    auto area = brush_region_area(canv.bounds(), loc, radius, aa_level);
    auto volume_per_area = total_volume / area;
    paint_particle paint = { new_pigment, volume_per_area };
    for (const auto& [loc, weight] : flo::brush_region(canv.bounds(), loc, radius, aa_level)) {
        const auto& existing_paint = canv[loc];
        if (weight == 1.0) {
            canv[loc] = paint;
            continue;
        }
        if (existing_paint.volume <= volume_per_area * weight) {
            canv[loc] = { new_pigment, volume_per_area * weight };
            continue;
        }
        auto new_pigment = mix_pigments(
            existing_paint.color, 
            existing_paint.volume - volume_per_area * weight,
            paint.color,
            paint.volume * weight
        );
        auto new_volume = existing_paint.volume - volume_per_area * weight + paint.volume * weight;

        canv[loc] = { new_pigment, new_volume };
    }
}

flo::canvas flo::image_to_canvas(const image& img, double vol_per_pixel) {

    return img.transform_to<paint_particle>(
        [vol_per_pixel](uint32_t pix)->paint_particle {
            auto pigment = rgb_to_pigment( pixel_to_rgb(pix) );
            return {
                pigment,
                vol_per_pixel
            };
        }
    );

}

flo::image flo::canvas_to_image(const canvas& canv)
{
    return canv.transform_to<uint32_t>(
        [](auto&& p)->uint32_t {
            return rgb_to_pixel(pigment_to_rgb(p.color));
        }
    );
}
