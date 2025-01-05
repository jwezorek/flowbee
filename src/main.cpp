#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <print>
#include <ranges>
#include <unordered_map>
#include <format>
#include "util.hpp"
#include "paint.hpp"
#include "gui.hpp"

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

    void write_color_image(const flo::rgb_color& color, const std::string& out_file) {
        flo::image img(100, 100, flo::rgb_to_pixel(color));
        flo::write_to_file(out_file, img);
    }

    void test_mix(const std::string& inp, const std::string& out_file) {

        auto img = flo::read_from_file(inp);

        std::unordered_map<uint32_t, int> pix_to_count;
        for (auto pix : img.entries()) {
            ++pix_to_count[pix];
        }

        auto particles = pix_to_count | rv::transform(
                [](const auto& pix_count)->flo::paint_particle {
                    const auto& [pix, count] = pix_count;
                    auto rgb = flo::pixel_to_rgb(pix);
                    return {
                        rgb_to_pigment(rgb),
                        static_cast<float>(count)
                    };
                }
            ) | r::to<std::vector>();

        auto mix_color = pigment_to_rgb(
            mix_paint_particles(particles)
        );

        write_color_image(mix_color, out_file);
    }
}


int main(int argc, char* argv[]) {

    std::println("flowbee...");
    
    /*
    for (int i = 1; i <= 5; ++i) {
        auto inp = std::format("D:\\test\\mix_test\\inp\\test{}.png", i);
        auto outp = std::format("D:\\test\\mix_test\\outp\\mix{}.png", i);
        test_mix(inp, outp);
    }
    */

    float min = std::numeric_limits<float>::max();
    float max = -std::numeric_limits<float>::max();

    int n = 100000000;
    for (int i = 0; i < n; ++i) {
        auto color = flo::random_rgb_color();
        auto latent = flo::rgb_to_pigment(color);
        auto [min_val, max_val] = r::minmax(latent.impl);
        min = std::min(min, min_val);
        max = std::max(max, max_val);
    }
    std::println("[{} , ... , {}]", min, max);
    //flo::do_gui();

    return 0;
}