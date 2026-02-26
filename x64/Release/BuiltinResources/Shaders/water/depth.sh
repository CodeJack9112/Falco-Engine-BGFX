//
// Description : Water color based on water depth and color extinction 
//
// based on Rendering Water as a Post-process Effect by Wojciech Toman
//

// waterTransparency - x = , y = water visibility along eye vector, 
// waterDepthValues - x = water depth in world space, y = view/accumulated water depth in world space
vec3 DepthRefraction(vec2 waterTransparency, vec2 waterDepthValues, float shoreRange, vec3 horizontalExtinction,
	vec3 refractionColor, vec3 shoreColor, vec3 surfaceColor, vec3 depthColor)
{
	float waterClarity = waterTransparency.x;
	float visibility = waterTransparency.y;
	float waterDepth = waterDepthValues.x;
	float viewWaterDepth = waterDepthValues.y;

	float accDepth = viewWaterDepth * waterClarity; // accumulated water depth
	float accDepthExp = saturate(accDepth / (2.5 * visibility));
	accDepthExp *= (1.0 - accDepthExp) * accDepthExp * accDepthExp + 1.0; // out cubic

	surfaceColor = mix(shoreColor, surfaceColor, saturate(waterDepth / shoreRange));
	vec3 waterColor = mix(surfaceColor, depthColor, saturate(waterDepth / horizontalExtinction));

	refractionColor = mix(refractionColor, surfaceColor * waterColor, saturate(accDepth / visibility));
	refractionColor = mix(refractionColor, depthColor, accDepthExp);
	refractionColor = mix(refractionColor, depthColor * waterColor, saturate(waterDepth / horizontalExtinction));
	
	return refractionColor;
}