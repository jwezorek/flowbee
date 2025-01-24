#pragma once

#include "types.hpp"
#include "matrix.hpp"
#include "paint.hpp"

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    class canvas {
        std::vector<pigment> palette_;
        matrix_3d<double> impl_;
    public:
        canvas() {}
        canvas(const std::vector<rgb_color>& palette, int wd, int hgt);

        int cols() const;
        int rows() const;
        int layers() const;
        dimensions bounds() const;
    };

    inline auto brush_region(const dimensions& canvas_dimensions,
            const point& brush_loc,
            double brush_radius,
            int anti_aliasing_level) {

        const int resolution = (1 << anti_aliasing_level); // 2^level subdivisions
        const double subcell_size = 1.0 / resolution;
        const double subcell_area = subcell_size * subcell_size;
        const double radius_squared = brush_radius * brush_radius;

        // Determine the bounding box for the circle on the canvas
        int min_x = static_cast<int>(std::floor(brush_loc.x - brush_radius));
        int max_x = static_cast<int>(std::ceil(brush_loc.x + brush_radius));
        int min_y = static_cast<int>(std::floor(brush_loc.y - brush_radius));
        int max_y = static_cast<int>(std::ceil(brush_loc.y + brush_radius));

        // Clamp bounds to canvas dimensions
        min_x = std::max(min_x, 0);
        max_x = std::min(max_x, canvas_dimensions.wd - 1);
        min_y = std::max(min_y, 0);
        max_y = std::min(max_y, canvas_dimensions.hgt - 1);

        // Create a range for all cells in the bounding box
        auto cell_range = std::views::cartesian_product(
            std::views::iota(min_y, max_y + 1),
            std::views::iota(min_x, max_x + 1)
        );

        // Transform each cell to calculate its weight
        return cell_range | std::views::transform(
                [=](const auto& cell) -> std::pair<coords, double> {
                    auto [y, x] = cell;
                    double total_weight = 0.0;

                    // Subdivide the cell into smaller cells for anti-aliasing
                    for (int sy = 0; sy < resolution; ++sy) {
                        for (int sx = 0; sx < resolution; ++sx) {
                            // Compute the center of the subcell
                            double sub_x = x + (sx + 0.5) * subcell_size;
                            double sub_y = y + (sy + 0.5) * subcell_size;

                            // Check if the subcell center is within the circle
                            double dx = sub_x - brush_loc.x;
                            double dy = sub_y - brush_loc.y;
                            if ((dx * dx + dy * dy) <= radius_squared) {
                                total_weight += subcell_area;
                            }
                        }
                    }

                    return { {x, y}, total_weight };
                }
            ) | std::views::filter(
                [](const std::pair<coords, double>& cell_data) {
                    return cell_data.second > 0.0;
                }
            );
    }

    double brush_region_area(const dimensions& canvas_dimensions,
        const point& brush_loc,
        double brush_radius,
        int anti_aliasing_level);

    void fill(canvas& canv, const point& loc, double radius, int aa_level,
        const paint& paint);
    void overlay(canvas& canv, const point& loc, double radius, int aa_level,
        const paint& paint);
    void mix(canvas& canv, const point& loc, double radius, int aa_level);

    canvas image_to_canvas(const image& img, int n, double vol_per_pixel = 1.0);
    image canvas_to_image(const canvas& canv);
}