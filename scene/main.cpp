#include <graphics_framework.h>
#include <glm/glm.hpp>
#include "Node.h" // For Basic Hierarchy

using namespace std;
using namespace graphics_framework;
using namespace glm;

// Types of fog
#define FOG_LINEAR 0
#define FOG_EXP 1
#define FOG_EXP2 2

//string currentBuffer = "greyscale";

geometry grassGeom; // Geometry for grass
geometry screen_quad; // A quad in front of the screen to display post processing effects

// Effects
effect normals_debug_eff;
effect terrain_eff; // Effect for procedural terrain generation
effect sky_eff; // Effect for skybox
effect water_eff; // Effect for water
effect model_eff; // Effect for models
effect grass_eff; // Effect for grass
effect gouraud_eff; // Effect for gouraud shading
effect phong_eff; // Effect for phong shading
effect reflection_eff; // Effect for water reflection
effect greyscale_eff; // Effect for greyscale
effect blur_eff; // Effect for blur
effect motion_blur_eff; // Effect for motion blur
effect tex_eff; // Simple texturing effect
effect dof_eff; // Effect for depth of field
effect vignette_eff; // Effect for vignette

bool greyscaleVisible = false;
bool motionBlurVisible = true;
bool depthOfFieldVisible = true;
bool vignetteVisible = true;

float range = 1.0f;
float focus = 0.3f;

// Meshes
mesh terr; // Terrain mesh
mesh skybox; // Skybox mesh
mesh water; // Water mesh
map<string, mesh> meshesHierarchy; // Meshes for Hierarchy
map<string, mesh> meshes; // Other meshes
//mesh spaceFighter;
map<string, mesh> grassMeshes; // Meshes for grass
map<string, mesh> gouraudMeshes; // Meshes using gouraud shading only
map<string, mesh> phongMeshes; // Meshes using phong shading only

mesh render_cube;

// Nodes, for Hiearchy
Node root;
Node child1;
Node child2;

// Cube map
cubemap cube_map; // Cube map for skybox

// Cameras
free_camera cam; // Free camera, W, A, S, D movement
target_camera target_cam; // Target camera, 4 diferrent angles
chase_camera chase_cam; // Chase camera, chasing spaceFighter

// Lights
directional_light light; // Directional light
vector<point_light> points(2); // Point lights
vector<spot_light> spots(2); // Spot lights

// Textures
texture terrainTex[4]; // Textures for terrain
texture waterTex; // Texture for water
texture textures[7]; // Textures for models
texture childrenTextures[3]; // Textures used in Hiearchy rendering
texture grassTex; // Texture for grass
texture grassDissolve; // Dissolve texture
texture tex;
texture alpha_map; // Alpha map for vignette effect

// Normal maps
texture terrain_normal_map; // Normal map for terrain
texture water_normal_map; // Normal map for water
texture spaceFighter_normal_map; // Normal map for spaceFighter model
texture tree_normal_map; // Normal map for trees

vec3 xrot(1.0f, 0.0f, 0.0f); // X axis rotation
vec3 yrot(0.0f, 1.0f, 0.0f); // Y axis rotation
vec3 pos; // Used for changing position of objects based on a sine wave
vec3 sc; // Used for changing scale of objects based on a sine wave

const vec4 fog_colour = vec4(1.00f, 1.00f, 1.00f, 1.00f); //vec4(1.00f, 0.68f, 0.28f, 1.0f); // Colour of fog on the scene
vec4 light_colour = vec4(1.0f, 1.0f, 1.0f, 1.0f); // Colour of directional light on the scene
vec4 ambient_light_colour = vec4(0.5f, 0.5f, 0.5f, 1.0f); // Colour of ambient component of directional light on the scene

const float fog_start = 80.0f;
const float fog_end = 400.0f;
float fog_density = 0.025f;

// Dissolve factor to set on grass shader
float dissolve_factor = 0.7f; // Determines how much the grass meshes dissolve

float theta = 20.0f; // Used for rotation
float rho = 0.0f; // Used for rotation
float total_time = 0.0f;

// Variables used for movement of water
GLfloat waveTime = 0.5f,
	waveWidth = 0.85f,
	waveHeight = 0.25f,
	waveFreq = 0.005f;

quat rx, ry; // For torus rotation

// Initial mouse input
double cursor_x = 0.0;
double cursor_y = 0.0;

string currentCamera = "free"; // Camera Identifier, initiliase with free

frame_buffer greyFrame;
frame_buffer blurFrame;
frame_buffer vignetteMask;
// Frames for motion blur
frame_buffer frames[2];
frame_buffer temp_frame;
unsigned int current_frame = 0;

// Frames for depth of field
frame_buffer first_pass;
frame_buffer temp_frames[2];

material terrainmat;

// Contains terrain position data
vector<vec3> positions;
// Contains grass position data
vector<vec3> grassPositions;

bool initialise()
{
	// Set input mode - hide the cursor
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Capture initial mouse position
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);

	return true;
}

void generate_terrain(geometry &geom, const texture &height_map, unsigned int width, unsigned int depth, float height_scale)
{
	// Contains our normal data
	vector<vec3> normals;
	// Contains our texture coordinate data
	vector<vec2> tex_coords;
	// Contains our texture weights
	vector<vec4> tex_weights;
	// Contains our index data
	vector<unsigned int> indices;
	// For normal mapping
	vector<vec3> binormals;
	vector<vec3> tangents;

	// ***************************************
	// Extract the texture data from the image
	// ***************************************
	glBindTexture(GL_TEXTURE_2D, height_map.get_id());
	auto data = new vec4[height_map.get_width() * height_map.get_height()];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, (void*)data);

	// Determine ratio of height map to geometry
	float width_point = static_cast<float>(width) / static_cast<float>(height_map.get_width());
	float depth_point = static_cast<float>(depth) / static_cast<float>(height_map.get_height());

	// Point to work on
	vec3 point;

	// ***********************************************************************
	// Part 1 - Iterate through each point, calculate vertex and add to vector
	// ***********************************************************************
	for (unsigned int x = 0; x < height_map.get_width(); ++x)
	{
		// *****************************
		// Calculate x position of point
		// *****************************
		point.x = -(width / 2.0f) + (width_point * static_cast<float>(x));

		for (unsigned int z = 0; z < height_map.get_height(); ++z)
		{
			// *****************************
			// Calculate z position of point
			// *****************************
			point.z = -(depth / 2.0f) + (depth_point * static_cast<float>(z));
			// ****************************************************
			// Y position based on red component of height map data
			// ****************************************************
			point.y = data[(z * height_map.get_width()) + x].y * height_scale;
			// **************************
			// Add point to position data
			// **************************
			positions.push_back(point);
		}
	}

	// ***********************
	// Part 1 - Add index data
	// ***********************
	for (unsigned int x = 0; x < height_map.get_width() - 1; ++x)
	{
		for (unsigned int y = 0; y < height_map.get_height() - 1; ++y)
		{
			// *************************
			// Get four corners of patch
			// *************************
			unsigned int top_left = (y * height_map.get_width()) + x;
			unsigned int top_right = (y * height_map.get_width()) + x + 1;
			unsigned int bottom_left = ((y + 1) * height_map.get_width()) + x;
			unsigned int bottom_right = ((y + 1) * height_map.get_width()) + x + 1;
			// ********************************
			// Push back indices for triangle 1
			// ********************************
			indices.push_back(top_left);
			indices.push_back(bottom_right);
			indices.push_back(bottom_left);
			// ********************************
			// Push back indices for triangle 2
			// ********************************
			indices.push_back(top_left);
			indices.push_back(top_right);
			indices.push_back(bottom_right);
		}
	}

	// Resize the normals buffer
	normals.resize(positions.size());

	// *********************************************
	// Part 2 - Calculate normals for the height map
	// *********************************************
	for (unsigned int i = 0; i < indices.size() / 3; ++i)
	{
		// ****************************
		// Get indices for the triangle
		// ****************************
		auto idx1 = indices[i * 3];
		auto idx2 = indices[i * 3 + 1];
		auto idx3 = indices[i * 3 + 2];

		// ***********************************
		// Calculate two sides of the triangle
		// ***********************************
		vec3 side1 = positions[idx1] - positions[idx3];
		vec3 side2 = positions[idx1] - positions[idx2];

		// ******************************************
		// Normal is cross product of these two sides
		// ******************************************
		vec3 normal = normalize(cross(side2, side1));

		// **********************************************************************
		// Add to normals in the normal buffer using the indices for the triangle
		// **********************************************************************
		normals[idx1] += normal;
		normals[idx2] += normal;
		normals[idx3] += normal;
	}

	binormals.resize(normals.size());
	tangents.resize(normals.size());

	// *************************
	// Part 2 - Normalize all the normals
	// *************************

	// Calculate binormals and tangents
	for (unsigned int i = 0; i < normals.size(); i++)
	{
		binormals[i] = normalize(cross(normals[i], vec3(0.0f, 0.0f, 1.0f)));
		tangents[i] = normalize(cross(normals[i], vec3(1.0f, 0.0f, 0.0f)));
	}

	for (auto &n : normals)
	{
		n = normalize(n);
	}

	// *********************************************
	// Part 3 - Add texture coordinates for geometry
	// *********************************************
	for (unsigned int x = 0; x < height_map.get_width(); ++x)
	{
		for (unsigned int z = 0; z < height_map.get_height(); ++z)
		{
			tex_coords.push_back(vec2(width_point * x / 2, depth_point * z / 2));
		}
	}

	// **************************************************
	// Part 4 - Calculate texture weights for each vertex
	// **************************************************
	for (unsigned int x = 0; x < height_map.get_width(); ++x)
	{
		for (unsigned int z = 0; z < height_map.get_height(); ++z)
		{
			// ********************
			// Calculate tex weight
			// ********************
			vec4 tex_weight(
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.0f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.15f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.5f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.9f) / 0.25f, 0.0f, 1.0f));

			// ********************************
			// Sum the components of the vector
			// ********************************
			auto total = tex_weight.x + tex_weight.y + tex_weight.z + tex_weight.w;

			// ********************
			// Divide weight by sum
			// ********************
			tex_weight /= total;

			// *************************
			// Add tex weight to weights
			// *************************
			tex_weights.push_back(tex_weight);
		}
	}

	// *************************************
	// Add necessary buffers to the geometry
	// *************************************
	geom.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	geom.add_buffer(normals, BUFFER_INDEXES::NORMAL_BUFFER);
	geom.add_buffer(binormals, BUFFER_INDEXES::BINORMAL_BUFFER);
	geom.add_buffer(tangents, BUFFER_INDEXES::TANGENT_BUFFER);
	geom.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	geom.add_buffer(tex_weights, BUFFER_INDEXES::TEXTURE_COORDS_1);
	geom.add_index_buffer(indices);

	// ***********
	// Delete data
	// ***********
	delete[] data;
}

