#pragma once

#include <string>

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    class canvas;
    class brush;

    void do_gui(const std::string& img_file, int n);
    void do_gui( canvas& c,  brush& b);
}