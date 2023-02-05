#version 440

// Forward declarations of used functions
float calculate_fog(in float fog_coord, in vec4 fog_colour, in float fog_start, in float fog_end, in float fog_density, in int fog_type);

// Cubemap texture
uniform samplerCube cubemap;

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

// Camera space position
layout(location = 6) in vec4 CS_position;

// Incoming 3D texture coordinate
layout (location = 0) in vec3 tex_coord;

// Outgoing colour
layout (location = 0) out vec4 colour;

void main()
{
	// Calculate fog coord
	// - convert from homogeneous
	// - ensure value is positive (we want the size of the value)
	float fog_coord = abs(CS_position.z / CS_position.w);

	// Calculate fog factor
	float fog_factor = calculate_fog(fog_coord, fog_colour, fog_start, fog_end, fog_density, fog_type);

	// Sample texture
	colour = texture(cubemap, tex_coord);

	// Colour is mix between colour and fog colour based on factor
	colour = mix(colour, fog_colour, fog_factor);
}