uniform sampler2D cubemap;

varying vec3 uv;

void main()
{
	vec4 skyColor = texture2D(cubemap, uv.xy);
	
	gl_FragColor = vec4(skyColor.xyz, 1.0);
}
