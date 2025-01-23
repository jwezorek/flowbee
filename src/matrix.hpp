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
        int depth;

        dimensions(int w, int h, int d = 1) {
            wd = w;
            hgt = h;
            depth = d;
        }
    };

    template<typename T>
    T locations(const dimensions& dim) {
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
        matrix() : cols_{0}, rows_{ 0 }
        {
        }

        matrix(int cols, int rows) : cols_(cols), rows_(rows), impl_(cols* rows)
        {
        }

        matrix(int cols, int rows, const T& v) : cols_(cols), rows_(rows), impl_(cols* rows, v)
        {
        }

        matrix(const dimensions& dim) : matrix(dim.wd, dim.hgt)
        {
        }

        matrix(const dimensions& dim, const T& v) : matrix(dim.wd, dim.hgt, v)
        {
        }

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
            return { cols_,rows_ };
        }

    };

    template<typename T>
    class matrix_3d {
        std::vector<T> impl_;
        int cols_;
        int rows_;
        int layers_;
    public:
        matrix_3d() : cols_{ 0 }, rows_{ 0 }, layers_{ 0 }
        {
        }

        matrix_3d(int cols, int rows, int layers) :
            cols_(cols), rows_(rows), layers_(layers), impl_(cols * rows * layers)
        {
        }

        matrix_3d(int cols, int rows, int layers, const T& v) :
            cols_(cols), rows_(rows), layers_(layers), impl_(cols * rows * layers, v)
        {
        }

        matrix_3d(const dimensions& dim) : matrix(dim.wd, dim.hgt, dim.depth)
        {
        }

        matrix_3d(const dimensions& dim, const T& v) : matrix(dim.wd, dim.hgt, dim.depth, v)
        {
        }

        T& operator[](int x, int y, int z) {
            return impl_[z * rows_ * cols_ + y * cols_ + x];
        }

        const T& operator()(int x, int y, int z) const {
            return impl_[z * rows_ * cols_ + y * cols_ + x];
        }

        T& operator[](const coords_3d& loc) {
            return (*this)[loc.x, loc.y, loc.z];
        }

        const T& operator[](const coords_3d& loc) const {
            return (*this)[loc.x, loc.y, loc.z];
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

        int layers() const {
            return cols_;
        }

        dimensions bounds() const {
            return { cols_, rows_, layers_ };
        }

    };

}