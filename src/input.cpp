#include "input.hpp"
#include "third-party/json.hpp"
#include <fstream>
#include <string_view>

namespace {
    namespace js = nlohmann;
    using json = nlohmann::json;

    const std::string k_perlin = "perlin";
    const std::string k_zigzag = "zigzag";
    const std::string k_normalize = "normalize";
    const std::string k_multiply = "multiply";
    const std::string k_add = "add";
    const std::string k_op = "op";
    const std::string k_arg = "arg";
    const std::string k_arg1 = "arg1";
    const std::string k_arg2 = "arg2";
    const std::string k_dimensions = "dimensions";
    const std::string k_def = "def";
    const std::string k_octaves = "octaves";
    const std::string k_freq = "freq";
    const std::string k_exponent = "exponent";
    const std::string k_normalized = "normalized";
    const std::string k_radius = "radius";
    const std::string k_filename = "filename";
    const std::string k_canvas_color = "canvas_color";
    const std::string k_alpha_threshold = "alpha_threshold";
    const std::string k_radius_ramp_in_time = "radius_ramp_in_time";
    const std::string k_mix = "mix";
    const std::string k_overlay = "overlay";
    const std::string k_fill = "fill";
    const std::string k_mode = "mode";
    const std::string k_aa_level = "aa_level";
    const std::string k_paint_transfer_coeff = "paint_transfer_coeff";
    const std::string k_stroke_lifetime = "stroke_lifetime";
    const std::string k_mean = "mean";
    const std::string k_stddev = "stddev";
    const std::string k_ramp_out_time = "ramp_out_time";
    const std::string k_particle_volume = "particle_volume";
    const std::string k_max_particle_history = "max_particle_history";
    const std::string k_dead_particle_area_sz = "dead_particle_area_sz";
    const std::string k_delta_t = "delta_t";
    const std::string k_num_particles = "num_particles";
    const std::string k_populate_white_space = "populate_white_space";
    const std::string k_iterations = "iterations";
    const std::string k_palette_subset = "palette_subset";
    const std::string k_diffusion_rate = "diffusion_rate";
    const std::string k_jitter = "jitter";
    const std::string k_weight = "weight";
    const std::string k_brush = "brush";
    const std::string k_rand_seed = "rand_seed";
    const std::string k_output = "output";
    const std::string k_palette = "palette";
    const std::string k_layers = "layers";
    const std::string k_flow = "flow";
    const std::string k_params = "params";
    const std::string k_type = "type";
    const std::string k_inward = "inward";
    const std::string k_outward = "outward";
    const std::string k_clockwise = "clockwise";
    const std::string k_counterclockwise = "counterclockwise";
    const std::string k_centers_dist = "centers_dist";
    const std::string k_theta_rate = "theta_rate";
    const std::string k_growth_rate = "growth_rate";
    const std::string k_circular = "circular";
    const std::string k_elliptic = "elliptic";
    const std::string k_loxo_spiral = "loxo_spiral";
    const std::string k_log_spiral = "log_spiral";

    flo::vector_field vector_field_from_json_aux(const flo::dimensions& dim, const json& json_obj);

    flo::circle_field_type parse_circle_field_type(const json& json_value) {
        if (!json_value.is_string()) {
            throw std::invalid_argument("Expected a string for circle_field_type");
        }

        std::string mode_str = json_value.get<std::string>();

        if (mode_str == k_inward) {
            return flo::circle_field_type::inward;
        } else if (mode_str == k_outward) {
            return flo::circle_field_type::outward;
        } else if (mode_str == k_clockwise) {
            return flo::circle_field_type::clockwise;
        } else if (mode_str == k_counterclockwise) {
            return flo::circle_field_type::counterclockwise;
        } else {
            throw std::invalid_argument("Invalid circle_field_type: " + mode_str);
        }
    }

    flo::vector_field perlin_field_fn(const flo::dimensions& dim, const json& node) {
        return perlin_vector_field(dim, node[k_octaves], node[k_freq],
            node.value(k_exponent, 1.0), node.value(k_normalized, true));
    }

    flo::vector_field zigzag_field_fn(const flo::dimensions& dim, const json& node) {
        return zigzag_vector_field(dim, node[k_radius]);
    }

