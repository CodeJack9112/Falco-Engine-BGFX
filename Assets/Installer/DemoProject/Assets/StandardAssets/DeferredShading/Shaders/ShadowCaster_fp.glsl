/* uniform vec4 lightPos;
uniform vec4 lightParams;
uniform float farClipDistance;
uniform float nearClipDistance;

//varying vec2 depth;
varying vec3 worldPos;

void main()
{
	vec3 LightToVertex;
	float finalDepth = 0.0;
	
	if (lightPos.w > 0 & lightParams != vec4(1.0, 0.0, 0.0, 1.0))
	{
		LightToVertex = (lightPos.xyz - worldPos);
		float distanceToLight = length(LightToVertex);
		finalDepth = (distanceToLight - nearClipDistance) / (farClipDistance - nearClipDistance);
		finalDepth = clamp(finalDepth, 0.0, 1.0);
	}
	else
	{
		//Directional light
		finalDepth = gl_FragCoord.z / gl_FragCoord.w;
	}
	
	gl_FragColor = vec4(finalDepth, finalDepth, finalDepth, 1.0);
} */

//#version 150
#version 120

uniform float cFarDistance;
uniform int lightType;

varying vec3 oViewPos;
varying vec2 depth;

void main()
{
    float depthCube = length(oViewPos) / 1000.0;
	float depthDir = depth.x / depth.y;
	
	if (lightType == 0)
		gl_FragColor = vec4(depthCube, depthCube, depthCube, 1.0);
	
	if (lightType == 1)
		gl_FragColor = vec4(depthDir, depthDir, depthDir, 1.0);
	
	if (lightType == 2)
		gl_FragColor = vec4(depthDir, depthDir, depthDir, 1.0);
}