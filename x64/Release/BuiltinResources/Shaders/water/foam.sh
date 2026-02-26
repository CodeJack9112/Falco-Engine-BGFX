#ifndef _SHADER_FOAM_

//
// Description : Foam color based on water depth, near the shore
//

float FoamColor(sampler2D tex, vec2 texCoord, vec2 texCoord2, vec2 ranges, vec2 factors, float waterDepth, float baseColor)
{
	float f1 = texture2D(tex, texCoord).r;
	float f2 = texture2D(tex, texCoord2).r;
	return mix(f1 * factors.x + f2 * factors.y, baseColor, smoothstep(ranges.x, ranges.y, waterDepth));
}

// surfacePosition, depthPosition, eyeVec in world space
// waterDepth is the horizontal water depth in world space
float FoamValue(sampler2D shoreTexture, sampler2D foamTexture, vec2 foamTiling,
	vec4 foamNoise, vec2 foamSpeed, vec3 foamRanges, float maxAmplitude,
	vec3 surfacePosition, vec3 depthPosition, vec3 eyeVec, float waterDepth,
	vec2 timedWindDir, float timer)
{
	vec2 position = (surfacePosition.xz + eyeVec.xz * 0.1) * 0.5;

	float s = sin(timer * 0.01 + depthPosition.x);
	vec2 texCoord = position + timer * 0.01 * foamSpeed + s * 0.05;
	s = sin(timer * 0.01 + depthPosition.z);
	vec2 texCoord2 = (position + timer * 0.015 * foamSpeed + s * 0.05) * -0.5; // also flip
	vec2 texCoord3 = texCoord * foamTiling.x;
	vec2 texCoord4 = (position + timer * 0.015 * -foamSpeed * 0.3 + s * 0.05) * -0.5 * foamTiling.x; // reverse direction
	texCoord *= foamTiling.y;
	texCoord2 *= foamTiling.y;

	vec2 ranges = foamRanges.xy;
	ranges.x += snoise(surfacePosition.xz + foamNoise.z * timedWindDir) * foamNoise.x;
	ranges.y += snoise(surfacePosition.xz + foamNoise.w * timedWindDir) * foamNoise.y;
	ranges = clamp(ranges, 0.0, 10.0);

	float foamEdge = max(ranges.x, ranges.y);
	float deepFoam = FoamColor(foamTexture, texCoord, texCoord2, vec2(ranges.x, foamEdge), vec2(1.0, 0.5), waterDepth, 0.0);
	float foam = FoamColor(shoreTexture, texCoord3 * 0.25, texCoord4, vec2(0.0, ranges.x), vec2(0.75, 1.5), waterDepth, deepFoam);

	// high waves foam
	if (surfacePosition.y - foamRanges.z > 0.0001f)
	{
		float amount = saturate((surfacePosition.y - foamRanges.z) / maxAmplitude) * 0.25;
		foam += (texture2D(shoreTexture, texCoord3).x + texture2D(shoreTexture, texCoord4).x * 0.5f) * amount;
	}

	return foam;
}
#endif