#pragma once

#include "types.hpp"
#include "flowbee.hpp"
#include <vector>
#include <string>
#include <optional>
#include <expected>

namespace flo {

    struct input {
        std::optional<uint32_t> rand_seed;
        output_params output;
        std::vector<rgb_color> palette;
        std::vector<layer_params> layers;
    };

    std::expected<input, std::string> parse_input(
        const std::string& inp, const std::string& outp
    );

}