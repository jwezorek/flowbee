#pragma once

#include "matrix.hpp"

namespace flo {

    struct dimensions_3d {
        int wd;
        int hgt;
        int depth;
    };

    template<typename T>
    class matrix_3d {
        std::vector<T> impl_;
        int cols_;
        int rows_;
        int layers_;

        // Proxy class for handling operations across layers
        class cell_proxy {
            T* data_;
            int layers_;

        public:
            cell_proxy(T* data, int layers) : data_(data), layers_(layers) {}

            cell_proxy(const std::vector<double>& v) : data_(const_cast<T*>(v.data())), layers_(v.size()) {}

            // Conversion to std::span<T> for read/write access
            operator std::span<T>() {
                return std::span<T>(data_, layers_);
            }

            // Conversion to std::vector<T> for read access
            operator std::vector<T>() const {
                return std::vector<T>{data_, data_ + layers_};
            }

            T operator[](int i) const {
                return data_[i];
            }

            cell_proxy& operator=(const std::vector<T>& values) {
                if (values.size() != static_cast<size_t>(layers_)) {
                    throw std::invalid_argument("Vector size must match the number of layers.");
                }
                std::copy(values.begin(), values.end(), data_);
                return *this;
            }

            cell_proxy& operator=(std::span<const T> values) {
                if (values.size() != static_cast<size_t>(layers_)) {
                    throw std::invalid_argument("Span size must match the number of layers.");
                }
                std::copy(values.begin(), values.end(), data_);
                return *this;
            }

            cell_proxy operator+=(cell_proxy other) {
                for (int i = 0; i < layers_; ++i) {
                    data_[i] += other.data_[i];
                }
                return *this;
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

        matrix_3d(const dimensions_3d& dim) : matrix_3d(dim.wd, dim.hgt, dim.depth) {}

        matrix_3d(const dimensions_3d& dim, const T& v) : matrix_3d(dim.wd, dim.hgt, dim.depth, v) {}

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

        cell_proxy operator[](const coords& loc) {
            if (loc.x < 0 || loc.x >= cols_ || loc.y < 0 || loc.y >= rows_) {
                throw std::out_of_range("Coordinates out of bounds");
            }
            auto start_index = loc.y * (cols_ * layers_) + loc.x * layers_;
            return cell_proxy(&impl_[start_index], layers_);
        }

        const std::span<const T> operator[](const coords& loc) const {
            if (loc.x < 0 || loc.x >= cols_ || loc.y < 0 || loc.y >= rows_) {
                throw std::out_of_range("Coordinates out of bounds");
            }
            auto start_index = loc.y * (cols_ * layers_) + loc.x * layers_;
            return std::span<const T>(&impl_[start_index], layers_);
        }

        cell_proxy operator[](int x, int y) {
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
            return { cols_, rows_ };
        }

    };

}