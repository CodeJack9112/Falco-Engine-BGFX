name "Deferred/Nature/Triplanar"
render_mode deferred

params
{
	sampler2D "Albedo 1" texture1_albedo 0
	sampler2D "Normal 1" texture1_normal 1
	sampler2D "Roughness 1" texture1_roughness 2
	sampler2D "Albedo 2" texture2_albedo 3
	sampler2D "Normal 2" texture2_normal 4
	sampler2D "Roughness 2" texture2_roughness 5
	sampler2D "Albedo 3" texture3_albedo 6
	sampler2D "Normal 3" texture3_normal 7
	sampler2D "Roughness 3" texture3_roughness 9
	sampler2D "Detail normal" detail_normal_1 10
	sampler2D "Detail overlay" detail_overlay_1 11
	float "Normal scale" normal_scale 1.0
	float "Texture 1 scale" texture1_scale 1.0
	float "Texture 2 scale" texture2_scale 1.0
	float "Texture 3 scale" texture3_scale 1.0
	float "Detail scale" detail_scale 0.2
}

pass
{
	tags
	{
		backface_culling cw
	}
	
	varying
	{
		vec3 a_position  : POSITION;
		vec3 a_normal    : NORMAL;
		vec3 a_tangent   : TANGENT;
		vec3 a_bitangent : BITANGENT;
		vec4 a_color0    : COLOR0;
		
		vec3 v_position  : POSITION1 = vec3(0.0, 0.0, 1.0);
		vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 1.0);
		vec3 v_normal_a  : POSITION2 = vec3(0.0, 0.0, 1.0);
		vec3 v_tangent   : TANGENT   = vec3(1.0, 0.0, 0.0);
		vec3 v_bitangent : BITANGENT = vec3(0.0, 1.0, 0.0);
		vec4 v_color     : COLOR0    = vec4(1.0, 1.0, 1.0, 1.0);
	}

	vertex
	{
		$input a_position, a_normal, a_tangent, a_bitangent, a_color0
		$output v_position, v_normal, v_tangent, v_bitangent, v_normal_a, v_color
		
		#include "common.sh"
		
		uniform mat3 u_normalMatrix;
		
		void main()
		{
			v_normal = u_normalMatrix * a_normal;
			v_tangent = u_normalMatrix * a_tangent;
			v_bitangent = u_normalMatrix * a_bitangent;
			
			v_normal_a = pow(abs(a_normal), vec3(1.0));
			v_normal_a /= dot(v_normal_a, vec3(1.0));
			
			float scalingFactor = sqrt(u_normalMatrix[0][0] * u_normalMatrix[0][0] +
										u_normalMatrix[0][1] * u_normalMatrix[0][1] +
										u_normalMatrix[0][2] * u_normalMatrix[0][2]);
			
			v_position = a_position * scalingFactor;
			v_color = a_color0;
			
			gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
		}
	}
		
	fragment
	{
		$input v_position, v_normal, v_tangent, v_bitangent, v_normal_a, v_color
		
		#include "common.sh"
		#include "shaderlib.sh"
		
		uniform mat3 u_normalMatrix;
		
		uniform vec4 normal_scale;
		uniform vec4 texture1_scale;
		uniform vec4 texture2_scale;
		uniform vec4 texture3_scale;
		uniform vec4 detail_scale;
		
		SAMPLER2D(texture1_albedo, 0);
		SAMPLER2D(texture1_normal, 1);
		SAMPLER2D(texture1_roughness, 2);
		SAMPLER2D(texture2_albedo, 3);
		SAMPLER2D(texture2_normal, 4);
		SAMPLER2D(texture2_roughness, 5);
		SAMPLER2D(texture3_albedo, 6);
		SAMPLER2D(texture3_normal, 7);
		SAMPLER2D(texture3_roughness, 9);
		SAMPLER2D(detail_normal_1, 10);
		SAMPLER2D(detail_overlay_1, 11);
		
		vec3 triplanar_texture(sampler2D p_sampler, vec3 p_triplanar_pos)
		{
			vec3 samp = vec3(0.0);
			samp += texture2D(p_sampler, p_triplanar_pos.xy).xyz * v_normal_a.z;
			samp += texture2D(p_sampler, p_triplanar_pos.xz).xyz * v_normal_a.y;
			samp += texture2D(p_sampler, p_triplanar_pos.zy).xyz * v_normal_a.x;
			return samp;
		}

		vec3 blend(vec3 texture1, vec3 texture2, vec3 texture3, vec4 color){
			return ((texture1 * color.r) + (texture2 * color.b) + (texture3 * color.g)).rgb;
		}

		vec3 NormalBlend_Linear(vec3 n1, vec3 n2)
		{
			n1 = normalize(n1 * 2.0 - 1.0);
			n2 = normalize(n2 * 2.0 - 1.0);
			
			return normalize(n1 + n2);
		}
		
		vec3 overlay(vec3 n1, vec3 n2)
		{  
			return (vec3(n1.xy * n2.z + n2.xy * n1.z, n1.z * n2.z));
		}
				
		void main()
		{
			vec3 albedo_texture = blend(triplanar_texture(texture1_albedo, v_position * texture1_scale.x), triplanar_texture(texture2_albedo, v_position * texture2_scale.x) , triplanar_texture(texture3_albedo, v_position * texture3_scale.x), v_color);
			vec3 overlayertex = triplanar_texture(detail_overlay_1, v_position * detail_scale.x);
			vec3 ALBEDO = overlay(albedo_texture.rgb, overlayertex.rgb);
			vec3 orm_texture = blend(triplanar_texture(texture1_roughness, v_position * texture1_scale.x), triplanar_texture(texture2_roughness, v_position * texture2_scale.x), triplanar_texture(texture3_roughness, v_position * texture3_scale.x), v_color);
			vec3 detail_norm_tex = triplanar_texture(detail_normal_1, v_position * detail_scale.x);
			
			vec3 normal = NormalBlend_Linear(blend(triplanar_texture(texture1_normal, v_position * texture1_scale.x), triplanar_texture(texture2_normal, v_position * texture2_scale.x), triplanar_texture(texture3_normal, v_position * texture3_scale.x), v_color), detail_norm_tex);
			mat3 TBN = mat3(v_tangent, v_bitangent, v_normal);
			normal = normalize(TBN * normal);
			
			if (gl_FrontFacing)
				normal = -normal;
			
			gl_FragData[0] = vec4(ALBEDO, 0.0);
			gl_FragData[1] = vec4(encodeNormalUint(normal), gl_FrontFacing ? 1.0 : 0.0);
			gl_FragData[2] = vec4(0.0, orm_texture.r, 1.0, 0.5);
			gl_FragData[3] = vec4(vec3(0.0), 0.0);
		}
	}
}
