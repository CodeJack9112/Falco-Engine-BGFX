#include <string>

namespace shaders
{
	static std::string simpleTextureVarying =
		"vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);\n"
		"\n"
		"vec3 a_position  : POSITION;\n"
		"vec2 a_texcoord0 : TEXCOORD0;\n";

	static std::string simpleTextureVertex =
		"$input a_position, a_texcoord0\n"
		"$output v_texcoord0\n"
		"\n"
		"uniform mat4 u_modelViewProj;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = u_modelViewProj * vec4(a_position, 1.0);\n"
		"	v_texcoord0 = a_texcoord0;\n"
		"}\n";

	static std::string simpleTextureFragment =
		"$input v_texcoord0\n"
		"\n"
		"uniform sampler2D u_albedoMap;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = texture2D(u_albedoMap, v_texcoord0);\n"
		"}\n";
}