#version 440

// A directional light structure
struct directional_light
{
	vec4 ambient_intensity;
	vec4 light_colour;
	vec3 light_dir;
};

// A material structure
struct material
{
	vec4 emissive;
	vec4 diffuse_reflection;
	vec4 specular_reflection;
	float shininess;
};

// The model matrix
uniform mat4 M;
// The transformation matrix
uniform mat4 MVP;
// The normal matrix
uniform mat3 N;
// Directional light for the scene
uniform directional_light light;
// Material of the object
uniform material mat;
// Position of the camera
uniform vec3 eye_pos;

// Incoming position
layout (location = 0) in vec3 position;
// Incoming normal
layout (location = 2) in vec3 normal;

// Outgoing primary colour
layout (location = 0) out vec4 primary;
// Outgoing secondary colour
layout (location = 1) out vec4 secondary;

void main()
{
	// ******************
	// Calculate position
	// ******************
	gl_Position = MVP * vec4(position, 1.0f);

	// ***************************
	// Calculate ambient component
	// ***************************
	vec4 ambient = light.ambient_intensity * mat.diffuse_reflection;

	// ********************
	// Transform the normal
	// ********************
	vec3 transformed_normal = N * normal;

	// ***************************
	// Calculate diffuse component
	// ***************************
	// Calculate k
	const float k = max(dot(transformed_normal, light.light_dir), 0.0f);
	// Calculate diffuse
	vec4 diffuse = k * (mat.diffuse_reflection * light.light_colour);
	

	// **********************************
	// Calculate world position of vertex
	// **********************************
	vec4 worldPosition = M * vec4(position, 1.0f);
	vec3 worldPositionFinal = vec3(worldPosition);

	// ************************
	// Calculate view direction
	// ************************
	vec3 view_dir = (eye_pos - worldPositionFinal) / length(eye_pos - worldPositionFinal);

	// ****************************************************
	// Calculate half vector between view_dir and light_dir
	// ****************************************************
	vec3 half_vector = (light.light_dir + view_dir) / length(light.light_dir + view_dir);

	// ****************************
	// Calculate specular component
	// ****************************
	// Calculate k
	const float k2 = pow(max(dot(transformed_normal, vec3(half_vector)), 0.0f), mat.shininess);
	// Calculate specular
	vec4 specular = k2 * (mat.diffuse_reflection * light.light_colour);
	

	// ***********
	// Set primary
	// ***********
	primary = mat.emissive + ambient + diffuse;

	// *************
	// Set secondary
	// *************
	secondary = specular;

	// *****************************************
	// Ensure primary and secondary alphas are 1
	// *****************************************
	primary.a = 1.0f;
	secondary.a = 1.0f;

}