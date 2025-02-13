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

    bool is_particle_alive(const paint_glob& p, const flo::dimensions& bounds, 
            int max_particle_history, int dead_particle_area_sz) {
        if (!p.brush.is_alive()) {
            return false;
        }
        if (!flo::in_bounds(p.history.back(), bounds)) {
            return false;
        }
        
        if (p.history.size() == max_particle_history) {
            auto points = p.history | r::to<std::vector>();
            auto hull_dim = flo::convex_hull_bounds(points);
            if (hull_dim.wd < dead_particle_area_sz && hull_dim.hgt <dead_particle_area_sz) {
                return false;
            }
        }

        return true;
    }

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
            const flo::brush_params& params,
            const std::vector<int> palette,
            bool populate_white_space, double elapsed, 
            std::optional<double> total_time) {

        auto dim = canv.bounds();
        int area = dim.wd * dim.hgt;
        flo::point rand_loc = (populate_white_space && canv.num_blank_locs() < area / 2) ?
            random_blank_loc(canv) :
            random_loc(dim);

        auto brush = flo::brush(
            params,
            flo::make_one_color_paint(
                canv.palette_size(), flo::random_item(palette), 1.0
            )
        );

        // if the brush has a ramp out period that extends beyond the known
        // duration of the simulation, truncate the brush's lifespan so that
        // it ends when the simulation ends.

        if (brush.lifespan() && total_time) {
            if (elapsed + *brush.lifespan() > *total_time) {
                brush.set_lifespan(*total_time - elapsed);
            }
        }

        paint_glob p = {
            0.0,
            brush,
            {}
        };
        p.history.push_back( rand_loc );
        return p;
    }

    double pcnt_done(const flo::canvas& canv, int iters, const flo::flowbee_params& params) {
        
        if (params.termination_criterion > 1.0) {
            return static_cast<double>(iters) / static_cast<double>(params.termination_criterion);
        }
        auto dim = canv.bounds();
        auto area = static_cast<double>(dim.wd * dim.hgt);
        return (area - static_cast<double>(canv.num_blank_locs())) / area;
    }

    bool is_done(const flo::canvas& canv, int iters, const flo::flowbee_params& params) {
        if (params.termination_criterion > 1.0) {
            int max_iters = static_cast<int>(params.termination_criterion);
            return iters >= max_iters;
        }
        return pcnt_done(canv, iters, params) >= params.termination_criterion;
    }

    void display_progress(int iters, const flo::canvas& canv, const flo::flowbee_params& params) {
        if (iters == 0) {
            std::print("    ");
        }
        iters++;
        if (iters % 50 == 0) {
            std::print(".");
        }
        if (iters > 0 && iters % 500 == 0) {
            std::print(" {:.4f}%\n    ",
                100.0 * pcnt_done(canv, iters, params)
            );
        }
    }

    void apply_laplacian_diffusion(flo::canvas& canvas, double diffusion_rate) {
        auto& cells = canvas.cells();
        auto dims = canvas.bounds();

        flo::matrix<flo::paint_particle> new_cells = cells;

        for (int y = 1; y < dims.hgt - 1; ++y) {
            for (int x = 1; x < dims.wd - 1; ++x) {
                // Compute the Laplacian: sum of neighbors minus 4 * center
                flo::paint_particle laplacian =
                    cells[x + 1, y] + cells[x - 1, y] +
                    cells[x, y + 1] + cells[x, y - 1] -
                    4.0 * cells[x, y];

                // Diffuse paint based on the Laplacian (scaled by diffusion rate)
                new_cells[x, y] = cells[x, y] + diffusion_rate * laplacian;
            }
        }

        cells = std::move(new_cells);
    }

    flo::point position_delta(
            const flo::point& loc, const flo::vector_field& flow, double delta_t,
            const std::optional<flo::jitter_params>& jitter) {
        flo::point velocity = vector_from_field(flow, loc);
        if (jitter) {
            auto flow_theta = std::atan2(velocity.y, velocity.x);
            auto jitter_theta = flo::normal_rand(0.0, jitter->stddev);
            auto theta = flow_theta + jitter->weight * jitter_theta;
            velocity = {
                std::cos(theta),
                std::sin(theta)
            };
        }
        return delta_t * velocity;
    }

    int flowbee_layer(
        flo::canvas& canvas, const flo::vector_field& flow, const flo::flowbee_params& params) {

        auto dim = canvas.bounds();
        int iters = 0;
        double elapsed = 0.0;

        auto palette = params.palette_subset.empty() ?
            rv::iota(0, canvas.palette_size()) | r::to<std::vector>() :
            params.palette_subset;

        std::optional<double> total_time;
        if (params.termination_criterion > 1.0) {
            total_time = params.termination_criterion * params.delta_t;
        }

        std::vector<paint_glob> particles = rv::iota(0, params.num_particles) | rv::transform(
            [&](auto)->paint_glob {
                return random_paint_glob(canvas, params.brush, palette, false, elapsed, total_time);
            }
        ) | r::to<std::vector>();

        while (!is_done(canvas, iters, params)) {

            display_progress(iters, canvas, params);

            for (auto& p : particles) {
                auto loc = p.history.back();

                p.brush.apply(canvas, loc, { params.delta_t, p.elapsed });
                p.elapsed += params.delta_t;
                
                loc = loc + position_delta(loc, flow, params.delta_t, params.jitter);
                p.history.push_back(loc);
                if (p.history.size() > params.max_particle_history) {
                    p.history.pop_front();
                }
            }

            particles = particles | rv::filter(
                [&](const auto& p) {
                    auto alive = is_particle_alive(
                        p, dim, params.max_particle_history, params.dead_particle_area_sz
                    );
                    return alive;
                }
            ) | r::to<std::vector>();

            while (particles.size() < params.num_particles) {
                particles.push_back(
                    random_paint_glob(
                        canvas, params.brush, palette, params.populate_white_space,
                        elapsed, total_time
                    )
                );
            }

            if (params.diffusion_rate && *params.diffusion_rate > 0.0) {
                apply_laplacian_diffusion(canvas, *params.diffusion_rate);
            }

            ++iters;
            elapsed += params.delta_t;
        }
        std::println("");
        return iters;
    }
}

