#include <string>

static std::string sphere_config_template = R"({
  "integrator": { 
    "type": "path",
    "spp": 128, 
    "max_depth": 64, 
    "rr_threshold": 0.1, 
    "profile": "{{0}}" },
  "film": { "resolution": [100, 100] },
  "camera": {
    "position": [0.0, 1.0, 6.8],
    "look_at": [0.0, 1.0, 0.0],
    "ref_up": [0.0, 1.0, 0.0],
    "fov": 19.5,
    "focal_length": 1.0 },
  "textures": { "white_background": { "type": "constant", "color": [1.0, 1.0, 1.0] } },
  "materials": { "diffuse": { "type": "diffuse", "texture_name": "white_background" } },
  "environment_map": { "texture_name": "white_background" },
  "objects": [ { "type": "sphere", "center": [0.0, 1.0, 0.0], "radius": 1.0, "material_name": "diffuse" } ]
})";

static std::string glass_sphere_config_template = R"({
  "integrator": {
    "type": "path",
    "spp": 128,
    "profile": "{{0}}",
    "rr_threshold": 0.1,
    "max_depth": 64 },
  "film": { "resolution": [ 100, 100 ] },
  "camera": {
    "position": [ 0.0, 1.0, 6.8 ],
    "look_at": [ 0.0, 1.0, 0.0 ],
    "ref_up": [ 0.0, 1.0, 0.0 ],
    "fov": 19.5,
    "focal_length": 1.0 },
  "textures": { "white_background": { "type": "constant", "color": [1.0, 1.0, 1.0] } },
  "materials": { "glass": { "type": "glass", "eta": 1.8 } },
  "environment_map": { "texture_name": "white_background" },
  "objects": [ { "type": "sphere", "center": [ 0.0, 1.0, 0.0 ], "radius": 1.0, "material_name": "glass" } ]
})";

static std::string two_spheres_config_template = R"({
 "integrator": { 
    "type": "path", 
    "spp": 128, 
    "profile": "{{0}}", 
    "rr_threshold": 0.1, 
    "max_depth": 64 }, 
  "film": { "resolution": [100, 100] }, 
  "camera": { 
    "position": [0.0, 1.0, 6.8], 
    "look_at": [0.0, 1.0, 0.0], 
    "ref_up": [0.0, 1.0, 0.0], 
    "fov": 19.5, 
    "focal_length": 1.0 }, 
	"textures": { "white": { "type":"constant", "color": [1.0, 1.0, 1.0] } },
  "materials": { "diffuse": { "type": "diffuse", "texture_name": "white" } }, 
  "objects": [ 
    { "type": "sphere", "center": [0.0, 1.4, 0.0], "radius": 0.3, "light": { "type": "area", "radiance": [5.0, 5.0, 5.0] } },
    { "type": "sphere", "center": [0.0, 0.0, 0.0], "radius": 1.0, "material_name": "diffuse" } ]
})";