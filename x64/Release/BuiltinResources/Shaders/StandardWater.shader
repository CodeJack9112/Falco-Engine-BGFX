name "Hidden/Forward/Standard Water"
render_mode forward

params
{
	define bool "Displacement" USE_DISPLACEMENT true
	define bool "Mean sky radiance" USE_MEAN_SKY_RADIANCE false
	define bool "Filtering" USE_FILTERING true
	define bool "Foam" USE_FOAM true
	define bool "Caustics" USE_CAUSTICS true
	define bool "Blinn Phong" BLINN_PHONG false
	hidden define bool "Reflections" USE_REFLECTIONS true
	float "Ambient Intensity" _AmbientDensity 0.15 : 0 1
	float "Diffuse Intensity" _DiffuseDensity 0.1 : 0 1
	color "Surface Color" _SurfaceColor 0.0078 0.5176 0.7 1.0
	color "Shore Tint Color" _ShoreColor 0.0078 0.5176 0.7 1.0
	color "Deep Color" _DepthColor 0.0039 0.00196 0.145 1.0
	samplerCube "Sky Texture" _SkyTexture 0
	sampler2D "Normal Texture" _NormalTexture 1
	float "Normal Intensity" _NormalIntensity 0.5 : 0 1
	float "Texture Tiling" _TextureTiling 1.0
	vec4 "Wind Direction" _WindDirection -4.5 -6 0 0
	sampler2D "Height Texture" _HeightTexture 2
	float "Height Intensity" _HeightIntensity 0.5 : 0 1
	float "Wave Tiling" _WaveTiling 1.0
	float "Wave Amplitude Factor" _WaveAmplitudeFactor 1.0
	float "Wave Steepness" _WaveSteepness 0.5 : 0 1
	vec4 "Waves Amplitude" _WaveAmplitude 0.13 0.3 0.1 0.05
	vec4 "Waves Intensity" _WavesIntensity 5 3 2 1.3
	vec4 "Waves Noise" _WavesNoise 0.15 0.32 0.15 0.15
	float "Water Clarity" _WaterClarity 0.75 : 0 3
	float "Water Transparency" _WaterTransparency 10.0 : 0 30
	vec4 "Horizontal Extinction" _HorizontalExtinction 3.0 11.6 16.7 1
	vec4 "Refraction/Reflection" _RefractionValues 0.17 0.11 0.31 1
	float "Refraction Scale" _RefractionScale 0.005 : 0 0.03
	float "Shininess" _Shininess 0.25 : 0 3
	vec4 "Specular Intensity" _SpecularValues 30 768 0.8 1
	float "Distortion" _Distortion 0.0345 : 0 0.15
	float "Radiance Factor" _RadianceFactor 0.2 : 0 1.0
	sampler2D "Foam Texture" _FoamTexture 3
	sampler2D "Shore Texture" _ShoreTexture 4
	vec4 "Foam Tiling" _FoamTiling 2.0 0.5 0.0 1.0
	vec4 "Foam Ranges" _FoamRanges 2.25 3.3 10.4 1.0
	vec4 "Foam Noise" _FoamNoise 0.37 0.5 -0.3 0.05
	vec2 "Foam Speed" _FoamSpeed 10 10
	float "Foam Intensity" _FoamIntensity 0.3 : 0 1
	float "Shore Fade" _ShoreFade 0.3 : 0.1 3
	sampler2D "Caustics Texture" _CausticsTex 5
	vec4 "Caustics 1 ST" _Caustics1_ST 2 2 0 0
	vec4 "Caustics 2 ST" _Caustics2_ST 2 2 0.5 0.5
	float "Caustics 1 Speed" _Caustics1_Speed 0.01
	float "Caustics 2 Speed" _Caustics2_Speed -0.007
	float "Caustics Intensity" _CausticsIntensity 0.7 : 0 1
	float "Caustics RGB Split" _CausticsRGBSplit 0.008 : 0 0.02
	hidden sampler2D backBufferDepth "Depth Texture" _CameraDepthTexture 6
	hidden sampler2D "Reflection Texture" u_reflectionTexture 7
	hidden sampler2D backBufferColor "Refraction Texture" _RefractionTexture 8
}

