#pragma once

#include <vector>
#include <string>

namespace flo {

    class image {
        std::vector<uint32_t> impl_;
        int cols_;
        int rows_;
    public:
        image(int cols, int rows);
        uint32_t& operator[](int x, int y);
        const uint32_t& operator[](int x, int y) const;
        void* data() const;
        int cols() const;
        int rows() const;
    };

    void write_to_file(const std::string& fname, const image& img);

}