#pragma once

#include <vector>
#include "types.hpp"
#include "pigment.hpp"
#include <unordered_map>

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    class paint_particle {
        std::vector<double> mixture_;

        friend paint_particle operator*(double k, const paint_particle& p);
        friend paint_particle& operator+=(paint_particle& paint_lhs, const paint_particle& paint_rhs);

    public:
        paint_particle(const std::vector<double>& mixture = {});
        const std::vector<double>& mixture() const;
        std::vector<double>& mixture();
        void normalize();
        double volume() const;
    };

    paint_particle operator*(double k, const paint_particle& paint);
    paint_particle& operator+=(paint_particle& lhs, const paint_particle& rhs);
    paint_particle& operator-=(paint_particle& lhs, const paint_particle& rhs);
    paint_particle operator+(const paint_particle& lhs, const paint_particle& rhs);
    paint_particle operator-(const paint_particle& lhs, const paint_particle& rhs);
    paint_particle normalize(const paint_particle& p);

    paint_particle make_one_color_paint(int palette_sz, int color_index, double volume);
    std::string display(const paint_particle& p);
}