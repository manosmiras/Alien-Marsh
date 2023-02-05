#include "Node.h"

void Node::add_child(Node* child)
{
	this->m_children.push_back(child);
}

void Node::print_number_of_children()
{
	cout << "Number of children: " << this->m_children.size() << endl;
}


void Node::update_children(mat4 transform)
{

	m_worldTransform = (transform * m_transform);


	for (unsigned int i = 0; i < this->m_children.size(); i++)
	{
		m_children[i]->update_children(m_worldTransform);
	}
}

void Node::update_local(mat4 transform)
{
	m_worldTransform *= (transform * m_transform);
	//m_worldTransform = (transform * m_transform);
	//m_transform = transform * m_worldTransform;
}



void Node::render_children(mat4 view, mat4 proj, vec3 camPos, effect eff, directional_light light, vec4 fog_colour, float fog_start, float fog_end, float fog_density, int fog_type)
{
	auto M = m_worldTransform;
	auto V = view;
	auto P = proj;
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(
		eff.get_uniform_location("MVP"), // Location of uniform
		1, // Number of values - 1 mat4
		GL_FALSE, // Transpose the matrix?
		value_ptr(MVP)); // Pointer to matrix data


	glUniformMatrix4fv(
		eff.get_uniform_location("MV"), // Location of uniform
		1, // Number of values - 1 mat4
		GL_FALSE, // Transpose the matrix?
		value_ptr(V * M)); // Pointer to matrix data


	// ********************
	// Set M matrix uniform
	// ********************
	glUniformMatrix4fv(
		eff.get_uniform_location("M"), // Location of uniform
		1, // Number of values - 1 mat4
		GL_FALSE, // Transpose the matrix?
		value_ptr(M)); // Pointer to matrix data

	// ***********************
	// Set N matrix uniform
	// - remember - 3x3 matrix
	// ***********************
	auto N = m_mesh.get_transform().get_normal_matrix();

	glUniformMatrix3fv(
		eff.get_uniform_location("N"), // Location of uniform
		1, // Number of values - 1 mat4
		GL_FALSE, // Transpose the matrix?
		value_ptr(N)); // Pointer to normal matrix data

	// **************
	// Set fog colour
	// **************
	glUniform4fv(eff.get_uniform_location("fog_colour"), 1, value_ptr(fog_colour));
	// *************
	// Set fog start
	// *************
	glUniform1f(eff.get_uniform_location("fog_start"), fog_start);
	// ***********
	// Set fog end
	// ***********
	glUniform1f(eff.get_uniform_location("fog_end"), fog_end);
	// ***************
	// Set fog density
	// ***************
	glUniform1f(eff.get_uniform_location("fog_density"), fog_density);
	// ************
	// Set fog type
	// ************
	glUniform1i(eff.get_uniform_location("fog_type"), fog_type);

	// *************
	// Bind material
	// *************
	renderer::bind(m_mesh.get_material(), "mat");

	// **********
	// Bind light
	// **********
	renderer::bind(light, "light");

	// ************
	// Bind texture
	// ************
	renderer::bind(m_tex, 0);

	// ***************
	// Set tex uniform
	// ***************
	glUniform1i(eff.get_uniform_location("tex"), 0);
	// Mirror texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); // width
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); // height

	// *****************************
	// Set eye position
	// - Get this from active camera
	// *****************************
	glUniform3f(eff.get_uniform_location("eye_pos"), camPos.x, camPos.y, camPos.z);

	// Render mesh
	renderer::render(m_mesh);


	for (unsigned int i = 0; i < this->m_children.size(); i++)
	{
		m_children[i]->render_children(view, proj, camPos, eff, light, fog_colour, fog_start, fog_end, fog_density, fog_type);
	}

}


Node::Node()
{
	m_transform = mat4(1.0f);
}

Node::~Node()
{

}