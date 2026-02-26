name "Deferred/Decal"
render_mode deferred

params
{
	define bool "Cutout" CUTOUT true
	float "Cutout threshold" cutoutVal 0.2 : 0.0 1.0
	color "Color" vColor 1 1 1 1
	float "Specular" specIntensity 0.0 : 0.0 1.0
	define bool "Has albedo map" HAS_ALBEDO_MAP true
	sampler2D "Albedo map" t_albedoMap 5
	define bool "Has normal map" HAS_NORMAL_MAP false
	sampler2D "Normal map" t_normalMap 6
	float "Normal map scale" normalMapScale 1.0
	define bool "Has metallic map" HAS_METALLIC_MAP false
	sampler2D "Metallic map" t_metallicMap 7
	float "Metalness" metalness 0.0 : 0.0 1.0
	define bool "Has roughness map" HAS_ROUGHNESS_MAP false
	sampler2D "Roughness map" t_roughnessMap 8
	float "Roughness" roughnessVal 0.0 : 0.0 1.0
	define bool "Has AO map" HAS_AO_MAP false
	sampler2D "AO map" t_aoMap 9
	define bool "Has emission map" HAS_EMISSION_MAP false
	sampler2D "Emission map" t_emissionMap 10
	float "Emission" emissionVal 0.0
	vec2 "UV scale" uvScale 1 1
	vec2 "UV offset" uvOffset 0 0
}

pass
{
	tags
	{
		
	}
	
	varying
	{
		vec3 a_position  : POSITION;
		vec4 v_position  : POSITION0 = vec4(0.0, 0.0, 0.0, 0.0);
	}
	
	vertex
	{
		$input a_position
		$output v_position
		
		#include "common.sh"
		
		uniform mat4 u_invVP;
		
		void main()
		{
			vec4 world_pos = u_invVP * vec4(a_position, 1.0);
			v_position = u_viewProj * world_pos;
			gl_Position = v_position;
		}
	}
	
	fragment
	{
		$input v_position
		
		#include "common.sh"
		#include "shaderlib.sh"
		
		uniform vec4 vColor;
		uniform vec4 specIntensity;
		
		uniform mat4 u_VP;
		
		SAMPLER2D(u_albedoMap, 0);
		SAMPLER2D(u_normalMap, 1);
		SAMPLER2D(u_mraMap, 2);
		SAMPLER2D(u_lightMap, 3);
		SAMPLER2D(u_depthMap, 4);
		
#if CUTOUT == 1
		uniform vec4 cutoutVal;
#endif
		
#if HAS_ALBEDO_MAP == 1
		SAMPLER2D(t_albedoMap, 5);
#endif
#if HAS_NORMAL_MAP == 1
		SAMPLER2D(t_normalMap, 6);
		uniform vec4 normalMapScale;
#endif
#if HAS_METALLIC_MAP == 1
		SAMPLER2D(t_metallicMap, 7);
#endif
		uniform vec4 metalness;
#if HAS_ROUGHNESS_MAP == 1
		SAMPLER2D(t_roughnessMap, 8);
#endif
#if HAS_AO_MAP == 1
		SAMPLER2D(t_aoMap, 9);
#endif
#if HAS_EMISSION_MAP == 1
		SAMPLER2D(t_emissionMap, 10);
#endif
		
		uniform vec4 u_ambientColor;
		uniform vec4 uvScale;
		uniform vec4 uvOffset;
		uniform vec4 roughnessVal;
		uniform vec4 emissionVal;
		
		vec3 world_position_from_depth(vec2 screen_pos, float ndc_depth)
		{
			float depth = ndc_depth * 2.0 - 1.0;
			vec4 ndc_pos = vec4(screen_pos, depth, 1.0);
			vec4 world_pos = u_invViewProj * ndc_pos;
			return world_pos.xyz / world_pos.w;
		}
		
		void main()
		{
			vec2 screen_pos = v_position.xy / v_position.w;
			vec2 tex_coords = screen_pos * 0.5 + 0.5;
			
			float depth = texture2D(u_depthMap, tex_coords).x;
			vec3  world_pos = world_position_from_depth(screen_pos, depth);
			
			vec4 ndc_pos = u_VP * vec4(world_pos, 1.0);
			ndc_pos.xyz /= ndc_pos.w;
			
			if (ndc_pos.x < -1.0 || ndc_pos.x > 1.0 || ndc_pos.y < -1.0 || ndc_pos.y > 1.0 || ndc_pos.z < -1.0 || ndc_pos.z > 1.0)
				discard;
			
			vec2 decal_tex_coord = ndc_pos.xy * 0.5 + 0.5;
			decal_tex_coord.x = 1.0 - decal_tex_coord.x;
			decal_tex_coord = decal_tex_coord * uvScale.xy + uvOffset.xy;
			
#if HAS_ALBEDO_MAP == 1
			vec4 diffuseColor = texture2D(t_albedoMap, decal_tex_coord) * vColor;
			
	#if CUTOUT == 1
			if (diffuseColor.a < cutoutVal.x)
				discard;
	#endif
#else
			vec4 diffuseColor = vColor;
#endif
#if HAS_METALLIC_MAP == 1
			float metallicVal  = texture2D(t_metallicMap, decal_tex_coord).r * metalness.x;
#else
			float metallicVal = metalness.x;
#endif
#if HAS_ROUGHNESS_MAP == 1
			float roughness = texture2D(t_roughnessMap, decal_tex_coord).r * roughnessVal.x;
#else
			float roughness = roughnessVal.x;
#endif
#if HAS_AO_MAP == 1
			float ao = texture2D(t_aoMap, decal_tex_coord).r;
#else
			float ao = 1.0;
#endif
#if HAS_EMISSION_MAP == 1
			float emission = texture2D(t_emissionMap, decal_tex_coord).r * emissionVal.x;
#else
			float emission = emissionVal.x;
#endif
#if HAS_NORMAL_MAP == 1
			vec3 normal = texture2D(t_normalMap, decal_tex_coord).rgb;
			normal = normalize(normal * 2.0 - 1.0);
			normal.xy *= normalMapScale.x;
#else
			vec3 normal = vec3(0.0);
#endif
			
			vec4 albedoSrc = texture2D(u_albedoMap, tex_coords);
			vec4 normalSrc = texture2D(u_normalMap, tex_coords);
			vec4 mraSrc = texture2D(u_mraMap, tex_coords);
			vec4 lightMapSrc = texture2D(u_lightMap, tex_coords);
			
			gl_FragData[0] = vec4(diffuseColor.rgb * (emission + 1.0), albedoSrc.a + emission);
			gl_FragData[1] = vec4(encodeNormalUint(normalize(normal + (normalSrc.xyz * 2.0 - 1.0))), normalSrc.a);
			gl_FragData[2] = vec4(metallicVal + mraSrc.r, roughness + mraSrc.g, ao + mraSrc.b, specIntensity.x + mraSrc.a);
			gl_FragData[3] = lightMapSrc;
		}
	}
}