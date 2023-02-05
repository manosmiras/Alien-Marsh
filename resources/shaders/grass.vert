#version 440

// MVP transformation matrix
uniform mat4 MVP;
// MV transformation
uniform mat4 MV;
// M transformation matrix
uniform mat4 M;
// N transformation matrix
uniform mat3 N;

// Vertex position
layout (location = 0) in vec3 position;

// Incoming normal
layout (location = 2) in vec3 normal_input;

layout (location = 10) in vec2 tex_coord_in;


// Outgoing vertex position
layout (location = 2) out vec3 vertex_position;
// Transformed normal
layout (location = 1) out vec3 transformed_normal;

layout (location = 0) out vec2 tex_coord_out;

// Camera space position
layout(location = 6) out vec4 CS_position;

void main()
{
	// Pass through position to geometry shader
	gl_Position = vec4(position, 1.0);

	// Normalize normal
	vec3 normal = normalize(normal_input);

	// Calculate vertex world position
	vertex_position = vec3(M * vec4(position, 1.0f));

	// Transform normal
	transformed_normal = N * normal;

	CS_position = MV * vec4(position, 1.0);

	// Pass through texture coordinate
	tex_coord_out = tex_coord_in;
}