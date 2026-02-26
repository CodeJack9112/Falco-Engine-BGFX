#include <string>

namespace shaders
{
	static std::string outlineVarying =
		"vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);\n"
		"vec3 v_normal : NORMAL = vec3(0.0, 0.0, 1.0);\n"
		"\n"
		"vec3 a_position  : POSITION;\n"
		"vec2 a_texcoord0 : TEXCOORD0;\n"
		"vec3 a_normal    : NORMAL;\n";

	static std::string outlineVertex =
		"$input a_position, a_texcoord0, a_normal\n"
		"$output v_texcoord0, v_normal\n"
		"\n"
		"uniform mat4 u_modelViewProj;\n"
		"uniform mat4 u_viewProj;\n"
		"uniform mat4 u_model[32];\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = u_modelViewProj * vec4(a_position, 1.0);\n"
		"	v_texcoord0 = a_texcoord0;\n"
		"	v_normal = normalize((u_viewProj * normalize(vec4(mat3(u_model[0]) * a_normal, 1.0))).xyz);\n"
		"}\n";

	static std::string outlineFragment =
		"$input v_texcoord0, v_normal\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = vec4(v_normal * 0.5 + 0.5, 1.0);\n"
		"}\n";
}