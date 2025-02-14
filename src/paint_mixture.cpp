#include "paint_mixture.hpp"
namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/


double flo::volume(const paint_mixture& p)
{
    return r::fold_left(p, 0.0, std::plus<>());
}

void flo::normalize_in_place(paint_mixture& p) {
    auto sum = volume(p);
    if (sum == 0.0) {
        return;
    }
    for (auto& val : p) {
        val /= sum;
    }
}

flo::paint_mixture flo::normalize(const paint_mixture& p) {
    auto norm = p;
    normalize_in_place(norm);
    return norm;
}


flo::paint_mixture flo::operator*(double k, const paint_mixture& p) {
    auto prod = p;
    for (auto& val : prod) {
        val *= k;
    }
    return prod;
}

flo::paint_mixture& flo::operator+=(flo::paint_mixture& paint_lhs, const flo::paint_mixture& paint_rhs) {
    auto new_mixture = rv::zip(paint_lhs, paint_rhs) |
        rv::transform(
            [&](const auto& pair) {
                const auto& [lhs, rhs] = pair;
                return lhs + rhs;
            }
        ) | r::to<std::vector>();

    paint_lhs = std::move(new_mixture);
    return paint_lhs;
}

flo::paint_mixture& flo::operator-=(paint_mixture& lhs, const paint_mixture& rhs) {
    return lhs += (-1.0) * rhs;
}

flo::paint_mixture flo::operator+(const paint_mixture& lhs, const paint_mixture& rhs) {
    auto sum = lhs;
    sum += rhs;
    return sum;
}

flo::paint_mixture flo::operator-(const paint_mixture& lhs, const paint_mixture& rhs) {
    auto difference = lhs;
    difference -= rhs;
    return difference;
}

flo::paint_mixture flo::make_one_color_paint(int palette_sz, int color_index, double volume) {
    auto mixture = std::vector<double>(palette_sz, 0.0);
    mixture[color_index] = 1.0;
    return mixture;
}

std::string flo::display(const paint_mixture& p)
{
    auto mixture_str = rv::join_with(
        p | std::views::transform(
            [](double v) { return std::format("{:.4g}", v); }
        ), ", ") | r::to<std::string>();
    return std::format("[ {} ]", mixture_str);
}
