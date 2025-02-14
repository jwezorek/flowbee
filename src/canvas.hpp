#pragma once

#include "types.hpp"
#include "pigment.hpp"
#include "matrix_3d.hpp"
#include "paint_mixture.hpp"

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    class canvas {
        std::vector<pigment> palette_;
        matrix_3d<double> impl_;
    public:
        canvas() {}
        canvas(const std::vector<rgb_color>& palette, int wd, int hgt);
        canvas(const std::vector<rgb_color>& palette, const dimensions& dim);
        canvas(const std::vector<rgb_color>& palette, int wd, int hgt, int bkgd, double amnt);

        matrix_3d<double>& cells();
        const matrix_3d<double>& cells() const;

        int cols() const;
        int rows() const;
        int layers() const;
        dimensions bounds() const;

        pigment color_at(int x, int y) const;
        int palette_size() const;
        int num_blank_locs() const;
        std::vector<coords> blank_locs() const;
        double volume_at(int x, int y) const;
    };

    double brush_region_area(const dimensions& canvas_dimensions,
        const point& brush_loc,
        double brush_radius,
        int anti_aliasing_level);

    paint_mixture all_paint_in_brush_region(flo::canvas& canvas, const flo::point& loc, double radius, int aa_level);

    void fill(canvas& canv, const point& loc, double radius, int aa_level,
        const paint_mixture& paint);
    void overlay(canvas& canv, const point& loc, double radius, int aa_level,
        const paint_mixture& paint);
    void mix(canvas& canv, const point& loc, double radius, int aa_level);

    canvas image_to_canvas(const image& img, const std::vector<rgb_color>& palette, double vol_per_pixel = 1.0);
    canvas image_to_canvas(const image& img, int n, double vol_per_pixel = 1.0);

    image canvas_to_image(const canvas& canv, double alpha_threshold, 
        const rgb_color& canvas_color = {255,255,255});

}