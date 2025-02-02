
#include "util.hpp"
#include "vector_field.hpp"
#include "paint_particle.hpp"
#include "canvas.hpp"
#include "brush.hpp"
#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <print>
#include <ranges>
#include <unordered_map>
#include <format>
#include <deque>
#include <numbers>

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

    struct particle {
        flo::brush brush;
        std::deque<flo::point> history;
    };

    particle random_particle(int num_colors, const flo::dimensions& dim) {
        particle p = {
            flo::brush(
                flo::brush_params{
                    .radius = 5.0,
                    .mix = true,
                    .mode = flo::paint_mode::fill,
                    .aa_level = 4,
                    .paint_transfer_coeff = 0.7
                },
                flo::make_one_color_paint(
                    num_colors, flo::rand_number(0,num_colors - 1), 1.0
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

    void basic_flowbee(const std::string& out_path, const flo::vector_field& flow,
            const std::vector<flo::rgb_color>& palette, double delta_t, int iterations,
            int n) {
        auto dim = flow.x.bounds();
        flo::canvas canvas(palette, dim);
     
        int num_colors = static_cast<int>(palette.size());
        std::vector<particle> particles = rv::iota(0, n) | rv::transform(
            [&](auto)->particle {
                return random_particle(num_colors, dim);
            }
        ) | r::to<std::vector>();

        double elapsed = 0;
        for (int i = 0; i < iterations; ++i) {
            if (i % 100 == 0) {
                std::println("{} {}", i, particles.size());
            }

            for (auto& p : particles) {
                auto loc = p.history.back();

                p.brush.apply(canvas, loc, { delta_t, elapsed });
                flo::point velocity = vector_from_field(flow, loc);
                loc = loc + delta_t * velocity;
                p.history.push_back(loc);
                if (p.history.size() > 20) {
                    p.history.pop_front();
                }
            }

            particles = particles | rv::filter(
                [&](const auto& p) {
                    if (!flo::in_bounds(p.history.back(), dim)) {
                        return false;
                    }
                    if (p.history.size() == 20) {
                        auto points = p.history | r::to<std::vector>();
                        auto hull_dim = flo::convex_hull_bounds(points);
                        if (hull_dim.wd < 3.0 && hull_dim.hgt < 3.0) {
                            return false;
                        }
                    }
                    return true;
                }
            ) | r::to<std::vector>();

            while (particles.size() < n) {
                particles.push_back(random_particle(num_colors, dim));
            }

            elapsed += delta_t;
        }

        flo::img_to_file(
            out_path,
            flo::canvas_to_image(canvas, 1.0)
        );
    }
}


int main(int argc, char* argv[]) {

    std::println("flowbee...");


    std::vector<flo::rgb_color> palette = {
        {21,  36,  95 },
        {249, 255, 174},
        {255, 253, 118},
        {255, 197, 72 },
        {219, 63,  63 }
    };

    /*
    std::vector<flo::rgb_color> palette = {
        {255, 189, 67 },
        {32,  133, 130},
        {72,  72,  152},
        {174, 181, 219},
        { 137, 132, 132}
    };
    */

    auto x_comp = flo::pow(flo::perlin_noise({ 1000, 1000 }, 724164, 8, 8.0), 0.5);
    auto y_comp = flo::pow(flo::perlin_noise({ 1000, 1000 }, 362474, 8, 8.0), 0.5);
    auto flow = flo::vector_field_from_scalar_fields(x_comp, y_comp);
    auto circle = flo::circular_vector_field({ 1000, 1000 }, flo::circle_field_type::clockwise);
    flow = flo::normalize( 0.5 * flow + circle);


    basic_flowbee(
        "D:\\test\\flow1.png",
        flow,
        palette,
        1.0,
        12000,
        200
    );

    return 0;
}