void generate_grass()
{
	default_random_engine e;
	uniform_int_distribution<int> dist(0, 1000);

	for each (vec3 pos in positions)
	{

		if (dist(e) >= 997 && pos.y < 2.0f)
		{
			grassPositions.push_back(vec3(pos.x, pos.y + 0.5f, pos.z));

		}

	}
	// Create geometry using these points
	grassGeom.add_buffer(grassPositions, BUFFER_INDEXES::POSITION_BUFFER);
	// Set geometry type to points
	grassGeom.set_type(GL_POINTS);
}

bool load_content()
{

	// Create frame buffers
	greyFrame = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	blurFrame = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	vignetteMask = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	frames[0] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	frames[1] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	temp_frame = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());

	// Create frame buffers - use screen width and height
	for (unsigned int i = 0; i < 2; ++i)
		temp_frames[i] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	first_pass = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());

	// Cube to render to
	render_cube = mesh(geometry_builder::create_box());
	render_cube.get_transform().scale = vec3(5.0f, 5.0f, 5.0f);

	// GREYSCALE

	screen_quad.set_type(GL_QUADS);
	// Screen quad
	// - positions
	// - tex coords
	vector<vec3> SQPositions
	{
		vec3(1.0f, 1.0f, 0.0f),
		vec3(-1.0f, 1.0f, 0.0f),
		vec3(-1.0f, -1.0f, 0.0f),
		vec3(1.0f, -1.0f, 0.0f)
	};
	vector<vec2> SQTex_coords
	{
		vec2(1.0f, 1.0f),
		vec2(0.0f, 1.0f),
		vec2(0.0f, 0.0f),
		vec2(1.0f, 0.0f)
	};
	screen_quad.add_buffer(SQPositions, POSITION_BUFFER);
	screen_quad.add_buffer(SQTex_coords, TEXTURE_COORDS_0);

	// Set node relationships
	root.add_child(&child1);

	child1.add_child(&child2);

	// Load texture
	tex = texture("..\\resources\\textures\\checked.gif", true, true);

	meshes["spaceFighter"] = mesh(geometry("..\\resources\\models\\space_fighter.obj"));
	meshesHierarchy["sphere"] = mesh(geometry_builder::create_sphere(20, 20));
	meshesHierarchy["planet1"] = mesh(geometry_builder::create_sphere(20, 20));
	meshesHierarchy["planet2"] = mesh(geometry_builder::create_sphere(20, 20));

	//meshes["cylinder"] = mesh(geometry_builder::create_cylinder());
	meshes["torus"] = mesh(geometry_builder::create_torus());
	meshes["palm1"] = mesh(geometry("..\\resources\\models\\Hyophorbe_lagenicaulis.obj"));
	meshes["palm2"] = mesh(geometry("..\\resources\\models\\Hyophorbe_lagenicaulis.obj"));
	meshes["palm3"] = mesh(geometry("..\\resources\\models\\Hyophorbe_lagenicaulis.obj"));
	meshes["palm4"] = mesh(geometry("..\\resources\\models\\Hyophorbe_lagenicaulis.obj"));

	gouraudMeshes["sphere"] = mesh(geometry_builder::create_sphere(20, 20));
	phongMeshes["sphere"] = mesh(geometry_builder::create_sphere(20, 20));
	
	meshesHierarchy["sphere"].get_transform().translate(vec3(0.0f, 2.5f, 0.0f));
	meshesHierarchy["planet1"].get_transform().scale = vec3(10.0f, 10.0f, 10.0f);
	meshesHierarchy["planet2"].get_transform().scale = vec3(0.85f, 0.85f, 0.85f);
	meshesHierarchy["planet1"].get_transform().translate(vec3(0.0f, 2.5f, 100.0f));
	meshesHierarchy["planet2"].get_transform().translate(vec3(0.0f, 0.0f, 5.0f));

	gouraudMeshes["sphere"].get_transform().translate(vec3(-14.0f, 13.0f, -7.0f));
	phongMeshes["sphere"].get_transform().translate(vec3(-14.0f, 10.0f, -7.0f));

	meshes["torus"].get_transform().translate(vec3(0.0f, 5.0f, 0.0f));
	meshes["torus"].get_transform().scale *= 0.5f;

	meshes["palm1"].get_transform().scale *= 0.025f;
	meshes["palm1"].get_transform().translate(vec3(10.0f, 0.0f, 20.0f));

	meshes["palm2"].get_transform().scale *= 0.025f;
	meshes["palm2"].get_transform().translate(vec3(-20.0f, 1.0f, -15.0f));

	meshes["palm3"].get_transform().scale *= 0.025f;
	meshes["palm3"].get_transform().translate(vec3(18.0f, 1.5f, 5.0f));

	meshes["palm4"].get_transform().scale *= 0.025f;
	meshes["palm4"].get_transform().translate(vec3(32.0f, 0.0f, -5.0f));

	water = mesh(geometry_builder::create_plane());

	water.get_transform().position = vec3(0.0f, 0.35f, 0.0f);
	water.get_transform().scale = vec3(0.8f, 1.0f, 0.8f);
	meshes["spaceFighter"].get_transform().position = vec3(10.0f, 8.0f, 10.0f);
	meshes["spaceFighter"].get_transform().scale *= 0.01f;

	pos = meshes["torus"].get_transform().position;
	sc = meshes["torus"].get_transform().scale;

	light.set_ambient_intensity(ambient_light_colour);
	light.set_light_colour(light_colour);
	light.set_direction(normalize(vec3(1.0f, 1.0f, 1.0f)));

	// Set materials
	vec4 colour_black = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 colour_white = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float standardShininess = 40.0f;

	material standard(colour_black, colour_white, colour_white, standardShininess);

	//terr.get_material().set_shininess(1.0f);
	//terr.get_material().set_specular(vec4(0.3f, 0.3f, 0.3f, 0.3f));
	//terr.get_material().set_diffuse(vec4(0.3f, 0.3f, 0.3f, 0.3f));

	water.get_material().set_diffuse(vec4(0.7f, 0.7f, 0.7f, 1.0f));
	water.get_material().set_specular(colour_white);
	water.get_material().set_shininess(standardShininess);
	water.get_material().set_emissive(colour_black);

	meshes["spaceFighter"].get_material().set_diffuse(colour_white);
	meshes["spaceFighter"].get_material().set_specular(vec4(0.4f, 0.4f, 0.4f, 1.0f));
	meshes["spaceFighter"].get_material().set_shininess(1.0f);
	meshes["spaceFighter"].get_material().set_emissive(colour_black);

	meshes["torus"].set_material(standard);
	meshes["palm1"].set_material(standard);
	meshes["palm2"].set_material(standard);
	meshes["palm3"].set_material(standard);
	meshes["palm4"].set_material(standard);
	meshesHierarchy["planet1"].set_material(standard);
	meshesHierarchy["planet2"].set_material(standard);

	gouraudMeshes["sphere"].get_material().set_diffuse(vec4(0.7f, 0.5f, 0.05f, 1.0f));
	gouraudMeshes["sphere"].get_material().set_specular(colour_white);
	gouraudMeshes["sphere"].get_material().set_shininess(standardShininess);
	gouraudMeshes["sphere"].get_material().set_emissive(colour_black);

	phongMeshes["sphere"].get_material().set_diffuse(vec4(0.7f, 0.5f, 0.05f, 1.0f));
	phongMeshes["sphere"].get_material().set_specular(colour_white);
	phongMeshes["sphere"].get_material().set_shininess(standardShininess);
	phongMeshes["sphere"].get_material().set_emissive(colour_black);

	terrainTex[0] = texture("..\\resources\\textures\\beach_sand.jpg", true, true);
	terrainTex[1] = texture("..\\resources\\textures\\grass.png", true, true);
	terrainTex[2] = texture("..\\resources\\textures\\rock.dds", true, true);
	terrainTex[3] = texture("..\\resources\\textures\\grass.png", true, true);
	waterTex = texture("..\\resources\\textures\\water.jpg", true, true);

	root.m_transform = meshesHierarchy["sphere"].get_transform().get_transform_matrix();
	child1.m_transform = meshesHierarchy["planet1"].get_transform().get_transform_matrix();
	child2.m_transform = meshesHierarchy["planet2"].get_transform().get_transform_matrix();

	root.m_mesh = meshesHierarchy["sphere"];
	child1.m_mesh = meshesHierarchy["planet1"];
	child2.m_mesh = meshesHierarchy["planet2"];

	// Children textures
	childrenTextures[0] = texture("..\\resources\\textures\\starfieldsphere.png", true, true);
	childrenTextures[1] = texture("..\\resources\\textures\\3.jpg", true, true);
	childrenTextures[2] = texture("..\\resources\\textures\\Planet_2.png", true, true);
	root.m_tex = childrenTextures[0];
	child1.m_tex = childrenTextures[1];
	child2.m_tex = childrenTextures[2];

	terrain_normal_map = texture("..\\resources\\textures\\water_normal.jpg", true, true);
	water_normal_map = texture("..\\resources\\textures\\water_normal.jpg", true, true);
	spaceFighter_normal_map = texture("..\\resources\\textures\\space_fighter_normal.png", true, true);
	tree_normal_map = texture("..\\resources\\textures\\Hyophorbe_lagenicaulis_normal.png", true, true);

	// Geometry to load into
	geometry geom;

	// Load height map
	texture height_map("..\\resources\\textures\\heightmaps\\01.png");

	// Generate terrain
	generate_terrain(geom, height_map, 80, 80, 10.0f);

	// Use geometry to create terrain mesh
	terr = mesh(geom);

	terrainmat = material(colour_black, colour_white, vec4(0.3f, 0.3f, 0.3f, 0.3f), 40.0f);

	terr.set_material(terrainmat);

	// Create cube geometry for skybox
	geometry geom2;
	geom2.set_type(GL_QUADS);
	vector<vec3> positions
	{
		vec3(-1.0f, 1.0f, -1.0f),	/* 5 */
		vec3(-1.0f, 1.0f, 1.0f),	/* 1 */
		vec3(-1.0f, -1.0f, 1.0f),	/* 2 */
		vec3(-1.0f, -1.0f, -1.0f),	/* 7 */

		vec3(1.0f, 1.0f, 1.0f),		/* 4 */
		vec3(1.0f, 1.0f, -1.0f),	/* 6 */
		vec3(1.0f, -1.0f, -1.0f),	/* 8 */
		vec3(1.0f, -1.0f, 1.0f),	/* 3 */

		vec3(-1.0f, 1.0f, -1.0f),	/* 5 */
		vec3(1.0f, 1.0f, -1.0f),	/* 6 */
		vec3(1.0f, 1.0f, 1.0f),		/* 4 */
		vec3(-1.0f, 1.0f, 1.0f),	/* 1 */

		vec3(-1.0f, -1.0f, 1.0f),	/* 2 */
		vec3(1.0f, -1.0f, 1.0f),	/* 3 */
		vec3(1.0f, -1.0f, -1.0f),	/* 8 */
		vec3(-1.0f, -1.0f, -1.0f),	/* 7 */

		vec3(-1.0f, -1.0f, -1.0f),	/* 7 */
		vec3(1.0f, -1.0f, -1.0f),	/* 8 */
		vec3(1.0f, 1.0f, -1.0f),	/* 6 */
		vec3(-1.0f, 1.0f, -1.0f),	/* 5 */

		vec3(1.0f, -1.0f, 1.0f),	/* 3 */
		vec3(-1.0f, -1.0f, 1.0f),	/* 2 */
		vec3(-1.0f, 1.0f, 1.0f),	/* 1 */
		vec3(1.0f, 1.0f, 1.0f)		/* 4 */
	};
	geom2.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	skybox = mesh(geom2);
	// ***********************************
	// Scale box by 100 - allows a distance
	// ***********************************
	skybox.get_transform().scale *= 100;

	// ******************************************************
	// Load the cubemap
	// - create array of six filenames +x, -x, +y, -y, +z, -z
	// ******************************************************
	array<string, 6> filenames =
	{
		"..\\resources\\textures\\cubemaps\\ThickCloudsWater\\ThickCloudsWaterRight2048.png",
		"..\\resources\\textures\\cubemaps\\ThickCloudsWater\\ThickCloudsWaterLeft2048.png",
		"..\\resources\\textures\\cubemaps\\ThickCloudsWater\\ThickCloudsWaterUp2048.png",
		"..\\resources\\textures\\cubemaps\\ThickCloudsWater\\ThickCloudsWaterDown2048.png",
		"..\\resources\\textures\\cubemaps\\ThickCloudsWater\\ThickCloudsWaterFront2048.png",
		"..\\resources\\textures\\cubemaps\\ThickCloudsWater\\ThickCloudsWaterBack2048.png"
	};

	// Create cube_map
	cube_map = cubemap(filenames);

	//grassGeom.set_type(GL_QUADS);
	//vector<vec3> grass_positions
	//{
	//	// Front
	//	vec3(1.0f, 1.0f, 1.0f),
	//	vec3(-1.0f, 1.0f, 1.0f),
	//	vec3(-1.0f, -1.0f, 1.0f),
	//	vec3(1.0f, -1.0f, 1.0f),
	//	// Right
	//	vec3(-0.5f, 1.0f, 1.5f),
	//	vec3(0.5f, 1.0f, 0.5f),
	//	vec3(0.5f, -1.0f, 0.5f),
	//	vec3(-0.5f, -1.0f, 1.5f),
	//	// Left
	//	vec3(0.0f, 1.0f, 0.0f),
	//	vec3(0.0f, 1.0f, 2.0f),
	//	vec3(0.0f, -1.0f, 2.0f),
	//	vec3(0.0f, -1.0f, 0.0f),
	//};
	//// Texture coordinates
	//vector<vec2> grass_tex_coords;
	//// Six sides, 6 vertices per side
	//for (unsigned int i = 0; i < 3; ++i)
	//{
	//	grass_tex_coords.push_back(vec2(1.0f, 1.0f));
	//	grass_tex_coords.push_back(vec2(0.0f, 1.0f));
	//	grass_tex_coords.push_back(vec2(0.0f, 0.0f));
	//	grass_tex_coords.push_back(vec2(1.0f, 0.0f));
	//}
	//
	//vector<vec3> grass_normals
	//{
	//	//CrossProduct((v2 - v1), (v3 - v1))
	//	cross(vec3(-1.0f, 1.0f, 1.0f) - vec3(1.0f, 1.0f, 1.0f), vec3(-1.0f, -1.0f, 1.0f) - vec3(1.0f, 1.0f, 1.0f)),
	//	cross(vec3(-1.0f, 1.0f, 1.0f) - vec3(1.0f, 1.0f, 1.0f), vec3(-1.0f, -1.0f, 1.0f) - vec3(1.0f, 1.0f, 1.0f)),
	//	cross(vec3(-1.0f, 1.0f, 1.0f) - vec3(1.0f, 1.0f, 1.0f), vec3(-1.0f, -1.0f, 1.0f) - vec3(1.0f, 1.0f, 1.0f)),
	//	cross(vec3(-1.0f, 1.0f, 1.0f) - vec3(1.0f, 1.0f, 1.0f), vec3(-1.0f, -1.0f, 1.0f) - vec3(1.0f, 1.0f, 1.0f)),
	//
	//	cross(vec3(0.5f, 1.0f, 0.5f) - vec3(-0.5f, 1.0f, 1.5f), vec3(0.5f, -1.0f, 0.5f) - vec3(-0.5f, 1.0f, 1.5f)),
	//	cross(vec3(0.5f, 1.0f, 0.5f) - vec3(-0.5f, 1.0f, 1.5f), vec3(0.5f, -1.0f, 0.5f) - vec3(-0.5f, 1.0f, 1.5f)),
	//	cross(vec3(0.5f, 1.0f, 0.5f) - vec3(-0.5f, 1.0f, 1.5f), vec3(0.5f, -1.0f, 0.5f) - vec3(-0.5f, 1.0f, 1.5f)),
	//	cross(vec3(0.5f, 1.0f, 0.5f) - vec3(-0.5f, 1.0f, 1.5f), vec3(0.5f, -1.0f, 0.5f) - vec3(-0.5f, 1.0f, 1.5f)),
	//
	//	cross(vec3(0.0f, 1.0f, 2.0f) - vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -1.0f, 2.0f) - vec3(0.0f, 1.0f, 0.0f)),
	//	cross(vec3(0.0f, 1.0f, 2.0f) - vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -1.0f, 2.0f) - vec3(0.0f, 1.0f, 0.0f)),
	//	cross(vec3(0.0f, 1.0f, 2.0f) - vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -1.0f, 2.0f) - vec3(0.0f, 1.0f, 0.0f)),
	//	cross(vec3(0.0f, 1.0f, 2.0f) - vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -1.0f, 2.0f) - vec3(0.0f, 1.0f, 0.0f)),
	//};
	//
	//// Add to the geometry
	//grassGeom.add_buffer(grass_positions, BUFFER_INDEXES::POSITION_BUFFER);
	//grassGeom.add_buffer(grass_tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	//grassGeom.add_buffer(grass_normals, BUFFER_INDEXES::NORMAL_BUFFER);
	//
	//grassMeshes["1"] = mesh(grassGeom);
	//grassMeshes["2"] = mesh(grassGeom);
	//grassMeshes["3"] = mesh(grassGeom);
	//grassMeshes["4"] = mesh(grassGeom);
	//grassMeshes["5"] = mesh(grassGeom);
	//
	//grassMeshes["1"].get_transform().translate(vec3(-25.0f, 1.0f, 0.0f));
	//grassMeshes["2"].get_transform().translate(vec3(-15.0f, 1.0f, 20.0f));
	//grassMeshes["3"].get_transform().translate(vec3(-0.0f, 1.0f, 25.0f));
	//grassMeshes["4"].get_transform().translate(vec3(-10.0f, 1.0f, -10.0f));
	//grassMeshes["5"].get_transform().translate(vec3(25.0f, 1.0f, -10.0f));
	//
	//material grassMat;
	//grassMat.set_emissive(colour_black);
	//grassMat.set_diffuse(vec4(1.00f, 0.78f, 0.28f, 1.0f));
	//grassMat.set_shininess(40.0f);
	//grassMat.set_specular(colour_white);
	//
	//grassMeshes["1"].set_material(grassMat);
	//grassMeshes["2"].set_material(grassMat);
	//grassMeshes["3"].set_material(grassMat);
	//grassMeshes["4"].set_material(grassMat);
	//grassMeshes["5"].set_material(grassMat);

	// Lighting
	// Point 0
	points[0].set_position(vec3(-25.0f, 5.0f, -15.0f));
	// Blue
	points[0].set_light_colour(vec4(0.0f, 0.0f, 1.0f, 1.0f));

	// Point 1
	points[1].set_position(vec3(10.0f, 1.0f, 9.0f));

	// Red
	points[1].set_light_colour(vec4(1.0f, 0.0f, 0.0f, 1.0f));

	// Spot 0
	// Position (-25, 10, -15)
	spots[0].set_position(vec3(0.0f, 2.0f, 10.0f));
	// Orange?
	spots[0].set_light_colour(vec4(1.0f, 0.6f, 0.0f, 1.0f));
	// Direction (1, -1, -1) normalized
	spots[0].set_direction(vec3(1.0f, -1.0f, -1.0f));
	// 20 range
	spots[0].set_range(20.0f);
	// 0.5 power
	spots[0].set_power(0.5f);

	// Spot 1
	// Position (-25, 10, -35)
	spots[1].set_position(vec3(10.0f, 2.0f, 0.0f));
	// Green
	spots[1].set_light_colour(vec4(0.3f, 0.0f, 1.0f, 1.0f));
	// Direction (1, -1, -1) normalized
	spots[1].set_direction(vec3(-1.0f, -1.0f, 1.0f));
	// 20 range
	spots[1].set_range(20.0f);
	// 0.5 power
	spots[1].set_power(0.5f);


	// SKYBOX EFFECT
	sky_eff.add_shader("..\\resources\\shaders\\skybox.vert", GL_VERTEX_SHADER);
	sky_eff.add_shader("..\\resources\\shaders\\skybox.frag", GL_FRAGMENT_SHADER);
	sky_eff.add_shader("..\\resources\\shaders\\fog.frag", GL_FRAGMENT_SHADER);

	// Build effect
	sky_eff.build();

	// TERRAIN EFFECT
	terrain_eff.add_shader("../resources/shaders/terrain.vert", GL_VERTEX_SHADER);
	vector<string> terrain_frag_shaders
	{
		"../resources/shaders/terrain.frag",
		"../resources/shaders/direction.frag",
		"../resources/shaders/weighted_texture.frag",
		"../resources/shaders/normal_map_tbn.frag",
		"../resources/shaders/fog.frag",
		"../resources/shaders/point.frag",
		"../resources/shaders/spot.frag"
	};
	terrain_eff.add_shader(terrain_frag_shaders, GL_FRAGMENT_SHADER);

	// Build effect
	terrain_eff.build();

	// WATER EFFECT
	water_eff.add_shader("../resources/shaders/water.vert", GL_VERTEX_SHADER);
	water_eff.add_shader("../resources/shaders/wave.vert", GL_VERTEX_SHADER);
	vector<string> water_frag_shaders
	{
		"../resources/shaders/water.frag",
		"../resources/shaders/direction.frag",
		"../resources/shaders/normal_map.frag", // ???
		"../resources/shaders/fog.frag",
		"../resources/shaders/point.frag",
		"../resources/shaders/spot.frag"
	};
	water_eff.add_shader(water_frag_shaders, GL_FRAGMENT_SHADER);

	// Build effect
	water_eff.build();

	// MODEL EFFECT (Used for loading in models)
	model_eff.add_shader("..//resources/shaders/model.vert", GL_VERTEX_SHADER);
	vector<string> model_frag_shaders
	{
		"../resources/shaders/model.frag",
		"../resources/shaders/direction.frag",
		"../resources/shaders/normal_map.frag",
		"../resources/shaders/fog.frag",
		"../resources/shaders/point.frag",
		"../resources/shaders/spot.frag"
	};
	model_eff.add_shader(model_frag_shaders, GL_FRAGMENT_SHADER);

	// Build effect
	model_eff.build();

	// Grass shader
	//grass_eff.add_shader("../resources/shaders/grass.vert", GL_VERTEX_SHADER);
	//vector<string> grass_frag_shaders
	//{
	//	"../resources/shaders/grass.frag",
	//	"../resources/shaders/fog.frag",
	//	"../resources/shaders/direction.frag"
	//};
	//grass_eff.add_shader(grass_frag_shaders, GL_FRAGMENT_SHADER);
	//
	//// ********************
	//// Load geometry shader
	//// ********************
	//grass_eff.add_shader("../resources/shaders/grass.geom", GL_GEOMETRY_SHADER);
	//
	//// Build effect
	//grass_eff.build();

	// Gouraud effect
	// Add shaders
	gouraud_eff.add_shader("../resources/shaders/gouraud.vert", GL_VERTEX_SHADER);
	gouraud_eff.add_shader("../resources/shaders/gouraud.frag", GL_FRAGMENT_SHADER);

	gouraud_eff.build(); // Build gouraud effect

	// Phong effect
	// Add shaders
	phong_eff.add_shader("../resources/shaders/phong.frag", GL_FRAGMENT_SHADER);
	phong_eff.add_shader("../resources/shaders/direction_simple.frag", GL_FRAGMENT_SHADER);
	phong_eff.add_shader("../resources/shaders/phong.vert", GL_VERTEX_SHADER);

	phong_eff.build(); // Build phong effect

	// NORMAL DEBUG EFFECT
	normals_debug_eff.add_shader("../resources/shaders/debug_normal_shader.vert", GL_VERTEX_SHADER);
	normals_debug_eff.add_shader("../resources/shaders/debug_normal_shader.frag", GL_FRAGMENT_SHADER);
	normals_debug_eff.add_shader("../resources/shaders/normals.geom", GL_GEOMETRY_SHADER);

	normals_debug_eff.build();

	// WATER REFLECTION EFFECT
	reflection_eff.add_shader("../resources/shaders/simple_texture.vert", GL_VERTEX_SHADER);
	reflection_eff.add_shader("../resources/shaders/simple_texture.frag", GL_FRAGMENT_SHADER);
	// Build effect
	reflection_eff.build();

	// GREYSCALE EFFECT
	greyscale_eff.add_shader("../resources/shaders/simple_texture.vert", GL_VERTEX_SHADER);
	greyscale_eff.add_shader("../resources/shaders/greyscale.frag", GL_FRAGMENT_SHADER);

	greyscale_eff.build();


	// BLUR EFFECT
	blur_eff.add_shader("../resources/shaders/simple_texture.vert", GL_VERTEX_SHADER);
	blur_eff.add_shader("../resources/shaders/blur.frag", GL_FRAGMENT_SHADER);

	blur_eff.build();

	// SIMPLE TEXTURING EFFECT
	tex_eff.add_shader("..\\resources\\shaders\\simple_texture.vert", GL_VERTEX_SHADER);
	tex_eff.add_shader("..\\resources\\shaders\\simple_texture.frag", GL_FRAGMENT_SHADER);

	tex_eff.build();

	// MOTION BLUR EFFECT
	motion_blur_eff.add_shader("..\\resources\\shaders\\simple_texture.vert", GL_VERTEX_SHADER);
	motion_blur_eff.add_shader("..\\resources\\shaders\\motion_blur.frag", GL_FRAGMENT_SHADER);
	motion_blur_eff.build();

	dof_eff.add_shader("..\\resources\\shaders\\simple_texture.vert", GL_VERTEX_SHADER);
	dof_eff.add_shader("..\\resources\\shaders\\depth_of_field.frag", GL_FRAGMENT_SHADER);

	dof_eff.build();

	vignette_eff.add_shader("..\\resources\\shaders\\simple_texture.vert", GL_VERTEX_SHADER);
	vignette_eff.add_shader("..\\resources\\shaders\\mask.frag", GL_FRAGMENT_SHADER);
	// Build effects
	vignette_eff.build();

	// Load shader
	grass_eff.add_shader("..\\resources\\shaders\\grass_shader.vert", GL_VERTEX_SHADER);
	grass_eff.add_shader("..\\resources\\shaders\\billboard.geom", GL_GEOMETRY_SHADER);
	grass_eff.add_shader("..\\resources\\shaders\\grass_shader.frag", GL_FRAGMENT_SHADER);
	grass_eff.add_shader("..\\resources\\shaders\\fog.frag", GL_FRAGMENT_SHADER);
	grass_eff.build();

	// Generate grass
	generate_grass();

	// Mesh textures
	textures[0] = texture("..\\resources\\textures\\Hyophorbe_lagenicaulis_dif2.png", true, true);
	textures[1] = texture("..\\resources\\textures\\Hyophorbe_lagenicaulis_dif2.png", true, true);
	textures[2] = texture("..\\resources\\textures\\Hyophorbe_lagenicaulis_dif2.png", true, true);
	textures[3] = texture("..\\resources\\textures\\Hyophorbe_lagenicaulis_dif2.png", true, true);
	textures[4] = texture("..\\resources\\textures\\space_fighter_tex.jpg", true, true);
	textures[5] = texture("..\\resources\\textures\\4.jpg", true, true);

	// Load in grass textures
	grassTex = texture("../resources/textures/low_poly_grass.png");
	grassDissolve = texture("../resources/textures/low_poly_grass_opacity.png");

	// Load in alpha map for vignette
	alpha_map = texture("..\\resources\\textures\\vignette.png");

	// Set free camera properties
	cam.set_position(vec3(0.0f, 20.0f, 10.0f));
	cam.set_target(vec3(0.0f, 0.0f, 0.0f));

	auto aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height()); // get aspect

	cam.set_projection(quarter_pi<float>(), aspect, 0.414f, 1000.0f);
	
	// Set target camera properties
	target_cam.set_position(vec3(50.0f, 10.0f, 50.0f));
	target_cam.set_target(vec3(0.0f, 0.0f, 0.0f));
	target_cam.set_projection(quarter_pi<float>(), aspect, 0.414f, 1000.0f);

	// Set chase camera properties
	chase_cam.set_pos_offset(vec3(0.0f, 2.0f, 10.0f));
	chase_cam.set_springiness(0.5f);
	chase_cam.move(meshes["spaceFighter"].get_transform().position, eulerAngles(meshes["spaceFighter"].get_transform().orientation));
	chase_cam.set_projection(quarter_pi<float>(), aspect, 0.414f, 1000.0f);
	return true;
}

