#version 440

//Dissolve factor value
uniform float dissolve_factor;
uniform sampler2D dissolve;

// Texture to use on billboards
uniform sampler2D tex;

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

// Incoming texture coordinate
layout (location = 0) in vec2 tex_coord;
// Camera space position
layout(location = 6) in vec4 CS_position;
// Outgoing colour
layout (location = 0) out vec4 colour;

// Forward decleration of used functions
float calculate_fog(in float fog_coord, in vec4 fog_colour, in float fog_start, in float fog_end, in float fog_density, in int fog_type);



void main()
{
	// Get dissolve value from the dissolve texture
	vec4 dissolve_value = texture(dissolve, tex_coord);

	// If b component is greater than dissolve factor, discard
	if (dissolve_value.b > dissolve_factor)
		discard;

	// Calculate fog coord
	// - convert from homogeneous
	// - ensure value is positive (we want the size of the value)
	float fog_coord = abs(CS_position.z / CS_position.w);

	// Calculate fog factor
	float fog_factor = calculate_fog(fog_coord, fog_colour, fog_start, fog_end, fog_density, fog_type);

	// Get texture colour
	colour = texture(tex, tex_coord);
	colour =  mix(colour, fog_colour, fog_factor);
}