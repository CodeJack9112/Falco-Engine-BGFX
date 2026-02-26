name "Forward/Particles/Standard"
render_mode forward

params
{
	color "Color" vcolor 1 1 1 1
	sampler2D "Texture" baseColor 0
	define bool "Additive" IS_ADDITIVE false
	define bool "Double sided" DOUBLE_SIDED false
	define bool "Z-Write" WRITE_Z false
	define bool "Ambient light" AMBIENT_LIGHT false
}

pass
{
	tags
	{
		blend_mode = IS_ADDITIVE ? add : alpha
		depth_write = WRITE_Z ? on : off
		backface_culling = DOUBLE_SIDED ? off : cw
	}
	
	varying
	{
		vec3 a_position  : POSITION;
		vec2 a_texcoord0 : TEXCOORD0;
		vec4 a_color0    : COLOR0;

		vec2 v_texcoord0   : TEXCOORD0 = vec2(0.0, 0.0);
		vec4 v_color0      : COLOR0 = vec4(1.0, 1.0, 1.0, 1.0);
	}
	
	vertex
	{
		$input a_position, a_texcoord0, a_color0
		$output v_texcoord0, v_color0

		#include "common.sh"
		
		void main()
		{
			gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
			
			v_texcoord0 = a_texcoord0;
			v_color0 = a_color0;
		}
	}
	
	fragment
	{
		$input v_texcoord0, v_color0
		
		#include "common.sh"

		uniform vec4 vcolor;
		SAMPLER2D(baseColor, 0);
		
#if AMBIENT_LIGHT == 1
		uniform vec4 u_ambientColor;
#endif
		
		void main()
		{
			vec4 tex = texture2D(baseColor, v_texcoord0) * vcolor * v_color0;
			vec4 color = vec4(0.0);
			
#if IS_ADDITIVE == 1
			color = vec4(tex.rgb * vcolor.a * v_color0.a, 1.0);
#else
			color = tex;
#endif

#if AMBIENT_LIGHT == 1
			color.rgb = color.rgb * u_ambientColor.rgb;
#endif
			gl_FragColor = color;
		}
	}
}