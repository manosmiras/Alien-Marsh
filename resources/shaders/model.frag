#version 440

// A directional light structure
#ifndef DIRECTIONAL_LIGHT
#define DIRECTIONAL_LIGHT
struct directional_light
{
	vec4 ambient_intensity;
	vec4 light_colour;
	vec3 light_dir;
};
#endif

// Point light structure
#ifndef POINT_LIGHT
#define POINT_LIGHT
struct point_light
{
	vec4 light_colour;
	vec3 position;
	float constant;
	float linear;
	float quadratic;
};
#endif

// Spot light structure
#ifndef SPOT_LIGHT
#define SPOT_LIGHT
struct spot_light
{
	vec4 light_colour;
	vec3 position;
	vec3 direction;
	float constant;
	float linear;
	float quadratic;
	float power;
};
#endif

#ifndef MATERIAL
#define MATERIAL
// A material structure
struct material
{
	vec4 emissive;
	vec4 diffuse_reflection;
	vec4 specular_reflection;
	float shininess;
};
#endif

// Forward declarations of used functions
vec4 calculate_direction(in directional_light light, in material mat, in vec3 normal, in vec3 view_dir, in vec4 tex_colour);
vec4 calculate_point(in point_light point, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour);
vec4 calculate_spot(in spot_light spot, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour);
vec3 calc_normal(in vec3 normal, in vec3 tangent, in vec3 binormal, in sampler2D normal_map, in vec2 tex_coord);
float calculate_fog(in float fog_coord, in vec4 fog_colour, in float fog_start, in float fog_end, in float fog_density, in int fog_type);

// Directional light for the scene
uniform directional_light light;
// Point lights being used in the scene
uniform point_light points[2];
// Spot lights being used in the scene
uniform spot_light spots[2];
// Material of the object
uniform material mat;
// Position of the camera
uniform vec3 eye_pos;
// Textures
uniform sampler2D tex;
// Normal map to sample from
uniform sampler2D normal_map;

// Fog colour
uniform vec4 fog_colour;
// Fog start position
uniform float fog_start;
// Fog end position
uniform float fog_end;
// Fog density
uniform float fog_density;
// Fog type
uniform int fog_type;

// Incoming vertex position
layout (location = 0) in vec3 position;
// Incoming normal
layout (location = 1) in vec3 normal;
// Incoming tex_coord
layout (location = 2) in vec2 tex_coord;

// Outgoing tangent
layout(location = 4) in vec3 tangent;
// Outgoing binormal
layout(location = 5) in vec3 binormal;

// Camera space position
layout(location = 6) in vec4 CS_position;

// Outgoing colour
layout (location = 0) out vec4 colour;

void main()
{
	//// Calculate ambient component
	//vec4 ambient = mat.diffuse_reflection * light.ambient_intensity;
	//// Calculate diffuse component
	//vec4 diffuse = (mat.diffuse_reflection * light.light_colour) * max(dot(normal, light.light_dir), 0);
	//// Calculate view direction
	vec3 view_dir = normalize(eye_pos - position);
	//// Calculate half vector
	//vec3 half_vector = normalize(light.light_dir + view_dir);
	//// Calculate specular component
	//vec4 specular = (mat.specular_reflection * light.light_colour) * pow(max(dot(normal, half_vector), 0), mat.shininess);

	// **************
	// Get tex colour
	// **************
	vec4 tex_colour = texture(tex, tex_coord);  //vec4(0.0, 0.0, 0.0, 1.0); // Change!!!
	vec3 calculated_normal = calc_normal(normal, tangent, binormal, normal_map, tex_coord);
	// Calculate primary colour component
	//vec4 primary = mat.emissive + ambient + diffuse;
	colour = calculate_direction(light, mat, calculated_normal, view_dir, tex_colour);

	// Sum point lights
	for (int i = 0; i < 2; ++i)
		colour += calculate_point(points[i], mat, position, calculated_normal, view_dir, tex_colour);
	for (int i = 0; i < 2; ++i)
		colour += calculate_spot(spots[i], mat, position, calculated_normal, view_dir, tex_colour);

	// Calculate final colour
	//colour = primary * tex_colour + specular;
	

	// **********************************************************
	// Calculate fog coord
	// - convert from homogeneous
	// - ensure value is positive (we want the size of the value)
	// **********************************************************
	float fog_coord = abs(CS_position.z / CS_position.w);
	// ********************
	// Calculate fog factor
	// ********************
	float fog_factor = calculate_fog(fog_coord, fog_colour, fog_start, fog_end, fog_density, fog_type);
	// ***********************************************************
	// Colour is mix between colour and fog colour based on factor
	// ***********************************************************
	colour = mix(colour, fog_colour, fog_factor);
	//clamp(colour, 0.0f, 1.0f);
	colour.a = 1.0f;
}