#version 150

in vec2 oUv0;
in vec3 oRay;

out vec4 oColour;

uniform sampler2D Tex0;
uniform sampler2D Tex1;
uniform mat4 proj;
uniform vec4 ambientColor;
uniform float farClipDistance;

float finalDepth(vec4 p)
{
    // GL needs it in [0..1]
    return (p.z / p.w) * 0.5 + 0.5;
}

void main()
{
	vec4 a0 = texture(Tex0, oUv0); // Attribute 0: Diffuse color+shininess
	vec4 a1 = texture(Tex1, oUv0); // Attribute 1: Normal+depth

	// Clip fragment if depth is too close, so the skybox can be rendered on the background
	if((a1.w - 0.0001) < 0.0)
        discard;

	// Calculate ambient colour of fragment
	oColour = vec4(ambientColor * vec4(a0.rgb,0));

	// Calculate depth of fragment;
	vec3 viewPos = normalize(oRay) * farClipDistance * a1.w;
	vec4 projPos = proj * vec4(viewPos, 1);
	gl_FragDepth = finalDepth(projPos);
}
