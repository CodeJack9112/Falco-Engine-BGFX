#ifndef __SHADOWS_SH__
#define __SHADOWS_SH__

#include "common.sh"
#include "shaderlib.sh"

float texcoordInRange(vec2 _texcoord)
{
	bool inRange = all(greaterThan(_texcoord, vec2_splat(0.0)))
				&& all(lessThan   (_texcoord, vec2_splat(1.0)));

	return float(inRange);
}

float hardShadow(sampler2DShadow _sampler, vec4 _shadowCoord, float _bias)
{
	vec3 coord = _shadowCoord.xyz / _shadowCoord.w;
	float visibility = shadow2D(_sampler, vec3(coord.xy, coord.z - _bias));

	return visibility;
}

float PCF(sampler2DShadow _sampler, vec4 _shadowCoord, float _bias, vec4 _pcfParams, vec2 _texelSize)
{
	vec2 texCoord = _shadowCoord.xy/_shadowCoord.w;
	bool outside = any(greaterThan(texCoord, vec2_splat(1.0))) || any(lessThan(texCoord, vec2_splat(0.0)));
	if (outside)
		return 1.0;
	
	float result = 0.0;
	vec2 offset = _pcfParams.zw * _texelSize * _shadowCoord.w;
	float samples = _pcfParams.x;
	
	if (samples > 1)
	{
		for(float x = -offset.x; x < offset.x; x += offset.x / (samples * 0.5))
		{
			for(float y = -offset.y; y < offset.y; y += offset.y / (samples * 0.5))
			{
				result += hardShadow(_sampler, _shadowCoord + vec4(vec2(x * samples, y * samples), 0.0, 0.0), _bias);
			}
		}
		result /= (samples * samples);
	}
	else
		result = hardShadow(_sampler, _shadowCoord, _bias);
	
	return result;
}

float computeVisibility(sampler2DShadow _sampler, vec4 _shadowCoord, float _bias, vec4 _samplingParams, vec2 _texelSize)
{
	float visibility = 1.0;
	vec4 shadowcoord = _shadowCoord;
	
	visibility = PCF(_sampler, shadowcoord, _bias, _samplingParams, _texelSize);

	return visibility;
}

float getShadow(int lightType,
				vec4 v_texcoord1,
				vec4 v_texcoord2,
				vec4 v_texcoord3,
				vec4 v_texcoord4,
				vec4 lightShadowBias,
				vec4 shadowSamplingParams,
				vec3 lightPosition,
				float shadowMapTexelSize,
				sampler2DShadow shadowMap0,
				sampler2DShadow shadowMap1,
				sampler2DShadow shadowMap2,
				sampler2DShadow shadowMap3
				)
{
	//vec3 colorCoverage = vec3(1, 1, 1);
	float visibility = 1.0;
	
	if (lightType == 2) // Directional light shadow
	{
		vec2 texelSize = vec2_splat(shadowMapTexelSize);
		vec2 texcoord1 = v_texcoord1.xy/v_texcoord1.w;
		vec2 texcoord2 = v_texcoord2.xy/v_texcoord2.w;
		vec2 texcoord3 = v_texcoord3.xy/v_texcoord3.w;
		vec2 texcoord4 = v_texcoord4.xy/v_texcoord4.w;
		
		bool selection0 = all(lessThan(texcoord1, vec2_splat(0.99))) && all(greaterThan(texcoord1, vec2_splat(0.01)));
		bool selection1 = all(lessThan(texcoord2, vec2_splat(0.99))) && all(greaterThan(texcoord2, vec2_splat(0.01)));
		bool selection2 = all(lessThan(texcoord3, vec2_splat(0.99))) && all(greaterThan(texcoord3, vec2_splat(0.01)));
		bool selection3 = all(lessThan(texcoord4, vec2_splat(0.99))) && all(greaterThan(texcoord4, vec2_splat(0.01)));

		if (selection0)
		{
			vec4 shadowcoord = v_texcoord1;

			//float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.4;
			//colorCoverage = vec3(-coverage, coverage, -coverage);
			visibility = computeVisibility(shadowMap0
							, shadowcoord
							, lightShadowBias.x
							, shadowSamplingParams
							, texelSize
							);
		}
		else if (selection1)
		{
			vec4 shadowcoord = v_texcoord2;

			//float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.4;
			//colorCoverage = vec3(coverage, coverage, -coverage);
			visibility = computeVisibility(shadowMap1
							, shadowcoord
							, lightShadowBias.y
							, shadowSamplingParams
							, texelSize/2.0
							);
		}
		else if (selection2)
		{
			vec4 shadowcoord = v_texcoord3;

			//float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.4;
			//colorCoverage = vec3(-coverage, -coverage, coverage);
			visibility = computeVisibility(shadowMap2
							, shadowcoord
							, lightShadowBias.z
							, shadowSamplingParams
							, texelSize/3.0
							);
		}
		else //selection3
		{
			vec4 shadowcoord = v_texcoord4;

			//float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.4;
			//colorCoverage = vec3(coverage, -coverage, -coverage);
			visibility = computeVisibility(shadowMap3
							, shadowcoord
							, lightShadowBias.w
							, shadowSamplingParams
							, texelSize/4.0
							);
		}
	}
	else if (lightType == 1) // Spot light shadow
	{
		vec2 texelSize = vec2_splat(shadowMapTexelSize);

		//float coverage = texcoordInRange(v_texcoord1.xy/v_texcoord1.w) * 0.3;
		//colorCoverage = vec3(coverage, -coverage, -coverage);

		visibility = computeVisibility(shadowMap0
						, v_texcoord1
						, lightShadowBias.x
						, shadowSamplingParams
						, texelSize
						);
	}
	else // Point light shadow
	{
		vec3 m_tetraNormalGreen = vec3(0.0, -0.57735026, 0.81649661);
		vec3 m_tetraNormalYellow = vec3(0.0, -0.57735026, -0.81649661);
		vec3 m_tetraNormalBlue = vec3(-0.81649661, 0.57735026, 0.0);
		vec3 m_tetraNormalRed = vec3(0.81649661, 0.57735026, 0.0);

		vec2 texelSize = vec2_splat(shadowMapTexelSize/4.0);

		vec4 faceSelection;
		vec3 pos = lightPosition;
		faceSelection.x = dot(m_tetraNormalGreen,  pos);
		faceSelection.y = dot(m_tetraNormalYellow, pos);
		faceSelection.z = dot(m_tetraNormalBlue,   pos);
		faceSelection.w = dot(m_tetraNormalRed,    pos);

		vec4 shadowcoord;
		float faceMax = max(max(faceSelection.x, faceSelection.y), max(faceSelection.z, faceSelection.w));
		if (faceSelection.x == faceMax)
		{
			shadowcoord = v_texcoord1;

			//float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.3;
			//colorCoverage = vec3(-coverage, coverage, -coverage);
		}
		else if (faceSelection.y == faceMax)
		{
			shadowcoord = v_texcoord2;

			//float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.3;
			//colorCoverage = vec3(coverage, coverage, -coverage);
		}
		else if (faceSelection.z == faceMax)
		{
			shadowcoord = v_texcoord3;

			//float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.3;
			//colorCoverage = vec3(-coverage, -coverage, coverage);
		}
		else // (faceSelection.w == faceMax)
		{
			shadowcoord = v_texcoord4;

			//float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.3;
			//colorCoverage = vec3(coverage, -coverage, -coverage);
		}

		visibility = computeVisibility(shadowMap0
						, shadowcoord
						, lightShadowBias.x
						, shadowSamplingParams
						, texelSize
						);
	}
	
	return visibility;
}
#endif