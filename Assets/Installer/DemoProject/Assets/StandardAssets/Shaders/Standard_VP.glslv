uniform int passNumber;

uniform vec4 lightPosition;
uniform vec3 eyePosition;
uniform mat4 worldViewProj;
uniform mat4 modelMatrix;
uniform mat4 world;

uniform mat4 texViewProj;

attribute vec4 vertex;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec4 uv0;

varying vec4 oUv0;
varying vec4 fragPos;
varying vec3 vNormal;
varying vec3 vTangent;
varying vec4 oUv;

void main()
{
	gl_Position = worldViewProj * vertex;
	fragPos = vertex;
	vNormal = normal;
	vTangent = tangent;
	oUv0 = uv0;
	vec4 worldPos = world * vertex;
	oUv = texViewProj * worldPos;
}
