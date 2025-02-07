
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
        std::array<std::string, 5>{ "e63946","f1faee","a8dadc","457b9d","1d3557" } |
        rv::transform(flo::hex_str_to_rgb) | r::to<std::vector>();

    auto img = flo::to_gray_scale(
        flo::img_from_file("D:\\test\\myrna.png")
    );

    auto flow = flo::gradient(img, 5, true);

    auto params = flo::flowbee_params(
        flo::brush_params{
            .radius = 3.0,
            .radius_ramp_in_time = 10.0,
            .mix = true,
            .mode = flo::paint_mode::fill,
            .aa_level = 4,
            .paint_transfer_coeff = 0.55,
        },
        50000,
        300
    );

    params.dead_particle_area_sz = 25.0;

    flo::do_flowbee(
        "D:\\test\\from_image_notnorm.png",
        palette,
        flow,
        params
    );

    return 0;
}