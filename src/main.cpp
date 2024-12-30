#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <print>
#include <ranges>
#include "image.hpp"

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

}

int main(int argc, char* argv[]) {

    auto img = flo::image(100, 100);
    for (auto [x, y] : rv::cartesian_product(rv::iota(0, 100), rv::iota(0, 100))) {
        if ((x - 50) * (x - 50) + (y - 50) * (y - 50) < 1600) {
            img[x, y] = 0xFF0000FF;
        } else {
            img[x, y] = 0xFFFFFFFF;
        }
    }

    flo::write_to_file("D:\\test\\test_img.png", img);

    return 0;
}