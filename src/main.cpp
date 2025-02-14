
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

    void test() {
        std::vector<flo::rgb_color> pal = { {255,255,255},{255,0,0} };
        flo::canvas canv(pal, 100, 100);
        flo::brush_params br_params = {
            .radius = 10.0,
            .mix = true,
            .mode = flo::paint_mode::fill,
            .aa_level = 4,
            .paint_transfer_coeff = 0.75,
        };
        flo::brush br(br_params, { 0.0,1.0 });
        br.apply(canv, { 50.0,50.0 }, { 1.0, 0 });
        br.apply(canv, { 52.0,50.0 }, { 1.0, 1.0 });

        flo::img_to_file(
            "D:\\test\\debug_flat_canv.png",
            flo::canvas_to_image(
                canv, 1.0, { 255,255,255 }
            )
        );
    }

}

int main(int argc, char* argv[]) {

    //test();

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