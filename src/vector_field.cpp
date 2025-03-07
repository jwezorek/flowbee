#include "vector_field.hpp"
#include "util.hpp"
#include <numbers>

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

    flo::scalar_field signed_pow(const flo::scalar_field& field, double exp) {
        auto power = [exp](double v) {
            return flo::sgn(v) * std::pow(std::abs(v), exp);
            };
        return field.transform_to(power);
    }

    flo::matrix<double> gaussian_neighborhood(int sz) {
        if (sz % 2 == 0) {
            throw std::invalid_argument("Size must be odd");
        }

        flo::matrix<double> kernel(sz, sz);
        double sigma = sz / 3.0; // Standard deviation
        int half_sz = sz / 2;
        double sum = 0.0;

        // Compute Gaussian function values
        for (int y = 0; y < sz; ++y) {
            for (int x = 0; x < sz; ++x) {
                if (x == half_sz && y == half_sz) {
                    kernel[x, y] = 0.0; // Ensure center is zero
                }
                else {
                    double dx = x - half_sz;
                    double dy = y - half_sz;
                    kernel[x, y] = std::exp(-(dx * dx + dy * dy) / (2 * sigma * sigma));
                    sum += kernel[x, y];
                }
            }
        }

        // Normalize the kernel so the sum is 1.0
        if (sum != 0.0) {
            for (int y = 0; y < sz; ++y) {
                for (int x = 0; x < sz; ++x) {
                    kernel[x, y] /= sum;
                }
            }
        }

        return kernel;
    }
    /*
    flo::point tangent_of_loxodromic_spiral(
            bool outward,
            double x, double y, double dist, double width,
            double height, double theta_rate) {

        using namespace flo;

        // Calculate the spiral centers
        point center1 = { width / 2.0 - dist / 2.0, height / 2.0 };
        point center2 = { width / 2.0 + dist / 2.0, height / 2.0 };

        // Compute distances to each spiral center
        double r1 = std::sqrt((x - center1.x) * (x - center1.x) + (y - center1.y) * (y - center1.y)) + 1e-6;
        double r2 = std::sqrt((x - center2.x) * (x - center2.x) + (y - center2.y) * (y - center2.y)) + 1e-6;

        // Compute the gradient of the scalar potential field with scaling factor rho
        double dx = (((x - center2.x) / (r2 * r2)) - ((x - center1.x) / (r1 * r1)));
        double dy = (((y - center2.y) / (r2 * r2)) - ((y - center1.y) / (r1 * r1)));

        // Compute the tangent vector by rotating the gradient by 90 degrees
        double tangent_x = -dy;
        double tangent_y = dx;

        // Compute separate angles for each spiral and blend them
        double angle1 = theta_rate * std::log(r1);
        double angle2 = theta_rate * std::log(r2);

        // Blend the angles based on inverse distance weighting
        double weight1 = 1.0 / r1;
        double weight2 = 1.0 / r2;
        double angle = (weight1 * angle1 + weight2 * angle2) / (weight1 + weight2);

        if (outward) {
            angle += std::numbers::pi;
        }

        // Apply rotation
        double rotated_x = tangent_x * std::cos(angle) - tangent_y * std::sin(angle);
        double rotated_y = tangent_x * std::sin(angle) + tangent_y * std::cos(angle);

        // Normalize the tangent vector
        double magnitude = std::sqrt(rotated_x * rotated_x + rotated_y * rotated_y);
        double normalized_x = rotated_x / magnitude;
        double normalized_y = rotated_y / magnitude;

        // Return the normalized tangent vector field as point
        return point{ normalized_x, normalized_y };
    }
    */

    flo::point tangent_of_loxodromic_spiral(
            bool outward,
            double x, double y, double dist, double width,
            double height, double theta_rate) {

        using namespace flo;

        theta_rate = 1.0 / theta_rate;

        // Define the spiral centers
        point center1 = { width / 2.0 - dist / 2.0, height / 2.0 }; // Left center
        point center2 = { width / 2.0 + dist / 2.0, height / 2.0 }; // Right center

        // Compute polar coordinates relative to each center
        double dx1 = x - center1.x, dy1 = y - center1.y;
        double r1 = std::sqrt(dx1 * dx1 + dy1 * dy1) + 1e-6; // Avoid div by zero
        double theta1 = std::atan2(dy1, dx1);

        double dx2 = x - center2.x, dy2 = y - center2.y;
        double r2 = std::sqrt(dx2 * dx2 + dy2 * dy2) + 1e-6;
        double theta2 = std::atan2(dy2, dx2);

        // Determine spiral directions
        double direction1 = outward ? 1.0 : -1.0;  // Left spiral follows 'outward'
        double direction2 = -direction1;          // Right spiral is the opposite

        // Compute tangent vectors for logarithmic spirals
        double tangent_x1 = direction1 * (-std::sin(theta1) + theta_rate * std::cos(theta1));
        double tangent_y1 = direction1 * (std::cos(theta1) + theta_rate * std::sin(theta1));

        double tangent_x2 = direction2 * (-std::sin(theta2) + theta_rate * std::cos(theta2));
        double tangent_y2 = direction2 * (std::cos(theta2) + theta_rate * std::sin(theta2));

        // Weight the influence of each spiral inversely by distance
        double weight1 = 1.0 / (r1 + 1e-6);
        double weight2 = 1.0 / (r2 + 1e-6);
        double total_weight = weight1 + weight2;

        // Blend the tangent vectors
        double tangent_x = (tangent_x1 * weight1 + tangent_x2 * weight2) / total_weight;
        double tangent_y = (tangent_y1 * weight1 + tangent_y2 * weight2) / total_weight;

        // Normalize the final tangent vector
        double magnitude = std::sqrt(tangent_x * tangent_x + tangent_y * tangent_y);
        return point{ tangent_x / magnitude, tangent_y / magnitude };
    }

    flo::point logarithmic_spiral_vector(const flo::dimensions& dim, double x, double y, double b, bool inward, bool clockwise) {
        // Compute the distance from the center
        double center_x = dim.wd / 2.0;
        double center_y = dim.hgt / 2.0;
        double dx = x - center_x;
        double dy = y - center_y;
        double r2 = dx * dx + dy * dy;

        // Avoid division by zero
        if (r2 < 1e-6) {
            return { 0.0, 0.0 };
        }

        // Compute the original gradient components
        double grad_x = (dx - (dy / b)) / r2;
        double grad_y = (dy + (dx / b)) / r2;

        // Normalize the vector
        double magnitude = std::sqrt(grad_x * grad_x + grad_y * grad_y);
        grad_x /= magnitude;
        grad_y /= magnitude;

        // Adjust for inward/outward direction
        if (inward) {
            grad_x = -grad_x;
            grad_y = -grad_y;
        }

        // Adjust for clockwise/counterclockwise rotation
        if (clockwise) {
            double temp = grad_x;
            grad_x = grad_y;
            grad_y = -temp;
        }

        return { grad_x, grad_y };
    }


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

    flo::point circular_vector(double x, double y, 
            const flo::point& center, flo::circle_field_type type) {
        auto outward_x = x - center.x;
        auto outward_y = y - center.y;
        auto hypot = std::hypot(outward_x, outward_y);
        auto outward = (1.0 / hypot) * flo::point{ outward_x, outward_y };
        flo::point vec;
        switch (type) {
            case flo::circle_field_type::outward:
                vec = outward;
                break;
            case flo::circle_field_type::inward:
                vec = -1.0 * outward;
                break;
            case flo::circle_field_type::clockwise:
                vec = rotate_90(outward, false);
                break;
            case flo::circle_field_type::counterclockwise:
                vec = rotate_90(outward, true);
                break;
        }
        return vec;
    }
}

