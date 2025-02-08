
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

    flo::dimensions dim{ 800,800 };
    auto rand = flo::perlin_vector_field(dim, 23232, 45342, 4, 4.0, true);
    auto flow = flo::normalize(flo::point{1.5,0} + rand);

    auto params = flo::flowbee_params(
        flo::brush_params{
            .radius = 8.0,
            .radius_ramp_in_time = 50.0,
            .mix = true,
            .mode = flo::paint_mode::fill,
            .aa_level = 4,
            .paint_transfer_coeff = 0.6
        },
        1000,
        150
    );

    params.palette_subset = { 0,1,2 };

    flo::do_flowbee(
        "D:\\test\\test_img.png",
        palette,
        flow,
        params
    );

    return 0;
}