// Returns the correct camera view matrix from a selection of cameras
mat4 GetView(string currentCamera, free_camera free, target_camera target, chase_camera chase)
{
	if (currentCamera == "free")
	{
		return free.get_view();
	}
	else if (currentCamera == "target")
	{
		return target.get_view();
	}
	else
	{
		return chase.get_view();
	}
}

// Returns the correct camera projection matrix from a selection of cameras
mat4 GetProj(string currentCamera, free_camera free, target_camera target, chase_camera chase)
{
	if (currentCamera == "free")
	{
		return free.get_projection();
	}
	else if (currentCamera == "target")
	{
		return target.get_projection();
	}
	else
	{
		return chase.get_projection();
	}
}

// Returns the correct camera position from a selection of cameras
vec3 GetPos(string currentCamera, free_camera free, target_camera target, chase_camera chase)
{
	if (currentCamera == "free")
	{
		return free.get_position();
	}
	else if (currentCamera == "target")
	{
		return target.get_position();
	}
	else
	{
		return chase.get_position();
	}
}

bool update(float delta_time)
{
	// Flip frames for motion blur effect
	if (current_frame == 0)
	{
		current_frame = 1;
	}
	else
	{
		current_frame = 0;
	}

	// The target object (For chase cam)
	static mesh &target_mesh = meshes["spaceFighter"];

	// For torus movement

	// Accumulate time
	total_time += delta_time;
	// Update the scale - base on sin wave
	sc = vec3(1.0f + sinf(total_time), 1.0f + sinf(total_time), 1.0f + sinf(total_time));
	// Multiply by 1.0f
	sc *= 1.0f;

	pos = vec3(0.0f, 1.0f + sinf(total_time), 0.0f);

	pos *= 40.0f;

	// Point light movement

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_RIGHT))
		points[1].set_position(points[1].get_position() + vec3(1.0f, 0.0f, 0.0f));
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT))
		points[1].set_position(points[1].get_position() + vec3(-1.0f, 0.0f, 0.0f));
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP))
		points[1].set_position(points[1].get_position() + vec3(0.0f, 0.0f, 1.0f));
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN))
		points[1].set_position(points[1].get_position() + vec3(0.0f, 0.0f, -1.0f));

	// Increase light
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_L))
	{
		ambient_light_colour += 0.1f;
		light_colour += 0.1f;
	}
	// Decrease light
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_K))
	{
		ambient_light_colour -= 0.1f;
		light_colour -= 0.1f;
	}

	// Reset Light with R
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_R))
	{
		light_colour = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		ambient_light_colour = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	}
	light.set_light_colour(light_colour);
	light.set_ambient_intensity(ambient_light_colour);

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_T))
		currentCamera = "target";
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_F))
		currentCamera = "free";
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_C))
		currentCamera = "chase";

	if (currentCamera == "target")
	{
		if (glfwGetKey(renderer::get_window(), GLFW_KEY_1))
			target_cam.set_position(vec3(50.0f, 10.0f, 50.0f));
		if (glfwGetKey(renderer::get_window(), GLFW_KEY_2))
			target_cam.set_position(vec3(-50.0f, 10.0f, 50.0f));
		if (glfwGetKey(renderer::get_window(), GLFW_KEY_3))
			target_cam.set_position(vec3(-50.0f, 10.0f, -50.0f));
		if (glfwGetKey(renderer::get_window(), GLFW_KEY_4))
			target_cam.set_position(vec3(50.0f, 10.0f, -50.0f));
	}
	// The ratio of pixels to rotation - remember the fov
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height = (quarter_pi<float>() * (static_cast<float>(renderer::get_screen_height()) / static_cast<float>(renderer::get_screen_width()))) / static_cast<float>(renderer::get_screen_height());

	double current_x;
	double current_y;

	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);

	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;

	// Multiply deltas by ratios - gets actual change in orientation
	delta_x = delta_x * ratio_width;
	delta_y = delta_y * ratio_height;

	// Rotate cameras by delta
	// delta_y - x-axis rotation
	// delta_x - y-axis rotation
	cam.rotate((float)delta_x, -(float)delta_y); // -delta_y, otherwise up and down is reversed
	chase_cam.rotate(vec3(delta_y, delta_x, 0.0));

	// Use keyboard to move the camera
	// - WSAD

	const float speed = 0.5f; // Speed of camera movement

	vec3 translation(0.0f, 0.0f, 0.0f);

	if (currentCamera == "free")
	{
		// Check if key is pressed
		if (glfwGetKey(renderer::get_window(), GLFW_KEY_W))
			cam.move(vec3(0.0f, 0.0f, speed)); 
		if (glfwGetKey(renderer::get_window(), GLFW_KEY_S))
			cam.move(vec3(0.0f, 0.0f, -speed));
		if (glfwGetKey(renderer::get_window(), GLFW_KEY_A))
			cam.move(vec3(-speed, 0.0f, 0.0f));
		if (glfwGetKey(renderer::get_window(), GLFW_KEY_D))
			cam.move(vec3(speed, 0.0f, 0.0f));
	}
	
	//cout << "Cam x: " << cam.get_position().x << ", Cam y: " << cam.get_position().y << ", Cam z: " << cam.get_position().z << endl;
	const float planetSpeedMultiplier = 0.100f;
	rho += pi<float>() * planetSpeedMultiplier * delta_time;
	meshesHierarchy["sphere"].get_transform().orientation = rotate(quat(), rho, yrot);


	// Update meshes of hierarchy
	root.m_mesh = meshesHierarchy["sphere"];
	child1.m_mesh = meshesHierarchy["planet1"];
	child2.m_mesh = meshesHierarchy["planet2"];

	// Update children of root
	root.update_children(meshesHierarchy["sphere"].get_transform().get_transform_matrix());

	// Update child2, so it can rotate around child1
	child2.update_local(meshesHierarchy["sphere"].get_transform().get_transform_matrix()); // ?

	// Update the cameras
	if (currentCamera == "free")
		cam.update(delta_time);
	if (currentCamera == "target")
		target_cam.update(delta_time);
	if (currentCamera == "chase")
		chase_cam.update(delta_time);

	// Set skybox position to camera position (camera in centre of skybox)
	skybox.get_transform().position = GetPos(currentCamera, cam, target_cam, chase_cam);

	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;

	waveTime += waveFreq;

	// Use up an down to modify the dissolve factor
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP))
	{
		dissolve_factor = clamp(dissolve_factor + 0.5f * delta_time, 0.0f, 1.0f);
		cout << dissolve_factor << endl;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN))
	{
		dissolve_factor = clamp(dissolve_factor - 0.5f * delta_time, 0.0f, 1.0f);
		cout << dissolve_factor << endl;
	}

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_G))
	{
		fog_density = 0.00f;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_H))
	{
		fog_density = 0.025f;
	}

	theta -= pi<float>() * delta_time;

	// Update chase cam to move with translation of spaceFighter, no rotation
	chase_cam.move(target_mesh.get_transform().position + pos * meshes["spaceFighter"].get_transform().scale, vec3(0.0f, 0.0f, 0.0f));

	// Post processing effects ON / OFF switches

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_5))
		greyscaleVisible = true;

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_6))
		greyscaleVisible = false;

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_8))
		motionBlurVisible = true;

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_7))
		motionBlurVisible = false;

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_0))
		depthOfFieldVisible = true;

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_9))
		depthOfFieldVisible = false;

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_MINUS))
		vignetteVisible = false;

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_EQUAL))
		vignetteVisible = true;


	if (glfwGetKey(renderer::get_window(), GLFW_KEY_X))
	{
		focus += 0.01f;
		cout << focus << endl;
	}

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_Z))
	{
		focus -= 0.01f;
		cout << focus << endl;
	}

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_RIGHT))
	{
		range += 0.01f;
		cout << range << endl;
	}

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT))
	{
		range -= 0.01f;
		cout << range << endl;
	}
	

	return true;
}

