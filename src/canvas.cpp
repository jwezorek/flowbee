#include "canvas.hpp"
#include "brush.hpp"
#include "util.hpp"
#include <ranges>
#include <functional>
#include <numeric>
#include <print>

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {
    flo::pigment to_pigment(const flo::rgb_color& col) {
        return rgb_to_pigment(col);
    }
    flo::paint brush_region_sum(flo::canvas& canvas, const flo::point& loc, double radius, int aa_level) {
        flo::paint sum = flo::create_paint(canvas.palette_size());
        auto& canv = canvas.cells();
        for (const auto& [loc, paint_pcnt] : brush_region(canv.bounds(), loc, radius, aa_level)) {
            sum += paint_pcnt * canv[loc];
        }
        return sum;
    }
}

flo::canvas::canvas(const std::vector<rgb_color>& palette, int wd, int hgt) :
    palette_{
        palette | rv::transform( to_pigment ) | r::to<std::vector>()
    },
    impl_{
        wd, hgt, static_cast<int>(palette.size()), 0.0
    }
{
}

flo::canvas::canvas( const std::vector<rgb_color>& palette, int wd, int hgt, int bkgd, double amnt) :
        canvas(palette, wd, hgt) {
    for (int y = 0; y < hgt; ++y) {
        for (int x = 0; x < wd; ++x) {
            impl_[x, y, bkgd] = amnt;
        }
    }
}

flo::matrix_3d<double>& flo::canvas::cells() {
    return impl_;
}

const flo::matrix_3d<double>& flo::canvas::cells() const {
    return impl_;
}

int flo::canvas::cols() const {
    return impl_.cols();
}

int flo::canvas::rows() const {
    return  impl_.rows();
}

int flo::canvas::layers() const {
    return  impl_.layers();
}

flo::dimensions flo::canvas::bounds() const {
    return impl_.bounds();
}

flo::pigment flo::canvas::color_at(int x, int y) const {
    pigment_map<double> color_to_weight;

    for (auto [i, pigment] : rv::enumerate(palette_)) {
        color_to_weight[pigment] = impl_[x, y, static_cast<int>(i)];
    }

    return mix_paint(color_to_weight);
}

int flo::canvas::palette_size() const {
    return static_cast<int>(palette_.size());
}

double flo::brush_region_area(const dimensions& dim, const point& loc, double rad, int aa) {
    return r::fold_left(
        flo::brush_region(dim, loc, rad, aa) | rv::values,
        0.0,
        std::plus<double>()
    );
}

void flo::fill(canvas& canvas, const point& loc, double radius, int aa_level, const paint& paint) {
    auto& canv = canvas.cells();
    for (const auto& [loc, paint_pcnt] : brush_region(canv.bounds(), loc, radius, aa_level)) {
        canv[loc] = (1.0 - paint_pcnt) * canv[loc] + paint_pcnt * paint;
    }
}

void flo::overlay(canvas& canvas, const point& loc, double radius, int aa_level, const paint& paint) {
    auto& canv = canvas.cells();
    for (const auto& [loc, paint_pcnt] : brush_region(canv.bounds(), loc, radius, aa_level)) {
        canv[loc] += paint_pcnt * paint;
    }
}

void flo::mix(canvas& canv, const point& loc, double radius, int aa_level) {
    auto area = brush_region_area(canv.bounds(), loc, radius, aa_level);
    auto paint_sum = brush_region_sum(canv, loc, radius, aa_level);
    auto mean_color = (1.0 / area) * paint_sum;
    fill(canv, loc, radius, aa_level, mean_color);
}

flo::canvas flo::image_to_canvas(const image& img, int n, double vol_per_pixel)
{
    return canvas();
}

flo::image flo::canvas_to_image(const canvas& canv)
{
    flo::image img(canv.bounds());
    for (auto [x,y] : locations(img.bounds())) {
        img[x, y] = rgb_to_pixel(pigment_to_rgb(canv.color_at(x, y)));
    }
    return img;
}