flo::vector_field flo::perlin_vector_field(
        const flo::dimensions& sz, int octaves, double freq,
        double exponent, bool normalized) {

    auto x_noise = perlin_noise(sz, octaves, freq);
    auto y_noise = perlin_noise(sz, octaves, freq);

    auto x_comp = 2.0 * x_noise - 1.0;
    auto y_comp = 2.0 * y_noise - 1.0;

    if (exponent != 1.0) {
        x_noise = signed_pow(x_noise, exponent);
        y_noise = signed_pow(y_noise, exponent);
    }

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
    auto center = point{
        static_cast<double>(dim.wd) / 2.0,
        static_cast<double>(dim.hgt) / 2.0
    };

    for (auto [x, y] : locations(dim)) {
        auto vec = circular_vector(x, y, center, type);
        x_comp[x, y] = vec.x;
        y_comp[x, y] = vec.y;
    }

    return { x_comp, y_comp  };
}

flo::vector_field flo::elliptic_vector_field(const dimensions& dim, circle_field_type type)
{
    scalar_field x_comp(dim);
    scalar_field y_comp(dim);
    auto o_x = static_cast<double>(dim.wd) / 2.0;
    auto o_y = static_cast<double>(dim.hgt) / 2.0;

    // Define the axes of the ellipse
    double a = o_x;  // Semi-major axis (along the x-direction)
    double b = o_y;  // Semi-minor axis (along the y-direction)

    for (auto [x, y] : locations(dim)) {
        auto outward_x = x - o_x;
        auto outward_y = y - o_y;

        // Apply the ellipse scaling factors (a for x-axis, b for y-axis)
        auto scale_x = outward_x / a;
        auto scale_y = outward_y / b;

        auto hypot = std::hypot(scale_x, scale_y);  // Hypotenuse in the scaled ellipse
        auto outward = (1.0 / hypot) * flo::point{ scale_x, scale_y };
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

flo::vector_field flo::loxodromic_spiral_vector_field(
        const dimensions& dim, bool outward, double centers_dist, double theta_rate) {
    auto xx = flo::scalar_field(dim);
    auto yy = flo::scalar_field(dim);
    for (auto [x, y] : flo::locations(dim)) {
        auto vec2 = tangent_of_loxodromic_spiral(
            outward, x, y, centers_dist, dim.wd, dim.hgt, theta_rate
        );
        xx[x, y] = vec2.x;
        yy[x, y] = vec2.y;
    }
    return flo::vector_field(xx, yy);
}

flo::vector_field flo::logarithmic_spiral_vector_field(
        const dimensions& dim, double b, bool inward, bool clockwise) {
    auto xx = flo::scalar_field(dim);
    auto yy = flo::scalar_field(dim);
    for (auto [x, y] : flo::locations(dim)) {
        auto vec2 = logarithmic_spiral_vector(dim, x, y, b, inward, clockwise);
        xx[x, y] = vec2.x;
        yy[x, y] = vec2.y;
    }
    return flo::vector_field(xx, yy);
}

flo::vector_field flo::zigzag_vector_field(const flo::dimensions& dim, double radius) {

    flo::scalar_field x_comp(dim);
    flo::scalar_field y_comp(dim);

    for (auto [x, y] : flo::locations(dim)) {
        int row = y / static_cast<int>(radius);
        bool rightward = (row % 2 == 0);
        double left_boundary = radius;
        double right_boundary = dim.wd - radius;

        if (x > left_boundary && x < right_boundary) {
            x_comp[x, y] = rightward ? 1.0 : -1.0;
            y_comp[x, y] = 0.0;
            continue;
        }

        circle_field_type orientation;
        int center_row;
        double cen_x;
        if (x >= right_boundary) {
            center_row = row % 2 == 0 ? row + 1 : row;
            cen_x = right_boundary,
            orientation = flo::circle_field_type::clockwise;
        } else {
            center_row = row % 2 == 0 ? row : row + 1;
            cen_x = left_boundary;
            orientation = flo::circle_field_type::counterclockwise;
        }
        auto vec = circular_vector(x, y, { cen_x, center_row * radius }, orientation);
        x_comp[x, y] = vec.x;
        y_comp[x, y] = vec.y;
    }

    return { x_comp, y_comp };
}

flo::vector_field flo::gradient(const scalar_field& img, int kernel_sz, bool hamiltonian) {
    if (kernel_sz % 2 == 0) {
        throw std::invalid_argument("Kernel size must be odd");
    }

    int half_sz = kernel_sz / 2;
    auto kernel = gaussian_neighborhood(kernel_sz);
    vector_field grad{ scalar_field(img.cols(), img.rows(), 0.0), scalar_field(img.cols(), img.rows(), 0.0) };

    for (int y = 0; y < img.rows(); ++y) {
        for (int x = 0; x < img.cols(); ++x) {
            double grad_x = 0.0, grad_y = 0.0;
            double center_val = img[x, y];

            for (int ky = -half_sz; ky <= half_sz; ++ky) {
                for (int kx = -half_sz; kx <= half_sz; ++kx) {
                    int nx = x + kx;
                    int ny = y + ky;
                    double neighbor_val = 0.0;

                    if (nx >= 0 && nx < img.cols() && ny >= 0 && ny < img.rows()) {
                        neighbor_val = img[nx, ny];
                    }

                    double weight = kernel[kx + half_sz, ky + half_sz];
                    grad_x += weight * (neighbor_val - center_val) * kx;
                    grad_y += weight * (neighbor_val - center_val) * ky;
                }
            }

            // Normalize gradient to range [-1, 1]
            double magnitude = std::hypot(grad_x, grad_y);
            if (magnitude > 0) {
                grad_x /= magnitude;
                grad_y /= magnitude;
            }
            else {
                grad_x = 0.0;
                grad_y = 0.0;
            }

            if (hamiltonian) {
                grad.x[x, y] = -grad_y;
                grad.y[x, y] = grad_x;
            }
            else {
                grad.x[x, y] = grad_x;
                grad.y[x, y] = grad_y;
            }
        }
    }

    return grad;
}

flo::vector_field flo::gravity(const dimensions& dim, const std::vector<point_mass>& masses, double grav_const, bool normalize) {
    vector_field field{ 
        scalar_field(dim.wd, dim.hgt, 0.0), 
        scalar_field(dim.wd, dim.hgt, 0.0) 
    };

    for (auto [x,y] : locations(dim)) {
        double field_x = 0.0, field_y = 0.0;

        for (const auto& mass : masses) {
            double dx = mass.loc.x - x;
            double dy = mass.loc.y - y;
            double dist_sq = dx * dx + dy * dy;
            double dist = std::sqrt(dist_sq);

            if (dist_sq > 1e-6) { // Avoid singularity
                double force = grav_const * mass.mass / dist_sq;
                field_x += force * (dx / dist);
                field_y += force * (dy / dist);
            }
        }

        if (normalize) {
            double magnitude = std::hypot(field_x, field_y);
            if (magnitude > 1e-6) {
                field_x /= magnitude;
                field_y /= magnitude;
            }
            else {
                field_x = 0.0;
                field_y = 0.0;
            }
        }

        field.x[x, y] = field_x;
        field.y[x, y] = field_y;
    }

    return field;
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


