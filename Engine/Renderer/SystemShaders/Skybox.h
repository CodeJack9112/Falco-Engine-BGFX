#include <string>

namespace shaders
{
	static std::string skyboxVarying =
		"vec3 v_texcoord0 : TEXCOORD0 = vec3(0.0, 0.0, 0.0);\n"
		"\n"
		"vec3 a_position  : POSITION;\n"
		"vec2 a_texcoord0 : TEXCOORD0;\n";

	static std::string skyboxVertex =
		"$input a_position, a_texcoord0\n"
		"$output v_texcoord0\n"
		"\n"
		"uniform mat4 u_modelViewProj;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = u_modelViewProj * vec4(a_position, 1.0);\n"
		"	v_texcoord0 = -normalize(a_position);\n"
		"}\n";

	static std::string skyboxFragment =
		"$input v_texcoord0\n"
		"\n"
		"uniform samplerCube u_albedoMap;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = textureCube(u_albedoMap, v_texcoord0);\n"
		"}\n";
}