void RenderSkybox()
{
	// Disable depth test and depth mask
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	// Bind skybox effect
	renderer::bind(sky_eff);

	mat4 V; // view
	mat4 P; // projection

	// Calculate MVP for the skybox
	auto M = skybox.get_transform().get_transform_matrix();
	// Get the camera view matrix
	V = GetView(currentCamera, cam, target_cam, chase_cam);
	// Get the camera projection matrix
	P = GetProj(currentCamera, cam, target_cam, chase_cam);

	auto MVP = P * V * M;

	// Set MVP matrix uniform
	glUniformMatrix4fv(
		sky_eff.get_uniform_location("MVP"),
		1,
		GL_FALSE,
		value_ptr(MVP));

	glUniformMatrix4fv(
		sky_eff.get_uniform_location("MV"), // Location of uniform
		1, // Number of values - 1 mat4
		GL_FALSE, // Transpose the matrix?
		value_ptr(V * M)); // Pointer to matrix data

	// Set cubemap uniform
	renderer::bind(cube_map, 0);

	// Set fog colour
	glUniform4fv(sky_eff.get_uniform_location("fog_colour"), 1, value_ptr(fog_colour));
	// Set fog start
	glUniform1f(sky_eff.get_uniform_location("fog_start"), fog_start);
	// Set fog end
	glUniform1f(sky_eff.get_uniform_location("fog_end"), fog_end);
	// Set fog density
	glUniform1f(sky_eff.get_uniform_location("fog_density"), fog_density);
	// Set fog type
	glUniform1i(sky_eff.get_uniform_location("fog_type"), FOG_EXP2);

	// Render skybox
	renderer::render(skybox);

	// Enable depth test and depth mask
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

void RenderTerrain()
{
	// Bind terrain effect
	renderer::bind(terrain_eff);

	// Create MVP matrix
	auto M = terr.get_transform().get_transform_matrix();
	// Get the camera view matrix
	auto V = GetView(currentCamera, cam, target_cam, chase_cam);
	// Get the camera projection matrix
	auto P = GetProj(currentCamera, cam, target_cam, chase_cam);

	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(
		terrain_eff.get_uniform_location("MVP"),
		1,
		GL_FALSE,
		value_ptr(MVP));

	glUniformMatrix4fv(
		terrain_eff.get_uniform_location("MV"), // Location of uniform
		1, // Number of values - 1 mat4
		GL_FALSE, // Transpose the matrix?
		value_ptr(V * M)); // Pointer to matrix data

	// Set other necessary uniforms
	glUniformMatrix4fv(
		terrain_eff.get_uniform_location("M"),
		1,
		GL_FALSE,
		value_ptr(M));
	glUniformMatrix3fv(
		terrain_eff.get_uniform_location("N"),
		1,
		GL_FALSE,
		value_ptr(terr.get_transform().get_normal_matrix()));
	renderer::bind(terr.get_material(), "mat");
	renderer::bind(light, "light");

	// Bind point lights
	renderer::bind(points, "points");
	// Bind spot lights
	renderer::bind(spots, "spots");

	renderer::bind(terrainTex[0], 0);
	glUniform1i(terrain_eff.get_uniform_location("tex[0]"), 0);

	renderer::bind(terrainTex[1], 1);
	glUniform1i(terrain_eff.get_uniform_location("tex[1]"), 1);

	renderer::bind(terrainTex[2], 2);
	glUniform1i(terrain_eff.get_uniform_location("tex[2]"), 2);
	// Mirror texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); // width
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); // height

	renderer::bind(terrainTex[3], 3);
	glUniform1i(terrain_eff.get_uniform_location("tex[3]"), 3);

	vec3 cameraPos = GetPos(currentCamera, cam, target_cam, chase_cam);

	glUniform3fv(terrain_eff.get_uniform_location("eye_pos"), 1, value_ptr(cameraPos));

	renderer::bind(terrain_normal_map, 4);

	// Set normal_map uniform
	glUniform1i(terrain_eff.get_uniform_location("normal_map"), 4);

	// Set fog colour
	glUniform4fv(terrain_eff.get_uniform_location("fog_colour"), 1, value_ptr(fog_colour));
	// Set fog start
	glUniform1f(terrain_eff.get_uniform_location("fog_start"), fog_start);
	// Set fog end
	glUniform1f(terrain_eff.get_uniform_location("fog_end"), fog_end);
	// Set fog density
	glUniform1f(terrain_eff.get_uniform_location("fog_density"), fog_density);
	// Set fog type
	glUniform1i(terrain_eff.get_uniform_location("fog_type"), FOG_EXP2);

	// Render terrain
	renderer::render(terr);
}

