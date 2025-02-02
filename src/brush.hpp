#pragma once

#include <functional>
#include <ranges>
#include "types.hpp"
#include "canvas.hpp"
#include "paint_particle.hpp"
#include "util.hpp"
#include <print>

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    struct brush_params {
        double radius;
        paint_particle paint;
        bool mix;
        int antialias;
    };

    struct application_params {
        point loc;
        double time;
    };

    using brush_fn = std::function<void(canvas&, brush_params&, const application_params&)>;

    struct brush {
        brush_params params;
        brush_fn apply;
    };

    brush create_mixing_brush(double radius, int aa_level);
    brush create_absolute_brush(double radius, const paint_particle& pp, int aa_level, double k);

    void apply_brush(canvas& canv, brush& brush, const point& loc, double t);

    struct region_pixel {
        coords loc;
        double weight;
    };

    namespace detail {

        struct memo_key {
            int x;
            int y;
            int r;
            int aa_level;

            memo_key(flo::point loc, double radius, int aa);
            bool operator==(const memo_key& other) const;
        };

        struct memo_key_hash {
            size_t operator()(const memo_key& key) const;
        };

        using memoization_tbl =
            std::unordered_map<memo_key, std::vector<flo::region_pixel>, memo_key_hash>;

        std::vector<flo::region_pixel> brush_region_aux(
            const flo::point& brush_loc,
            double brush_radius,
            int anti_aliasing_level);
    
    }

    inline auto brush_region(const flo::dimensions& dim,
            const flo::point& loc,
            double brush_radius,
            int aa_level) {

        namespace r = std::ranges;
        namespace rv = std::ranges::views;

        static detail::memoization_tbl memos;
        point int_loc = { std::floor(loc.x), std::floor(loc.y) };
        point unit_square_loc = loc - int_loc;
        auto key = detail::memo_key(int_loc, brush_radius, aa_level);

        if (!memos.contains(key)) {
            memos[key] = detail::brush_region_aux(unit_square_loc, brush_radius, aa_level);
        }

        return memos.at(key) | rv::transform(
            [int_loc](auto&& rp)->flo::region_pixel {
                return { to_coords(int_loc) + rp.loc, rp.weight };
            }
        ) | rv::filter(
            [dim](const flo::region_pixel& rp) {
                return in_bounds(rp.loc, dim);
            }
        );
    }
}