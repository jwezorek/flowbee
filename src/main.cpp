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
#include "canvas.hpp"
#include "gui.hpp"

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

    void write_color_image(const flo::rgb_color& color, const std::string& out_file) {
        flo::image img(100, 100, flo::rgb_to_pixel(color));
        flo::img_to_file(out_file, img);
    }

    void test_mix(const std::string& inp, const std::string& out_file) {
        /*
        auto img = flo::img_from_file(inp);

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
        */
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

    /*
    auto image = flo::img_from_file("D:\\test\\mix_test\\inp\\test2.png");
    auto canvas = flo::image_to_canvas(image, 10.0);

    flo::mix(canvas, { 50.5,50.5 }, 25.0, 4);

    flo::img_to_file("D:\\test\\test_mix_brush.png",
        flo::canvas_to_image(canvas)
    );
    */

    //flo::do_gui("D:\\test\\flower.png");

    std::vector<flo::rgb_color> colors = {
        {255,255,255},
        {0,255,0},
        {0,0,255}
    };
    flo::canvas canv(colors, 512, 512, 0, 10.0);
    flo::fill(canv, { 256,256 }, 50.0, 4, { 0,10.0,0 });
    flo::img_to_file("D:\\test\\test_refactor.png",
        flo::canvas_to_image(canv)
    );

    return 0;
}