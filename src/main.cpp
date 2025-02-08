
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
    auto rand1 = flo::perlin_vector_field(dim, 23232, 45342, 4, 4.0, true);
    auto flow1 = flo::normalize(flo::point{1.5,0} + rand1);
    auto layer1 = flo::flowbee_params(
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
    layer1.palette_subset = { 0,1,2 };

    auto rand2 = flo::perlin_vector_field(dim, 314159, 2333788, 4, 4.0, true);
    auto flow2 = flo::normalize(flo::point{0,1.5 } + rand2);
    auto layer2 = flo::flowbee_params(
        flo::brush_params{
            .radius = 8.0,
            .radius_ramp_in_time = 50.0,
            .mix = true,
            .mode = flo::paint_mode::fill,
            .aa_level = 4,
            .paint_transfer_coeff = 0.95
        },
        1000,
        15
    );
    layer2.palette_subset = { 3,4,5 };

    flo::do_flowbee(
        { "D:\\test\\test_img.png" },
        palette,
        std::vector<flo::layer_params>{{flow1, layer1}, { flow2,layer2 }}
    );

    return 0;
}