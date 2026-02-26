#ifndef _SHADER_DISPLACEMENT_
//
// Description : Utilties for waves displacement
//

vec2 GetNoise(in vec2 position, in vec2 timedWindDir)
{
	vec2 noise;
	noise.x = snoise(position * 0.015 + timedWindDir * 0.0005); // large and slower noise 
	noise.y = snoise(position * 0.1 + timedWindDir * 0.002); // smaller and faster noise
	return saturate(noise);
}

void AdjustWavesValues(in vec2 noise, inout vec4 wavesNoise, inout vec4 wavesIntensity)
{
	wavesNoise = wavesNoise * vec4(noise.y * 0.25, noise.y * 0.25, noise.x + noise.y, noise.y);
	wavesIntensity = wavesIntensity + vec4(saturate(noise.y - noise.x), noise.x, noise.y, noise.x + noise.y);
	wavesIntensity = clamp(wavesIntensity, 0.01, 10);
}

#ifndef _VERTEX_
// uv in texture space, normal in world space
vec3 ComputeNormal(sampler2D normalTexture, vec2 worldPos, vec2 texCoord,
	vec3 normal, vec3 tangent, vec3 bitangent,
	vec4 wavesNoise, vec4 wavesIntensity, vec2 timedWindDir)
{
	vec2 noise = GetNoise(worldPos, timedWindDir * 0.5);
	AdjustWavesValues(noise, wavesNoise, wavesIntensity);

	vec2 texCoords[4];
	texCoords[0] = texCoord * 1.6 + timedWindDir * 0.064 + wavesNoise.x;
	texCoords[1] = texCoord * 0.8 + timedWindDir * 0.032 + wavesNoise.y;
	texCoords[2] = texCoord * 0.5 + timedWindDir * 0.016 + wavesNoise.z;
	texCoords[3] = texCoord * 0.3 + timedWindDir * 0.008 + wavesNoise.w;

	vec3 wavesNormal = vec3(0, 1, 0);
#if USE_DISPLACEMENT == 1
	normal = normalize(normal);
	tangent = normalize(tangent);
	bitangent = normalize(bitangent);
	for (int i = 0; i < 4; ++i)
	{
		wavesNormal += ComputeSurfaceNormal(normal, tangent, bitangent, normalTexture, texCoords[i]) * wavesIntensity[i];
	}
#else
	for (int i = 0; i < 4; ++i)
	{
		wavesNormal += UnpackNormal(texture2D(normalTexture, texCoords[i])) * wavesIntensity[i];
	}
	wavesNormal.xyz = wavesNormal.xzy; // flip zy to avoid btn multiplication
#endif // #ifdef USE_DISPLACEMENT

	return wavesNormal;
}
#endif

#if USE_DISPLACEMENT == 1
float ComputeNoiseHeight(sampler2D heightTexture, vec4 wavesIntensity, vec4 wavesNoise, vec2 texCoord, vec2 noise, vec2 timedWindDir)
{
	AdjustWavesValues(noise, wavesNoise, wavesIntensity);

	vec2 texCoords[4];
	texCoords[0] = texCoord * 1.6 + timedWindDir * 0.064 + wavesNoise.x;
	texCoords[1] = texCoord * 0.8 + timedWindDir * 0.032 + wavesNoise.y;
	texCoords[2] = texCoord * 0.5 + timedWindDir * 0.016 + wavesNoise.z;
	texCoords[3] = texCoord * 0.3 + timedWindDir * 0.008 + wavesNoise.w;
	float height = 0;
	for (int i = 0; i < 4; ++i)
	{
		height += texture2D(heightTexture, texCoords[i]).x * wavesIntensity[i];
	}

	return height;
}

vec3 ComputeDisplacement(vec3 worldPos, float cameraDistance, vec2 noise, float timer,
	vec4 waveSettings, vec4 waveAmplitudes, vec4 wavesIntensity, vec4 waveNoise,
	out vec3 normal, out vec3 tangent)
{
	vec2 windDir = waveSettings.xy;
	float waveSteepness = waveSettings.z;
	float waveTiling = waveSettings.w;

	//TODO: improve motion/simulation instead of just noise
	//TODO: fix UV due to wave distortion

	wavesIntensity = normalize(wavesIntensity);
	waveNoise = vec4(noise.x - noise.x * 0.2 + noise.y * 0.1, noise.x + noise.y * 0.5 - noise.y * 0.1, noise.x, noise.x) * waveNoise;
	vec4 wavelengths = vec4(1, 4, 3, 6) + waveNoise;
	vec4 amplitudes = waveAmplitudes + vec4(0.5, 1, 4, 1.5) * waveNoise;

	// reduce wave intensity base on distance to reduce aliasing
	wavesIntensity *= 1.0 - saturate(vec4(cameraDistance / 120.0, cameraDistance / 150.0, cameraDistance / 170.0, cameraDistance / 400.0));

	// compute position and normal from several sine and gerstner waves
	tangent = normal = vec3(0, 1, 0);
	vec2 timers = vec2(timer * 0.5, timer * 0.25);
	for (int i = 2; i < 4; ++i)
	{
		float A = wavesIntensity[i] * amplitudes[i];
		vec3 vals = SineWaveValues(worldPos.xz * waveTiling, windDir, A, wavelengths[i], timer);
		normal += wavesIntensity[i] * SineWaveNormal(windDir, A, vals);
		tangent += wavesIntensity[i] * SineWaveTangent(windDir, A, vals);
		worldPos.y += SineWaveDelta(A, vals);
	}

	// using normalized wave steepness, transform to Q
	vec2 Q = waveSteepness / ((2 * 3.14159265 / wavelengths.xy) * amplitudes.xy);
	for (int j = 0; j < 2; ++j)
	{
		float A = wavesIntensity[j] * amplitudes[j];
		vec3 vals = GerstnerWaveValues(worldPos.xz * waveTiling, windDir, A, wavelengths[j], Q[j], timer);
		normal += wavesIntensity[j] * GerstnerWaveNormal(windDir, A, Q[j], vals);
		tangent += wavesIntensity[j] * GerstnerWaveTangent(windDir, A, Q[j], vals);
		worldPos += GerstnerWaveDelta(windDir, A, Q[j], vals);
	}

	normal = normalize(normal);
	tangent = normalize(tangent);
	if (length(wavesIntensity) < 0.01)
	{
		normal = vec3(0, 1, 0);
		tangent = vec3(0, 0, 1);
	}

	return worldPos;
}
#endif
#endif
