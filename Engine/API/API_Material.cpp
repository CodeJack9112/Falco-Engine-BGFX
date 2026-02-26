#include "API_Material.h"

#include "../glm/glm.hpp"

#include "../Core/Engine.h"
#include "../Classes/StringConverter.h"
#include "../Core/APIManager.h"
#include "../Assets/Material.h"
#include "../Assets/Shader.h"
#include "../Assets/Texture.h"
#include "../Classes/GUIDGenerator.h"

namespace GX
{
	void API_Material::ctor(MonoObject* this_ptr, MonoString* name)
	{
		Material* mat = new Material();

		std::string _name = CP_SYS(mono_string_to_utf8(name));

		mat->setLocation(Engine::getSingleton()->getAssetsPath());
		mat->setName("Runtime/" + GUIDGenerator::genGuid() + "/" + _name);
		mat->load();

		mat->setManagedObject(this_ptr);
	}

	MonoObject* API_Material::load(MonoString* name)
	{
		std::string matName = (const char*)mono_string_to_utf8((MonoString*)name);
		matName = CP_SYS(matName);

		Material* mat = Material::load(Engine::getSingleton()->getAssetsPath(), matName);

		if (mat != nullptr)
			return mat->getManagedObject();

		return nullptr;
	}

	MonoObject* API_Material::getShader(MonoObject* this_ptr)
	{
		Material* mat = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return nullptr;

		Shader* shader = mat->getShader();
		if (shader != nullptr)
			return shader->getManagedObject();

		return nullptr;
	}

