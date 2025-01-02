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

    std::println("flowbee...");

    auto flow = flo::perlin_vector_field({ 800,800 }, 12345, 54321, 8, 8.0);
    auto density = flo::white_noise(800, 800);

    return 0;
}