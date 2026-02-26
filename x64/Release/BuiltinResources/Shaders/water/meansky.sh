#ifndef _SHADER_MEANSKY_

//
// Description : Mean sky radiance
//
// based on Real-time Realistic Ocean Lighting using Seamless Transitions from Geometry to BRDF by Eric Bruneton
//

#if USE_MEAN_SKY_RADIANCE == 1

// V, N, Tx, Ty in world space
vec2 U(vec2 zeta, vec3 V, vec3 N, vec3 Tx, vec3 Ty)
{
	vec3 f = normalize(vec3(-zeta, 1.0)); // tangent space
	vec3 F = f.x * Tx + f.y * Ty + f.z * N; // world space
	vec3 R = 2.0 * dot(F, V) * F - V;
	return vec2(dot(F, V));
}

// viewDir and normal in world space
vec3 MeanSkyRadiance(samplerCube skyTexture, vec3 viewDir, vec3 normal)
{
	if (dot(viewDir, normal) < 0.0)
	{
		normal = reflect(normal, viewDir);
	}
	vec3 ty = normalize(vec3(0.0, normal.z, -normal.y));
	vec3 tx = cross(ty, normal);

	const float eps = 0.001;
	vec2 u0 = U(vec2(0, 0), viewDir, normal, tx, ty) * 0.05;
	vec2 dux = 2.0 * (vec2(eps, 0.0) - u0) / eps;
	vec2 duy = 2.0 * (vec2(0, eps) - u0) / eps;
	return textureCube(skyTexture, vec3(u0.xy, 1.0)).rgb; //TODO: transform hemispherical cordinates to cube or use a 2d texture
}

#endif // #ifdef USE_MEAN_SKY_RADIANCE
#endif