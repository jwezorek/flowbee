#pragma once

#include <vector>

namespace flo {

    template<typename T>
    class matrix {
        std::vector<T> impl_;
        int cols_;
        int rows_;
    public:
        matrix(int cols, int rows) : cols_(cols), rows_(rows), impl_(cols* rows)
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

        int cols() const {
            return cols_;
        }

        int rows() const {
            return rows_;
        }
    };

}