flo::flowbee_params::flowbee_params(const brush_params& b, int iters, int n_particles) :
    brush(b),
    particle_volume(1.0),
    max_particle_history(10),
    dead_particle_area_sz(3.0),
    delta_t(1.0),
    termination_criterion(iters),
    num_particles(n_particles),
    populate_white_space(true)
{
}

flo::flowbee_params::flowbee_params(const brush_params& b, int n_particles) :
    flowbee_params(b, 0, n_particles)
{
    termination_criterion = 1.0;
}

void flo::do_flowbee(
        const output_params& output, const std::vector<flo::rgb_color>& palette,
        const vector_field& flow, const flowbee_params& params) {

    flo::canvas canvas(palette, flow.x.bounds());
    auto iters = flowbee_layer(canvas, flow, params);

    flo::img_to_file(
        output.filename,
        flo::canvas_to_image(
            canvas, output.alpha_threshold, output.canvas_color
        )
    );

    std::println("\n    complete.\n    {} iterations", iters);
}

void flo::do_flowbee(
        const output_params& output,
        const std::vector<flo::rgb_color>& palette, const std::vector<layer_params>& layers) {

    if (layers.size() == 1) {
        const auto& layer = layers.front();
        do_flowbee(output, palette, layer.flow, layer.params);
        return;
    }

    flo::canvas canvas(palette, layers.front().flow.x.bounds());
    int iters = 0;
    for (const auto& [layer_index,layer] : rv::enumerate(layers)) {
        std::println(" - layer {} -", layer_index + 1);
        iters += flowbee_layer(canvas, layer.flow, layer.params);
    }

    flo::img_to_file(
        output.filename,
        flo::canvas_to_image(
            canvas, output.alpha_threshold, output.canvas_color
        )
    );

    std::println("\ncomplete.\n(after {} iterations)", iters);
}

