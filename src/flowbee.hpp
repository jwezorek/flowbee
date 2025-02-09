#pragma once

#include "types.hpp"
#include "brush.hpp"
#include "canvas.hpp"
#include "vector_field.hpp"
#include <variant>
#include <optional>
#include <string>

namespace flo {

    struct output_params {
        std::string filename;
        rgb_color canvas_color;
        double alpha_threshold;

        output_params(const std::string& fname);
    };

    struct flowbee_params {
        brush_params brush;
        double particle_volume;
        int max_particle_history;
        double dead_particle_area_sz;
        double delta_t;
        std::optional<int> iterations;
        int num_particles;
        bool populate_white_space;
        std::vector<int> palette_subset; 
        std::optional<double> diffusion_rate;

        flowbee_params(const brush_params& b, int iters, int n_particles);
        flowbee_params(const brush_params& b, int n_particles);
    };

    struct layer_params {
        vector_field flow;
        flowbee_params params;
    };

    void do_flowbee(
        const output_params& output,
        const std::vector<flo::rgb_color>& palette,
        const vector_field& flow,
        const flowbee_params& params
    );

    void do_flowbee(
        const output_params& output,
        const std::vector<flo::rgb_color>& palette,
        const std::vector<layer_params>& layers
    );

}