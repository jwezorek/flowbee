#pragma once

#include <vector>
#include <ranges>
#include <algorithm>
#include <span>
#include "vec2.hpp"
#include "vec3.hpp"

namespace flo {

    using coords = vec2<int>;
    using coords_3d = vec3<int>;

    struct dimensions {
        int wd;
        int hgt;

        dimensions(int w, int h) {
            wd = w;
            hgt = h;
        }
    };

    inline auto locations(const dimensions& dim) {
        return std::views::cartesian_product(
            std::views::iota(0, dim.wd),
            std::views::iota(0, dim.hgt)
        );
    }

    template<typename T>
    class matrix {
        std::vector<T> impl_;
        int cols_;
        int rows_;
    public:
        matrix() : cols_{ 0 }, rows_{ 0 } {}

        matrix(int cols, int rows) : cols_(cols), rows_(rows), impl_(cols* rows) {}

        matrix(int cols, int rows, const T& v) : cols_(cols), rows_(rows), impl_(cols* rows, v) {}

        matrix(const dimensions& dim) : matrix(dim.wd, dim.hgt) {}

        matrix(const dimensions& dim, const T& v) : matrix(dim.wd, dim.hgt, v) {}

        T& operator[](int x, int y) {
            return impl_[y * cols_ + x];
        }

        const T& operator[](int x, int y) const {
            return impl_[y * cols_ + x];
        }

        T& operator[](const coords& loc) {
            return (*this)[loc.x, loc.y];
        }

        const T& operator[](const coords& loc) const {
            return (*this)[loc.x, loc.y];
        }

        void* data() const {
            return reinterpret_cast<void*>(const_cast<T*>(impl_.data()));
        }

        std::span<const T> entries() const {
            return impl_;
        }

        std::span<T> entries() {
            return impl_;
        }

        template <typename Func>
        void transform(Func&& func) {
            for (auto& entry : impl_) {
                entry = std::invoke(std::forward<Func>(func), entry);
            }
        }

        template <typename Func>
        matrix<T> transform_to(Func&& func) const {
            matrix<T> result(cols_, rows_);
            std::transform(impl_.begin(), impl_.end(), result.impl_.begin(),
                std::forward<Func>(func));
            return result;
        }

        template <typename U, typename Func>
        matrix<U> transform_to(Func&& func) const {
            matrix<U> result(cols_, rows_);
            std::transform(impl_.begin(), impl_.end(), result.entries().begin(),
                std::forward<Func>(func));
            return result;
        }

        int cols() const {
            return cols_;
        }

        int rows() const {
            return rows_;
        }

        dimensions bounds() const {
            return { cols_, rows_ };
        }
    };

    template<typename T>
    matrix<T> operator+(const matrix<T>& lhs, T rhs) {
        auto sum = lhs;
        for (auto [x, y] : locations(lhs.bounds())) {
            sum[x, y] = lhs[x, y] + rhs;
        }
        return sum;
    }

    template<typename T>
    matrix<T> operator-(const matrix<T>& lhs, T rhs) {
        auto diff = lhs;
        for (auto [x, y] : locations(lhs.bounds())) {
            diff[x, y] = lhs[x, y] - rhs;
        }
        return diff;
    }

    template<typename T>
    matrix<T> operator+(const matrix<T>& lhs, const matrix<T>& rhs) {
        auto sum = lhs;
        for (auto [x, y] : locations(lhs.bounds())) {
            sum[x, y] = lhs[x, y] + rhs[x, y];
        }
        return sum;
    }

    template<typename T>
    matrix<T> operator-(const matrix<T>& lhs, const matrix<T>& rhs) {
        auto diff = lhs;
        for (auto [x, y] : locations(lhs.bounds())) {
            diff[x, y] = lhs[x, y] - rhs[x, y];
        }
        return diff;
    }

    template<typename T>
    matrix<T> operator*(T lhs, const matrix<T>& rhs) {
        auto prod = rhs;
        for (auto [x, y] : locations(rhs.bounds())) {
            prod[x, y] = lhs * rhs[x, y];
        }
        return prod;
    }

}