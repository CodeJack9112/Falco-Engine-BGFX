#ifndef _SHADER_NORMALS_

// Project the surface gradient (dhdx, dhdy) onto the surface (n, dpdx, dpdy).
vec3 ComputeSurfaceGradient(vec3 n, vec3 dpdx, vec3 dpdy, float dhdx, float dhdy)
{
	vec3 r1 = cross(dpdy, n);
	vec3 r2 = cross(n, dpdx);

	return (r1 * dhdx + r2 * dhdy) / dot(dpdx, r1);
}

// Move the normal away from the surface normal in the opposite surface gradient direction.
vec3 PerturbNormal(vec3 n, vec3 dpdx, vec3 dpdy, float dhdx, float dhdy)
{
	return normalize(n - ComputeSurfaceGradient(n, dpdx, dpdy, dhdx, dhdy));
}

// Returns the surface normal using screen-space partial derivatives of the height field.
vec3 ComputeSurfaceNormal(vec3 position, vec3 normal, float height)
{
	vec3 dpdx = dFdx(position);
	vec3 dpdy = dFdy(position);

	float dhdx = dFdx(height);
	float dhdy = dFdy(height);

	return PerturbNormal(normal, dpdx, dpdy, dhdx, dhdy);
}

float ApplyChainRule(float dhdu, float dhdv, float dud_, float dvd_)
{
	return dhdu * dud_ + dhdv * dvd_;
}

// Calculate the surface normal using the uv-space gradient (dhdu, dhdv)
// Requires height field gradient, double storage.
vec3 CalculateSurfaceNormal(vec3 position, vec3 normal, vec2 gradient, vec2 duvdx, vec2 duvdy)
{
	vec3 dpdx = dFdx(position);
	vec3 dpdy = dFdy(position);

	float dhdx = ApplyChainRule(gradient.x, gradient.y, duvdx.x, duvdx.y);
	float dhdy = ApplyChainRule(gradient.x, gradient.y, duvdy.x, duvdy.y);

	return PerturbNormal(normal, dpdx, dpdy, dhdx, dhdy);
}

// Returns the surface normal using screen-space partial derivatives of world position.
// Will result in hard shading normals.
vec3 ComputeSurfaceNormal(vec3 position)
{
	return normalize(cross(dFdx(position), dFdy(position)));
}

// portability reasons
vec3 mul2x3(vec2 val, vec3 row1, vec3 row2)
{
	vec3 res;
	for (int i = 0; i < 3; i++)
	{
		vec2 col = vec2(row1[i], row2[i]);
		res[i] = dot(val, col);
	}

	return res;
}

vec3 ComputeSurfaceNormal(vec3 normal, vec3 tangent, vec3 bitangent, sampler2D tex, vec2 uv)
{
	mat3 tangentFrame = mat3(normalize(bitangent), normalize(tangent), normal);

#if USE_FILTERING != 1
	normal = UnpackNormal(texture2D(tex, uv));
#else
	vec2 duv1 = dFdx(uv) * 2.0;
	vec2 duv2 = dFdy(uv) * 2.0;
	normal = UnpackNormal(texture2DGrad(tex, uv, duv1, duv2));
#endif
	return normalize(mul(normal, tangentFrame));
}

mat3 ComputeTangentFrame(vec3 normal, vec3 position, vec2 uv)
{
	vec3 dp1 = dFdx(position);
	vec3 dp2 = dFdy(position);
	vec2 duv1 = dFdx(uv);
	vec2 duv2 = dFdy(uv);

	mat3 M = mat3(dp1, dp2, cross(dp1, dp2));
	vec3 inverseM1 = vec3(cross(M[1], M[2]));
	vec3 inverseM2 = vec3(cross(M[2], M[0]));
	vec3 T = mul2x3(vec2(duv1.x, duv2.x), inverseM1, inverseM2);
	vec3 B = mul2x3(vec2(duv1.y, duv2.y), inverseM1, inverseM2);

	return mat3(normalize(T), normalize(B), normal);
}

// Returns the surface normal using screen-space partial derivatives of the uv and position coordinates.
vec3 ComputeSurfaceNormal(vec3 normal, vec3 position, sampler2D tex, vec2 uv)
{
	mat3 tangentFrame = ComputeTangentFrame(normal, position, uv);

#if USE_FILTERING != 1
	normal = UnpackNormal(texture2D(tex, uv));
#else
	vec2 duv1 = dFdx(uv) * 2.0;
	vec2 duv2 = dFdy(uv) * 2.0;
	normal = UnpackNormal(texture2DGrad(tex, uv, duv1, duv2));
#endif
	return normalize(mul(normal, tangentFrame));
}

vec3 ComputeNormal(vec4 heights, float strength)
{
	float hL = heights.x;
	float hR = heights.y;
	float hD = heights.z;
	float hT = heights.w;

	vec3 normal = vec3(hL - hR, strength, hD - hT);
	return normalize(normal);
}

vec3 ComputeNormal(sampler2D tex, vec2 uv, float texelSize, float strength)
{
	vec3 off = vec3(texelSize, texelSize, 0.0);
	vec4 heights;
	heights.x = texture2D(tex, uv - off.xz).x; // hL
	heights.y = texture2D(tex, uv + off.xz).x; // hR
	heights.z = texture2D(tex, uv - off.zy).x; // hD
	heights.w = texture2D(tex, uv + off.zy).x; // hT

	return ComputeNormal(heights, strength);
}
#endif