void RenderModels()
{
	// Bind model effect
	renderer::bind(model_eff);
	int meshCount = 0;
	for (auto &e : meshes)
	{
		auto m = e.second;

		auto M = m.get_transform().get_transform_matrix();
		// Get the camera view matrix
		auto V = GetView(currentCamera, cam, target_cam, chase_cam);
		// Get the camera projection matrix
		auto P = GetProj(currentCamera, cam, target_cam, chase_cam);
		if (meshCount == 5)
		{
			mat4 S = scale(M, sc);
			mat4 T = translate(M, pos);

			//rx = rotate(quat(), rho, xrot);
			ry = rotate(quat(), theta, yrot);

			mat4 R = mat4_cast(ry);
			M = T * (R * S);
		}

		if (meshCount == 4)
		{
			mat4 T = translate(M, pos);

			M = T;
		}
		auto MVP = P * V * M;
		glUniformMatrix4fv(
			model_eff.get_uniform_location("MVP"),
			1,
			GL_FALSE,
			value_ptr(MVP));

		glUniformMatrix4fv(
			model_eff.get_uniform_location("MV"), // Location of uniform
			1, // Number of values - 1 mat4
			GL_FALSE, // Transpose the matrix?
			value_ptr(V * M)); // Pointer to matrix data

		// Set other necessary uniforms
		glUniformMatrix4fv(
			model_eff.get_uniform_location("M"),
			1,
			GL_FALSE,
			value_ptr(M));

		auto N = m.get_transform().get_normal_matrix();

		glUniformMatrix3fv(
			model_eff.get_uniform_location("N"),
			1,
			GL_FALSE,
			value_ptr(N));

		renderer::bind(m.get_material(), "mat");
		renderer::bind(light, "light");

		// Bind point lights
		renderer::bind(points, "points");
		// Bind spot lights
		renderer::bind(spots, "spots");

		renderer::bind(textures[meshCount], 0);
		glUniform1i(model_eff.get_uniform_location("tex"), 0);

		vec3 cameraPos = GetPos(currentCamera, cam, target_cam, chase_cam);

		glUniform3fv(model_eff.get_uniform_location("eye_pos"), 1, value_ptr(cameraPos));

		if (meshCount == 0 || meshCount == 1 || meshCount == 2 || meshCount == 3)
		{
			renderer::bind(tree_normal_map, 1);
			//Set normal_map uniform
			glUniform1i(model_eff.get_uniform_location("normal_map"), 1);
		}

		if (meshCount == 4)
		{
			renderer::bind(spaceFighter_normal_map, 1);
			//Set normal_map uniform
			glUniform1i(model_eff.get_uniform_location("normal_map"), 1);
		}
		// Set fog colour
		glUniform4fv(model_eff.get_uniform_location("fog_colour"), 1, value_ptr(fog_colour));
		// Set fog start
		glUniform1f(model_eff.get_uniform_location("fog_start"), fog_start);
		// Set fog end
		glUniform1f(model_eff.get_uniform_location("fog_end"), fog_end);
		// Set fog density
		glUniform1f(model_eff.get_uniform_location("fog_density"), fog_density);
		// Set fog type
		glUniform1i(model_eff.get_uniform_location("fog_type"), FOG_EXP2);

		renderer::render(m);
		meshCount++;
	}
}

