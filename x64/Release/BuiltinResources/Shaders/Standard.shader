name "Forward/Standard"
render_mode forward

params
{
	define bool "Transparent" TRANSPARENT false
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
	define bool "Realtime global illumination" ENABLE_GI true
	define bool "Double sided" DOUBLE_SIDED false
}

pass //Ambient pass
{
	tags
	{
		iteration_mode default
		blend_mode = TRANSPARENT ? alpha : replace
		backface_culling = DOUBLE_SIDED ? off : cw
	}
	
	varying
	{
		vec3 a_position  : POSITION;
		vec3 a_normal    : NORMAL;
		vec3 a_tangent   : TANGENT;
		vec3 a_bitangent : BITANGENT;
		vec2 a_texcoord0 : TEXCOORD0;
		vec2 a_texcoord1 : TEXCOORD1;
		vec4 a_weight    : BLENDWEIGHT;
		vec4 a_indices   : BLENDINDICES;

		vec3 v_position    : POSITION1 = vec3(0.0, 0.0, 0.0);
		vec3 v_ls_position : POSITION2 = vec3(0.0, 0.0, 0.0);
		vec3 v_normal      : NORMAL    = vec3(0.0, 0.0, 0.0);
		vec3 v_tangent     : TANGENT   = vec3(0.0, 0.0, 0.0);
		vec3 v_bitangent   : BITANGENT = vec3(0.0, 0.0, 0.0);
		vec2 v_texcoord0   : TEXCOORD0 = vec2(0.0, 0.0);
		vec2 v_texcoord1   : TEXCOORD1 = vec2(0.0, 0.0);
		vec4 v_texcoord2   : TEXCOORD2 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_texcoord3   : TEXCOORD3 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_texcoord4   : TEXCOORD4 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_texcoord5   : TEXCOORD5 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_worldpos    : TEXCOORD6 = vec4(0.0, 0.0, 0.0, 0.0);
	}
	
	vertex
	{
		$input a_position, a_normal, a_tangent, a_bitangent, a_texcoord0, a_texcoord1, a_weight, a_indices
		$output v_position, v_normal, v_tangent, v_bitangent, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_ls_position, v_worldpos

		#include "common.sh"

		uniform mat3 u_normalMatrix;
		uniform mat4 u_shadowMatrix[4];
		uniform vec4 u_skinned;
		uniform mat4 u_boneMatrix[128];
		
		uniform vec4 u_lightPosition;
		uniform vec4 u_lightType;
		uniform vec4 u_lightCastShadows;

		void main()
		{
			mat4 model = u_model[0];
			
			if (u_skinned.x == 1.0)
			{
				model = mul(u_model[0], a_weight.x * u_boneMatrix[int(a_indices.x)] + 
										a_weight.y * u_boneMatrix[int(a_indices.y)] +
										a_weight.z * u_boneMatrix[int(a_indices.z)] +
										a_weight.w * u_boneMatrix[int(a_indices.w)]);
											
				mat3 norm = mul(u_normalMatrix, a_weight.x * mat3(u_boneMatrix[int(a_indices.x)]) + 
												a_weight.y * mat3(u_boneMatrix[int(a_indices.y)]) +
												a_weight.z * mat3(u_boneMatrix[int(a_indices.z)]) +
												a_weight.w * mat3(u_boneMatrix[int(a_indices.w)]));
											
				vec3 vwpos = mul(model, vec4(a_position, 1.0)).xyz;
				gl_Position = mul(u_viewProj, vec4(vwpos, 1.0));
				
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
			
			v_position = mul(model, vec4(a_position, 1.0)).xyz;
			v_texcoord0 = a_texcoord0;
			v_texcoord1 = a_texcoord1;
			v_worldpos = gl_Position;
			
			vec4 wpos = vec4(0.0, 0.0, 0.0, 1.0);
			
			if (u_lightCastShadows.x == 1)
			{
				if (u_lightType.x == 2) // Directional light
				{
					wpos = mul(model, vec4(a_position, 1.0));
					
					v_texcoord2 = mul(u_shadowMatrix[0], wpos);
					v_texcoord3 = mul(u_shadowMatrix[1], wpos);
					v_texcoord4 = mul(u_shadowMatrix[2], wpos);
					v_texcoord5 = mul(u_shadowMatrix[3], wpos);
				}
				else if (u_lightType.x == 1) // Spot light
				{
					wpos = mul(model, vec4(a_position, 1.0));
					v_texcoord2 = mul(u_shadowMatrix[0], wpos);
				}
				else // Point light
				{
					wpos = vec4(-u_lightPosition.xyz, 0.0) + mul(model, vec4(a_position, 1.0));
					v_ls_position = wpos.xyz;
					v_texcoord2 = mul(u_shadowMatrix[0], wpos);
					v_texcoord3 = mul(u_shadowMatrix[1], wpos);
					v_texcoord4 = mul(u_shadowMatrix[2], wpos);
					v_texcoord5 = mul(u_shadowMatrix[3], wpos);
				}
			}
		}
	}
	
	fragment
	{
		$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_ls_position, v_worldpos
		
		#include "common.sh"
		
		uniform vec4 vColor;
#if HAS_ALBEDO_MAP == 1
		SAMPLER2D(albedoMap, 0);
#endif

#if HAS_NORMAL_MAP == 1
		SAMPLER2D(normalMap, 1);
		uniform vec4 normalMapScale;
#endif

#if HAS_ROUGHNESS_MAP == 1
		SAMPLER2D(roughnessMap, 3);
#endif
		
		uniform vec4 roughnessVal;
		
#if HAS_EMISSION_MAP == 1
		SAMPLER2D(emissionMap, 5);
#endif

#if ENABLE_GI == 1
		SAMPLERCUBE(u_envMap, 9);
#endif
		
#if CUTOUT == 1
		uniform vec4 cutoutVal;
#endif
		
		SAMPLER2D(u_lightMap, 8);
		uniform vec4 u_hasLightMap;
		
		uniform vec4 u_ambientColor;
		uniform vec4 uvScale;
		uniform vec4 emissionVal;
		uniform vec4 u_giParams;
		uniform vec4 u_camPos;
		
		void main()
		{
			vec2 uvs = v_texcoord0 * uvScale.xy;
			
			vec3 albedo = vColor.rgb;
			float alpha = vColor.a;
			
#if HAS_ALBEDO_MAP == 1
			vec4 tex = texture2D(albedoMap, uvs);
			albedo = tex.rgb * vColor.rgb;
			
	#if CUTOUT == 1
			if (tex.a < cutoutVal.x)
				discard;
	#else
			alpha = tex.a * vColor.a;
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
			
			//GI
			vec3 gi = vec3(1.0);

#if ENABLE_GI
			if (u_giParams.x == 1.0)
			{
#if HAS_ROUGHNESS_MAP == 1
				float roughness = texture2D(roughnessMap, uvs).r * roughnessVal.x;
#else
				float roughness = roughnessVal.x;
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
				if (gl_FrontFacing)
					normal = -normal;
		
				vec3 N = normalize(normal);
				vec3 V = normalize(u_camPos.xyz - v_position);
		
				vec3 R = reflect(V, N);
		
				float MAX_REFLECTION_LOD = 10.0;
				vec3 prefilteredColor = textureCubeLod(u_envMap, R, roughness * MAX_REFLECTION_LOD).rgb;
				gi = prefilteredColor * u_giParams.y;
			}
#endif

			vec3 ambient = (u_ambientColor.rgb * (albedo * gi)) + (albedo * emission);
			gl_FragColor = vec4(ambient + (albedo * lightMap), alpha * vColor.a);
		}
	}
}

pass //Lighting pass. Use varyings and vertex program from previous pass to avoid z-fighting
{
	tags
	{
		blend_mode add
		iteration_mode per_light
		depth_write off
		backface_culling = DOUBLE_SIDED ? off : cw
	}
	
	fragment
	{
		$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_ls_position, v_worldpos

		#include "common.sh"
		#include "shadows.sh"
		#include "pbr.sh"

		uniform vec4 vColor;
		uniform vec4 specIntensity;

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
		uniform vec4 roughnessVal;
#if HAS_AO_MAP == 1
		SAMPLER2D(aoMap, 4);
#endif

		SAMPLER2D(u_lightMap, 8);
		uniform vec4 u_hasLightMap;
		
#if ENABLE_GI == 1
		SAMPLERCUBE(u_envMap, 11);
#endif

		SAMPLER2DSHADOW(u_shadowMap0, 12);
		SAMPLER2DSHADOW(u_shadowMap1, 13);
		SAMPLER2DSHADOW(u_shadowMap2, 14);
		SAMPLER2DSHADOW(u_shadowMap3, 15);
		
#if CUTOUT == 1
		uniform vec4 cutoutVal;
#endif

		uniform vec4 uvScale;

		uniform vec4 u_camPos;
		uniform vec4 u_lightPosition;
		uniform vec4 u_lightColor;
		uniform vec4 u_lightIntensity;
		uniform vec4 u_lightRadius;
		uniform vec4 u_lightType; // 0 - Point, 1 - Spot, 2 - Directional
		uniform vec4 u_lightRenderMode; // 0 - Realtime, 1 - Mixed, 2 - Baked
		uniform vec4 u_lightDirection;
		uniform vec4 u_lightShadowBias;
		uniform vec4 u_lightCastShadows;
		uniform vec4 u_shadowMapTexelSize;
		uniform vec4 u_shadowSamplingParams;
		
#if ENABLE_GI == 1
		uniform vec4 u_giParams; // x - enabled, y - power
#endif

		const float PI = 3.14159265359;

		void main()
		{
			vec2 uvs = v_texcoord0 * uvScale.xy;
			
			float alpha = 1.0;
			
#if HAS_ALBEDO_MAP == 1
			vec4 tex = texture2D(albedoMap, uvs);
			vec3 albedo = tex.rgb * vColor.rgb;// * 2.2;
			alpha = tex.a * vColor.a;
			
	#if CUTOUT == 1
				if (tex.a < cutoutVal.x)
					discard;
	#endif
#else
			vec3 albedo = vColor.rgb;// * 2.2;
			alpha = vColor.a;
#endif

			vec4 lightMap = texture2D(u_lightMap, v_texcoord1);
			lightMap.a = u_hasLightMap.x;

#if HAS_NORMAL_MAP == 1
			vec3 normal = texture2D(normalMap, uvs).rgb;
			normal = normalize(normal * 2.0 - 1.0);
			normal.xy *= normalMapScale.x;
			
			mat3 TBN = mat3(v_tangent, v_bitangent, v_normal);
			normal = normalize(TBN * normal);
#else
			vec3 normal = normalize(v_normal);
#endif
			if (gl_FrontFacing)
				normal = -normal;
			
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

			vec3 N = normalize(normal);
			vec3 V = normalize(u_camPos.xyz - v_position);

			vec3 F0 = vec3(0.04);
			F0 = mix(F0, albedo, vec3(metallicVal));
			
			vec3 Lo = vec3(0.0);

			float radius = u_lightRadius.x;
			float innerRadius = u_lightRadius.y;
			float outerRadius = u_lightRadius.z;
			float intensity = u_lightIntensity.x * 10.0;// / 2.2;

			vec3 L;
			if (u_lightType.x != 2)
				L = normalize(u_lightPosition.xyz - v_position);
			else
				L = normalize(-u_lightDirection.xyz);
			
			if (gl_FrontFacing)
				L = -L;

			vec3 H = normalize(V + L);

			float distance = length(u_lightPosition.xyz - v_position);

			//Compute attenuation
			float attenuation = 0.0;
			if (u_lightType.x == 0) // Point light
			{
				attenuation = smoothstep(radius, 0.0, distance) * intensity;
			}
			else if (u_lightType.x == 1) // Spot light
			{
				float spotAttenuation = 1.0;
				float spotDot = dot(L, -u_lightDirection.xyz);

				outerRadius = clamp(58.0 - outerRadius, 0.0, 58.0);
				innerRadius = clamp(58.0 - innerRadius, 0.0, 58.0);
				float spot = clamp((spotDot - radians(outerRadius)) / (radians(innerRadius) - radians(outerRadius)), 0.0, 1.0);
				attenuation = smoothstep(radius, 0.0, distance) * intensity * spot;
			}
			else // Directional light
			{
				attenuation = 1.0 * intensity;
			}

			vec3 radiance = u_lightColor.xyz * attenuation;

			// Cook-Torrance BRDF
			float NDF = DistributionGGX(N, H, roughness);
			float G   = GeometrySmith(N, V, L, roughness);
			vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			kD *= 1.0 - metallicVal;

			vec3 numerator    = NDF * G * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
			vec3 specular = numerator / max(denominator, 0.001) * specIntensity.x * 10.0;

			float NdotL = max(dot(N, L), 0.0);
			
			//GI
			vec3 gi = vec3(1.0);

#if ENABLE_GI == 1
			if (u_giParams.x == 1.0)
			{
				vec3 R = reflect(V, N);
				
				float MAX_REFLECTION_LOD = 10.0;
				vec3 prefilteredColor = textureCubeLod(u_envMap, R, roughness * MAX_REFLECTION_LOD).rgb;
				gi = prefilteredColor * (F * 100.0 * u_giParams.y);
			}
#endif
			if (lightMap.a != 1 || u_lightRenderMode.x == 0)
			{
				Lo += (kD * (albedo * gi) / PI + specular) * radiance * NdotL;
			}
			else
			{
				vec3 lightInv = vec3(1.0) - lightMap.rgb;
				vec3 light = lightMap.rgb;
				
				Lo += ((kD * (albedo * gi)) * 0.5 / PI + specular - (1.0 - clamp(kD, 0.0, 1.0))) * radiance * NdotL;
			}

			float visibility = 1.0;

			if (u_lightCastShadows.x == 1)
			{
				if (u_lightRenderMode.x < 2) // Realtime or mixed
				{
					visibility = getShadow(int(u_lightType.x),
											v_texcoord2,
											v_texcoord3,
											v_texcoord4,
											v_texcoord5,
											u_lightShadowBias,
											u_shadowSamplingParams,
											v_ls_position,
											u_shadowMapTexelSize.x,
											u_shadowMap0,
											u_shadowMap1,
											u_shadowMap2,
											u_shadowMap3);
				}
			}
			
			//Lightmap
			if (lightMap.a == 1)
			{
				if (u_lightRenderMode.x > 0) // Mixed or baked
				{
					float lightMapShadow = min(visibility, max(max(lightMap.r, lightMap.g), lightMap.b));
					visibility = clamp(lightMapShadow * 10.0, 0.0, 1.0);
				}
			}
				
			vec3 color = Lo * ao * visibility;
			color = color * vec3(1.0 / 2.2);
			
			gl_FragColor = vec4(color * alpha, 0.0);
		}
	}
}