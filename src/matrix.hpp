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
    class matrix_3d {
        std::vector<T> impl_;
        int cols_;
        int rows_;
        int layers_;

        // Proxy class for handling layer operations
        class layer_proxy {
            T* data_;
            int layers_;

        public:
            layer_proxy(T* data, int layers) : data_(data), layers_(layers) {}

            layer_proxy(const std::vector<double>& v) : data_(const_cast<T*>(v.data())), layers_(v.size()) {}

            // Conversion to std::span<T> for read/write access
            operator std::span<T>() {
                return std::span<T>(data_, layers_);
            }

            // Conversion to std::vector<T> for read access
            operator std::vector<T>() const {
                return std::vector<T>{data_, data_ + layers_};
            }

            // Assignment operator to assign from a vector
            layer_proxy& operator=(const std::vector<T>& values) {
                if (values.size() != static_cast<size_t>(layers_)) {
                    throw std::invalid_argument("Vector size must match the number of layers.");
                }
                std::copy(values.begin(), values.end(), data_);
                return *this;
            }

            // Assignment operator to assign from another span
            layer_proxy& operator=(std::span<const T> values) {
                if (values.size() != static_cast<size_t>(layers_)) {
                    throw std::invalid_argument("Span size must match the number of layers.");
                }
                std::copy(values.begin(), values.end(), data_);
                return *this;
            }

            // Addition operator to add another layer_proxy
            layer_proxy operator+(const layer_proxy& other)  {
                for (int i = 0; i < layers_; ++i) {
                    data_[i] += other.data_[i];
                }
                return *this;
            }

            // Multiplication operator for scalar multiplication
            static friend layer_proxy operator*(double k, layer_proxy proxy) {
                for (int i = 0; i < proxy.layers_; ++i) {
                    proxy.data_[i] *= k;
                }
                return proxy;
            }
        };

    public:
        matrix_3d() : cols_{ 0 }, rows_{ 0 }, layers_{ 0 } {}

        matrix_3d(int cols, int rows, int layers) :
            cols_(cols), rows_(rows), layers_(layers), impl_(cols* rows* layers) {
        }

        matrix_3d(int cols, int rows, int layers, const T& v) :
            cols_(cols), rows_(rows), layers_(layers), impl_(cols* rows* layers, v) {
        }

        matrix_3d(const dimensions& dim) : matrix_3d(dim.wd, dim.hgt, dim.depth) {}

        matrix_3d(const dimensions& dim, const T& v) : matrix_3d(dim.wd, dim.hgt, dim.depth, v) {}

        T& operator[](int x, int y, int z) {
            int index = y * (cols_ * layers_) + x * layers_ + z;
            return impl_[index];
        }

        const T& operator[](int x, int y, int z) const {
            int index = y * (cols_ * layers_) + x * layers_ + z;
            return impl_[index];
        }

        T& operator[](const coords_3d& loc) {
            int index = loc.y * (cols_ * layers_) + loc.x * layers_ + loc.z;
            return impl_[index];
        }

        const T& operator[](const coords_3d& loc) const {
            int index = loc.y * (cols_ * layers_) + loc.x * layers_ + loc.z;
            return impl_[index];
        }

        T& operator[](const coords& loc, int layer) {
            return (*this)[coords_3d{ loc.x, loc.y, layer }];
        }

        const T& operator[](const coords& loc, int layer) const {
            return (*this)[coords_3d{ loc.x, loc.y, layer }];
        }

        layer_proxy operator[](const coords& loc) {
            if (loc.x < 0 || loc.x >= cols_ || loc.y < 0 || loc.y >= rows_) {
                throw std::out_of_range("Coordinates out of bounds");
            }
            auto start_index = loc.y * (cols_ * layers_) + loc.x * layers_;
            return layer_proxy(&impl_[start_index], layers_);
        }

        const std::span<const T> operator[](const coords& loc) const {
            if (loc.x < 0 || loc.x >= cols_ || loc.y < 0 || loc.y >= rows_) {
                throw std::out_of_range("Coordinates out of bounds");
            }
            auto start_index = loc.y * (cols_ * layers_) + loc.x * layers_;
            return std::span<const T>(&impl_[start_index], layers_);
        }

        layer_proxy operator[](int x, int y) {
            return (*this)[coords{ x, y }];
        }

        const std::span<const T> operator[](int x, int y) const {
            return (*this)[coords{ x, y }];
        }

        void* data() const {
            return reinterpret_cast<void*>(const_cast<T*>(impl_.data()));
        }

        int cols() const {
            return cols_;
        }

        int rows() const {
            return rows_;
        }

        int layers() const {
            return layers_;
        }

        dimensions bounds() const {
            return { cols_, rows_, layers_ };
        }
    };

}