#include "API_Shader.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Classes/StringConverter.h"
#include "../Assets/Shader.h"

namespace GX
{
	MonoObject* API_Shader::load(MonoString* name)
	{
		std::string _name = (const char*)mono_string_to_utf8(name);
		_name = CP_SYS(_name);

		Shader* shader = Shader::load(Engine::getSingleton()->getAssetsPath(), _name);

		if (shader != nullptr)
			return shader->getManagedObject();

		return nullptr;
	}
}