//void renderGrass()
//{
//	// Render grass
//	vec3 offset = vec3(1.0f, 0.0f, 0.0f);
//
//	// Bind effect
//	renderer::bind(grass_eff);
//
//	glDisable(GL_CULL_FACE);
//	for (auto &e : grassMeshes)
//	{
//		auto m = e.second;
//
//		auto M = m.get_transform().get_transform_matrix();
//
//		// Get the camera view matrix
//		auto V = getView(currentCamera, cam, target_cam, chase_cam);
//		// Get the camera projection matrix
//		auto P = getProj(currentCamera, cam, target_cam, chase_cam);
//		// Create MVP matrix
//
//		auto MVP = P * V * M;
//
//		// Set MVP matrix uniform
//		glUniformMatrix4fv(
//			grass_eff.get_uniform_location("MVP"), // Location of uniform
//			1, // Number of values - 1 mat4
//			GL_FALSE, // Transpose the matrix?
//			value_ptr(MVP)); // Pointer to matrix data
//
//		glUniformMatrix4fv(
//			grass_eff.get_uniform_location("MV"), // Location of uniform
//			1, // Number of values - 1 mat4
//			GL_FALSE, // Transpose the matrix?
//			value_ptr(V * M)); // Pointer to matrix data
//
//		glUniformMatrix4fv(
//			grass_eff.get_uniform_location("M"),
//			1,
//			GL_FALSE,
//			value_ptr(M));
//
//		glUniformMatrix3fv(
//			grass_eff.get_uniform_location("N"),
//			1,
//			GL_FALSE,
//			value_ptr(m.get_transform().get_normal_matrix()));
//
//		renderer::bind(m.get_material(), "mat");
//		renderer::bind(light, "light");
//
//		// Set the dissolve_factor uniform value
//		glUniform1f(grass_eff.get_uniform_location("dissolve_factor"), dissolve_factor);
//
//		// Bind the two textures - use different index for each
//		renderer::bind(grassTex, 0);
//		renderer::bind(grassDissolve, 1);
//
//		// Set the uniform values for textures - use correct index
//		glUniform1i(grass_eff.get_uniform_location("tex"), 0);
//		glUniform1i(grass_eff.get_uniform_location("dissolve"), 1);
//
//		vec3 cameraPos = getPos(currentCamera, cam, target_cam, chase_cam);
//
//		glUniform3fv(grass_eff.get_uniform_location("eye_pos"), 1, value_ptr(cameraPos));
//
//		// Set fog colour
//		glUniform4fv(grass_eff.get_uniform_location("fog_colour"), 1, value_ptr(fog_colour));
//		// Set fog start
//		glUniform1f(grass_eff.get_uniform_location("fog_start"), fog_start);
//		// Set fog end
//		glUniform1f(grass_eff.get_uniform_location("fog_end"), fog_end);
//		// Set fog density
//		glUniform1f(grass_eff.get_uniform_location("fog_density"), fog_density);
//		// Set fog type
//		glUniform1i(grass_eff.get_uniform_location("fog_type"), FOG_EXP2);
//
//		// Set offset
//		glUniform3fv(grass_eff.get_uniform_location("offset"), 1, value_ptr(offset));
//
//		// Render geometry
//		renderer::render(m);
//	}
//	glEnable(GL_CULL_FACE);
//}

