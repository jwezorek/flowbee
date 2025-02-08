
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
        std::array<std::string, 5>{ "cdb4db","ffc8dd","ffafcc","bde0fe","a2d2ff"  } |
        rv::transform(flo::hex_str_to_rgb) | r::to<std::vector>();

    auto img = flo::gradient(
        flo::to_gray_scale(flo::img_from_file("D:\\test\\myrna3.png")), 11, true
    );

    auto rand = flo::perlin_vector_field(img.x.bounds(), 23232, 45342, 8, 8.0, true);
    auto flow = flo::normalize(img + 0.25 * rand);

    auto params = flo::flowbee_params(
        flo::brush_params{
            .radius = 4.0,
            .radius_ramp_in_time = 10.0,
            .mix = true,
            .mode = flo::paint_mode::overlay,
            .aa_level = 4,
            .paint_transfer_coeff = 0.3
        },
        100000,
        500
    );

    flo::do_flowbee(
        "D:\\test\\test_img.png",
        palette,
        flow,
        params
    );

    return 0;
}