	void API_Material::setShader(MonoObject* this_ptr, MonoObject* value)
	{
		Material* mat = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		if (value != nullptr)
		{
			Shader* shader = nullptr;
			mono_field_get_value(value, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&shader));
			mat->setShader(shader);
		}
		else
			mat->setShader(nullptr);
	}

	void API_Material::setParameterMatrix3(MonoObject* this_ptr, MonoString* name, API::Matrix3* matrix)
	{
		Material* mat = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		glm::mat3x3 mat3 = glm::mat3x3(matrix->m00, matrix->m01, matrix->m02,
										matrix->m10, matrix->m11, matrix->m12,
										matrix->m20, matrix->m21, matrix->m22);

		mat->setUniform(paramName, mat3, mat->getUniformIsDefine(paramName));
	}

	void API_Material::setParameterMatrix4(MonoObject* this_ptr, MonoString* name, API::Matrix4* matrix)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);
		
		glm::mat4x4 mat4 = glm::mat4x4(matrix->m00, matrix->m01, matrix->m02, matrix->m03,
										matrix->m10, matrix->m11, matrix->m12, matrix->m13,
										matrix->m20, matrix->m21, matrix->m22, matrix->m23,
										matrix->m30, matrix->m31, matrix->m32, matrix->m33);

		mat->setUniform(paramName, mat4, mat->getUniformIsDefine(paramName));
	}

	void API_Material::setParameterVector2(MonoObject* this_ptr, MonoString* name, API::Vector2* vector2)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		glm::vec2 vec2 = glm::vec2(vector2->x, vector2->y);

		mat->setUniform(paramName, vec2, mat->getUniformIsDefine(paramName));
	}

	void API_Material::setParameterVector3(MonoObject* this_ptr, MonoString* name, API::Vector3* vector3)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		glm::vec3 vec3 = glm::vec3(vector3->x, vector3->y, vector3->z);

		mat->setUniform(paramName, vec3, mat->getUniformIsDefine(paramName));
	}

	void API_Material::setParameterVector4(MonoObject* this_ptr, MonoString* name, API::Vector4* vector4)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		glm::vec4 vec4 = glm::vec4(vector4->x, vector4->y, vector4->z, vector4->w);

		mat->setUniform(paramName, vec4, mat->getUniformIsDefine(paramName));
	}

	void API_Material::setParameterFloat(MonoObject* this_ptr, MonoString* name, float flt)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		mat->setUniform(paramName, flt, mat->getUniformIsDefine(paramName));
	}

	void API_Material::setParameterInt(MonoObject* this_ptr, MonoString* name, int val)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		mat->setUniform(paramName, val, mat->getUniformIsDefine(paramName));
	}

	void API_Material::setParameterBool(MonoObject* this_ptr, MonoString* name, bool val)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		mat->setUniform(paramName, val, mat->getUniformIsDefine(paramName));
	}

	void API_Material::setParameterColor(MonoObject* this_ptr, MonoString* name, API::Color* color)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		mat->setUniform(paramName, Color(color->r, color->g, color->b, color->a), mat->getUniformIsDefine(paramName));
	}

	void API_Material::setParameterTexture(MonoObject* this_ptr, MonoString* name, MonoObject* texture)
	{
		Material* mat = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		Texture* tex = nullptr;
		if (texture != nullptr)
			mono_field_get_value(texture, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&tex));

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		Sampler2DDef t = mat->getUniform<Sampler2DDef>(paramName);
		mat->setUniform<Sampler2DDef>(paramName, std::make_pair(t.first, tex), false);
	}

	//////////////////////////////////////////////

	void API_Material::getParameterMatrix3(MonoObject* this_ptr, MonoString* name, API::Matrix3* matrix)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		glm::mat3x3 m = mat->getUniform<glm::mat3x3>(paramName);

		matrix->m00 = m[0][0]; matrix->m01 = m[0][1]; matrix->m02 = m[0][2];
		matrix->m10 = m[1][0]; matrix->m11 = m[1][1]; matrix->m12 = m[1][2];
		matrix->m20 = m[2][0]; matrix->m21 = m[2][1]; matrix->m22 = m[2][2];
	}

	void API_Material::getParameterMatrix4(MonoObject* this_ptr, MonoString* name, API::Matrix4* matrix)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		glm::mat4x4 m = mat->getUniform<glm::mat4x4>(paramName);

		matrix->m00 = m[0][0]; matrix->m01 = m[0][1]; matrix->m02 = m[0][2]; matrix->m03 = m[0][3];
		matrix->m10 = m[1][0]; matrix->m11 = m[1][1]; matrix->m12 = m[1][2]; matrix->m13 = m[1][3];
		matrix->m20 = m[2][0]; matrix->m21 = m[2][1]; matrix->m22 = m[2][2]; matrix->m23 = m[2][3];
		matrix->m30 = m[3][0]; matrix->m31 = m[3][1]; matrix->m32 = m[3][2]; matrix->m33 = m[3][3];
	}

	void API_Material::getParameterVector2(MonoObject* this_ptr, MonoString* name, API::Vector2* vector2)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		glm::vec2 v = mat->getUniform<glm::vec2>(paramName);

		vector2->x = v.x;
		vector2->y = v.y;
	}

	void API_Material::getParameterVector3(MonoObject* this_ptr, MonoString* name, API::Vector3* vector3)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		glm::vec3 v = mat->getUniform<glm::vec3>(paramName);

		vector3->x = v.x;
		vector3->y = v.y;
		vector3->z = v.z;
	}

	void API_Material::getParameterVector4(MonoObject* this_ptr, MonoString* name, API::Vector4* vector4)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		glm::vec4 v = mat->getUniform<glm::vec4>(paramName);

		vector4->x = v.x;
		vector4->y = v.y;
		vector4->z = v.z;
		vector4->w = v.w;
	}

	float API_Material::getParameterFloat(MonoObject* this_ptr, MonoString* name)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return 0;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		return mat->getUniform<float>(paramName);
	}

	int API_Material::getParameterInt(MonoObject* this_ptr, MonoString* name)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return 0;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		return mat->getUniform<int>(paramName);
	}

	bool API_Material::getParameterBool(MonoObject* this_ptr, MonoString* name)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return 0;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		return mat->getUniform<bool>(paramName);
	}

	void API_Material::getParameterColor(MonoObject* this_ptr, MonoString* name, API::Color* color)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		Color c = mat->getUniform<Color>(paramName);

		color->r = c[0];
		color->g = c[1];
		color->b = c[2];
		color->a = c[3];
	}

	MonoObject* API_Material::getParameterTexture(MonoObject* this_ptr, MonoString* name)
	{
		Material* mat;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

		if (mat == nullptr)
			return nullptr;

		std::string paramName = (const char*)mono_string_to_utf8((MonoString*)name);
		paramName = CP_SYS(paramName);

		Sampler2DDef t = mat->getUniform<Sampler2DDef>(paramName);
		if (t.second != nullptr)
			return t.second->getManagedObject();

		return nullptr;
	}
}