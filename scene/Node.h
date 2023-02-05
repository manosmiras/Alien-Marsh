#ifndef NODE_H
#define NODE_H

#include <graphics_framework.h>
#include <glm\glm.hpp>

using namespace std;
using namespace graphics_framework;
using namespace glm;

class Node
{
public:
	Node();
	~Node();
	mat4 m_transform;
	mat4 m_worldTransform;
	mesh m_mesh;
	texture m_tex;
	vector<Node*> m_children; // Children of node.

	void add_child(Node* child);
	void print_number_of_children();
	void update(mat4 transform);
	void update_children(mat4 transform);
	void update_local(mat4 transform);
	void render_children(mat4 view, mat4 proj, vec3 camPos, effect eff, directional_light light, vec4 fog_colour, float fog_start, float fog_end, float fog_density, int fog_type);

private:

};

#endif