#include "vector_field.hpp"
#include "util.hpp"

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

    flo::vector_field normalized_vector_field(
        const flo::scalar_field& x_field, const flo::scalar_field& y_field) {

        auto x_comp = x_field;
        auto y_comp = y_field;

        for (auto [x, y] : flo::locations(x_field.bounds())) {
            auto horz = x_field[x, y];
            auto vert = y_field[x, y];
            auto hypot = std::hypot(horz, vert);
            x_comp[x, y] /= hypot;
            y_comp[x, y] /= hypot;
        }

        return { x_comp, y_comp };
    }

    flo::point rotate_90(const flo::point& p, bool clockwise) {
        if (clockwise) {
            return { p.y, -p.x };  // 90° clockwise
        }
        else {
            return { -p.y, p.x };  // 90° counterclockwise
        }
    }
}

flo::vector_field flo::perlin_vector_field(
    const flo::dimensions& sz, uint32_t seed1, uint32_t seed2,
    int octaves, double freq, bool normalized) {

    auto x_comp = 2.0 * perlin_noise(sz, seed1, octaves, freq) - 1.0;
    auto y_comp = 2.0 * perlin_noise(sz, seed2, octaves, freq) - 1.0;

    if (normalized) {
        return normalized_vector_field(x_comp, y_comp);
    }

    return { x_comp, y_comp };
}

flo::vector_field flo::vector_field_from_scalar_fields(
        const scalar_field& x, const scalar_field& y){

    auto x_comp = 2.0 * x - 1.0;
    auto y_comp = 2.0 * y - 1.0;

    return normalize(
        vector_field(x_comp, y_comp)
    );
}

flo::vector_field flo::normalize(const vector_field& vf) {
    return normalized_vector_field(vf.x, vf.y);
}

flo::point flo::vector_from_field(const vector_field& vf, const point& pt) {
    int x0 = static_cast<int>(std::floor(pt.x));
    int y0 = static_cast<int>(std::floor(pt.y));
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // Ensure bounds are valid
    int max_x = vf.x.cols() - 1;
    int max_y = vf.x.rows() - 1;

    x0 = std::clamp(x0, 0, max_x);
    y0 = std::clamp(y0, 0, max_y);
    x1 = std::clamp(x1, 0, max_x);
    y1 = std::clamp(y1, 0, max_y);

    // Bilinear interpolation weights
    double tx = pt.x - x0;
    double ty = pt.y - y0;

    // Fetch the four neighboring vectors
    double q11_x = vf.x[x0, y0];
    double q11_y = vf.y[x0, y0];
    double q21_x = vf.x[x1, y0];
    double q21_y = vf.y[x1, y0];
    double q12_x = vf.x[x0, y1];
    double q12_y = vf.y[x0, y1];
    double q22_x = vf.x[x1, y1];
    double q22_y = vf.y[x1, y1];

    // Interpolate in x direction
    double r1_x = (1 - tx) * q11_x + tx * q21_x;
    double r1_y = (1 - tx) * q11_y + tx * q21_y;
    double r2_x = (1 - tx) * q12_x + tx * q22_x;
    double r2_y = (1 - tx) * q12_y + tx * q22_y;

    // Interpolate in y direction
    double interpolated_x = (1 - ty) * r1_x + ty * r2_x;
    double interpolated_y = (1 - ty) * r1_y + ty * r2_y;

    return point{ interpolated_x, interpolated_y };
}

flo::vector_field flo::circular_vector_field(const dimensions& dim, circle_field_type type) {
    scalar_field x_comp(dim);
    scalar_field y_comp(dim);
    auto o_x = static_cast<double>(dim.wd) / 2.0;
    auto o_y = static_cast<double>(dim.hgt) / 2.0;

    for (auto [x, y] : locations(dim)) {
        auto outward_x = x - o_x;
        auto outward_y = y - o_y;
        auto hypot = std::hypot(outward_x, outward_y);
        auto outward = (1.0 / hypot) * flo::point{ outward_x, outward_y };
        point vec;
        switch (type) {
            case circle_field_type::outward:
                vec = outward;
                break;
            case circle_field_type::inward:
                vec = -1.0 * outward;
                break;
            case circle_field_type::clockwise:
                vec = rotate_90(outward, true);
                break;
            case circle_field_type::counterclockwise:
                vec = rotate_90(outward, false);
                break;
        }
        x_comp[x, y] = vec.x;
        y_comp[x, y] = vec.y;
    }

    return {
        x_comp,
        y_comp
    };
}

flo::vector_field flo::operator*(const point& v, const vector_field& field) {
    return {
        v.x * field.x,
        v.y * field.y
    };
}

flo::vector_field flo::operator*(double k, const flo::vector_field& field) {
    return {
        k * field.x,
        k * field.y
    };
}
flo::vector_field flo::operator+(const point& v, const flo::vector_field& field) {
    auto make_adder = [](double k) {
        return [k](double val) { return val + k; };
    };
    return {
        field.x.transform_to(make_adder(v.x)),
        field.y.transform_to(make_adder(v.y))
    };
}
flo::vector_field flo::operator+(double k, const flo::vector_field& field) {
    auto adder = [k](double v) {
        return v + k;
    };
    return {
        field.x.transform_to(adder),
        field.y.transform_to(adder)
    };
}

flo::vector_field flo::operator+(const vector_field& lhs, const vector_field& rhs) {
    return {
        lhs.x + rhs.x,
        lhs.y + rhs.y
    };
}

flo::scalar_field flo::pow(const scalar_field& field, double exp) {
    auto power = [exp](double v) {
        return std::pow(v, exp);
    };
    return field.transform_to(power);
}
