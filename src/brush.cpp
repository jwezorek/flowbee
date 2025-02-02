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

flo::brush flo::create_absolute_brush(
        double radius, const paint_particle& pp, int aa_level, double k) {

    return flo::brush{
        .params = {
            .radius = radius,
            .paint = pp,
            .mix = false,
            .antialias = aa_level
        },
        .apply = [k](canvas& canv, brush_params& bp, const application_params& args) {
            auto brush_rgn_area = 
                brush_region_area(canv.bounds(), args.loc, bp.radius, bp.antialias);
            auto paint_on_canvas = 
                all_paint_in_brush_region(canv, args.loc, bp.radius, bp.antialias);
            paint_on_canvas.normalize();
            auto new_paint = (paint_on_canvas.volume() > 0.0) ?
                (1.0 - k) * paint_on_canvas + k * bp.paint :
                bp.paint;
            bp.paint = new_paint;
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