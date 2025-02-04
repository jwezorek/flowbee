#include "brush.hpp"
#include "canvas.hpp"
#include "types.hpp"
#include <print>
#include <ranges>
#include <unordered_map>
#include <boost/functional/hash.hpp>

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

    constexpr double k_fixed_point_scale = 10000.0;

}

flo::detail::memo_key::memo_key(flo::point loc, double radius, int aa) {
    x = static_cast<int>(std::round(loc.x * k_fixed_point_scale));
    y = static_cast<int>(std::round(loc.y * k_fixed_point_scale));
    r = static_cast<int>(std::round(radius * k_fixed_point_scale));
    aa_level = aa;
}

bool flo::detail::memo_key::operator==(const flo::detail::memo_key& other) const {
    return x == other.x && y == other.y && r == other.r && aa_level == other.aa_level;
}

size_t flo::detail::memo_key_hash::operator()(const  flo::detail::memo_key & key) const {
    size_t seed = 0;
    boost::hash_combine(seed, key.x);
    boost::hash_combine(seed, key.y);
    boost::hash_combine(seed, key.r);
    boost::hash_combine(seed, key.aa_level);
    return seed;
}

std::vector<flo::region_pixel> flo::detail::brush_region_aux(
        const flo::point& brush_loc,
        double brush_radius,
        int anti_aliasing_level) {

    const int resolution = (1 << anti_aliasing_level); // 2^level subdivisions
    const double subcell_size = 1.0 / resolution;
    const double subcell_area = subcell_size * subcell_size;
    const double radius_squared = brush_radius * brush_radius;

    // Determine the bounding box for the circle on the canvas
    int min_x = static_cast<int>(std::floor(brush_loc.x - brush_radius));
    int max_x = static_cast<int>(std::ceil(brush_loc.x + brush_radius));
    int min_y = static_cast<int>(std::floor(brush_loc.y - brush_radius));
    int max_y = static_cast<int>(std::ceil(brush_loc.y + brush_radius));

    // Create a range for all cells in the bounding box
    auto cell_range = std::views::cartesian_product(
        std::views::iota(min_y, max_y + 1),
        std::views::iota(min_x, max_x + 1)
    );

    // Transform each cell to calculate its weight
    return cell_range | std::views::transform(
        [=](const auto& cell) -> flo::region_pixel {
            auto [y, x] = cell;
            double total_weight = 0.0;

            // Subdivide the cell into smaller cells for anti-aliasing
            for (int sy = 0; sy < resolution; ++sy) {
                for (int sx = 0; sx < resolution; ++sx) {
                    // Compute the center of the subcell
                    double sub_x = x + (sx + 0.5) * subcell_size;
                    double sub_y = y + (sy + 0.5) * subcell_size;

                    // Check if the subcell center is within the circle
                    double dx = sub_x - brush_loc.x;
                    double dy = sub_y - brush_loc.y;
                    if ((dx * dx + dy * dy) <= radius_squared) {
                        total_weight += subcell_area;
                    }
                }
            }

            return { {x, y}, total_weight };
        }
    ) | std::views::filter(
        [](const flo::region_pixel& cell_data) {
            return cell_data.weight > 0.0;
        }
    ) | r::to<std::vector>();
}

flo::brush::brush(const brush_params& params, const paint_particle& p) : 
        params_(params), paint_(p) {

}

void flo::brush::apply(canvas& canv, const point& loc, const elapsed_time& t) {

    double radius = (!params_.radius_ramp_in_time || t.elapsed > *params_.radius_ramp_in_time) ?
        params_.radius :
        (t.elapsed / *params_.radius_ramp_in_time) * (params_.radius - 1.0) + 1.0;


    if (params_.mix) {
        auto brush_rgn_area =
            brush_region_area(canv.bounds(), loc, radius, params_.aa_level);
        auto paint_on_canvas =
            all_paint_in_brush_region(canv, loc, radius, params_.aa_level);
        paint_on_canvas.normalize();

        auto k = params_.paint_transfer_coeff;
        auto new_paint = (paint_on_canvas.volume() > 0.0) ?
            (1.0 - k) * paint_on_canvas + k * paint_ :
            paint_;

        paint_ = new_paint;
    }

    if (params_.mode == paint_mode::overlay) {
        overlay(canv, loc, radius, params_.aa_level, paint_);
    } else if (params_.mode == paint_mode::fill) {
        fill(canv, loc, radius, params_.aa_level, paint_);
    } else {
        mix(canv, loc, radius, params_.aa_level);
    }
    
}