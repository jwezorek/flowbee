#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <print>
#include <ranges>
#include "util.hpp"

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

}

int main(int argc, char* argv[]) {

    auto s = flo::scalar_field(800, 800);
    for (auto [x, y] : rv::cartesian_product(rv::iota(0, 800), rv::iota(0, 800))) {
        s[x, y] = (x - 400.0) * (x - 400) + (y - 400) * (y - 400);
    }

    flo::write_to_file("D:\\test\\test_img.png", to_gray_scale_image(s) );
    flo::write_to_file("D:\\test\\test_img2.png",
        flo::to_gray_scale_image(
            flo::white_noise(800, 800).transform_to([](double v) {return std::sqrt(v); })
        )
    );
    std::println("flowbee...");

    return 0;
}