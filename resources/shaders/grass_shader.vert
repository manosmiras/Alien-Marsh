#version 440

// Model-view transformation
uniform mat4 MV;

// Incoming position data
layout (location = 0) in vec3 position;

// Camera space position
layout(location = 6) out vec4 CS_position;

void main()
{
    // Transform position into camera space
    gl_Position = MV * vec4(position, 1.0);
}