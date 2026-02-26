vec3 UnpackNormal(vec4 n)
{
	n.xyz = n.xyz * 2.0 - 1.0;
	return n.xyz;
}

vec3 UnpackNormalRecZ(vec4 packednormal)
{
	vec3 normal;
	normal.xy = packednormal.wy * 2 - 1;
	normal.z = sqrt(1 - normal.x*normal.x - normal.y * normal.y);
	return normal;
}