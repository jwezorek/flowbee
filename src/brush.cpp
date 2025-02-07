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

    double current_radius(double elapsed, double base_radius, std::optional<double> ramp_in_time,
            std::optional<double> stroke_lifespan, std::optional<double> ramp_out_time) {
        double in_radius = base_radius;
        double out_radius = base_radius;
        if (ramp_in_time && elapsed < *ramp_in_time) {
            in_radius = (elapsed / *ramp_in_time) * (base_radius - 1.0) + 1.0;
        }
        if (stroke_lifespan && ramp_out_time) {
            auto time_left = *stroke_lifespan - elapsed;
            if (time_left < 0.0) {
                out_radius = 0.0;
            }
            if (time_left < *ramp_out_time) {
                out_radius = (time_left / *ramp_out_time) * (base_radius - 1.0) + 1.0;
            }
        }
        return std::min(in_radius, out_radius);
    }
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
        radius_(params.radius),
        ramp_in_time_(params.radius_ramp_in_time),
        mix_(params.mix),
        mode_(params.mode),
        aa_level_(params.aa_level),
        paint_transfer_coeff_(params.paint_transfer_coeff),
        paint_(p),
        alive_(true) {

    if (params.stroke_lifetime) {
        lifespan_ = normal_rand(params.stroke_lifetime->mean, params.stroke_lifetime->stddev);
        ramp_out_time_ = params.stroke_lifetime->ramp_out_time;
    }

}

void flo::brush::apply(canvas& canv, const point& loc, const elapsed_time& t) {

    double radius = current_radius( t.elapsed, radius_, ramp_in_time_, lifespan_, ramp_out_time_);

    if (mix_) {
        auto brush_rgn_area =
            brush_region_area(canv.bounds(), loc, radius, aa_level_);
        auto paint_on_canvas =
            all_paint_in_brush_region(canv, loc, radius, aa_level_);
        paint_on_canvas.normalize();

        auto k = paint_transfer_coeff_;
        auto new_paint = (paint_on_canvas.volume() > 0.0) ?
            (1.0 - k) * paint_on_canvas + k * paint_ :
            paint_;

        paint_ = new_paint;
    }

    if (mode_ == paint_mode::overlay) {
        overlay(canv, loc, radius, aa_level_, paint_);
    } else if (mode_ == paint_mode::fill) {
        fill(canv, loc, radius, aa_level_, paint_);
    } else {
        mix(canv, loc, radius, aa_level_);
    }
    
    if (lifespan_ && t.elapsed >= lifespan_) {
        alive_ = false;
    }
}

bool flo::brush::is_alive() const {
    return alive_;
}

std::optional<double> flo::brush::lifespan() const {
    return lifespan_;
}

void flo::brush::set_lifespan(double duration) {
    lifespan_ = duration;
}

void flo::brush::set_radius(double rad) {
    radius_ = rad;
}
