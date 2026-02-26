name "Deferred/Standard"
render_mode deferred

params
{
	define bool "Cutout" CUTOUT false
	float "Cutout threshold" cutoutVal 0.25 : 0.0 1.0
	color "Color" vColor 1 1 1 1
	float "Specular" specIntensity 0.5 : 0.0 1.0
	define bool "Has albedo map" HAS_ALBEDO_MAP true
	sampler2D "Albedo map" albedoMap 0
	define bool "Has normal map" HAS_NORMAL_MAP false
	sampler2D "Normal map" normalMap 1
	float "Normal map scale" normalMapScale 1.0
	define bool "Has metallic map" HAS_METALLIC_MAP false
	sampler2D "Metallic map" metallicMap 2
	float "Metalness" metalness 0.0 : 0.0 1.0
	define bool "Has roughness map" HAS_ROUGHNESS_MAP false
	sampler2D "Roughness map" roughnessMap 3
	float "Roughness" roughnessVal 0.5 : 0.0 1.0
	define bool "Has AO map" HAS_AO_MAP false
	sampler2D "AO map" aoMap 4
	define bool "Has emission map" HAS_EMISSION_MAP false
	sampler2D "Emission map" emissionMap 5
	float "Emission" emissionVal 0.0
	vec2 "UV scale" uvScale 1 1
	vec2 "UV offset" uvOffset 0 0
	define bool "Double sided" DOUBLE_SIDED false
}

