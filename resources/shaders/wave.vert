
vec4 calculateWavePosition(in float waveTime, in float waveWidth, in float waveHeight, in vec3 position, in mat4 MVP)
{
	vec3 v = position;
	v.y = sin(waveWidth * v.x + waveTime) * cos(waveWidth * v.z + waveTime) * waveHeight;

	return MVP * vec4(v, 1.0f);
}