#version 440

// forward declaration
//vec4 dissolve(in sampler2D dissolve_texture, in vec2 tex_coord, in float dissolve_factor);

//uniform sampler2D tex;

// *****************************
// Declare dissolve factor value
// *****************************

//uniform float dissolve_factor;

// Incoming texture coordinates
//layout (location = 0) in vec2 tex_coord;

// Model view projection matrix
uniform mat4 MVP;

// Offset position
uniform vec3 offset;

// Layout of incoming data
layout (triangles) in;
// Layout of outgoing data
layout (triangle_strip, max_vertices = 63) out;

// Incoming colour
in vec4 vFragColorVs[];


// Outgoing colour for the vertex
layout (location = 0) out vec4 colour_out;

void main()
{
	for (int j = 1; j < 4; ++j)
	{
		// Starting triangle is in original position
		for (int i = 0; i < 3; ++i)
		{
			// Transform vertex into screen space
			gl_Position = MVP * gl_in[i].gl_Position;
			// Starting quad is red
			colour_out = vFragColorVs[i]; //dissolve(tex, tex_coord, dissolve_factor); //vec4(1.0, 0.0, 0.0, 1.0);
			// Emit the vertex
			EmitVertex();
		}
		// Finished triangle
		EndPrimitive();

		// Move on z
		for (int i = 0; i < 3; ++i)
		{
			// Transform vertex into screen space
			gl_Position = MVP * vec4(gl_in[i].gl_Position.xyz + vec3(0.0f, 0.0f, 2.0f), 1.0f);
			// Starting quad is red
			colour_out = vFragColorVs[i]; //dissolve(tex, tex_coord, dissolve_factor); //vec4(1.0, 0.0, 0.0, 1.0);
			// Emit the vertex
			EmitVertex();
		}
		// Finished triangle
		EndPrimitive();

		// *******************************************
		// Emit a copy of the triangle moved by offset
		// Offset triangle needs to be green
		// *******************************************
		for (int i = 0; i < 3; ++i)
		{
			// Transform vertex into screen space
			gl_Position = MVP * vec4(gl_in[i].gl_Position.xyz + offset * j, 1);
			// Starting quad is red
			colour_out = vFragColorVs[i]; //vec4(0.0, 1.0, 0.0, 1.0);
			// Emit the vertex
			EmitVertex();
		}
		// Finished triangle
		EndPrimitive();

		for (int i = 0; i < 3; ++i)
		{
			// Transform vertex into screen space
			gl_Position = MVP * vec4(gl_in[i].gl_Position.xyz + offset * j + vec3(0.0f, 0.0f, 2.0f), 1);
			// Starting quad is red
			colour_out = vFragColorVs[i]; //vec4(0.0, 1.0, 0.0, 1.0);
			// Emit the vertex
			EmitVertex();
		}
		// Finished triangle
		EndPrimitive();

		// ****************************************************
		// Emit a copy of the triangle moved by negative offset
		// Offset triangle needs to be blue
		// ****************************************************
		for (int i = 0; i < 3; ++i)
		{
			vec3 position = gl_in[i].gl_Position.xyz + offset * (j + 2);
			// Transform vertex into screen space
			gl_Position = MVP * vec4(position, 1);
			// Starting quad is red
			colour_out = vFragColorVs[i]; //vec4(0.0, 0.0, 1.0, 1.0);
			// Emit the vertex
			EmitVertex();
		}

		EndPrimitive();

		for (int i = 0; i < 3; ++i)
		{
			vec3 position = gl_in[i].gl_Position.xyz + offset * (j + 2) + vec3(0.0f, 0.0f, 2.0f);
			// Transform vertex into screen space
			gl_Position = MVP * vec4(position, 1);
			// Starting quad is red
			colour_out = vFragColorVs[i]; //vec4(0.0, 0.0, 1.0, 1.0);
			// Emit the vertex
			EmitVertex();
		}
		EndPrimitive();

		for (int i = 0; i < 3; ++i)
		{
			vec3 position = gl_in[i].gl_Position.xyz + offset * (j + 3);
			// Transform vertex into screen space
			gl_Position = MVP * vec4(position, 1);
			// Starting quad is red
			colour_out = vFragColorVs[i]; //vec4(0.0, 0.0, 1.0, 1.0);
			// Emit the vertex
			EmitVertex();
		}

		EndPrimitive();

		for (int i = 0; i < 3; ++i)
		{
			vec3 position = gl_in[i].gl_Position.xyz + offset * (j + 3) + vec3(0.0f, 0.0f, 2.0f);
			// Transform vertex into screen space
			gl_Position = MVP * vec4(position, 1);
			// Starting quad is red
			colour_out = vFragColorVs[i]; //vec4(0.0, 0.0, 1.0, 1.0);
			// Emit the vertex
			EmitVertex();
		}
		EndPrimitive();
		for (int i = 0; i < 3; ++i)
		{
			vec3 position = gl_in[i].gl_Position.xyz + offset * (j + 4);
			// Transform vertex into screen space
			gl_Position = MVP * vec4(position, 1);
			// Starting quad is red
			colour_out = vFragColorVs[i]; //vec4(0.0, 0.0, 1.0, 1.0);
			// Emit the vertex
			EmitVertex();
		}

		EndPrimitive();

		for (int i = 0; i < 3; ++i)
		{
			vec3 position = gl_in[i].gl_Position.xyz + offset * (j + 4) + vec3(0.0f, 0.0f, 2.0f);
			// Transform vertex into screen space
			gl_Position = MVP * vec4(position, 1);
			// Starting quad is red
			colour_out = vFragColorVs[i]; //vec4(0.0, 0.0, 1.0, 1.0);
			// Emit the vertex
			EmitVertex();
		}
		// Finished triangle
		EndPrimitive();
	}
}