pass
{
	tags
	{
		backface_culling = DOUBLE_SIDED ? off : cw
	}
	
	varying
	{
		vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
		vec2 v_texcoord1 : TEXCOORD1 = vec2(0.0, 0.0);
		vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 1.0);
		vec3 v_tangent   : TANGENT   = vec3(1.0, 0.0, 0.0);
		vec3 v_bitangent : BITANGENT = vec3(0.0, 1.0, 0.0);

		vec3 a_position  : POSITION;
		vec3 a_normal    : NORMAL;
		vec3 a_tangent   : TANGENT;
		vec3 a_bitangent : BITANGENT;
		vec2 a_texcoord0 : TEXCOORD0;
		vec2 a_texcoord1 : TEXCOORD1;
		vec4 a_weight    : BLENDWEIGHT;
		vec4 a_indices   : BLENDINDICES;
	}
	
	vertex
	{
		$input a_position, a_normal, a_tangent, a_bitangent, a_texcoord0, a_texcoord1, a_weight, a_indices
		$output v_normal, v_tangent, v_bitangent, v_texcoord0, v_texcoord1
		
		#include "common.sh"
		
		uniform mat3 u_normalMatrix;
		uniform vec4 u_skinned;
		uniform mat4 u_boneMatrix[128];
		
		void main()
		{
			if (u_skinned.x == 1.0)
			{
				mat4 model = mul(u_model[0], a_weight.x * u_boneMatrix[int(a_indices.x)] + 
											a_weight.y * u_boneMatrix[int(a_indices.y)] +
											a_weight.z * u_boneMatrix[int(a_indices.z)] +
											a_weight.w * u_boneMatrix[int(a_indices.w)]);
											
				mat3 norm = mul(u_normalMatrix, a_weight.x * mat3(u_boneMatrix[int(a_indices.x)]) + 
													a_weight.y * mat3(u_boneMatrix[int(a_indices.y)]) +
													a_weight.z * mat3(u_boneMatrix[int(a_indices.z)]) +
													a_weight.w * mat3(u_boneMatrix[int(a_indices.w)]));
											
				vec3 wpos = mul(model, vec4(a_position, 1.0)).xyz;
				gl_Position = mul(u_viewProj, vec4(wpos, 1.0));
				
				v_normal = mul(norm, a_normal);
				v_tangent = mul(norm, a_tangent);
				v_bitangent = mul(norm, a_bitangent);
			}
			else
			{
				gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
				
				v_normal = mul(u_normalMatrix, a_normal);
				v_tangent = mul(u_normalMatrix, a_tangent);
				v_bitangent = mul(u_normalMatrix, a_bitangent);
			}
			
			v_texcoord0 = a_texcoord0;
			v_texcoord1 = a_texcoord1;
		}
	}
	
	fragment
	{
		$input v_normal, v_tangent, v_bitangent, v_texcoord0, v_texcoord1
		
		#include "common.sh"
		#include "shaderlib.sh"
		
		uniform vec4 vColor;
		uniform vec4 specIntensity;
		
#if CUTOUT == 1
		uniform vec4 cutoutVal;
#endif
		
#if HAS_ALBEDO_MAP == 1
		SAMPLER2D(albedoMap, 0);
#endif
#if HAS_NORMAL_MAP == 1
		SAMPLER2D(normalMap, 1);
		uniform vec4 normalMapScale;
#endif
#if HAS_METALLIC_MAP == 1
		SAMPLER2D(metallicMap, 2);
#endif
		uniform vec4 metalness;
#if HAS_ROUGHNESS_MAP == 1
		SAMPLER2D(roughnessMap, 3);
#endif
#if HAS_AO_MAP == 1
		SAMPLER2D(aoMap, 4);
#endif
#if HAS_EMISSION_MAP == 1
		SAMPLER2D(emissionMap, 5);
#endif
		
		SAMPLER2D(u_lightMap, 8);
		uniform vec4 u_hasLightMap;
		
		uniform vec4 u_ambientColor;
		uniform vec4 uvScale;
		uniform vec4 uvOffset;
		uniform vec4 roughnessVal;
		uniform vec4 emissionVal;
		
		void main()
		{
			vec2 uvs = v_texcoord0 * uvScale.xy + uvOffset.xy;
			
#if HAS_ALBEDO_MAP == 1
			vec4 diffuseColor = texture2D(albedoMap, uvs) * vColor;
			
	#if CUTOUT == 1
			if (diffuseColor.a < cutoutVal.x)
				discard;
	#endif
#else
			vec4 diffuseColor = vColor;
#endif
#if HAS_METALLIC_MAP == 1
			float metallicVal  = texture2D(metallicMap, uvs).r * metalness.x;
#else
			float metallicVal = metalness.x;
#endif
#if HAS_ROUGHNESS_MAP == 1
			float roughness = texture2D(roughnessMap, uvs).r * roughnessVal.x;
#else
			float roughness = roughnessVal.x;
#endif
#if HAS_AO_MAP == 1
			float ao = texture2D(aoMap, uvs).r;
#else
			float ao = 1.0;
#endif
#if HAS_EMISSION_MAP == 1
			float emission = texture2D(emissionMap, uvs).r * emissionVal.x;
#else
			float emission = emissionVal.x;
#endif
#if HAS_NORMAL_MAP == 1
			vec3 normal = texture2D(normalMap, uvs).rgb;
			normal = normalize(normal * 2.0 - 1.0);
			normal.xy *= normalMapScale.x;
			
			mat3 TBN = mat3(v_tangent, v_bitangent, v_normal);
			normal = normalize(TBN * normal);
#else
			vec3 normal = normalize(v_normal);
#endif

			vec3 lightMap = vec3(0.0);
			
			if (u_hasLightMap.x == 1)
			{
				lightMap = texture2D(u_lightMap, v_texcoord1).rgb;
			}
			
			if (gl_FrontFacing)
				normal = -normal;
			
			gl_FragData[0] = vec4(diffuseColor.rgb * (emission + 1.0), emission);
			gl_FragData[1] = vec4(encodeNormalUint(normal), gl_FrontFacing ? 1.0 : 0.0);
			gl_FragData[2] = vec4(metallicVal, roughness, ao, specIntensity.x);
			gl_FragData[3] = vec4(lightMap, u_hasLightMap.x);
		}
	}
}