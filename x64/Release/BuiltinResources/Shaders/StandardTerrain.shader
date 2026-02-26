name "Forward/Nature/Standard Terrain"
render_mode forward

params
{
	float "Specular" specIntensity 0.1 : 0.0 1.0
	define bool "Has albedo map" HAS_ALBEDO true
	define bool "Has normal map" HAS_NORMAL_MAP true
	define bool "Realtime global illumination" ENABLE_GI true
}

pass //Ambient pass
{
	tags
	{
		iteration_mode default
	}
	
	varying
	{
		vec3 a_position  : POSITION;
		vec3 a_normal    : NORMAL;
		vec3 a_tangent   : TANGENT;
		vec3 a_bitangent : BITANGENT;
		vec2 a_texcoord0 : TEXCOORD0;

		vec3 v_position    : POSITION1 = vec3(0.0, 0.0, 0.0);
		vec3 v_ls_position : POSITION2 = vec3(0.0, 0.0, 0.0);
		vec3 v_normal      : NORMAL    = vec3(0.0, 0.0, 0.0);
		vec3 v_tangent     : TANGENT   = vec3(0.0, 0.0, 0.0);
		vec3 v_bitangent   : BITANGENT   = vec3(0.0, 0.0, 0.0);
		vec2 v_texcoord0   : TEXCOORD0 = vec2(0.0, 0.0);
		vec4 v_texcoord1   : TEXCOORD1 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_texcoord2   : TEXCOORD2 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_texcoord3   : TEXCOORD3 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_texcoord4   : TEXCOORD4 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_worldpos    : TEXCOORD6 = vec4(0.0, 0.0, 0.0, 0.0);
	}
	
	vertex
	{
		$input a_position, a_normal, a_tangent, a_bitangent, a_texcoord0
		$output v_position, v_normal, v_tangent, v_bitangent, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_ls_position, v_worldpos

		#include "common.sh"

		uniform mat3 u_normalMatrix;
		uniform mat4 u_shadowMatrix[4];
		
		uniform vec4 u_lightPosition;
		uniform vec4 u_lightType;
		uniform vec4 u_lightCastShadows;

		void main()
		{
			mat4 model = u_model[0];
			
			gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
			v_normal = mul(u_normalMatrix, a_normal);
			v_tangent = mul(u_normalMatrix, a_tangent);
			v_bitangent = mul(u_normalMatrix, a_bitangent);

			v_position = mul(model, vec4(a_position, 1.0)).xyz;
			v_texcoord0 = a_texcoord0;
			v_worldpos = gl_Position;
			
			vec4 wpos = vec4(0.0, 0.0, 0.0, 1.0);
			
			if (u_lightCastShadows.x == 1)
			{
				if (u_lightType.x == 2) // Directional light
				{
					wpos = mul(model, vec4(a_position, 1.0));
					
					v_texcoord1 = mul(u_shadowMatrix[0], wpos);
					v_texcoord2 = mul(u_shadowMatrix[1], wpos);
					v_texcoord3 = mul(u_shadowMatrix[2], wpos);
					v_texcoord4 = mul(u_shadowMatrix[3], wpos);
				}
				else if (u_lightType.x == 1) // Spot light
				{
					wpos = mul(model, vec4(a_position, 1.0));
					v_texcoord1 = mul(u_shadowMatrix[0], wpos);
				}
				else // Point light
				{
					wpos = vec4(-u_lightPosition.xyz, 0.0) + mul(model, vec4(a_position, 1.0));
					v_ls_position = wpos.xyz;
					v_texcoord1 = mul(u_shadowMatrix[0], wpos);
					v_texcoord2 = mul(u_shadowMatrix[1], wpos);
					v_texcoord3 = mul(u_shadowMatrix[2], wpos);
					v_texcoord4 = mul(u_shadowMatrix[3], wpos);
				}
			}
		}
	}
	
	fragment
	{
		$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_ls_position, v_worldpos
		
		#include "common.sh"
		
		uniform vec4 u_ambientColor;
		
		uniform vec4 u_textureCount;
		uniform vec4 u_textureSizes[8];
		
		uniform vec4 u_giParams; // x - enabled, y - power
		uniform vec4 u_camPos;
		
#if HAS_ALBEDO == 1
		SAMPLER2D(u_texture0, 0);
		SAMPLER2D(u_texture1, 1);
		SAMPLER2D(u_texture2, 2);
		SAMPLER2D(u_texture3, 3);
		SAMPLER2D(u_texture4, 4);
#endif

#if ENABLE_GI == 1
		SAMPLERCUBE(u_envMap, 9);
#endif

		SAMPLER2D(u_texture0Splat, 10);
		SAMPLER2D(u_texture1Splat, 11);
		
		void main()
		{
			vec4 albedo = vec4(1.0, 1.0, 1.0, 1.0);
			
			if (u_textureCount.x > 0)
			{
				vec2 uvs = v_texcoord0 * u_textureSizes[0].xy;
				
#if HAS_ALBEDO == 1
				albedo = texture2D(u_texture0, uvs);
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
				}
			}
			
			//GI
			vec3 gi = vec3(1.0);
#if ENABLE_GI
			if (u_giParams.x == 1.0)
			{
				float roughness = 1.0;

				vec3 normal = normalize(v_normal);

				if (gl_FrontFacing)
					normal = -normal;
		
				vec3 N = normalize(normal);
				vec3 V = normalize(u_camPos.xyz - v_position);
		
				vec3 R = reflect(V, N);
		
				float MAX_REFLECTION_LOD = 10.0;
				vec3 prefilteredColor = textureCubeLod(u_envMap, R, roughness * MAX_REFLECTION_LOD).rgb;
				vec2 envBRDF = vec2(1.0, 0.0);
				gi = prefilteredColor * u_giParams.y;
			}
#endif
			
			vec3 ambient = u_ambientColor.rgb * (albedo.rgb * gi);
			gl_FragColor = vec4(ambient, 1.0);
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
	}
	
	fragment
	{
		$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_ls_position, v_worldpos

		#include "common.sh"
		#include "pbr.sh"
		#include "shadows.sh"

		uniform vec4 specIntensity;

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
		
		uniform vec4 u_textureCount;
		uniform vec4 u_textureSizes[5];
		
		uniform vec4 u_giParams; // x - enabled, y - power
		
#if HAS_ALBEDO == 1
		SAMPLER2D(u_texture0, 0);
		SAMPLER2D(u_texture1, 1);
		SAMPLER2D(u_texture2, 2);
		SAMPLER2D(u_texture3, 3);
		SAMPLER2D(u_texture4, 4);
#endif

#if ENABLE_GI == 1
		SAMPLERCUBE(u_envMap, 9);
#else
	#if HAS_NORMAL_MAP == 1
		SAMPLER2D(u_texture0Normal, 5);
		SAMPLER2D(u_texture1Normal, 6);
		SAMPLER2D(u_texture2Normal, 7);
		SAMPLER2D(u_texture3Normal, 8);
		SAMPLER2D(u_texture4Normal, 9);
	#endif
#endif
		SAMPLER2D(u_texture0Splat, 10);
		SAMPLER2D(u_texture1Splat, 11);
		
		SAMPLER2DSHADOW(u_shadowMap0, 12);
		SAMPLER2DSHADOW(u_shadowMap1, 13);
		SAMPLER2DSHADOW(u_shadowMap2, 14);
		SAMPLER2DSHADOW(u_shadowMap3, 15);

		const float PI = 3.14159265359;

		void main()
		{
			vec4 albedo = vec4(1.0);
			vec3 normal = normalize(v_normal);

#if HAS_NORMAL_MAP == 1
	#if ENABLE_GI != 1
			mat3 TBN = mat3(v_tangent, v_bitangent, v_normal);
	#endif
#endif

			if (u_textureCount.x > 0)
			{
				vec2 uvs = v_texcoord0 * u_textureSizes[0].xy;
				
#if HAS_ALBEDO == 1
				albedo = texture2D(u_texture0, uvs);
#endif
#if HAS_NORMAL_MAP == 1
	#if ENABLE_GI != 1
				normal = texture2D(u_texture0Normal, uvs).rgb;
				normal = normalize(normal * 2.0 - 1.0);
				normal = normalize(TBN * normal);
	#endif
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
					
					albedo = ((albedo * vec4(vec3(1.0 - color_mask), 1.0)) + vec4(color * color_mask, 1.0));
#endif
#if HAS_NORMAL_MAP == 1
	#if ENABLE_GI != 1
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
#endif
				}
			}
			
			albedo = albedo * 2.2;
			float metallicVal = 0.0;
			float roughness = 1.0;
			float ao = 1.0;

			vec3 N = normalize(normal);
			vec3 V = normalize(u_camPos.xyz - v_position);

			vec3 F0 = vec3(0.04);
			F0 = mix(F0, albedo.rgb, vec3(metallicVal));
			
			// выражение отражающей способности
			vec3 Lo = vec3(0.0);

			float radius = u_lightRadius.x;
			float innerRadius = u_lightRadius.y;
			float outerRadius = u_lightRadius.z;
			float intensity = u_lightIntensity.x * 10.0 / 2.2;

			// расчет энергетической яркости для каждого источника света
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

			// прибавляем результат к исходящей энергетической яркости Lo
			
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
			
			float NdotL = max(dot(N, L), 0.0);
			Lo += (kD * (albedo.rgb * gi) / PI + specular) * radiance * NdotL;
			
			float visibility = 1.0;

			if (u_lightCastShadows.x == 1)
			{
				if (u_lightRenderMode.x < 2)
				{
					visibility = getShadow(int(u_lightType.x),
											v_texcoord1,
											v_texcoord2,
											v_texcoord3,
											v_texcoord4,
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

			vec3 color = Lo * ao * visibility;
			color = color * vec3(1.0 / 2.2);

			gl_FragColor = vec4(color, 0.0);
		}
	}
}