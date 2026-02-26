#ifndef _SHADER_WAVES_

//
// Description : Utilties for waves simulation
//
// based on GPU Gems Chapter 1. Effective Water Simulation from Physical Models, by Mark Finch and Cyan Worlds
//

vec3 GerstnerWaveValues(vec2 position, vec2 D, float amplitude, float wavelength, float Q, float timer)
{
	float w = 2 * 3.14159265 / wavelength;
	float dotD = dot(position, D);
	float v = w * dotD + timer;
	return vec3(cos(v), sin(v), w);
}

vec3 GerstnerWaveNormal(vec2 D, float A, float Q, vec3 vals)
{
	float C = vals.x;
	float S = vals.y;
	float w = vals.z;
	float WA = w * A;
	float WAC = WA * C;
	vec3 normal = vec3(-D.x * WAC, 1.0 - Q * WA * S, -D.y * WAC);
	return normalize(normal);
}

vec3 GerstnerWaveTangent(vec2 D, float A, float Q, vec3 vals)
{
	float C = vals.x;
	float S = vals.y;
	float w = vals.z;
	float WA = w * A;
	float WAS = WA * S;
	vec3 normal = vec3(Q * -D.x * D.y * WAS, D.y * WA * C, 1.0 - Q * D.y * D.y * WAS);
	return normalize(normal);
}

vec3 GerstnerWaveDelta(vec2 D, float A, float Q, vec3 vals)
{
	float C = vals.x;
	float S = vals.y;
	float QAC = Q * A * C;
	return vec3(QAC * D.x, A * S, QAC * D.y);
}

void GerstnerWave(vec2 windDir, float tiling, float amplitude, float wavelength, float Q, float timer, inout vec3 position, out vec3 normal)
{
	vec2 D = windDir;
	vec3 vals = GerstnerWaveValues(position.xz * tiling, D, amplitude, wavelength, Q, timer);
	normal = GerstnerWaveNormal(D, amplitude, Q, vals);
	position += GerstnerWaveDelta(D, amplitude, Q, vals);
}

vec3 SineWaveValues(vec2 position, vec2 D, float amplitude, float wavelength, float timer)
{
	float w = 2 * 3.14159265 / wavelength;
	float dotD = dot(position, D);
	float v = w * dotD + timer;
	return vec3(cos(v), sin(v), w);
}

vec3 SineWaveNormal(vec2 D, float A, vec3 vals)
{
	float C = vals.x;
	float w = vals.z;
	float WA = w * A;
	float WAC = WA * C;
	vec3 normal = vec3(-D.x * WAC, 1.0, -D.y * WAC);
	return normalize(normal);
}

vec3 SineWaveTangent(vec2 D, float A, vec3 vals)
{
	float C = vals.x;
	float w = vals.z;
	float WAC = w * A * C;
	vec3 normal = vec3(0.0, D.y * WAC, 1.0);
	return normalize(normal);
}

float SineWaveDelta(float A, vec3 vals)
{
	return vals.y * A;
}

void SineWave(vec2 windDir, float tiling, float amplitude, float wavelength, float timer, inout vec3 position, out vec3 normal)
{
	vec2 D = windDir;
	vec3 vals = SineWaveValues(position.xz * tiling, D, amplitude, wavelength, timer);
	normal = SineWaveNormal(D, amplitude, vals);
	position.y += SineWaveDelta(amplitude, vals);
}
#endif