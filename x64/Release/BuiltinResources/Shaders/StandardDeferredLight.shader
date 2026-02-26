pass
{
	varying
	{
		vec3 v_ls_position : POSITION2 = vec3(0.0, 0.0, 0.0);
		vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);

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
			gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
			v_texcoord0 = a_texcoord0;
		}
	}
	
	fragment
	{
		$input v_texcoord0
		
		#include "common.sh"
		#include "shaderlib.sh"
		#include "shadows.sh"
		#include "pbr.sh"
		
		SAMPLER2D(u_albedoMap, 0); // Rgb, 1.0
		SAMPLER2D(u_normalMap, 1); //Rgb, 1.0
		SAMPLER2D(u_mraMap, 2); //Metallic, roughness, ao, specularity
		SAMPLER2D(u_lightMap, 3); //LightMap
		SAMPLER2D(u_depthMap, 4); //Depth
		
		SAMPLERCUBE(u_envMap, 9);
		
		SAMPLER2DSHADOW(u_shadowMap0, 12);
		SAMPLER2DSHADOW(u_shadowMap1, 13);
		SAMPLER2DSHADOW(u_shadowMap2, 14);
		SAMPLER2DSHADOW(u_shadowMap3, 15);
		
		uniform mat4 u_invVP;
		
		uniform mat4 u_shadowMatrix[4];
		
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
		uniform vec4 u_giParams; // x - enabled, y - power
		
		const float PI = 3.14159265359;

		void main()
		{
			vec3  normal      = decodeNormalUint(texture2D(u_normalMap, v_texcoord0).xyz);
			float frontFace   = texture2D(u_normalMap, v_texcoord0).w;
			
			float deviceDepth = texture2D(u_depthMap, v_texcoord0).r;
			float depth       = toClipSpaceDepth(deviceDepth);

			vec3 clip = vec3(v_texcoord0 * 2.0 - 1.0, depth);
			vec3 wpos = clipToWorld(u_invVP, clip);
			
			vec3 v_ls_position = vec3(0.0, 0.0, 0.0);
			
			vec4 v_texcoord1 = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 v_texcoord2 = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 v_texcoord3 = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 v_texcoord4 = vec4(0.0, 0.0, 0.0, 0.0);
		
			if (u_lightCastShadows.x == 1 && u_lightRenderMode.x < 2)
			{
				if (u_lightType.x == 2) // Directional light
				{
					v_texcoord1 = mul(u_shadowMatrix[0], vec4(wpos, 1.0));
					v_texcoord2 = mul(u_shadowMatrix[1], vec4(wpos, 1.0));
					v_texcoord3 = mul(u_shadowMatrix[2], vec4(wpos, 1.0));
					v_texcoord4 = mul(u_shadowMatrix[3], vec4(wpos, 1.0));
				}
				else if (u_lightType.x == 1) // Spot light
				{
					v_texcoord1 = mul(u_shadowMatrix[0], vec4(wpos, 1.0));
				}
				else // Point light
				{
					vec3 _position = -u_lightPosition.xyz + wpos;
					v_ls_position = _position;
					v_texcoord1 = mul(u_shadowMatrix[0], vec4(_position, 1.0));
					v_texcoord2 = mul(u_shadowMatrix[1], vec4(_position, 1.0));
					v_texcoord3 = mul(u_shadowMatrix[2], vec4(_position, 1.0));
					v_texcoord4 = mul(u_shadowMatrix[3], vec4(_position, 1.0));
				}
			}

			vec3 albedo = texture2D(u_albedoMap, v_texcoord0).rgb;// * 2.2;
			vec4 lightMap = texture2D(u_lightMap, v_texcoord0);
			
			vec4 mra = texture2D(u_mraMap, v_texcoord0);
			float metallicVal  = mra.x;
			float roughness = mra.y;
			float ao = mra.z;

			vec3 N = normalize(normal);
			vec3 V = normalize(u_camPos.xyz - wpos);

			vec3 F0 = vec3(0.04);
			F0 = mix(F0, albedo, vec3(metallicVal));
			
			vec3 Lo = vec3(0.0);

			float radius = u_lightRadius.x;
			float innerRadius = u_lightRadius.y;
			float outerRadius = u_lightRadius.z;
			float intensity = u_lightIntensity.x * 10.0;// / 2.2;

			vec3 L;
			if (u_lightType.x != 2)
				L = normalize(u_lightPosition.xyz - wpos);
			else
				L = normalize(-u_lightDirection.xyz);
			
			if (frontFace == 1.0)
				L = -L;

			vec3 H = normalize(V + L);

			float distance = length(u_lightPosition.xyz - wpos);

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

			vec3 radiance = u_lightColor.rgb * attenuation;

			// Cook-Torrance BRDF
			float NDF = DistributionGGX(N, H, roughness);
			float G   = GeometrySmith(N, V, L, roughness);
			vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			kD *= 1.0 - metallicVal;

			vec3 numerator    = NDF * G * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
			vec3 specular = numerator / max(denominator, 0.001) * mra.w * 10.0;

			float NdotL = max(dot(N, L), 0.0);
			
			//GI
			vec3 gi = vec3(1.0);

			if (u_giParams.x == 1.0)
			{
				vec3 R = reflect(V, N);
				
				float MAX_REFLECTION_LOD = 10.0;
				vec3 prefilteredColor = textureCubeLod(u_envMap, R, roughness * MAX_REFLECTION_LOD).rgb;
				vec2 envBRDF = vec2(1.0, 0.0);
				gi = prefilteredColor * (F * 100.0 * u_giParams.y);
			}
			
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
			
			gl_FragColor = vec4(color, 0.0);
		}
	}
}