void RenderGrass()
{
	// Simply render the points.  All the work done in the geometry shader
	renderer::bind(grass_eff);
	// Get the camera view matrix
	auto V = GetView(currentCamera, cam, target_cam, chase_cam);
	//		// Get the camera projection matrix
	auto P = GetProj(currentCamera, cam, target_cam, chase_cam);
	auto MVP = P * V;
	glUniformMatrix4fv(
		grass_eff.get_uniform_location("MV"),
		1,
		GL_FALSE,
		value_ptr(V));
	glUniformMatrix4fv(
		grass_eff.get_uniform_location("P"),
		1,
		GL_FALSE,
		value_ptr(P));
	glUniform1f(grass_eff.get_uniform_location("point_size"), 2.0f);
	renderer::bind(grassTex, 0);
	renderer::bind(grassDissolve, 1);
	glUniform1i(grass_eff.get_uniform_location("tex"), 0);
	glUniform1i(grass_eff.get_uniform_location("dissolve"), 1);

	glUniform1f(grass_eff.get_uniform_location("dissolve_factor"), dissolve_factor);

	// Set fog colour
	glUniform4fv(grass_eff.get_uniform_location("fog_colour"), 1, value_ptr(fog_colour));
	// Set fog start
	glUniform1f(grass_eff.get_uniform_location("fog_start"), fog_start);
	// Set fog end
	glUniform1f(grass_eff.get_uniform_location("fog_end"), fog_end);
	// Set fog density
	glUniform1f(grass_eff.get_uniform_location("fog_density"), fog_density);
	// Set fog type
	glUniform1i(grass_eff.get_uniform_location("fog_type"), FOG_EXP2);

	renderer::render(grassGeom);
}

void RenderWater()
{

	// Bind water effect
	renderer::bind(water_eff);

	auto M = water.get_transform().get_transform_matrix();
	// Get the camera view matrix
	auto V = GetView(currentCamera, cam, target_cam, chase_cam);
	// Get the camera projection matrix
	auto P = GetProj(currentCamera, cam, target_cam, chase_cam);

	auto MVP = P * V * M;
	glUniformMatrix4fv(
		water_eff.get_uniform_location("MVP"),
		1,
		GL_FALSE,
		value_ptr(MVP));

	glUniformMatrix4fv(
		water_eff.get_uniform_location("MV"), // Location of uniform
		1, // Number of values - 1 mat4
		GL_FALSE, // Transpose the matrix?
		value_ptr(V * M)); // Pointer to matrix data

	glUniformMatrix4fv(
		water_eff.get_uniform_location("M"),
		1,
		GL_FALSE,
		value_ptr(M));
	glUniformMatrix3fv(
		water_eff.get_uniform_location("N"),
		1,
		GL_FALSE,
		value_ptr(water.get_transform().get_normal_matrix()));

	renderer::bind(water.get_material(), "mat");
	renderer::bind(light, "light");

	// Bind point lights
	renderer::bind(points, "points");
	// Bind spot lights
	renderer::bind(spots, "spots");

	renderer::bind(waterTex, 0);
	glUniform1i(water_eff.get_uniform_location("tex"), 0);

	vec3 cameraPos = GetPos(currentCamera, cam, target_cam, chase_cam);

	glUniform3fv(water_eff.get_uniform_location("eye_pos"), 1, value_ptr(cameraPos));

	// Set normal_map uniform
	renderer::bind(water_normal_map, 1);

	glUniform1i(water_eff.get_uniform_location("normal_map"), 1);

	glUniform1f(water_eff.get_uniform_location("waveTime"), waveTime);
	glUniform1f(water_eff.get_uniform_location("waveWidth"), waveWidth);
	glUniform1f(water_eff.get_uniform_location("waveHeight"), waveHeight);

	//glUniform3fv(water_eff.get_uniform_location("eye_pos"), 1, value_ptr(cameraPos));
	// Set fog colour
	glUniform4fv(water_eff.get_uniform_location("fog_colour"), 1, value_ptr(fog_colour));
	// Set fog start
	glUniform1f(water_eff.get_uniform_location("fog_start"), fog_start);
	// Set fog end
	glUniform1f(water_eff.get_uniform_location("fog_end"), fog_end);
	// Set fog density
	glUniform1f(water_eff.get_uniform_location("fog_density"), fog_density);
	// Set fog type
	glUniform1i(water_eff.get_uniform_location("fog_type"), FOG_EXP2);

	renderer::render(water);
}

void RenderGouraudMeshes()
{
	renderer::bind(gouraud_eff);

	for (auto &e : gouraudMeshes)
	{
		auto m = e.second;

		auto M = m.get_transform().get_transform_matrix();
		// Get the camera view matrix
		auto V = GetView(currentCamera, cam, target_cam, chase_cam);
		// Get the camera projection matrix
		auto P = GetProj(currentCamera, cam, target_cam, chase_cam);

		auto MVP = P * V * M;
		glUniformMatrix4fv(
			gouraud_eff.get_uniform_location("MVP"),
			1,
			GL_FALSE,
			value_ptr(MVP));

		// Set other necessary uniforms
		glUniformMatrix4fv(
			gouraud_eff.get_uniform_location("M"),
			1,
			GL_FALSE,
			value_ptr(M));

		auto N = m.get_transform().get_normal_matrix();

		glUniformMatrix3fv(
			gouraud_eff.get_uniform_location("N"),
			1,
			GL_FALSE,
			value_ptr(N));

		renderer::bind(m.get_material(), "mat");
		renderer::bind(light, "light");

		vec3 cameraPos = GetPos(currentCamera, cam, target_cam, chase_cam);

		glUniform3fv(gouraud_eff.get_uniform_location("eye_pos"), 1, value_ptr(cameraPos));

		renderer::render(m);
	}
}

