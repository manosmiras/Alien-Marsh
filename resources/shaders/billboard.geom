#version 440

// The projection transformation
uniform mat4 P;

// Point size for the billboards
uniform float point_size;

// Incoming data
layout (points) in;
// Outgoing data
layout (triangle_strip, max_vertices = 4) out;

layout(location = 6) out vec4 CS_position_out;

// Outgoing texture coordinate
layout (location = 0) out vec2 tex_coord;

void main()
{
    // Incoming position
    vec4 position = gl_in[0].gl_Position;
	// Set outgoing CS_position
	CS_position_out = position;
    // *********************************************************
    // Process is:
    // 1. Calculate position in camera space (position + offset)
    //     a. Multiply the offset by the point size for scaling
    // 2. Transform into camera space for gl_Position
    // 3. Set appropriate texture coordinate
    // 4. Emit
    // *********************************************************
	

    // ***********************
    // Vertex 1 is bottom left
    // ***********************
	vec4 v1 = position + vec4(-0.5f, -0.5f, 0.0f, 0.0f) * point_size;
	gl_Position = P * v1;
	tex_coord = vec2(0.0f, 0.0);

	EmitVertex();

    // ************************
    // Vertex 2 is bottom right
    // ************************
    vec4 v2 = position + vec4(0.5f, -0.5f, 0.0f, 0.0f) * point_size;
	gl_Position = P * v2;
	tex_coord = vec2(1.0f, 0.0f);

	EmitVertex();

    // ********************
    // Vertex 3 is top left
    // ********************
    vec4 v3 = position + vec4(-0.5f, 0.5f, 0.0f, 0.0f) * point_size;
	gl_Position = P * v3;
	tex_coord = vec2(0.0f, 1.0f);

	EmitVertex();

    // *********************
    // Vertex 4 is top right
    // *********************
    vec4 v4 = position + vec4(0.5f, 0.5f, 0.0f, 0.0f) * point_size;
	gl_Position = P * v4;
	tex_coord = vec2(1.0f, 1.0f);

	EmitVertex();

    // End Primitive
    EndPrimitive();
}