vec3 calc_normal(in vec3 normal, in vec3 tangent, in vec3 binormal, in sampler2D normal_map, in vec2 tex_coord)
{
	// ****************************************************************
	// Ensure normal, tangent and binormal are unit length (normalized)
	// ****************************************************************
	normal = normalize(normal);
	//tangent = normalize(tangent);
	//binormal = normalize(binormal);
	binormal = normalize(cross(normal, vec3(0.0f, 0.0f, 1.0f)));
	tangent = normalize(cross(normal, vec3(1.0f, 0.0f, 0.0f)));

	// *****************************
	// Sample normal from normal map
	// *****************************
	vec3 sampled_normal = vec3(texture(normal_map, tex_coord));

	// ************************************
	// Transform components to range [0, 1]
	// ************************************
	sampled_normal = 2.0f * sampled_normal - vec3(1.0f, 1.0f, 1.0f);

	// *******************
	// Generate TBN matrix
	// *******************
	mat3 TBN = mat3(tangent, binormal, normal);
	// ****************************************
	// Return sampled normal transformed by TBN
	// - remember to normalize
	// ****************************************
	return (TBN * sampled_normal);
}