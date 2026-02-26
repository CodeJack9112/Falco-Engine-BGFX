name "Forward/Skybox/Standard"
render_mode forward

params
{
	samplerCube "Cubemap" skyboxTexture 0
}

pass
{
	tags
	{
		backface_culling off
	}
	
	varying
	{
		vec3 v_texcoord0 : TEXCOORD0 = vec3(0.0, 0.0, 0.0);

		vec3 a_position  : POSITION;
		vec2 a_texcoord0 : TEXCOORD0;
	}
	
	vertex
	{
		$input a_position, a_texcoord0
		$output v_texcoord0
		
		#include "common.sh"
		
		void main()
		{
			gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
			v_texcoord0 = -normalize(a_position);
		}
	}
	
	fragment
	{
		$input v_texcoord0
		
		#include "common.sh"

		SAMPLERCUBE(skyboxTexture, 0);
		
		void main()
		{
			vec4 color = textureCube(skyboxTexture, v_texcoord0);
			gl_FragColor = color;
		}
	}
}