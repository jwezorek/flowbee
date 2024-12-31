#pragma once

#include "types.hpp"
#include <string>

namespace flo {

    void write_to_file(const std::string& fname, const image& img);
}