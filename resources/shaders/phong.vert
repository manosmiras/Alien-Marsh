#version 440

// MVP transformation matrix
uniform mat4 MVP;
// M transformation matrix
uniform mat4 M;
// N transformation matrix
uniform mat3 N;

// Incoming position
layout (location = 0) in vec3 position;
// Incoming normal
layout (location = 2) in vec3 normal_input;

// Outgoing vertex position
layout (location = 0) out vec3 vertex_position;
// Transformed normal
layout (location = 1) out vec3 transformed_normal;



void main()
{
	vec3 normal = normalize(normal_input);
	// Calculate screen position
	gl_Position = MVP * vec4(position, 1.0f);
	// Calculate vertex world position
	vertex_position = (M * vec4(position, 1.0f)).xyz;
	// Transform normal
	transformed_normal = N * normal;
}