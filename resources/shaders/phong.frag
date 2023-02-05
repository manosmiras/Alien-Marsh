#version 440

// Requires weighted_texture.frag

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
vec4 calculate_direction(in directional_light light, in material mat, in vec3 normal, in vec3 view_dir);

// Directional light for the scene
uniform directional_light light;

// Material of the object
uniform material mat;
// Position of the camera
uniform vec3 eye_pos;

// Incoming vertex position
layout (location = 0) in vec3 position;
// Incoming normal
layout (location = 1) in vec3 normal;

// Outgoing colour
layout (location = 0) out vec4 colour;

void main()
{
	vec3 view_dir = normalize(eye_pos - position);

	// Calculate primary colour component
	colour = calculate_direction(light, mat, normal, view_dir);

	colour.a = 1.0f;

}