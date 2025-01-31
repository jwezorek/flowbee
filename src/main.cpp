#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <print>
#include <ranges>
#include <unordered_map>
#include <format>
#include "util.hpp"
#include "paint_particle.hpp"
#include "canvas.hpp"
#include "brush.hpp"
#include "gui.hpp"

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

    void write_color_image(const flo::rgb_color& color, const std::string& out_file) {
        flo::image img(100, 100, flo::rgb_to_pixel(color));
        flo::img_to_file(out_file, img);
    }

    void test_mix(const std::string& inp, int n, const std::string& out_file) {
        auto img = flo::img_from_file(inp);
        auto dim = img.bounds();

        double radius = 3.0 * dim.wd / 8.0;
        double x = dim.wd / 2.0;
        double y = dim.hgt / 2.0;

        auto canvas = flo::image_to_canvas(img, n);
        flo::mix(canvas, { x,y }, radius, 4);

        flo::img_to_file( out_file, flo::canvas_to_image(canvas) );
    }
}


int main(int argc, char* argv[]) {

    std::println("flowbee...");

    /*
    std::vector<int> colors = { 3,4,3,5,3 };
    for (int i = 1; i <= 5; ++i) {
        auto inp = std::format("D:\\test\\mix_test\\inp\\test{}.png", i);
        auto outp = std::format("D:\\test\\mix_test\\outp\\mix{}.png", i);
        test_mix(inp, colors[i-1], outp);
    }
    */

    //flo::do_gui("D:\\test\\test.png", 6);
    //flo::do_gui("D:\\test\\mix_test\\inp\\test4.png", 5);

    std::vector<flo::rgb_color> pal = { {255,255,255}, {255,255,0}, {255,0,0} };
    auto canv = flo::canvas(pal, 200, 200, 0, 1.0);
    auto& mat = canv.cells();
    for (int y = 0; y < 100; ++y) {
        for (int x = 0; x < 100; ++x) {
            mat[x, y] = flo::make_one_color_paint(3, 1, 1.0);
        }
    }
    auto brush = flo::create_simple_brush(flo::make_one_color_paint(3, 2, 2000.0), 5.0, 0.002, 0.01);
    flo::do_gui(canv, brush);

    return 0;
}