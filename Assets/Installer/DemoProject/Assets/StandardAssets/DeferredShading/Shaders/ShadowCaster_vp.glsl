/* uniform mat4 worldViewProj;
uniform mat4 world;
uniform vec4 texelOffsets;

attribute vec4 vertex;

//varying float depth;
varying vec3 worldPos;

void main()
{
    vec4 outPos = worldViewProj * vertex;
    //outPos.xy += texelOffsets.zw * outPos.w;

    //depth = outPos.z;
    worldPos = (world * vertex).xyz;

    gl_Position = outPos;
} */

//#version 150
#version 120

uniform mat4 cWorldViewProj;
uniform mat4 cWorldView;
uniform vec4 texelOffsets;
uniform int lightType;

attribute vec4 vertex;
    
varying vec3 oViewPos;
varying vec2 depth;

void main()
{
    gl_Position = cWorldViewProj * vertex;
    oViewPos = (cWorldView * vertex).xyz;

	vec4 outPos = cWorldViewProj * vertex;
    outPos.xy += texelOffsets.zw * outPos.w;
    depth = outPos.zw;
	
	//gl_Position = outPos;
}