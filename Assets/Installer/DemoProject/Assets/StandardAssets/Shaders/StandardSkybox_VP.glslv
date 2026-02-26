uniform mat4 worldViewProj;

attribute vec4 vertex;
attribute vec4 uv0;

varying vec3 uv;

void main()
{
	uv = uv0.xyz;
    gl_Position = worldViewProj * vertex;
}
