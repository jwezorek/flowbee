#pragma once

#include <functional>
#include <ranges>
#include <print>
#include <optional>
#include "types.hpp"
#include "canvas.hpp"
#include "paint_mixture.hpp"
#include "util.hpp"

/*------------------------------------------------------------------------------------------------*/

namespace flo {

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

    enum class paint_mode {
        overlay,
        fill,
        mix
    };

    struct stroke_lifetime {
        double mean;
        double stddev;
        std::optional<double> ramp_out_time;
    };

    struct brush_params {
        double radius;
        std::optional<double> radius_ramp_in_time;
        bool mix;
        paint_mode mode;
        int aa_level;
        double paint_transfer_coeff;
        std::optional<stroke_lifetime> stroke_lifetime;
    };

    struct elapsed_time {
        double delta_t;
        double elapsed;
    };


    class brush {

        double radius_;
        std::optional<double> ramp_in_time_;
        std::optional<double> ramp_out_time_;
        bool mix_;
        paint_mode mode_;
        int aa_level_;
        double paint_transfer_coeff_;
        paint_mixture paint_;
        std::optional<double> lifespan_;
        bool alive_;
    public:
        brush() {}
        brush(const brush_params& params, const paint_mixture& p);
        void apply(canvas& canv, const point& loc, const elapsed_time& t);
        bool is_alive() const;
        std::optional<double> lifespan() const;
        void set_lifespan(double duration);
        void set_radius(double rad);
    };

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