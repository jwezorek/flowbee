
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

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

}


int main(int argc, char* argv[]) {

    std::println("flowbee...");

    std::vector<flo::rgb_color> palette =
        std::array<std::string, 4>{ "#ffffff", "#26547c", "#ef476f", "#ffd166" } |
        rv::transform(flo::hex_str_to_rgb) | r::to<std::vector>();

    auto x_comp = flo::pow(flo::perlin_noise({ 1600, 1200 }, 7516417, 4, 4.0), 2.0);
    auto y_comp = flo::pow(flo::perlin_noise({ 1600, 1200 }, 3627717, 4, 4.0), 2.0);
    auto flow = flo::vector_field_from_scalar_fields(x_comp, y_comp);
    auto circle = flo::circular_vector_field({ 1600, 1200 }, flo::circle_field_type::clockwise);
    auto out = flo::circular_vector_field({ 1600, 1200 }, flo::circle_field_type::outward);
    flow = flo::normalize( flow + circle + out);

    flo::do_flowbee(
        "D:\\test\\flowbee_test_5.png",
        palette,
        flow,
        flo::flowbee_params(
            flo::brush_params{
                .radius = 3.0,
                .mix = true,
                .mode = flo::paint_mode::overlay,
                .aa_level = 4,
                .paint_transfer_coeff = 0.75
            },
            50000,
            400
        )
    );

    return 0;
}