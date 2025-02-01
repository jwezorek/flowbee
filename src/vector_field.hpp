#pragma once 

#include "types.hpp"

namespace flo {

    struct vector_field {
        scalar_field x;
        scalar_field y;
    };

    vector_field perlin_vector_field(const flo::dimensions& sz, uint32_t seed1, uint32_t seed2, int octaves, double freq,
        bool normalized);
    vector_field vector_field_from_scalar_fields(const scalar_field& x, const scalar_field& y);
    vector_field normalize(const vector_field& vf);
    point vector_from_field(const vector_field& vf, const point& pt);

    vector_field operator*(const point& v, const vector_field& field);
    vector_field operator*(double k, const vector_field& field);
    vector_field operator+(const point& v, const vector_field& field);
    vector_field operator+(double k, const vector_field& field);
    scalar_field pow(const scalar_field& field, double exp);
}