    flo::vector_field normalize_field_fn(const flo::dimensions& dim, const json& node) {
        return normalize(vector_field_from_json_aux(dim, node[k_arg]));
    }

    flo::vector_field multiply_field_fn(const flo::dimensions& dim, const json& node) {
        if (node[k_arg1].is_array()) {
            flo::point v{ node[k_arg1][0], node[k_arg1][1] };
            return v * vector_field_from_json_aux(dim, node[k_arg2]);
        }
        else {
            double scalar = node[k_arg1];
            return scalar * vector_field_from_json_aux(dim, node[k_arg2]);
        }
    }

    flo::vector_field add_field_fn(const flo::dimensions& dim, const json& node) {
        if (node[k_arg1].is_array()) {
            flo::point v{ node[k_arg1][0], node[k_arg1][1] };
            return v + vector_field_from_json_aux(dim, node[k_arg2]);
        } else if (node[k_arg2].is_array()) {
            flo::point v{ node[k_arg2][0], node[k_arg2][1] };
            return v + vector_field_from_json_aux(dim, node[k_arg1]);
        } else if (node[k_arg1].is_number()) {
            double k = node[k_arg1];
            return k + vector_field_from_json_aux(dim, node[k_arg2]);
        } else {
            return vector_field_from_json_aux(dim, node[k_arg1]) +
                vector_field_from_json_aux(dim, node[k_arg2]);
        }
    }

    flo::vector_field circular_field_fn(const flo::dimensions& dim, const json& node) {
        return circular_vector_field(dim, parse_circle_field_type(node[k_type]));
    }

    flo::vector_field elliptic_field_fn(const flo::dimensions& dim, const json& node) {
        return elliptic_vector_field(dim, parse_circle_field_type(node[k_type]));
    }

    /*
    flo::vector_field loxo_spiral_field_fn(const flo::dimensions& dim, const json& node) {
        return loxodromic_spiral_vector_field(
            dim, node[k_outward], node[k_centers_dist], node[k_theta_rate]
        );
    }

    flo::vector_field log_spiral_field_fn(const flo::dimensions& dim, const json& node) {
        return logarithmic_spiral_vector_field(dim, node[k_growth_rate], node[k_inward], node[k_theta_offset]);
    }
    */

    flo::vector_field vector_field_from_json_aux(const flo::dimensions& dim, const json& json_obj) {
        using namespace flo;
        using vector_field_fn = std::function<flo::vector_field(const dimensions&, const json&)>;

        static const std::unordered_map<std::string, vector_field_fn> operations = {
            {k_perlin, perlin_field_fn},
            {k_zigzag, zigzag_field_fn},
            {k_normalize, normalize_field_fn},
            {k_multiply, multiply_field_fn},
            {k_add, add_field_fn},
            {k_circular, circular_field_fn},
            {k_elliptic, elliptic_field_fn},
            //{k_loxo_spiral, loxo_spiral_field_fn},
            //{k_log_spiral, log_spiral_field_fn}
        };

        const auto& fn = operations.at(json_obj[k_op].get<std::string>());
        return fn(dim, json_obj);
    }


    flo::vector_field vector_field_from_json(const json& json_obj) {
        using namespace flo;
        dimensions dim{ json_obj[k_dimensions][0], json_obj[k_dimensions][1] };
        const json& def = json_obj[k_def];
        return vector_field_from_json_aux(dim, def);
    }

    flo::output_params parse_output_params(const std::string out_file, const json& j) {
        flo::output_params out{
            out_file,
            flo::hex_str_to_rgb("#ffffff"),
            1.0
        };
        if (j.contains(k_output)) {
            const auto& out_params = j[k_output];
            out.canvas_color = out_params.contains(k_canvas_color) ?
                flo::hex_str_to_rgb(out_params[k_canvas_color].get<std::string>()) :
                flo::hex_str_to_rgb("#ffffff");
            out.alpha_threshold = out_params.value(k_alpha_threshold, 1.0);
        }
        return out;
    }

