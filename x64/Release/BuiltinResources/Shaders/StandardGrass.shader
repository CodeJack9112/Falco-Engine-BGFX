name "Forward/Nature/Standard Grass"
render_mode forward

params
{
	float "Cutout threshold" cutoutVal 0.25 : 0.0 1.0
	color "Color" vColor 1 1 1 1
	sampler2D "Albedo map" albedoMap 0
	define bool "Realtime global illumination" ENABLE_GI true
}

pass //Ambient pass
{
	tags
	{
		iteration_mode default
		backface_culling off
	}
	
	varying
	{
		vec3 a_position  : POSITION;
		vec2 a_texcoord0 : TEXCOORD0;
		vec2 a_texcoord1 : TEXCOORD1;
		vec3 a_normal    : NORMAL;

		vec3 v_position    : POSITION1 = vec3(0.0, 0.0, 0.0);
		vec3 v_ls_position : POSITION2 = vec3(0.0, 0.0, 0.0);
		vec2 v_texcoord0   : TEXCOORD0 = vec2(0.0, 0.0);
		vec2 v_texcoord1   : TEXCOORD1 = vec2(0.0, 0.0);
		vec4 v_texcoord2   : TEXCOORD2 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_texcoord3   : TEXCOORD3 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_texcoord4   : TEXCOORD4 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_texcoord5   : TEXCOORD5 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_worldpos    : TEXCOORD6 = vec4(0.0, 0.0, 0.0, 0.0);
		vec3 v_normal      : NORMAL    = vec3(0.0, 0.0, 0.0);
	}
	
	vertex
	{
		$input a_position, a_texcoord0, a_texcoord1, a_normal
		$output v_position, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_ls_position, v_worldpos, v_normal

		#include "common.sh"

		uniform mat4 u_shadowMatrix[4];
		uniform mat3 u_normalMatrix;
		
		uniform vec4 u_lightPosition;
		uniform vec4 u_lightType;
		uniform vec4 u_lightCastShadows;

		void main()
		{
			mat4 model = u_model[0];
			
			gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
			
			v_position = mul(model, vec4(a_position, 1.0)).xyz;
			v_normal = mul(u_normalMatrix, a_normal);
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
		$input v_position, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_ls_position, v_worldpos, v_normal
		
		#include "common.sh"
		
		uniform vec4 vColor;
		SAMPLER2D(albedoMap, 0);
		
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
			float alpha = 1.0f;

			vec4 tex = texture2D(albedoMap, uvs);
			albedo = tex.rgb * vColor.rgb;
			
			if (tex.a < cutoutVal.x)
				discard;
			
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
		$input v_position, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_ls_position, v_worldpos, v_normal

		#include "common.sh"
		#include "shadows.sh"

		uniform vec4 vColor;

		SAMPLER2D(albedoMap, 0);
		
		SAMPLER2D(u_lightMap, 8);
		uniform vec4 u_hasLightMap;
		
#if ENABLE_GI == 1
		SAMPLERCUBE(u_envMap, 9);
#endif

		SAMPLER2DSHADOW(u_shadowMap0, 12);
		SAMPLER2DSHADOW(u_shadowMap1, 13);
		SAMPLER2DSHADOW(u_shadowMap2, 14);
		SAMPLER2DSHADOW(u_shadowMap3, 15);
		
		uniform vec4 cutoutVal;

		uniform vec4 u_camPos;
		uniform vec4 u_lightPosition;
		uniform vec4 u_lightColor;
		uniform vec4 u_lightIntensity;
		uniform vec4 u_lightRadius;
		uniform vec4 u_lightType;
		uniform vec4 u_lightDirection;
		uniform vec4 u_lightShadowBias;
		uniform vec4 u_lightCastShadows;
		uniform vec4 u_lightRenderMode;
		uniform vec4 u_shadowMapTexelSize;
		uniform vec4 u_shadowSamplingParams;
		
		uniform vec4 u_giParams; // x - enabled, y - power

		const float PI = 3.14159265359;

		void main()
		{
			vec2 uvs = v_texcoord0;
			
			vec4 tex = texture2D(albedoMap, uvs);
			vec3 albedo = tex.rgb * vColor.rgb * 2.2;
			
			if (tex.a < cutoutVal.x)
				discard;
			
			vec4 lightMap = texture2D(u_lightMap, v_texcoord1);
			lightMap.a = u_hasLightMap.x;
			
			float radius = u_lightRadius.x;
			float innerRadius = u_lightRadius.y;
			float outerRadius = u_lightRadius.z;
			float intensity = u_lightIntensity.x;

			// расчет энергетической яркости для каждого источника света
			vec3 L;
			if (u_lightType.x != 2)
				L = normalize(u_lightPosition.xyz - v_position);
			else
				L = normalize(-u_lightDirection.xyz);
			
			if (gl_FrontFacing)
				L = -L;

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
				if (u_lightRenderMode.x > 1) // Mixed or baked
				{
					float lightMapShadow = min(visibility, max(max(lightMap.r, lightMap.g), lightMap.b));
					visibility = clamp(lightMapShadow * 2.0, 0.0, 1.0);
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
				gi = prefilteredColor * u_giParams.y * 5.0;
			}
#endif

			vec3 color = ((albedo * gi) * radiance) * visibility;
			color = color * vec3(1.0/2.2);

			gl_FragColor = vec4(color * vColor.a, 0.0);
		}
	}
}