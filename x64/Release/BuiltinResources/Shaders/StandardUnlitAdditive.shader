name "Forward/Unlit/Standard Additive"
render_mode forward

params
{
	define bool "Cutout" CUTOUT false
	float "Cutout threshold" cutoutVal 0.25 : 0.0 1.0
	color "Color" vColor 1 1 1 1
	define bool "Has albedo map" HAS_ALBEDO true
	sampler2D "Albedo map" albedoMap 0
	define bool "Has emission map" HAS_EMISSION_MAP false
	sampler2D "Emission map" emissionMap 1
	float "Emission" emissionVal 0.0
	define bool "Handle ambient light" USE_AMBIENT true
	vec2 "UV scale" uvScale 1 1
	define bool "Double sided" DOUBLE_SIDED false
	define bool "Depth write" DEPTH_WRITE true
}

pass
{
	tags
	{
		iteration_mode default
		blend_mode = TRANSPARENT ? alpha : add
		backface_culling = DOUBLE_SIDED ? off : cw
		depth_write = DEPTH_WRITE ? on : off
	}
	
	varying
	{
		vec3 a_position  : POSITION;
		vec2 a_texcoord0 : TEXCOORD0;
		vec2 a_texcoord1 : TEXCOORD1;
		vec4 a_weight    : BLENDWEIGHT;
		vec4 a_indices   : BLENDINDICES;

		vec3 v_position    : POSITION1 = vec3(0.0, 0.0, 0.0);
		vec2 v_texcoord0   : TEXCOORD0 = vec2(0.0, 0.0);
		vec2 v_texcoord1   : TEXCOORD1 = vec2(0.0, 0.0);
	}
	
	vertex
	{
		$input a_position, a_texcoord0, a_texcoord1, a_weight, a_indices
		$output v_position, v_texcoord0, v_texcoord1

		#include "common.sh"

		uniform vec4 u_skinned;
		uniform mat4 u_boneMatrix[128];

		void main()
		{
			mat4 model = u_model[0];
			
			if (u_skinned.x == 1.0)
			{
				model = mul(u_model[0], a_weight.x * u_boneMatrix[int(a_indices.x)] + 
										a_weight.y * u_boneMatrix[int(a_indices.y)] +
										a_weight.z * u_boneMatrix[int(a_indices.z)] +
										a_weight.w * u_boneMatrix[int(a_indices.w)]);
										
				vec3 vwpos = mul(model, vec4(a_position, 1.0)).xyz;
				gl_Position = mul(u_viewProj, vec4(vwpos, 1.0));
			}
			else
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
			}
			
			v_position = mul(model, vec4(a_position, 1.0)).xyz;
			v_texcoord0 = a_texcoord0;
			v_texcoord1 = a_texcoord1;
		}
	}
	
	fragment
	{
		$input v_position, v_texcoord0, v_texcoord1
		
		#include "common.sh"
		
		uniform vec4 vColor;
#if HAS_ALBEDO == 1
		SAMPLER2D(albedoMap, 0);
#endif
		
#if HAS_EMISSION_MAP == 1
		SAMPLER2D(emissionMap, 1);
#endif
		
#if CUTOUT == 1
		uniform vec4 cutoutVal;
#endif
		
		SAMPLER2D(u_lightMap, 8);
		uniform vec4 u_hasLightMap;
		
		uniform vec4 u_ambientColor;
		uniform vec4 uvScale;
		uniform vec4 emissionVal;
		
		void main()
		{
			vec2 uvs = v_texcoord0 * uvScale.xy;
			
			vec3 albedo = vColor.rgb;
			float alpha = 1.0f;
#if HAS_ALBEDO == 1
			vec4 tex = texture2D(albedoMap, uvs);
			albedo = tex.rgb * vColor.rgb;
			
	#if CUTOUT == 1
			if (tex.a < cutoutVal.x)
				discard;
	#else
			alpha = tex.a;
	#endif
#endif

#if HAS_EMISSION_MAP == 1
			float emission = texture2D(emissionMap, uvs).r * emissionVal.x;
#else
			float emission = emissionVal.x;
#endif

			vec3 lightMap = vec3(0.0);
			
			if (u_hasLightMap.x == 1)
			{
				lightMap = texture2D(u_lightMap, v_texcoord1).rgb;
			}

#if USE_AMBIENT
			vec3 ambient = (u_ambientColor.rgb * albedo) + (albedo * emission);
#else
			vec3 ambient = vec3(albedo + albedo * emission);
#endif
			
			gl_FragColor = vec4(ambient + (albedo * lightMap), alpha * vColor.a);
		}
	}
}