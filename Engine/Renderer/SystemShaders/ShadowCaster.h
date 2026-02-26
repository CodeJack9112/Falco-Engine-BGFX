#include <string>

namespace shaders
{
	static std::string shadowCasterVarying =
		"vec4 v_position : POSITION = vec4(0.0, 0.0, 0.0, 0.0);\n"
		"vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);\n"
		"vec3 a_position : POSITION;\n"
		"vec4 a_indices  : BLENDINDICES;\n"
		"vec4 a_weight   : BLENDWEIGHT;\n"
		"vec2 a_texcoord0 : TEXCOORD0;\n";

	static std::string shadowCasterVertex =
		"$input a_position, a_weight, a_indices, a_texcoord0, a_normal\n"
		"$output v_position, v_texcoord0, v_normal\n"
		"\n"
		"uniform mat4 u_modelViewProj;\n"
		"uniform mat4 u_model[32];\n"
		"uniform mat4 u_viewProj;\n"
		"uniform vec4 u_skinned;\n"
		"uniform mat4 u_boneMatrix[128];\n"
		"\n"
		"void main()\n"
		"{\n"
		"	if (u_skinned.x == 1.0)\n"
		"	{\n"
		"		mat4 model = u_model[0] * (a_weight.x * u_boneMatrix[int(a_indices.x)] +\n"
		"		a_weight.y * u_boneMatrix[int(a_indices.y)] +\n"
		"		a_weight.z * u_boneMatrix[int(a_indices.z)] +\n"
		"		a_weight.w * u_boneMatrix[int(a_indices.w)]);\n"
		"\n"
		"		vec3 wpos = (model * vec4(a_position, 1.0)).xyz;\n"
		"		gl_Position = u_viewProj * vec4(wpos, 1.0);\n"
		"\n"
		"	}\n"
		"	else\n"
		"	{\n"
		"		gl_Position = u_modelViewProj * vec4(a_position, 1.0);\n"
		"	}\n"
		"\n"
		"	v_position = gl_Position;\n"
		"	v_texcoord0 = a_texcoord0;\n"
		"}\n";

	static std::string shadowCasterFragment =
		"$input v_position, v_texcoord0, v_normal\n"
		"\n"
		"uniform sampler2D u_albedoMap;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	vec4 color = texture2D(u_albedoMap, v_texcoord0);\n"
		"	if (color.a < 0.25f) discard;\n"
		"}\n";
}