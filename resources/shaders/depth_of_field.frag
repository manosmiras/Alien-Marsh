#version 410

// Previous pass capture
uniform sampler2D tex;
// Initial render capture
uniform sampler2D sharp;
// Initial render depth capture
uniform sampler2D depth;

// The range value of the dof
uniform float range;
// The focus distance of the dof
uniform float focus;
// Checks if effect is enabled
uniform bool render;

// Incoming texture coordinate
layout (location = 0) in vec2 tex_coord;

// Outgoing colour
layout (location = 0) out vec4 colour;

void main()
{
	// Sample sharp texture
	vec4 sharp_tex = texture(sharp, tex_coord);

	if (render)
	{
		// Sample blur texture
		vec4 blur_tex = texture(tex, tex_coord);

		// Calculate distance from the camera - based on depth sample
		float distance = texture(depth, tex_coord).z * range;

		// Mix samples together based on distance
		colour = mix(sharp_tex, blur_tex, distance);
	}
	else
	{
		colour = sharp_tex;
	}
    // Ensure alpha is 1.0
	colour.a = 1.0f;
}