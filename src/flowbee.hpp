#pragma once

#include "types.hpp"
#include "brush.hpp"
#include "canvas.hpp"
#include "vector_field.hpp"
#include <variant>
#include <optional>

namespace flo {

    struct flowbee_params {
        brush_params brush;
        double particle_volume;
        int max_particle_history;
        double dead_particle_area_sz;
        double alpha_threshold;
        double delta_t;
        std::optional<int> iterations;
        int num_particles;
        bool populate_white_space;

        flowbee_params(const brush_params& b, int iters, int n_particles);
        flowbee_params(const brush_params& b, int n_particles);
    };

    void do_flowbee(
        const std::string& outfile_path,
        const std::vector<flo::rgb_color>& palette,
        const vector_field& flow,
        const flowbee_params& params
    );

}