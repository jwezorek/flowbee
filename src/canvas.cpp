#include "canvas.hpp"
#include "brush.hpp"
#include "util.hpp"
#include <ranges>
#include <algorithm>
#include <functional>
#include <numeric>
#include <print>
#include <unordered_map>

namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {
    flo::pigment to_pigment(const flo::rgb_color& col) {
        return rgb_to_pigment(col);
    }

    std::vector<uint32_t> top_n_colors(const flo::image& img, int n) {
        std::unordered_map<uint32_t, int> pixel_count;
        for (auto pixel : img.entries()) {
            pixel_count[pixel]++;
        }
        auto pixels = pixel_count | rv::keys | r::to<std::vector>();
        r::sort(pixels,
            [&](auto&& lhs, auto&& rhs)->bool {
                return pixel_count[lhs] > pixel_count[rhs];
            }
        );
        
        return pixels | rv::take(n) | r::to<std::vector>();
    }

    double distance(const flo::rgb_color& c1, const flo::rgb_color& c2) {
        auto diff_r = static_cast<double>(c1.red - c2.red);
        auto diff_g = static_cast<double>(c1.green - c2.green);
        auto diff_b = static_cast<double>(c1.blue - c2.blue);
        return std::sqrt(
            diff_r * diff_r + diff_g * diff_g + diff_b * diff_b
        );
    }

    int find_closest_color(
            const flo::rgb_color& color, const std::vector<flo::rgb_color>& palette) {
        int closest = -1;
        double closest_dist = std::numeric_limits<double>::max();
        for (auto [index, pal_col] : rv::enumerate(palette)) {
            auto dist = distance(color, pal_col);
            if (dist < closest_dist) {
                closest = static_cast<int>(index);
                closest_dist = dist;
            }
        }
        return closest;
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

flo::canvas::canvas(const std::vector<rgb_color>& palette, const dimensions& dim) :
    canvas(palette, dim.wd, dim.hgt)
{
}

flo::canvas::canvas( const std::vector<rgb_color>& palette, int wd, int hgt, int bkgd, double amnt) :
        canvas(palette, wd, hgt) {
    for (int y = 0; y < hgt; ++y) {
        for (int x = 0; x < wd; ++x) {
            impl_[x, y] = make_one_color_paint(palette.size(), bkgd, amnt);
        }
    }
}

int flo::canvas::cols() const {
    return impl_.cols();
}

int flo::canvas::rows() const {
    return  impl_.rows();
}

int flo::canvas::layers() const {
    return  palette_.size();
}

flo::dimensions flo::canvas::bounds() const {
    return impl_.bounds();
}

flo::pigment flo::canvas::color_at(int x, int y) const {
    pigment_map<double> color_to_weight;

    for (auto [i, pigment] : rv::enumerate(palette_)) {
        const auto& particle = impl_[x, y];
        color_to_weight[pigment] = particle[i];
    }

    return mix_paint(color_to_weight);
}

int flo::canvas::palette_size() const {
    return static_cast<int>(palette_.size());
}

int flo::canvas::num_blank_locs() const
{
    return r::count_if(
        locations(impl_.bounds()),
        [&](auto&& pp) {
            auto [x, y] = pp;
            return volume_at(x,y) == 0.0;
        }
    );
}

std::vector<flo::coords> flo::canvas::blank_locs() const
{
    return locations(impl_.bounds()) | rv::filter(
            [&](auto&& pair) {
                auto [x, y] = pair;
                return volume_at(x, y) == 0.0;
            }
        ) | rv::transform(
            [&](auto&& pair)->coords {
                auto [x, y] = pair;
                return { x,y };
            }
        ) | r::to<std::vector>();
}

double flo::canvas::volume_at(int x, int y) const
{
    double vol = 0;
    for (auto v : impl_[x, y]) {
        vol += v;
    }
    return vol;
}

double flo::brush_region_area(const dimensions& dim, const point& loc, double rad, int aa) {
    return r::fold_left(
        flo::brush_region(dim, loc, rad, aa) | rv::transform([](auto&& rp) {return rp.weight; }),
        0.0,
        std::plus<double>()
    );
}

void flo::fill(canvas& canv, const point& loc, double radius, int aa_level, const paint_mixture& paint) {
    for (const auto& [loc, paint_pcnt] : brush_region(canv.bounds(), loc, radius, aa_level)) {
        canv[loc] = (1.0 - paint_pcnt) * canv[loc] + paint_pcnt * paint;
    }
}

void flo::overlay(canvas& canv, const point& loc, double radius, int aa_level, const paint_mixture& paint) {
    for (const auto& [loc, paint_pcnt] : brush_region(canv.bounds(), loc, radius, aa_level)) {
        canv[loc] += paint_pcnt * paint;
    }
}

void flo::mix(canvas& canv, const point& loc, double radius, int aa_level) {
    auto area = brush_region_area(canv.bounds(), loc, radius, aa_level);
    auto paint_sum = all_paint_in_brush_region(canv, loc, radius, aa_level);
    auto mean_color = (1.0 / area) * paint_sum;
    fill(canv, loc, radius, aa_level, mean_color);
}

flo::canvas flo::image_to_canvas(const image& img, const std::vector<rgb_color>& palette, double vol_per_pixel) {
    int n = static_cast<int>(palette.size());
    canvas canv(palette, img.cols(), img.rows());
    for (auto [x, y] : locations(img.bounds())) {
        auto color = pixel_to_rgb(img[x, y]);
        int palette_index = find_closest_color(color, palette);
        canv[x, y] = make_one_color_paint(n, palette_index, vol_per_pixel);
    }
    return canv;
}

flo::canvas flo::image_to_canvas(const image& img, int n, double vol_per_pixel) {
    auto palette = top_n_colors(img, n) | rv::transform(pixel_to_rgb) | r::to<std::vector>();
    return image_to_canvas(img, palette, vol_per_pixel);
}

flo::image flo::canvas_to_image(const canvas& canv, double alpha_threshold,
        const rgb_color& canvas_color) {
    static const auto bkgd = rgb_to_pigment(canvas_color);
    flo::image img(canv.bounds());
    for (auto [x, y] : locations(img.bounds())) {
        auto pigment = canv.color_at(x, y);
        auto volume = canv.volume_at(x, y);
        if (alpha_threshold > 0.0) {
            auto alpha = (volume >= alpha_threshold) ? 1.0 : volume / alpha_threshold;
            pigment = mix_pigments(bkgd, (1.0 - alpha), pigment, alpha);
        }
        img[x, y] = rgb_to_pixel(pigment_to_rgb(pigment));
    }
    return img;
}

flo::paint_mixture flo::all_paint_in_brush_region(canvas& canv, const point& loc, double radius, int aa_level) {
    flo::paint_mixture sum(std::vector<double>(canv.palette_size(), 0.0));
    for (const auto& [loc, paint_pcnt] : brush_region(canv.bounds(), loc, radius, aa_level)) {
        sum += paint_pcnt * flo::paint_mixture{ canv[loc] };
    }
    return sum;
}