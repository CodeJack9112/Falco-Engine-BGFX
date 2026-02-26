#ifndef _SHADER_RADIANCE_

//
// Description : Reflected water radiance
//

// refractionValues, x = index of refraction constant, y = refraction strength
// normal and eyeVec in world space
float FresnelValue(vec2 refractionValues, vec3 normal, vec3 eyeVec)
{
	// R0 is a constant related to the index of refraction (IOR).
	float R0 = refractionValues.x;
	// This value modifies current fresnel term. If you want to weaken
	// reflections use bigger value.
	float refractionStrength = refractionValues.y;
#if SIMPLIFIED_FRESNEL == 1
	return R0 + (1.0f - R0) * pow(1.0f - dot(eyeVec, normal), 5.0f);
#else		
	float angle = 1.0f - saturate(dot(normal, eyeVec));
	float fresnel = angle * angle;
	fresnel *= fresnel;
	fresnel *= angle;
	return saturate(fresnel * (1.0f - saturate(R0)) + R0 - refractionStrength);
#endif // #ifdef SIMPLIFIED_FRESNEL
}

// lightDir, eyeDir and normal in world space
vec3 ReflectedRadiance(float shininess, vec3 specularValues, vec3 lightColor, vec3 lightDir, vec3 eyeDir, vec3 normal, float fresnel)
{
	float shininessExp = specularValues.z;

#if BLINN_PHONG == 1
	// a variant of the blinn phong shading
	float specularIntensity = specularValues.x * 0.0075;

	vec3 H = normalize(eyeDir + lightDir);
	float e = shininess * shininessExp * 800;
	float kS = saturate(dot(normal, lightDir));
	vec3 specular = vec3(kS * specularIntensity * pow(saturate(dot(normal, H)), e) * sqrt((e + 1) / 2));
	specular *= lightColor;
#else
	vec2 specularIntensity = specularValues.xy;
	// reflect the eye vector such that the incident and emergent angles are equal
	vec3 mirrorEye = reflect(-eyeDir, normal);
	float dotSpec = saturate(dot(mirrorEye, lightDir) * 0.5f + 0.5f);
	vec3 specular = (1.0f - fresnel) * saturate(lightDir.y) * pow(dotSpec, specularIntensity.y) * (shininess * shininessExp + 0.2f) * lightColor;
	specular += specular * specularIntensity.x * saturate(shininess - 0.05f) * lightColor;
#endif // #ifdef BLINN_PHONG
	return specular;
}
#endif