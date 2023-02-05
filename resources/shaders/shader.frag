#version 440

// Outgoing vertex colour
layout (location = 0) out vec4 colour;

void main()
{
	// Set outgoing colour to red
	colour = vec4(1.0, 0.0, 0.0, 1.0);
}