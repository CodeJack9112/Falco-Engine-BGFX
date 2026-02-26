name "Deferred/Nature/Standard Terrain"
render_mode deferred

params
{
	float "Specular" specIntensity 0.1 : 0.0 1.0
	define bool "Has albedo map" HAS_ALBEDO true
	define bool "Has normal map" HAS_NORMAL_MAP true
}

pass
{
	varying
	{
		vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
		vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 1.0);
		vec3 v_tangent   : TANGENT   = vec3(1.0, 0.0, 0.0);
		vec3 v_bitangent : BITANGENT = vec3(0.0, 1.0, 0.0);

		vec3 a_position  : POSITION;
		vec3 a_normal    : NORMAL;
		vec3 a_tangent   : TANGENT;
		vec3 a_bitangent : BITANGENT;
		vec2 a_texcoord0 : TEXCOORD0;
	}
	
	vertex
	{
		$input a_position, a_normal, a_tangent, a_bitangent, a_texcoord0
		$output v_normal, v_tangent, v_bitangent, v_texcoord0
		
		#include "common.sh"
		
		uniform mat3 u_normalMatrix;
		
		void main()
		{
			gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
			
			v_normal = mul(u_normalMatrix, a_normal);
			v_tangent = mul(u_normalMatrix, a_tangent);
			v_bitangent = mul(u_normalMatrix, a_bitangent);

			v_texcoord0 = a_texcoord0;
		}
	}
	
	fragment
	{
		$input v_normal, v_tangent, v_bitangent, v_texcoord0
		
		#include "common.sh"
		#include "shaderlib.sh"
		
		uniform vec4 specIntensity;
		uniform vec4 u_ambientColor;
		
		uniform vec4 u_textureCount;
		uniform vec4 u_textureSizes[8];
		
#if HAS_ALBEDO == 1
		SAMPLER2D(u_texture0, 0);
		SAMPLER2D(u_texture1, 1);
		SAMPLER2D(u_texture2, 2);
		SAMPLER2D(u_texture3, 3);
		SAMPLER2D(u_texture4, 4);
#endif
#if HAS_NORMAL_MAP == 1
		SAMPLER2D(u_texture0Normal, 5);
		SAMPLER2D(u_texture1Normal, 6);
		SAMPLER2D(u_texture2Normal, 7);
		SAMPLER2D(u_texture3Normal, 8);
		SAMPLER2D(u_texture4Normal, 9);
#endif
		SAMPLER2D(u_texture0Splat, 10);
		SAMPLER2D(u_texture1Splat, 11);
		
		void main()
		{
			vec4 albedo = vec4(1.0);
			vec3 normal = normalize(v_normal);

#if HAS_NORMAL_MAP == 1
			mat3 TBN = mat3(v_tangent, v_bitangent, v_normal);
#endif

			if (u_textureCount.x > 0)
			{
				vec2 uvs = v_texcoord0 * u_textureSizes[0].xy;
				
#if HAS_ALBEDO == 1
				albedo = texture2D(u_texture0, uvs);
#endif
#if HAS_NORMAL_MAP == 1
				normal = texture2D(u_texture0Normal, uvs).rgb;
				normal = normalize(normal * 2.0 - 1.0);
				normal = normalize(TBN * normal);
#endif
			}
			
			if (u_textureCount.x > 0)
			{
				for (int i = 1; i < u_textureCount.x; ++i)
				{
					vec2 uvs = v_texcoord0 * u_textureSizes[i].xy;
					
#if HAS_ALBEDO == 1
					float color_mask = 1.0;
					vec3 color = vec3(1.0);
					
					//Mix textures together
					if (i == 1)
					{
						color_mask = texture2D(u_texture0Splat, v_texcoord0).g;
						color = texture2D(u_texture1, uvs).rgb;
					}
					
					if (i == 2)
					{
						color_mask = texture2D(u_texture0Splat, v_texcoord0).b;
						color = texture2D(u_texture2, uvs).rgb;
					}
					
					if (i == 3)
					{
						color_mask = texture2D(u_texture0Splat, v_texcoord0).a;
						color = texture2D(u_texture3, uvs).rgb;
					}
					
					if (i == 4)
					{
						color_mask = texture2D(u_texture1Splat, v_texcoord0).r;
						color = texture2D(u_texture4, uvs).rgb;
					}
					
					albedo = (albedo * vec4(vec3(1.0 - color_mask), 1.0)) + vec4(color * color_mask, 1.0);
#endif
#if HAS_NORMAL_MAP == 1
					float normal_mask = 1.0;
					vec3 normal_color = vec3(1.0);
					
					if (i == 1)
					{
						normal_mask = texture2D(u_texture0Splat, v_texcoord0).g;
						normal_color = texture2D(u_texture1Normal, uvs).rgb;
					}
					
					if (i == 2)
					{
						normal_mask = texture2D(u_texture0Splat, v_texcoord0).b;
						normal_color = texture2D(u_texture2Normal, uvs).rgb;
					}
					
					if (i == 3)
					{
						normal_mask = texture2D(u_texture0Splat, v_texcoord0).a;
						normal_color = texture2D(u_texture3Normal, uvs).rgb;
					}
					
					if (i == 4)
					{
						normal_mask = texture2D(u_texture1Splat, v_texcoord0).r;
						normal_color = texture2D(u_texture4Normal, uvs).rgb;
					}
					
					normal_color = normalize(normal_color * 2.0 - 1.0);
					normal_color = normalize(TBN * normal_color);
					
					normal = (normal * vec3(1.0 - normal_mask)) + (normal_color * normal_mask);
#endif
				}
			}
			
			float metallicVal = 0.0;
			float roughness = 1.0;
			float ao = 1.0;
			
			if (gl_FrontFacing)
				normal = -normal;
			
			gl_FragData[0] = vec4(albedo.rgb, 0.0);
			gl_FragData[1] = vec4(encodeNormalUint(normal), gl_FrontFacing ? 1.0 : 0.0);
			gl_FragData[2] = vec4(metallicVal, roughness, ao, specIntensity.x);
			gl_FragData[3] = vec4(0.0);
		}
	}
}