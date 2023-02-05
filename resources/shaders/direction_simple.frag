
// Directional light structure
#ifndef DIRECTIONAL_LIGHT
#define DIRECTIONAL_LIGHT
struct directional_light
{
	vec4 ambient_intensity;
	vec4 light_colour;
	vec3 light_dir;
};
#endif

// A material structure
#ifndef MATERIAL
#define MATERIAL
struct material
{
	vec4 emissive;
	vec4 diffuse_reflection;
	vec4 specular_reflection;
	float shininess;
};
#endif

// Calculates the directional light
vec4 calculate_direction(in directional_light light, in material mat, in vec3 normal, in vec3 view_dir)
{
	normal = normalize(normal);

	// Ambient component
	vec4 ambient = mat.diffuse_reflection * light.ambient_intensity;
	// Diffuse component
	vec4 diffuse = (mat.diffuse_reflection * light.light_colour) * max(dot(normal, light.light_dir), 0.0f);
	// Half vector
	vec3 half_vector = normalize(light.light_dir + view_dir);
	// Specular component
	vec4 specular = (mat.specular_reflection * light.light_colour) * pow(max(dot(normal, half_vector), 0.0f), mat.shininess);
	// Colour to return
	vec4 colour = (mat.emissive + ambient + diffuse) + specular;
	colour.a = 1.0;
	// Return colour
	return colour;
}