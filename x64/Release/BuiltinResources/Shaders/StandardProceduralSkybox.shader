name "Forward/Skybox/Procedural"
render_mode forward

params
{
	define bool "Enable clouds" ENABLE_CLOUDS false
	sampler2D "Clouds 1" clouds_tex1 0
	sampler2D "Clouds 2" clouds_tex2 1
	sampler2D "Stars" stars_tex 2
	float "Clouds speed" clouds_speed 0.0025
	float "Clouds opacity" clouds_opacity 0.5
	float "Sun speed" sun_speed 0.01 : 0.0 1.0
	float "Sun size" sun_size 0.998 : 0.5 0.999
	float "Sun azimuth" sun_azimuth 0.0
	color "Nitrogen color" nitrogen_color 0.741 0.659 0.564 1
	float "Time" m_time 0.0
}

pass
{
	tags
	{
		
	}
	
	varying
	{
		vec3 a_position  : POSITION;
		vec2 a_texcoord0 : TEXCOORD0;
		
		vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
		vec4 v_color0 : COLOR0 = vec4(1.0, 1.0, 1.0, 1.0);
		vec3 fsun : POSITION1 = vec3(1.0, 1.0, 1.0);
		vec3 pos : POSITION2 = vec3(1.0, 1.0, 1.0);
	}
	
	vertex
	{
		$input a_position, a_texcoord0
		$output v_texcoord0, fsun, pos
		
		#include "common.sh"
		
		uniform vec4 m_time;
		uniform vec4 sun_speed;
		uniform vec4 sun_azimuth;
		
		void main()
		{
			v_texcoord0 = a_texcoord0;
			
			gl_Position = u_modelViewProj * vec4(a_position, 1.0);
			
			float time = m_time.x;

			pos = vec3(a_position.x, a_position.y, a_position.z);
			fsun = vec3(sin(time * sun_speed.x) * sun_azimuth.x, sin(time * sun_speed.x), cos(time * sun_speed.x));

			fsun.y = clamp(fsun.y, -0.1, 1.0);
		}
	}
	
	fragment
	{
		$input v_texcoord0, fsun, pos
		
		#include "common.sh"
		
#if ENABLE_CLOUDS == 1
		SAMPLER2D(clouds_tex1, 0);
		SAMPLER2D(clouds_tex2, 1);
		SAMPLER2D(stars_tex, 2);
		
		uniform vec4 clouds_speed;
		uniform vec4 clouds_opacity;
#endif

		uniform vec4 m_time;
		uniform vec4 nitrogen_color;
		uniform vec4 sun_size;

		const float Br = 0.0025;
		const float Bm = 0.0015;
		float g = sun_size.x;
		vec3 nitrogen = nitrogen_color.rgb;
		vec3 Kr = Br / pow(nitrogen, vec3(4.0));
		vec3 Km = Bm / pow(nitrogen, vec3(0.84));

		void main()
		{
			vec3 color = vec3(0.0, 0.0, 0.0);
			vec3 _pos = vec3(pos.x, abs(pos.y), pos.z);
			
			float time = m_time.x;

			// Atmosphere Scattering
			float mu = dot(normalize(pos), normalize(fsun));
			float rayleigh = 3.0 / (8.0 * 3.14) * (1.0 + mu * mu);

			vec3 mie = (Kr + Km * (1.0 - g * g) / (2.0 + g * g) / pow(1.0 + g * g - 2.0 * g * mu, 1.5)) / (Br + Bm);
			vec3 day_extinction = exp(-exp(-((_pos.y + fsun.y * 8.0) * (exp(-_pos.y * 16.0) + 0.1) / 80.0) / Br) * (exp(-_pos.y * 16.0) + 0.1) * Kr / Br) * exp(-_pos.y * exp(-_pos.y * 8.0 ) * 4.0) * exp(-_pos.y * 2.0) * 4.0;
			vec3 night_extinction = vec3(1.0 - exp(fsun.y)) * 0.2;
			vec3 extinction = mix(day_extinction, night_extinction, -fsun.y * 0.2 + 0.5);

			color.rgb = rayleigh * (mie * 0.5) * extinction;
			
#if ENABLE_CLOUDS == 1
			vec4 clouds_1 = texture2D(clouds_tex1, v_texcoord0 + vec2(-m_time.x * clouds_speed.x * 0.25, 0.0));
			vec4 clouds_2 = texture2D(clouds_tex2, v_texcoord0 + vec2(-m_time.x * clouds_speed.x, 0.0));
			vec4 stars = texture2D(stars_tex, (v_texcoord0 * 10.0) + vec2(m_time.x * clouds_speed.x * 0.1, 0.0));

			vec3 clouds = clouds_1.rgb + (clouds_2.rgb * clouds_opacity.x);
			
			color.rgb = mix(color.rgb, clouds, max(extinction * clouds_opacity.x, 0.0));
			color.rgb = mix(color.rgb, stars.rgb, max((1.0 - extinction) * 0.25, 0.0));
#endif

			gl_FragColor = vec4(color, 1.0);
		}
	}
}
