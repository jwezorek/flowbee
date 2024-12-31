#pragma once

#include "matrix.hpp"

namespace flo {

    using scalar_field = matrix<float>;

    struct vector_field {
        scalar_field x;
        scalar_field y;
    };

    using image = matrix<uint32_t>;

}