    flo::paint_mode parse_paint_mode(const json& json_value) {
        if (!json_value.is_string()) {
            throw std::invalid_argument("Expected a string for paint_mode");
        }

        std::string mode_str = json_value.get<std::string>();

        if (mode_str == k_overlay) {
            return flo::paint_mode::overlay;
        } else if (mode_str == k_fill) {
            return flo::paint_mode::fill;
        } else if (mode_str == k_mix) {
            return flo::paint_mode::mix;
        } else {
            throw std::invalid_argument("Invalid paint_mode: " + mode_str);
        }
    }

    flo::brush_params parse_brush_params(const json& j) {
        flo::brush_params brush;
        brush.radius = j[k_radius].get<double>();
        if (j.contains(k_radius_ramp_in_time)) {
            brush.radius_ramp_in_time = j[k_radius_ramp_in_time].get<double>();
        }
        brush.mix = j[k_mix].get<bool>();
        brush.mode = parse_paint_mode(j[k_mode]);
        brush.aa_level = j[k_aa_level].get<int>();
        brush.paint_transfer_coeff = j[k_paint_transfer_coeff].get<double>();

        if (j.contains(k_stroke_lifetime)) {
            flo::stroke_lifetime lifetime;
            lifetime.mean = j[k_stroke_lifetime][k_mean].get<double>();
            lifetime.stddev = j[k_stroke_lifetime][k_stddev].get<double>();
            if (j[k_stroke_lifetime].contains(k_ramp_out_time)) {
                lifetime.ramp_out_time = j[k_stroke_lifetime][k_ramp_out_time].get<double>();
            }
            brush.stroke_lifetime = lifetime;
        }
        return brush;
    }

    flo::flowbee_params parse_flowbee_params(const json& j) {
        flo::flowbee_params params;
        params.particle_volume = j[k_particle_volume].get<double>();
        params.max_particle_history = j[k_max_particle_history].get<int>();
        params.dead_particle_area_sz = j[k_dead_particle_area_sz].get<double>();
        params.delta_t = j[k_delta_t].get<double>();
        params.num_particles = j[k_num_particles].get<int>();
        params.populate_white_space = j[k_populate_white_space].get<bool>();

        if (j.contains(k_iterations)) {
            params.iterations = j[k_iterations].get<int>();
        }

        if (j.contains(k_palette_subset)) {
            for (const auto& index : j[k_palette_subset]) {
                params.palette_subset.push_back(index.get<int>());
            }
        }

        if (j.contains(k_diffusion_rate)) {
            params.diffusion_rate = j[k_diffusion_rate].get<double>();
        }

        if (j.contains(k_jitter)) {
            flo::jitter_params jitter;
            jitter.weight = j[k_jitter][k_weight].get<double>();
            jitter.stddev = j[k_jitter][k_stddev].get<double>();
            params.jitter = jitter;
        }

        if (j.contains(k_brush)) {
            params.brush = parse_brush_params(j[k_brush]);
        }

        return params;
    }
}

std::expected<flo::input, std::string> flo::parse_input(
        const std::string& inp, const std::string& outp) {
    std::ifstream file(inp);
    if (!file.is_open()) {
        return std::unexpected("Failed to open file: " + inp);
    }

    try {
        json j;
        file >> j;
        file.close();

        input parsed_input;

        if (j.contains(k_rand_seed)) {
            parsed_input.rand_seed = j[k_rand_seed].get<uint32_t>();
        }

        parsed_input.output = parse_output_params(outp, j);

        for (const auto& color_str : j[k_palette]) {
            parsed_input.palette.push_back(hex_str_to_rgb(color_str.get<std::string>()));
        }

        for (const auto& layer : j[k_layers]) {
            layer_params lp;
            lp.flow = vector_field_from_json(layer[k_flow]);
            lp.params = parse_flowbee_params(layer[k_params]);
            parsed_input.layers.push_back(lp);
        }

        return parsed_input;

    } catch (std::runtime_error err) {
        return std::unexpected(err.what());
    } catch (const js::json::parse_error& e) {
        return std::unexpected(std::format("parser eror: {}", e.what()));
    } catch (const js::json::type_error& e) {
        return std::unexpected(std::format("type error: {}", e.what()));
    } catch (const js::json::out_of_range& e) {
        return std::unexpected(std::format("out of range: {}", e.what()));
    } catch (const js::json::exception& e) {
        return std::unexpected(std::format("jason exception: {}", e.what()));
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    } catch (...) {
    }
    return std::unexpected("Unknown error");
}


