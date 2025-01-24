#include "canvas.hpp"
#include "brush.hpp"
#include "util.hpp"
#include <ranges>
#include <functional>
#include <numeric>

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {
    flo::pigment to_pigment(const flo::rgb_color& col) {
        return rgb_to_pigment(col);
    }
}

double flo::brush_region_area(const dimensions& dim, const point& loc, double rad, int aa)
{
    return r::fold_left(
        flo::brush_region(dim, loc, rad, aa) | rv::values,
        0.0,
        std::plus<double>()
    );
}

void flo::fill(canvas& canv, const point& loc, double radius, int aa_level, const paint& paint)
{
}

void flo::overlay(canvas& canv, const point& loc, double radius, int aa_level, const paint& paint)
{
}

void flo::mix(canvas& canv, const point& loc, double radius, int aa_level)
{
}

flo::canvas flo::image_to_canvas(const image& img, int n, double vol_per_pixel)
{
    return canvas();
}

flo::image flo::canvas_to_image(const canvas& canv)
{
    return image();
}

flo::canvas::canvas(const std::vector<rgb_color>& palette, int wd, int hgt) :
    palette_{
        palette | rv::transform( to_pigment ) | r::to<std::vector>()
    },
    impl_{
        wd, hgt, static_cast<int>(palette.size())
    }
{
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