void RenderPhongMeshes()
{
	renderer::bind(phong_eff);

	for (auto &e : phongMeshes)
	{
		auto m = e.second;

		auto M = m.get_transform().get_transform_matrix();
		// Get the camera view matrix
		auto V = GetView(currentCamera, cam, target_cam, chase_cam);
		// Get the camera projection matrix
		auto P = GetProj(currentCamera, cam, target_cam, chase_cam);

		auto MVP = P * V * M;
		glUniformMatrix4fv(
			phong_eff.get_uniform_location("MVP"),
			1,
			GL_FALSE,
			value_ptr(MVP));

		// Set other necessary uniforms
		glUniformMatrix4fv(
			phong_eff.get_uniform_location("M"),
			1,
			GL_FALSE,
			value_ptr(M));

		auto N = m.get_transform().get_normal_matrix();

		glUniformMatrix3fv(
			phong_eff.get_uniform_location("N"),
			1,
			GL_FALSE,
			value_ptr(N));

		renderer::bind(m.get_material(), "mat");
		renderer::bind(light, "light");

		vec3 cameraPos = GetPos(currentCamera, cam, target_cam, chase_cam);

		glUniform3fv(phong_eff.get_uniform_location("eye_pos"), 1, value_ptr(cameraPos));

		renderer::render(m);
	}
}

void RenderHierarchy()
{
	// Get the camera view matrix
	auto V = GetView(currentCamera, cam, target_cam, chase_cam);
	// Get the camera projection matrix
	auto P = GetProj(currentCamera, cam, target_cam, chase_cam);

	vec3 cameraPos = GetPos(currentCamera, cam, target_cam, chase_cam);
	// for planets
	root.render_children(V, P, cameraPos, model_eff, light, fog_colour, fog_start, fog_end, fog_density, FOG_EXP2);
}

frame_buffer Greyscale(frame_buffer input_frame)
{
	// Set render target back to temp_frame
	renderer::set_render_target(frames[current_frame]);

	// Clear frame
	renderer::clear();

	// Bind texture shader
	renderer::bind(greyscale_eff);

	// Bind texture from blur frame buffer
	renderer::bind(input_frame.get_frame(), 1);

	auto MVP = mat4();
	// Set MVP matrix uniform
	glUniformMatrix4fv(
		greyscale_eff.get_uniform_location("MVP"), // Location of uniform
		1, // Number of values - 1 mat4
		GL_FALSE, // Transpose the matrix?
		value_ptr(MVP)); // Pointer to matrix data

	glUniform1i(greyscale_eff.get_uniform_location("tex"), 1);
	glUniform3fv(greyscale_eff.get_uniform_location("intensity"), 1, value_ptr(vec3(0.299, 0.587, 0.184)));
	glUniform1i(greyscale_eff.get_uniform_location("render"), greyscaleVisible);
	

	// Render the screen quad
	renderer::render(screen_quad);

	return frames[current_frame];
}

void MotionBlur(frame_buffer input_frame1, frame_buffer input_frame2)
{
	//// Set render target to temp frame
	//renderer::set_render_target(frames[current_frame]);
	//
	//// Clear frame
	//renderer::clear();

	// Set render target to current frame
	renderer::set_render_target(temp_frame);

	// Clear frame
	renderer::clear();

	// Bind motion blur effect
	renderer::bind(motion_blur_eff);

	// MVP is now the identity matrix
	auto MVP = mat4();

	glUniformMatrix4fv(
		motion_blur_eff.get_uniform_location("MVP"), // Location of uniform
		1, // Number of values - 1 mat4
		GL_FALSE, // Transpose the matrix?
		value_ptr(MVP)); // Pointer to matrix data

	// Bind frames

	renderer::bind(input_frame1.get_frame(), 0);
	// If motion blur is enabled bind the previous frame
	if (motionBlurVisible)
	{
		renderer::bind(input_frame2.get_frame(), 1);
	}
	else // If not bind the same frame
	{
		renderer::bind(input_frame1.get_frame(), 1);
	}
	glUniform1i(motion_blur_eff.get_uniform_location("tex"), 0);
	glUniform1i(motion_blur_eff.get_uniform_location("previous_frame"), 1);

	// Set blend factor
	glUniform1f(motion_blur_eff.get_uniform_location("blend_factor"), 0.5f);

	// Render screen quad
	renderer::render(screen_quad);

	// Set render target back to the screen
	renderer::set_render_target(first_pass);
	renderer::clear();

	// Use texturing effect
	renderer::bind(tex_eff);

	// Set MVP matrix uniform
	glUniformMatrix4fv(
		tex_eff.get_uniform_location("MVP"), // Location of uniform
		1, // Number of values - 1 mat4
		GL_FALSE, // Transpose the matrix?
		value_ptr(MVP)); // Pointer to matrix data

	// Bind texture from frame buffer
	renderer::bind(temp_frame.get_frame(), 1);

	// Set the uniform
	glUniform1i(tex_eff.get_uniform_location("tex"), 1);

	// Render the screen quad
	renderer::render(screen_quad);
}

void DepthOfField()
{
	frame_buffer last_pass = first_pass;

	// Perform blur twice
	for (unsigned int i = 0; i < 2; ++i)
	{
		// Set render target to temp_frames[i]
		renderer::set_render_target(temp_frames[i]);

		// Clear frame
		renderer::clear();

		// Bind motion blur effect
		renderer::bind(blur_eff);

		// MVP is now the identity matrix
		auto MVP = mat4();

		glUniformMatrix4fv(
			blur_eff.get_uniform_location("MVP"), // Location of uniform
			1, // Number of values - 1 mat4
			GL_FALSE, // Transpose the matrix?
			value_ptr(MVP)); // Pointer to matrix data

		// Bind frames
		renderer::bind(last_pass.get_frame(), 0);

		glUniform1i(blur_eff.get_uniform_location("tex"), 0);

		// Set inverse width
		glUniform1f(blur_eff.get_uniform_location("inverse_width"), 1.0f / renderer::get_screen_width());

		// Set inverse height
		glUniform1f(blur_eff.get_uniform_location("inverse_height"), 1.0f / renderer::get_screen_height());

		// Render screen quad
		renderer::render(screen_quad);

		// Set last pass to this pass
		last_pass = temp_frames[i];
	}
	// !!!!!!!!!!!!!!! SCREEN PASS !!!!!!!!!!!!!!!!

	// Set render target back to vignette mask
	renderer::set_render_target(vignetteMask);
	
	renderer::clear();

	// Bind depth of field effect
	renderer::bind(dof_eff);

	// Set MVP matrix uniform
	auto MVP = mat4();

	glUniformMatrix4fv(
		dof_eff.get_uniform_location("MVP"), // Location of uniform
		1, // Number of values - 1 mat4
		GL_FALSE, // Transpose the matrix?
		value_ptr(MVP)); // Pointer to matrix data

	// Bind texture from last pass
	renderer::bind(last_pass.get_frame(), 0);

	// Set the uniform
	glUniform1i(dof_eff.get_uniform_location("tex"), 0);

	// Sharp texture is taken from first pass
	renderer::bind(first_pass.get_frame(), 1);
	glUniform1i(dof_eff.get_uniform_location("sharp"), 1);

	// Depth also taken from first pass
	renderer::bind(first_pass.get_depth(), 2);
	glUniform1i(dof_eff.get_uniform_location("depth"), 2);

	// Set range and focus values
	// - range distance to chaser (get from camera)
	// - focus 0.3
	glUniform1f(dof_eff.get_uniform_location("range"), range);//glm::length(cam.get_target_offset()));
	glUniform1f(dof_eff.get_uniform_location("focus"), focus); // 0.3f
	glUniform1i(dof_eff.get_uniform_location("render"), depthOfFieldVisible);

	// Render the screen quad
	renderer::render(screen_quad);
}

void Vignette()
{
	// Set render target back to the screen
	renderer::set_render_target();

	renderer::bind(vignette_eff);

	// MVP is now the identity matrix
	auto MVP = mat4(1.0f);
	// Set MVP matrix uniform
	glUniformMatrix4fv(vignette_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

	// Bind texture from frame buffer
	renderer::bind(vignetteMask.get_frame(), 0);
	// Set the uniform
	glUniform1i(vignette_eff.get_uniform_location("tex"), 0);

	// Set alpha map
	// Bind texture from frame buffer
	renderer::bind(alpha_map, 1);
	// Set the uniform
	glUniform1i(vignette_eff.get_uniform_location("alpha_map"), 1);

	glUniform1i(vignette_eff.get_uniform_location("render"), vignetteVisible);

	// Render the screen quad
	renderer::render(screen_quad);
}

bool render()
{
	
	// Set render target to blur frame buffer
	renderer::set_render_target(greyFrame);
	
	// Clear frame
	renderer::clear(); 

	RenderSkybox();
	RenderTerrain();
	RenderModels();
	RenderHierarchy();
	RenderGrass();
	RenderGouraudMeshes();
	RenderPhongMeshes();
	RenderWater();
	frame_buffer toPass = Greyscale(greyFrame);
	MotionBlur(frames[current_frame], frames[1 - current_frame]);
	DepthOfField();
	Vignette();

	return true;
}

void main()
{
	// Create application
	app application;
	// Set methods
	application.set_load_content(load_content);
	application.set_initialise(initialise); // For free camera
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}