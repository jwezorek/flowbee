# Flowbee

Flowbee is a tool for generating images of paint particles moving in vector fields, mixing in a semi-realistic manner. It simulates the movement of particles in a flow field, allowing for the creation of intricate, swirling patterns that resemble natural paint mixing and dispersion. Flowbee provides a high degree of control over brush settings, paint behavior, and particle dynamics, making it a powerful tool for generative art and computational painting.

## Usage

To run Flowbee, pass a JSON file specifying the parameters along with an output file path where the generated image will be saved:

```sh
flowbee input.json output.png
```

Flowbee supports any image format compatible with stb-image-write, such as PNG, JPEG, or BMP.

## Example JSON Configuration

The following JSON file defines a generative artwork setup with a structured flow field, color palette, and paint simulation settings:

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
  - **Flow**: Defines the vector field used to guide paint particles.
    - **op: vector\_field**: Specifies that a vector field is being used.
    - **dimensions**: The size of the field.
    - **def**: Defines how the field is generated.
      - A log spiral field is combined with Perlin noise to create a dynamic vector field.
      - The spiral has a growth rate of 2.0, is outward-expanding, and rotates clockwise.
      - Perlin noise is multiplied by 0.25 and blended with the spiral.
  - **Params**: Defines the brush and particle behavior.
    - **Brush**:
      - `radius`: Defines the brush size.
      - `mix`: Enables color mixing.
      - `mode`: Determines how paint is applied ("fill" mode used here).
      - `radius_ramp_in_time`: Time steps over which the brush radius changes.
      - `aa_level`: Anti-aliasing level for smoother results.
      - `paint_transfer_coeff`: Controls how much paint transfers between particles and the canvas.
    - **Particle Parameters**:
      - `particle_volume`: The total volume of paint particles.
      - `max_particle_history`: The maximum number of steps a particle persists before being reset.
      - `dead_particle_area_sz`: The size of areas where particles stop moving.
      - `delta_t`: Simulation timestep.
      - `num_particles`: The number of particles in the system.
      - `populate_white_space`: Ensures that unpainted regions are filled dynamically.

## Running the Example

Save the above JSON into a file (e.g., `example.json`) and execute:

```sh
flowbee example.json output.png
```

Flowbee will generate an image based on the specified parameters and save it to `output.png`.

## Customization

Modify the JSON parameters to experiment with different vector fields, brush settings, and particle behaviors. Combining different flow field operations and adjusting paint properties allows for unique generative artworks.

## License

Flowbee is licensed for non-commercial use under the [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/) license.
 ... seriously. I don't even know what this is going to be yet -- but want a place to experiment with some ideas.

