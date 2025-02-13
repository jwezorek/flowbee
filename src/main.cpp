
#include "util.hpp"
#include "vector_field.hpp"
#include "paint_particle.hpp"
#include "canvas.hpp"
#include "brush.hpp"
#include "flowbee.hpp"
#include "input.hpp"
#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <print>
#include <ranges>
#include <unordered_map>
#include <format>
#include <deque>
#include <numbers>
#include <chrono>

/*------------------------------------------------------------------------------------------------*/
namespace {

    std::string filename(const std::string& str) {
        return std::filesystem::path(str).filename().string();
    }

}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        for (int i = 0; i < argc; ++i) {
            std::println("{} ", argv[i]);
        }
        std::println(" usage is 'flowbee.exe params.json output_image.png'");
        return -1;
    }

    flo::display_title();

    auto input = flo::parse_input( argv[1], argv[2] );
    if (!input) {
        std::println("[error] {}", input.error());
        return -1;
    }

    std::println("  processing '{}'...\n", filename(argv[1]));

    auto start_time = std::chrono::high_resolution_clock::now();

    flo::do_flowbee(
        input->output,
        input->palette,
        input->layers
    );

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::println("    {} seconds\n", elapsed.count());
    std::println("  generated '{}'.", filename(input->output.filename));

    return 0;
}