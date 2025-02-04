#include "flowbee.hpp"
#include <deque>
#include <ranges>

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

    struct paint_glob {
        double elapsed;
        flo::brush brush;
        std::deque<flo::point> history;
    };

    flo::point random_loc(const flo::dimensions& dim) {
        return flo::point{
            flo::uniform_rand(0, dim.wd - 1),
            flo::uniform_rand(0, dim.hgt - 1)
        };
    }

    flo::point random_blank_loc(const flo::canvas& canv) {
        auto blank_locs = canv.blank_locs();
        if (blank_locs.empty()) {
            return random_loc(canv.bounds());
        }
        auto index = flo::rand_number(0, blank_locs.size() - 1);
        return flo::to_point(blank_locs.at(index));
    }

    paint_glob random_paint_glob(
            const flo::canvas& canv,
            const flo::brush_params& params, int num_colors,
            bool populate_white_space) {

        auto dim = canv.bounds();
        int area = dim.wd * dim.hgt;
        flo::point rand_loc = (populate_white_space && canv.num_blank_locs() < area / 2) ?
            random_blank_loc(canv) :
            random_loc(dim);

        paint_glob p = {
            0.0,
            flo::brush(
                params,
                flo::make_one_color_paint(
                    num_colors, flo::rand_number(0, num_colors - 1), 1.0
                )
            ),
            {}
        };
        p.history.push_back( rand_loc );
        return p;
    }

    bool is_done(const flo::canvas& canv, int iters, const flo::flowbee_params& params) {
        if (params.iterations) {
            return iters >= params.iterations.value();
        }
        return canv.num_blank_locs() == 0;
    }

    double pcnt_done(const flo::canvas& canv, int iters, const flo::flowbee_params& params) {
        
        if (params.iterations) {
            return static_cast<double>(iters) / static_cast<double>(*params.iterations);
        }
        auto dim = canv.bounds();
        auto area = static_cast<double>(dim.wd * dim.hgt);
        return (area - static_cast<double>(canv.num_blank_locs())) / area;
    }

    void display_progress(int& iters, const flo::canvas& canv, const flo::flowbee_params& params) {
        if (++iters % 50 == 0) {
            std::print(".");
        }
        if (iters > 0 && iters % 500 == 0) {
            std::println(" {:.4f}%",
                100.0 * pcnt_done(canv, iters, params)
            );
        }
    }
}


flo::flowbee_params::flowbee_params(const brush_params& b, int iters, int n_particles) :
    brush(b),
    particle_volume(1.0),
    max_particle_history(10),
    dead_particle_area_sz(3.0),
    alpha_threshold(1.0),
    delta_t(2.0),
    iterations(iters),
    num_particles(n_particles),
    populate_white_space(true)
{
}

flo::flowbee_params::flowbee_params(const brush_params& b, int n_particles) :
    flowbee_params(b, 0, n_particles)
{
    iterations = {};
}

void flo::do_flowbee(
        const std::string& outfile_path, const std::vector<flo::rgb_color>& palette, 
        const vector_field& flow, const flowbee_params& params) {

    auto dim = flow.x.bounds();
    flo::canvas canvas(palette, dim);
    int num_colors = static_cast<int>(palette.size());

    std::vector<paint_glob> particles = rv::iota(0, params.num_particles) | rv::transform(
        [&](auto)->paint_glob {
            return random_paint_glob(canvas, params.brush, num_colors, false);
        }
    ) | r::to<std::vector>();

    int iters = 0;
    while (! is_done(canvas, iters, params)) {

        display_progress(iters, canvas, params);
        
        for (auto& p : particles) {
            auto loc = p.history.back();

            p.brush.apply(canvas, loc, { params.delta_t, p.elapsed });
            p.elapsed += params.delta_t;

            flo::point velocity = vector_from_field(flow, loc);
            loc = loc + params.delta_t * velocity;
            p.history.push_back(loc);
            if (p.history.size() > params.max_particle_history) {
                p.history.pop_front();
            }
        }

        particles = particles | rv::filter(
            [&](const auto& p) {
                if (!flo::in_bounds(p.history.back(), dim)) {
                    return false;
                }
                if (p.history.size() == params.max_particle_history) {
                    auto points = p.history | r::to<std::vector>();
                    auto hull_dim = flo::convex_hull_bounds(points);
                    if (hull_dim.wd < params.dead_particle_area_sz && 
                            hull_dim.hgt < params.dead_particle_area_sz) {
                        return false;
                    }
                }
                return true;
            }
        ) | r::to<std::vector>();

        while (particles.size() < params.num_particles) {
            particles.push_back(
                random_paint_glob(
                    canvas, params.brush, num_colors, params.populate_white_space
                )
            );
        }
    }

    flo::img_to_file(
        outfile_path,
        flo::canvas_to_image(canvas, params.alpha_threshold)
    );

    std::println("\ncomplete.\n(after {} iterations)", iters);

}


