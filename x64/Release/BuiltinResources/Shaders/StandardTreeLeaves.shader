name "Forward/Nature/Standard Tree Leaves"
render_mode forward

params
{
	float "Cutout threshold" cutoutVal 0.25 : 0.0 1.0
	color "Color" vColor 1 1 1 1
	float "Specular" specIntensity 0.5 : 0.0 1.0
	define bool "Has albedo map" HAS_ALBEDO true
	sampler2D "Albedo map" albedoMap 0
	define bool "Has normal map" HAS_NORMAL_MAP false
	sampler2D "Normal map" normalMap 1
	float "Normal map scale" normalMapScale 1.0
	define bool "Realtime global illumination" ENABLE_GI true
}

pass //Ambient pass
{
	tags
	{
		backface_culling off
	}
	
	varying
	{
		vec3 a_position  : POSITION;
		vec3 a_normal    : NORMAL;
		vec3 a_tangent   : TANGENT;
		vec3 a_bitangent : BITANGENT;
		vec2 a_texcoord0 : TEXCOORD0;
		vec2 a_texcoord1 : TEXCOORD1;

		vec3 v_position    : POSITION1 = vec3(0.0, 0.0, 0.0);
		vec3 v_normal      : NORMAL    = vec3(0.0, 0.0, 0.0);
		vec3 v_tangent     : TANGENT   = vec3(0.0, 0.0, 0.0);
		vec3 v_bitangent   : BITANGENT = vec3(0.0, 0.0, 0.0);
		vec2 v_texcoord0   : TEXCOORD0 = vec2(0.0, 0.0);
		vec2 v_texcoord1   : TEXCOORD1 = vec2(0.0, 0.0);
		vec4 v_worldpos    : TEXCOORD6 = vec4(0.0, 0.0, 0.0, 0.0);
	}
	
	vertex
	{
		$input a_position, a_normal, a_tangent, a_bitangent, a_texcoord0, a_texcoord1
		$output v_position, v_normal, v_tangent, v_bitangent, v_texcoord0, v_texcoord1, v_worldpos

		#include "common.sh"

		uniform mat3 u_normalMatrix;
		uniform mat4 u_shadowMatrix[4];
		
		uniform vec4 u_lightPosition;
		uniform vec4 u_lightType;

		void main()
		{
			mat4 model = u_model[0];
			
			gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
			v_normal = mul(u_normalMatrix, a_normal);
			v_tangent = mul(u_normalMatrix, a_tangent);
			v_bitangent = mul(u_normalMatrix, a_bitangent);

			v_position = mul(model, vec4(a_position, 1.0)).xyz;
			v_texcoord0 = a_texcoord0;
			v_texcoord1 = a_texcoord1;
			v_worldpos = gl_Position;
		}
	}
	
	fragment
	{
		$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord0, v_texcoord1, v_worldpos
		
		#include "common.sh"
		
		uniform vec4 vColor;
#if HAS_ALBEDO == 1
		SAMPLER2D(albedoMap, 0);
#endif

		SAMPLER2D(u_lightMap, 8);
		uniform vec4 u_hasLightMap;
		
#if ENABLE_GI == 1
		SAMPLERCUBE(u_envMap, 9);
#endif
		
		uniform vec4 cutoutVal;
		uniform vec4 u_ambientColor;
		uniform vec4 u_camPos;
		uniform vec4 u_giParams; // x - enabled, y - power
		
		void main()
		{
			vec2 uvs = v_texcoord0;
			
			vec3 albedo = vColor.rgb;
			float alpha = 1.0;
#if HAS_ALBEDO == 1
			vec4 tex = texture2D(albedoMap, uvs);
			albedo = tex.rgb * vColor.rgb;
			
			if (tex.a < cutoutVal.x)
				discard;
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
				float roughness = 1.0;

				vec3 normal = normalize(v_normal);

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

			vec3 ambient = (u_ambientColor.rgb * (albedo * gi));
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
		backface_culling off
	}
	
	fragment
	{
		$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord0, v_texcoord1, v_worldpos

		#include "common.sh"
		#include "pbr.sh"

		uniform vec4 vColor;
		uniform vec4 specIntensity;

#if HAS_ALBEDO == 1
		SAMPLER2D(albedoMap, 0);
#endif
#if HAS_NORMAL_MAP == 1
		SAMPLER2D(normalMap, 1);
		uniform vec4 normalMapScale;
#endif

		SAMPLER2D(u_lightMap, 8);
		uniform vec4 u_hasLightMap;
		
		#if ENABLE_GI == 1
		SAMPLERCUBE(u_envMap, 9);
#endif

		uniform vec4 cutoutVal;

		uniform vec4 u_camPos;
		uniform vec4 u_lightPosition;
		uniform vec4 u_lightColor;
		uniform vec4 u_lightIntensity;
		uniform vec4 u_lightRadius;
		uniform vec4 u_lightType;
		uniform vec4 u_lightDirection;
		uniform vec4 u_lightRenderMode;
		uniform vec4 u_lightCastShadows;
		uniform vec4 u_giParams; // x - enabled, y - power

		const float PI = 3.14159265359;

		void main()
		{
			vec2 uvs = v_texcoord0;
			
#if HAS_ALBEDO == 1
			vec4 tex = texture2D(albedoMap, uvs);
			vec3 albedo = tex.rgb * vColor.rgb * 2.2;

			if (tex.a < cutoutVal.x)
				discard;
#else
			vec3 albedo = vColor.rgb * 2.2;
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
			
			float metallicVal = 0.0;
			float roughness = 0.5;

			vec3 N = normalize(normal);
			vec3 V = normalize(u_camPos.xyz - v_position);

			vec3 F0 = vec3(0.04);
			F0 = mix(F0, albedo, vec3(metallicVal));
			
			vec3 Lo = vec3(0.0);

			float radius = u_lightRadius.x;
			float innerRadius = u_lightRadius.y;
			float outerRadius = u_lightRadius.z;
			float intensity = u_lightIntensity.x * 10.0 / 2.2;

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
			
			//GI
			vec3 gi = vec3(1.0);
			
#if ENABLE_GI == 1
			if (u_giParams.x == 1.0)
			{
				vec3 R = reflect(V, N);
				
				float MAX_REFLECTION_LOD = 10.0;
				vec3 prefilteredColor = textureCubeLod(u_envMap, R, 1.0 * MAX_REFLECTION_LOD).rgb;
				gi = prefilteredColor * (F * 100.0 * u_giParams.y);
			}
#endif

			float NdotL = max(dot(N, L), 0.0);
			vec3 light = vec3(1.0) - (lightMap.rgb * lightMap.a);
			
			Lo += (kD * (albedo * gi) / PI * light + specular) * radiance * NdotL; 

			float visibility = 1.0;
			
			//Lightmap
			if (lightMap.a == 1)
			{
				if (u_lightRenderMode.x > 1) // Mixed or baked
				{
					float lightMapShadow = max(max(lightMap.r, lightMap.g), lightMap.b);
					visibility = clamp(lightMapShadow * 2.0, 0.0, 1.0);
				}
			}
			
			vec3 color = (Lo * vec3(1.0/2.2)) * visibility;

			gl_FragColor = vec4(color * vColor.a, 0.0);
		}
	}
}