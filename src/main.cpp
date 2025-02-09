
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
        std::array<std::string, 6>{ "ef476f","f78c6b","ffd166","06d6a0","118ab2","073b4c" } |
        rv::transform(flo::hex_str_to_rgb) | r::to<std::vector>();

    flo::dimensions dim{ 800,800 };
    auto rand = flo::perlin_vector_field(dim, 4, 4.0, 0.5, true);
    auto flow = flo::logarithmic_spiral_vector_field( dim, 1.0, false, false );
    auto params = flo::flowbee_params(
        flo::brush_params{
            .radius = 8.0,
            .radius_ramp_in_time = 50.0,
            .mix = true,
            .mode = flo::paint_mode::fill,
            .aa_level = 4,
            .paint_transfer_coeff = 0.3
        },
        150
    );

    flo::do_flowbee(
        { "D:\\test\\test_img.png" },
        palette,
        flow,
        params
    );

    return 0;
}