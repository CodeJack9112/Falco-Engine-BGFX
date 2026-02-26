#version 150

in vec4 vertex;

out vec4 oPos;
out vec2 oUv0;

uniform mat4 worldViewProj;

void main()
{
	gl_Position = worldViewProj * vertex;
	oPos = gl_Position;
	oUv0 = gl_Position.xy;
}
