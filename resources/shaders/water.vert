#version 440

// MVP transformation matrix
uniform mat4 MVP;
// MV transformation
uniform mat4 MV;
// M transformation matrix
uniform mat4 M;
// N transformation matrix
uniform mat3 N;

// For wave calculation
uniform float waveTime;
uniform float waveWidth;
uniform float waveHeight;

// Eye position
uniform vec3 eye_pos;

// Incoming position
layout (location = 0) in vec3 position;
// Incoming normal
layout (location = 2) in vec3 normal_input;
// Incoming texture coordinate
layout (location = 10) in vec2 tex_coord;
// Incoming binormal
layout(location = 3) in vec3 binormal;
// Incoming tangent
layout(location = 4) in vec3 tangent;


// Outgoing vertex position
layout (location = 0) out vec3 vertex_position;
// Transformed normal
layout (location = 1) out vec3 transformed_normal;
// Outgoing tex_coord
layout (location = 2) out vec2 vertex_tex_coord;
// Outgoing tangent
layout(location = 4) out vec3 tangent_out;
// Outgoing binormal
layout(location = 5) out vec3 binormal_out;
// Camera space position
layout(location = 6) out vec4 CS_position;

vec4 calculateWavePosition(in float waveTime, in float waveWidth, in float waveHeight, in vec3 position, in mat4 MVP);

void main()
{
	vec4 wavePosition = calculateWavePosition(waveTime, waveWidth, waveHeight, position, MVP);
	vec3 normal = normalize(normal_input);
	// Calculate screen position
	gl_Position = wavePosition;
	// Calculate vertex world position
	vertex_position = vec3(M * vec4(position, 1.0f));
	// Transform normal
	transformed_normal = N * normal;
	// Pass through tex_coord
	vertex_tex_coord = tex_coord;

	CS_position = MV * vec4(position, 1.0);

	// Transform tangent
	tangent_out = N * tangent;

	// Transform binormal
	binormal_out = N * binormal;

}