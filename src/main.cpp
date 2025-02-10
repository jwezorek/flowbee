
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
#include <chrono>

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {
    void title_display() {
        std::string ascii_art = R"(
   __ _               _               
  / _| |             | |              
 | |_| | _____      _| |__   ___  ___ 
 |  _| |/ _ \ \ /\ / / '_ \ / _ \/ _ \
 | | | | (_) \ V  V /| |_) |  __/  __/
 |_| |_|\___/ \_/\_/ |_.__/ \___|\___|

)";
        std::println("{}", ascii_art);
    }
}


int main(int argc, char* argv[]) {

    title_display();

    auto start_time = std::chrono::high_resolution_clock::now();

    flo::set_rand_seed(2384);

    std::vector<flo::rgb_color> palette =
        std::array<std::string, 6>{ "ef476f","f78c6b","ffd166","06d6a0","118ab2","073b4c" } |
        rv::transform(flo::hex_str_to_rgb) | r::to<std::vector>();

    flo::dimensions dim{ 1200,900};
    auto rand = flo::perlin_vector_field(dim, 4, 4.0, 0.5, true);
    //auto flow = flo::logarithmic_spiral_vector_field( dim, 0.25, false, false );
    auto flow = flo::zigzag_vector_field(dim, 150);
    //auto flow = flo::circular_vector_field(dim, flo::circle_field_type::clockwise);
    auto params = flo::flowbee_params(
        flo::brush_params{
            .radius = 8.0,
            .radius_ramp_in_time = 50.0,
            .mix = true,
            .mode = flo::paint_mode::fill,
            .aa_level = 4,
            .paint_transfer_coeff = 0.6
        },
        50
    );
    params.jitter = { 1.0, std::numbers::pi / 10.0 };

    flo::do_flowbee(
        { "D:\\test\\test_zigzag.png" },
        palette,
        flow,
        params
    );

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    // Display timing results
    std::println("    {} seconds", elapsed.count());

    return 0;
}