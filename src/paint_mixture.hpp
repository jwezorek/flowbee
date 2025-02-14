#pragma once

#include <vector>
#include "types.hpp"
#include "pigment.hpp"
#include <unordered_map>

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    using paint_mixture = std::vector<double>;

    paint_mixture operator*(double k, const paint_mixture& paint);
    paint_mixture& operator+=(paint_mixture& lhs, const paint_mixture& rhs);
    paint_mixture& operator-=(paint_mixture& lhs, const paint_mixture& rhs);
    paint_mixture operator+(const paint_mixture& lhs, const paint_mixture& rhs);
    paint_mixture operator-(const paint_mixture& lhs, const paint_mixture& rhs);

    double volume(const paint_mixture& p);
    void normalize_in_place(paint_mixture& p);
    paint_mixture normalize(const paint_mixture& p);

    paint_mixture make_one_color_paint(int palette_sz, int color_index, double volume);
    std::string display(const paint_mixture& p);
}