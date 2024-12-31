#pragma once

#include "matrix.hpp"
#include <string>

namespace flo {

    using image = matrix<uint32_t>;

    void write_to_file(const std::string& fname, const image& img);
}