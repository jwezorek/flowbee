#pragma once

#include <vector>
#include "types.hpp"
#include "pigment.hpp"
#include <unordered_map>

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    class paint_particle {
        double volume_;
        std::vector<double> mixture_;

        void fix_sign();

        friend paint_particle operator*(double k, const paint_particle& p);
        friend paint_particle& operator+=(paint_particle& paint_lhs, const paint_particle& paint_rhs);

    public:
        paint_particle(double volume = 0.0, const std::vector<double>& mixture = {});
        double volume() const;
        const std::vector<double>& mixture() const;
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