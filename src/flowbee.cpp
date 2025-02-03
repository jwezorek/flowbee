#include "flowbee.hpp"
#include <deque>
#include <ranges>

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

    struct paint_glob {
        flo::brush brush;
        std::deque<flo::point> history;
    };

    paint_glob random_paint_glob(
            const flo::brush_params& params, int num_colors, const flo::dimensions& dim) {
        paint_glob p = {
            flo::brush(
                params,
                flo::make_one_color_paint(
                    num_colors, flo::rand_number(0, num_colors - 1), 1.0
                )
            ),
            {}
        };
        p.history.push_back(
            flo::point{
                flo::uniform_rand(0, dim.wd - 1),
                flo::uniform_rand(0, dim.hgt - 1)
            }
        );
        return p;
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
    num_particles(n_particles)
{
}

void flo::do_flowbee(
        const std::string& outfile_path, const std::vector<flo::rgb_color>& palette, 
        const vector_field& flow, const flowbee_params& params) {

    auto dim = flow.x.bounds();
    flo::canvas canvas(palette, dim);
    int num_colors = static_cast<int>(palette.size());

    std::vector<paint_glob> particles = rv::iota(0, params.num_particles) | rv::transform(
        [&](auto)->paint_glob {
            return random_paint_glob(params.brush, num_colors, dim);
        }
    ) | r::to<std::vector>();

    double elapsed = 0;
    for (int i = 0; i < params.iterations; ++i) {
        if (i % 100 == 0) {
            std::println("{}", i);
        }

        for (auto& p : particles) {
            auto loc = p.history.back();

            p.brush.apply(canvas, loc, { params.delta_t, elapsed });
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
                random_paint_glob(params.brush, num_colors, dim)
            );
        }

        elapsed += params.delta_t;
    }

    flo::img_to_file(
        outfile_path,
        flo::canvas_to_image(canvas, params.alpha_threshold)
    );

}


