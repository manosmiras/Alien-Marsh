#version 440

// Incoming texture containing frame information
uniform sampler2D tex;

// Our colour filter - calculates colour intensity
uniform vec3 intensity;

// Checks if effect is enabled
uniform bool render;

// Incoming texture coordinate
layout (location = 0) in vec2 tex_coord;

// Outgoing colour
layout (location = 0) out vec4 colour;

void main()
{
    // Sample texture colour
    vec4 sample_colour = texture(tex, tex_coord);

	if (render)
	{
		// Calculate grey value
		float greyscale = dot(intensity, sample_colour.xyz);

		// Use greyscale to as final colour
		// - ensure alpha is 1
		colour = vec4(greyscale, greyscale, greyscale, 1.0f);
	}
	else
	{
		colour = sample_colour;
	}
}