#version 440

// Texture to sample from
uniform sampler2D tex;

// Incoming primary colour
layout (location = 0) in vec4 primary;
// Incoming secondary colour
layout (location = 1) in vec4 secondary;

// Outgoing colour
layout (location = 0) out vec4 colour;

void main()
{
	// ****************
	// Calculate colour
	// ****************
	colour = primary + secondary;
}