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
float calculate_fog(in float fog_coord, in vec4 fog_colour, in float fog_start, in float fog_end, in float fog_density, in int fog_type);


// Directional light for the scene
uniform directional_light light;

// Material of the object
uniform material mat;
// Position of the camera
uniform vec3 eye_pos;

uniform sampler2D tex;
uniform sampler2D dissolve;

//Dissolve factor value
uniform float dissolve_factor;

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

// Incoming texture coordinates
layout (location = 0) in vec2 tex_coord;

// Incoming vertex colour
//layout (location = 0) in vec4 in_colour;

// Incoming vertex position
layout (location = 2) in vec3 position;
// Incoming normal
layout (location = 1) in vec3 normal;

// Camera space position
layout(location = 6) in vec4 CS_position;

// Outgoing pixel colour
layout (location = 0) out vec4 out_colour;

void main()
{
	// Calculate view direction
	vec3 view_dir = normalize(eye_pos - position);

	// Get dissolve value from the dissolve texture
	vec4 dissolve_value = texture(dissolve, tex_coord);

	// If b component is greater than dissolve factor, discard
	if (dissolve_value.b > dissolve_factor)
		discard;

	// Get texture colour
	vec4 tex_colour = texture(tex, tex_coord);

	out_colour = calculate_direction(light, mat, normal, view_dir, tex_colour);
	// Calculate fog coord
	// - convert from homogeneous
	// - ensure value is positive (we want the size of the value)
	float fog_coord = abs(CS_position.z / CS_position.w);

	// Calculate fog factor
	float fog_factor = calculate_fog(fog_coord, fog_colour, fog_start, fog_end, fog_density, fog_type);

	// Colour is mix between colour and fog colour based on factor
	out_colour = mix(out_colour, fog_colour, fog_factor);
	//out_colour = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	out_colour.a = 1.0f;
}