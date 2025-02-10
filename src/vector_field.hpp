#pragma once 

#include "types.hpp"

namespace flo {

    struct vector_field {
        scalar_field x;
        scalar_field y;
    };

    vector_field perlin_vector_field(const flo::dimensions& sz, int octaves, double freq,
        double exponent = 1.0, bool normalized = true);
    vector_field vector_field_from_scalar_fields(const scalar_field& x, const scalar_field& y);
    vector_field normalize(const vector_field& vf);
    point vector_from_field(const vector_field& vf, const point& pt);

    enum class circle_field_type {
        inward,
        outward,
        clockwise,
        counterclockwise
    };

    vector_field circular_vector_field(const dimensions& dim, circle_field_type type);
    vector_field elliptic_vector_field(const dimensions& dim, circle_field_type type);
    vector_field loxodromic_spiral_vector_field(const dimensions& dim,
        bool outward, double centers_dist, double theta_rate);
    vector_field logarithmic_spiral_vector_field(
        const dimensions& dim, double b, bool inward, bool clockwise);
    vector_field zigzag_vector_field(const dimensions& dim, double radius);
    vector_field gradient(const scalar_field& img, int kernel_sz, bool hamiltonian);
    vector_field operator*(const point& v, const vector_field& field);
    vector_field operator*(double k, const vector_field& field);
    vector_field operator+(const point& v, const vector_field& field);
    vector_field operator+(double k, const vector_field& field);
    vector_field operator+(const vector_field& lhs, const vector_field& rhs);
    scalar_field pow(const scalar_field& field, double exp);
}