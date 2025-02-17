
![perturbed_spiral](https://github.com/user-attachments/assets/ab74148f-91a3-4846-ace9-4587ab0f3353)

# Flowbee

Flowbee is a tool for generating images of paint particles moving in vector fields, mixing in a semi-realistic manner. 

The core concept is to construct a dynamic vector field by blending primitive vector components—such as circular flows, Perlin noise, and other patterns. Within this field, simulated paint particles move across the canvas, their trajectories shaped by the underlying flow. As they travel, these particles deposit paint and interact with existing paint, gradually shifting in color through a process of continuous mixing. Particles that become trapped in knots or sinks within the vector field, or those that drift beyond the canvas bounds, are removed and replaced with new random particles.

# Implementation Details

The code is C++23 with no external dependencies except Boost which is currently only being used for boost::hash_combine so would be easy to remove. Other third-party dependencies -- stb_image, mixbox, siv::PerlinNoise, and nlohmann::json -- are vendored in the third-party/ folder, meaning their source code is included directly in the project rather than being linked as external libraries.

The library [mixbox](https://scrtwpns.com/mixbox/) in particular is doing heavy lifting in this project; i.e., I didn't personally implement the Kubelka–Munk model of color mixing used here.

The main problem with this code right now is that it is slow. I think the major optimizations that could be done would be to use an expression template implementation of arithmetic over paint mixtures perhaps by changing the implementation to use Eigen for the main 1D, 2D, and 3D array classes rather than my roll-your-own matrix classes. The other big optimization possible would be to perform the main painting loop such that non-overlapping paint particles are simulated in parallel.

## Usage

To run Flowbee, pass a JSON file specifying the parameters along with an output file path where the generated image will be saved:

```sh
flowbee input.json output.png
```

Flowbee supports any image format compatible with stb-image-write, such as PNG, JPEG, or BMP.

## Example JSON Configuration

The following JSON file generates the image above:

```json
{
    "palette": [ "f1faee", "a8dadc", "457b9d", "1d3557" ],
    "layers": [
        {
            "flow": {
                "op": "vector_field",
                "dimensions": [1000, 1000],
                "def": {
                    "op": "normalize",
                    "arg": {
                        "op": "add",
                        "arg1": {
                            "op": "log_spiral",
                            "growth_rate": 2.0,
                            "inward": false,
                            "clockwise" :  true
                        },
                        "arg2": {
                            "op": "multiply",
                            "arg1": 0.25,
                            "arg2": {    
                                "op": "perlin",
                                "octaves": 8,
                                "freq": 8.0,
                                "exponent": 0.5,
                                "normalized": true
                            }  
                        }
                    }
                }
            },
            "params": {
                "brush": {
                    "radius" : 10.0,
                    "mix" : true,
                    "mode" : "fill",
                    "radius_ramp_in_time" : 200,
                    "aa_level" : 4,
                    "paint_transfer_coeff" : 0.6
                },
                "particle_volume": 1.0,
                "max_particle_history": 10,
                "dead_particle_area_sz": 3,
                "delta_t": 1.0,
                "num_particles": 50,
                "populate_white_space": true
            }
        }
    ]
}
```

## Explanation of Parameters

- **Palette**: Defines the color set used in the artwork. Colors are specified in hexadecimal format.
- **Layers**: Each layer has its own flow field and paint simulation settings.
  - **Flow**: Defines the vector field used to guide paint particles. The following is for example purposes. There are more vecotr field primitives. Look in the example JSON files in the repo to see what else is possible.
    - **op: vector\_field**: Top-level vector field.
    - **dimensions**: The size of the field. Only needed on the top-level.
    - **def**: Defines how the field is generated.
      - A log spiral field is combined with Perlin noise to create a dynamic vector field.
      - The spiral has a growth rate of 2.0, is outward-expanding, and rotates clockwise.
      - Perlin noise is multiplied by 0.25 and blended with the spiral.
  - **Params**: Defines the brush and particle behavior.
    - **Brush**:
      - `radius`: Defines the brush size.
      - `mix`: Enables color mixing. If this is false the color of the brush is not affected by paint on the canvas.
      - `mode`: Determines how paint is applied ("fill" mode used here; the other options are "overlay" and "mix".).
      - `radius_ramp_in_time`: Time steps over which the brush radius changes starting from one pixel.
      - `aa_level`: Anti-aliasing level. Valid options are [0...4]. 0 means no anti-aliasing. 4 means 256 distinct values.
      - `paint_transfer_coeff`: Controls how much paint transfers between particles and the canvas, needs to be in the range [0 ... 1.0].
    - **Particle Parameters**:
      - `particle_volume`: Volume of the paint at each pixel on the canvas. In practice this only matter when using the "overlay" brush mode.
      - `max_particle_history`: The maximum number of steps a retained by each particle to test for particle death.
      - `dead_particle_area_sz`: The minumum size of largest dimension of the bounds of the particle's history such that the particle is considered to still be moving and thus is not pruned. 
      - `delta_t`: Simulation timestep.
      - `num_particles`: The number of particles in the system. When particles die, more are generated such that there are always 'num_particles'.
      - `populate_white_space`: Ensures that unpainted regions are populated first when spawning new particles.

## License

Flowbee is licensed for non-commercial use under the [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/) license.

