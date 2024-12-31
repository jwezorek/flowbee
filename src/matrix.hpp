#pragma once

#include <vector>
#include <ranges>
#include <algorithm>
#include <span>

namespace flo {

    struct dimensions {
        int wd;
        int hgt;
    };

    template<typename T>
    class matrix {
        std::vector<T> impl_;
        int cols_;
        int rows_;
    public:
        matrix(int cols, int rows) : cols_(cols), rows_(rows), impl_(cols* rows)
        {
        }

        matrix(const dimensions& dim) : matrix(dim.wd, dim.hgt)
        {
        }

        T& operator[](int x, int y) {
            return impl_[y * cols_ + x];
        }

        const T& operator[](int x, int y) const {
            return impl_[y * cols_ + x];
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

}