pass
{
	tags
	{
		
	}
	
	varying
	{
		vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
		vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 1.0);
		vec3 v_tangent   : TANGENT   = vec3(1.0, 0.0, 0.0);
		vec3 v_bitangent : BITANGENT = vec3(0.0, 1.0, 0.0);
		vec2 v_timer	 : TEXCOORD1 = vec2(0.0, 0.0);
		vec4 v_pos		 : POSITION0 = vec4(0.0, 0.0, 0.0, 0.0);
		vec3 v_worldPos	 : POSITION1 = vec3(0.0, 0.0, 0.0);
		vec4 v_projPos	 : POSITION2 = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 v_wind		 : POSITION3 = vec4(0.0, 0.0, 0.0, 0.0);

		vec3 a_position  : POSITION;
		vec3 a_normal    : NORMAL;
		vec3 a_tangent   : TANGENT;
		vec3 a_bitangent : BITANGENT;
		vec2 a_texcoord0 : TEXCOORD0;
	}
	
	vertex
	{
		$input a_position, a_normal, a_tangent, a_bitangent, a_texcoord0
		$output v_normal, v_tangent, v_bitangent, v_texcoord0, v_timer, v_pos, v_worldPos, v_projPos, v_wind
		
		#define _VERTEX_
		
		#include "common.sh"
		#include "water/waves.sh"
		#include "snoise.sh"
		#include "unpack.sh"
		#include "water/displacement.sh"
		
		#undef _VERTEX_
		
		uniform vec4 u_timeScaled;
		uniform vec4 u_camPos;
		uniform mat4 u_invModel;
		uniform vec4 u_clipPlane;
		uniform vec4 u_screenParams;
		uniform mat3 u_normalMatrix;
		
		// Wind direction in world coordinates, amplitude encoded as the length of the vector
		uniform vec4 _WindDirection;
		uniform vec4 _WaveTiling;
		uniform vec4 _WaveSteepness;
		uniform vec4 _WaveAmplitudeFactor;
		// Displacement amplitude of multiple waves, x = smallest waves, w = largest waves
		uniform vec4 _WaveAmplitude;
		// Intensity of multiple waves, affects the frequency of specific waves, x = smallest waves, w = largest waves
		uniform vec4 _WavesIntensity;
		// Noise of multiple waves, x = smallest waves, w = largest waves
		uniform vec4 _WavesNoise;
		uniform vec4 _TextureTiling;
		uniform vec4 _HeightIntensity;
		uniform sampler2D _HeightTexture;
		
		vec4 _ProjectionParams = vec4(1.0, u_clipPlane.x, u_clipPlane.y, 1.0 / u_clipPlane.y);
		vec4 _ScreenParams = u_screenParams;
		
		vec4 ComputeScreenPos (vec4 pos)
		{
			vec4 o = pos * 0.5;
			o.xy += o.w;
			o.zw = pos.zw;
			return o;
		}

		void main()
		{
			vec2 windDir = _WindDirection.xy;
			float windSpeed = length(_WindDirection.xy);
			windDir /= windSpeed;
			float timer = u_timeScaled.x * windSpeed * 0.5;

			vec4 modelPos = vec4(a_position, 1.0);
			vec3 worldPos = mul(u_model[0], vec4(modelPos.xyz, 1.0)).xyz;
			vec3 normal = vec3(0, 1, 0);

#if USE_DISPLACEMENT == 1
			float cameraDistance = length(u_camPos.xyz - worldPos);
			vec2 noise = GetNoise(worldPos.xz, timer * windDir * 0.5);

			vec3 tangent;
			vec4 waveSettings = vec4(windDir, _WaveSteepness.x, _WaveTiling.x);
			vec4 waveAmplitudes = vec4(_WaveAmplitude.xyz * _WaveAmplitudeFactor.x, 1.0);
			worldPos = ComputeDisplacement(worldPos, cameraDistance, noise, timer,
				waveSettings, waveAmplitudes, _WavesIntensity, _WavesNoise,
				normal, tangent);

			// add extra noise height from a heightmap
			float heightIntensity = _HeightIntensity.x * (1.0 - cameraDistance / 100.0) * _WaveAmplitude.x;
			vec2 texCoord = worldPos.xz * (0.05 * _TextureTiling.x);
			if (heightIntensity > 0.02)
			{
				float height = ComputeNoiseHeight(_HeightTexture, _WavesIntensity, _WavesNoise, texCoord, noise, vec2(timer));
				worldPos.y += height * heightIntensity;
			}

			modelPos = mul(u_invModel, vec4(worldPos, 1));
			v_tangent = tangent;
			v_bitangent = cross(normal, tangent);
#endif
			vec2 uv = worldPos.xz;

			v_timer.x = timer;
			v_wind.xy = windDir;
			v_wind.zw = windDir * timer;

			v_texcoord0 = uv * (0.05 * _TextureTiling.x);
			v_pos = mul(u_modelViewProj, modelPos);
			v_worldPos = worldPos;
			v_projPos = ComputeScreenPos(v_pos);
			v_normal = normal;
			
			gl_Position = mul(u_modelViewProj, vec4(modelPos.xyz, 1.0));
		}
	}
	
	fragment
	{
		$input v_normal, v_tangent, v_bitangent, v_texcoord0, v_timer, v_pos, v_worldPos, v_projPos, v_wind
		
		#include "common.sh"
		#include "shaderlib.sh"
		#include "snoise.sh"
		#include "unpack.sh"
		#include "normals.sh"
		#include "water/depth.sh"
		#include "water/waves.sh"
		#include "water/displacement.sh"
		#include "water/foam.sh"
		#include "water/meansky.sh"
		#include "water/radiance.sh"
		
		uniform samplerCube _SkyTexture;
		uniform sampler2D _NormalTexture;
		uniform sampler2D _HeightTexture;
		uniform sampler2D _FoamTexture;
		uniform sampler2D _ShoreTexture;
		uniform sampler2D _CausticsTex;
		uniform sampler2D _CameraDepthTexture;
		uniform sampler2D u_reflectionTexture;
		uniform sampler2D _RefractionTexture;

		uniform vec4 u_camPos;
		uniform vec4 u_lightDirection;
		uniform vec4 u_lightColor;
		uniform vec4 u_ambientColor;

		mat4 _ViewProjectInverse = u_invViewProj;

		uniform vec4 _TimeEditor;
		uniform vec4 _AmbientDensity;
		uniform vec4 _DiffuseDensity;
		uniform vec4 _HeightIntensity;
		uniform vec4 _NormalIntensity;

		vec4 _LightColor0 = u_lightColor;
		uniform vec4 _SurfaceColor;
		uniform vec4 _ShoreColor;
		uniform vec4 _DepthColor;
		
		uniform vec4 _WaveAmplitude;
		uniform vec4 _WavesIntensity;
		uniform vec4 _WavesNoise;
		// Affects how fast the colors will fade out, thus, use smaller values (eg. 0.05f).
		// to have crystal clear water and bigger to achieve "muddy" water.
		uniform vec4 _WaterClarity;
		// Water transparency along eye vector
		uniform vec4 _WaterTransparency;
		// Horizontal extinction of the RGB channels, in world coordinates. 
		// Red wavelengths dissapear(get absorbed) at around 5m, followed by green(75m) and blue(300m).
		uniform vec4 _HorizontalExtinction;
		uniform vec4 _Shininess;
		// xy = Specular intensity values, z = shininess exponential factor.
		uniform vec4 _SpecularValues;
		// x = index of refraction constant, y = refraction intensity
		// if you want to empasize reflections use values smaller than 0 for refraction intensity.
		uniform vec4 _RefractionValues;
		// Amount of wave refraction, of zero then no refraction. 
		uniform vec4 _RefractionScale;
		// Reflective radiance factor.
		uniform vec4 _RadianceFactor;
		// Reflection distortion, the higher the more distortion.
		uniform vec4 _Distortion;
		// x = range for shore foam, y = range for near shore foam, z = threshold for wave foam
		uniform vec4 _FoamRanges;
		// x = noise for shore, y = noise for outer
		// z = speed of the noise for shore, y = speed of the noise for outer, not that speed can be negative
		uniform vec4 _FoamNoise;
		uniform vec4 _FoamTiling;
		// Extra speed applied to the wind speed near the shore
		uniform vec4 _FoamSpeed;
		uniform vec4 _FoamIntensity;
		uniform vec4 _ShoreFade;
		
		uniform vec4 _Caustics1_ST;
		uniform vec4 _Caustics2_ST;
		uniform vec4 _Caustics1_Speed;
		uniform vec4 _Caustics2_Speed;
		uniform vec4 _CausticsIntensity;
		uniform vec4 _CausticsRGBSplit;
		
		vec4 NdcToClipPos(vec3 ndc)
		{
			// map xy to -1,1
			vec4 clipPos = vec4(ndc.xy * 2.0 - 1.0, ndc.z, 1.0);

		//#if UNITY_REVERSED_Z == 1
			//D3d with reversed Z
			//clipPos.z = 1.0f - clipPos.z;
		//#elif UNITY_UV_STARTS_AT_TOP == 1
			//D3d without reversed z
		//#else
			//opengl, map to -1,1
			clipPos.z = clipPos.z * 2.0 - 1.0;
		//#endif

			return clipPos;
		}

		vec3 NdcToWorldPos(mat4 inverseVP, vec3 ndc)
		{
			vec4 clipPos = NdcToClipPos(ndc);
			vec4 pos = mul(inverseVP, clipPos);
			pos.xyz /= pos.w;

			return pos.xyz;
		}

		vec3 yInverseLerp(vec3 x, vec3 y, float a)
		{
			if (a > 0.0)
				return (y - x * (1 - a)) / a;
			return y;
		}
		
		void main()
		{
			float timer = v_timer.x;
			vec2 windDir = v_wind.xy;
			vec2 timedWindDir = v_wind.zw;
			vec2 ndcPos = vec2(v_projPos.xy / v_projPos.w);
			vec3 eyeDir = normalize(u_camPos.xyz - v_worldPos);
			vec3 surfacePosition = v_worldPos;
			vec3 lightColor = _LightColor0.rgb;

			//wave normal
#if USE_DISPLACEMENT == 1
			vec3 normal = ComputeNormal(_NormalTexture, surfacePosition.xz, v_texcoord0,
				v_normal, v_tangent, v_bitangent, _WavesNoise, _WavesIntensity, timedWindDir);
#else
			vec3 normal = ComputeNormal(_NormalTexture, surfacePosition.xz, v_texcoord0,
				v_normal, vec3(0.0), vec3(0.0), _WavesNoise, _WavesIntensity, timedWindDir);
#endif
			normal = normalize(mix(v_normal, normalize(normal), _NormalIntensity.x)).xyz;

			// compute refracted color
			float depth = texture2DProj(_CameraDepthTexture, v_projPos.xyww).r;
			vec3 depthPosition = NdcToWorldPos(_ViewProjectInverse, vec3(ndcPos, depth));
			float waterDepth = surfacePosition.y - depthPosition.y; // horizontal water depth
			float viewWaterDepth = length(surfacePosition - depthPosition); // water depth from the view direction(water accumulation)
			vec2 dudv = ndcPos;
			{
				// refraction based on water depth
				float refractionScale = _RefractionScale.x * min(waterDepth, 1.0f);
				vec2 delta = vec2(sin(timer + 3.0f * abs(depthPosition.y)), sin(timer + 5.0f * abs(depthPosition.y)));
				dudv += windDir * delta * refractionScale;
			}
			vec3 pureRefractionColor = texture2D(_RefractionTexture, dudv).rgb;

#if USE_CAUSTICS == 1
			vec2 uv1 = v_texcoord0 * _Caustics1_ST.xy + _Caustics1_ST.zw;
			uv1 += _Caustics1_Speed.x * timer;
			
			vec2 uv2 = v_texcoord0 * _Caustics2_ST.xy + _Caustics2_ST.zw;
			uv2.x += _Caustics2_Speed.x * timer;
			uv2.y += _Caustics2_Speed.x * timer * 0.5;
			
			float s = _CausticsRGBSplit.x;
			
			vec3 caustics1 = vec3(0.0);
			caustics1.r = texture2D(_CausticsTex, uv1 + vec2(+s, +s)).r;
			caustics1.g = texture2D(_CausticsTex, uv1 + vec2(+s, -s)).g;
			caustics1.b = texture2D(_CausticsTex, uv1 + vec2(-s, -s)).b;
			
			vec3 caustics2 = vec3(0.0);
			caustics2.r = texture2D(_CausticsTex, uv2 + vec2(+s, +s)).r;
			caustics2.g = texture2D(_CausticsTex, uv2 + vec2(+s, -s)).g;
			caustics2.b = texture2D(_CausticsTex, uv2 + vec2(-s, -s)).b;
			
			vec3 caustics = pureRefractionColor + min(caustics1, caustics2) * 0.25 * _CausticsIntensity.x;
			pureRefractionColor = mix(pureRefractionColor, caustics, waterDepth * 2.0);
#endif
			vec2 waterTransparency = vec2(_WaterClarity.x, _WaterTransparency.x);
			vec2 waterDepthValues = vec2(waterDepth, viewWaterDepth);
			float shoreRange = max(_FoamRanges.x, _FoamRanges.y) * 2.0;
			vec3 refractionColor = DepthRefraction(waterTransparency, waterDepthValues, shoreRange, _HorizontalExtinction.xyz,
													pureRefractionColor, _ShoreColor.rgb, _SurfaceColor.rgb, _DepthColor.rgb);

			// compute ligths's reflected radiance
			vec3 lightDir = normalize(-u_lightDirection.xyz);
			float fresnel = FresnelValue(_RefractionValues.xy, normal, eyeDir);
			vec3 specularColor = ReflectedRadiance(_Shininess.x, _SpecularValues.xyz, lightColor, lightDir, eyeDir, normal, fresnel);

			// compute sky's reflected radiance
#if USE_MEAN_SKY_RADIANCE == 1
			vec3 reflectColor = vec3(fresnel) * MeanSkyRadiance(_SkyTexture, eyeDir, normal) * _RadianceFactor.x;
#else
			vec3 reflectColor = vec3(0.0);
#endif

			// compute reflected color
			dudv = ndcPos + vec2(_Distortion.x) * normal.xz;
//#ifdef USE_FILTERING
//			reflectColor += tex2DBicubic(u_reflectionTexture, _ReflectionTexture_TexelSize.z, dudv).rgb;
//#else
#if USE_REFLECTIONS == 1
			reflectColor += texture2D(u_reflectionTexture, dudv).rgb;
#endif
//#endif

			// shore foam
#if USE_FOAM == 1
			float maxAmplitude = max(max(_WaveAmplitude.x, _WaveAmplitude.y), _WaveAmplitude.z);
			float foam = FoamValue(_ShoreTexture, _FoamTexture, _FoamTiling.xy,
				_FoamNoise, _FoamSpeed.xy * windDir, _FoamRanges.xyz, maxAmplitude,
				surfacePosition, depthPosition, eyeDir, waterDepth, timedWindDir, timer);
			foam *= _FoamIntensity.x;
#else
			float foam = 0;
#endif

			float shoreFade = saturate(waterDepth * _ShoreFade.x);
			// ambient + diffuse
			vec3 ambientColor = u_ambientColor.rgb * _AmbientDensity.x + saturate(dot(normal, lightDir)) * _DiffuseDensity.x;
			// refraction color with depth based color
			pureRefractionColor = mix(pureRefractionColor, reflectColor, fresnel * saturate(waterDepth / (_FoamRanges.x * 0.4)));
			pureRefractionColor = mix(pureRefractionColor, _ShoreColor.rgb, 0.30 * shoreFade);
			
			// compute final color
			vec3 color = mix(refractionColor, reflectColor, fresnel);
			color = saturate(ambientColor + color + max(specularColor, foam * lightColor));
			color = mix(pureRefractionColor + specularColor * shoreFade, color, shoreFade);

//#ifdef DEBUG_NORMALS
			//color.rgb = 0.5 + 2 * ambientColor + specularColor + clamp(dot(normal, lightDir), 0, 1) * 0.5;
//#endif

			gl_FragColor = vec4(color, 1.0);
		}
	}
}