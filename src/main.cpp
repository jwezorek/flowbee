#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <print>
#include <ranges>
#include <unordered_map>
#include "util.hpp"
#include "paint.hpp"

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

    void write_color_image(const flo::rgb_color& color, const std::string& out_file) {
        flo::image img(100, 100, flo::rgb_to_pixel(color));
        flo::write_to_file(out_file, img);
    }

    void test_mix(const flo::image& img, const std::string& out_file) {
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

    test_mix(flo::read_from_file("D:\\test\\mix_test\\test1.png"), "D:\\test\\mix1.png");

    return 0;
}