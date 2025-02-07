
#include "util.hpp"
#include "vector_field.hpp"
#include "paint_particle.hpp"
#include "canvas.hpp"
#include "brush.hpp"
#include "flowbee.hpp"
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
#include <complex>

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {
    
}


int main(int argc, char* argv[]) {

    std::println("flowbee...");

    std::vector<flo::rgb_color> palette =
        std::array<std::string, 5>{ "#ffac81","#ff928b","#fec3a6","#efe9ae","#cdeac0" } |
        rv::transform(flo::hex_str_to_rgb) | r::to<std::vector>();

    
    //std::vector<flo::rgb_color> palette =
    //    std::array<std::string, 6>{ "#ddedff","#ddddff","#ff595e", "#ffca3a", "#8ac926", "#1982c4" } |
    //    rv::transform(flo::hex_str_to_rgb) | r::to<std::vector>();
    



    /*
    auto x_comp = flo::pow(flo::perlin_noise({ 1600, 1200 }, 1751617, 8, 8.0), 0.5);
    auto y_comp = flo::pow(flo::perlin_noise({ 1600, 1200 }, 3677171, 8, 8.0), 0.5);
    auto flow = flo::vector_field_from_scalar_fields(x_comp, y_comp);
    auto circle = flo::circular_vector_field({ 1600, 1200 }, flo::circle_field_type::counterclockwise);
    auto out = flo::circular_vector_field({ 1600, 1200 }, flo::circle_field_type::outward);
    flow = flo::normalize( flow + circle + 2.0*out);
    */

    auto pi = std::numbers::pi;
    auto dim = flo::dimensions{ 800, 800 };
    auto spiral = flo::loxodromic_spiral_vector_field(dim, false, 400, 5*pi);
    auto x_comp = flo::pow(flo::perlin_noise(dim, 1117, 8, 8.0), 0.5);
    auto y_comp = flo::pow(flo::perlin_noise(dim, 7211, 8, 8.0), 0.5);
    auto rand = flo::vector_field_from_scalar_fields(x_comp, y_comp);

    auto flow = spiral + 1.1 * rand;
    auto params = flo::flowbee_params(
        flo::brush_params{
            .radius = 4.0,
            .radius_ramp_in_time = 15.0,
            .mix = true,
            .mode = flo::paint_mode::fill,
            .aa_level = 4,
            .paint_transfer_coeff = 0.55,
            //.stroke_lifetime = flo::stroke_lifetime{
            //    200, 10, 15.0
            // }
        },
        10000,
        30
    );

    params.max_particle_history = 25;

    flo::do_flowbee(
        "D:\\test\\flowbee_loxo_small.png",
        palette,
        flow,
        params
    );

    return 0;
}