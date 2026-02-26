#pragma once

#include "API.h"

namespace GX
{
	class API_Material
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Material::.ctor(string)", (void*)ctor);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_load", (void*)load);

			mono_add_internal_call("FalcoEngine.Material::get_shader", (void*)getShader);
			mono_add_internal_call("FalcoEngine.Material::set_shader", (void*)setShader);

			mono_add_internal_call("FalcoEngine.Material::INTERNAL_getParameterMat3", (void*)getParameterMatrix3);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_getParameterMat4", (void*)getParameterMatrix4);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_getParameterVec2", (void*)getParameterVector2);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_getParameterVec3", (void*)getParameterVector3);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_getParameterVec4", (void*)getParameterVector4);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_getParameterFloat", (void*)getParameterFloat);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_getParameterInt", (void*)getParameterInt);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_getParameterBool", (void*)getParameterBool);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_getParameterColor", (void*)getParameterColor);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_getParameterTexture", (void*)getParameterTexture);

			mono_add_internal_call("FalcoEngine.Material::INTERNAL_setParameterMat3", (void*)setParameterMatrix3);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_setParameterMat4", (void*)setParameterMatrix4);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_setParameterVec2", (void*)setParameterVector2);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_setParameterVec3", (void*)setParameterVector3);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_setParameterVec4", (void*)setParameterVector4);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_setParameterFloat", (void*)setParameterFloat);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_setParameterInt", (void*)setParameterInt);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_setParameterBool", (void*)setParameterBool);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_setParameterColor", (void*)setParameterColor);
			mono_add_internal_call("FalcoEngine.Material::INTERNAL_setParameterTexture", (void*)setParameterTexture);
		}

	private:
		
		//Constructor
		static void ctor(MonoObject* this_ptr, MonoString* name);

		//Load
		static MonoObject* load(MonoString * name);

		//Get shader
		static MonoObject* getShader(MonoObject* this_ptr);

		//Get shader
		static void setShader(MonoObject* this_ptr, MonoObject* value);

		//Get parameter Matrix3
		static void getParameterMatrix3(MonoObject* this_ptr, MonoString* name, API::Matrix3* matrix);

		//Get parameter Matrix4
		static void getParameterMatrix4(MonoObject* this_ptr, MonoString* name, API::Matrix4* matrix);

		//Get parameter Vector2
		static void getParameterVector2(MonoObject* this_ptr, MonoString* name, API::Vector2* vector2);

		//Get parameter Vector3
		static void getParameterVector3(MonoObject* this_ptr, MonoString* name, API::Vector3* vector3);

		//Get parameter Vector4
		static void getParameterVector4(MonoObject* this_ptr, MonoString* name, API::Vector4* vector4);

		//Get parameter float
		static float getParameterFloat(MonoObject* this_ptr, MonoString* name);

		//Get parameter int
		static int getParameterInt(MonoObject* this_ptr, MonoString* name);

		//Get parameter bool
		static bool getParameterBool(MonoObject* this_ptr, MonoString* name);

		//Get parameter color
		static void getParameterColor(MonoObject* this_ptr, MonoString* name, API::Color* color);

		//Get parameter texture
		static MonoObject* getParameterTexture(MonoObject* this_ptr, MonoString* name);

		//Set parameter Matrix3
		static void setParameterMatrix3(MonoObject* this_ptr, MonoString* name, API::Matrix3* matrix);

		//Set parameter Matrix4
		static void setParameterMatrix4(MonoObject* this_ptr, MonoString* name, API::Matrix4 * matrix);

		//Set parameter Vector2
		static void setParameterVector2(MonoObject* this_ptr, MonoString* name, API::Vector2* vector2);

		//Set parameter Vector3
		static void setParameterVector3(MonoObject* this_ptr, MonoString* name, API::Vector3* vector3);

		//Set parameter Vector4
		static void setParameterVector4(MonoObject* this_ptr, MonoString* name, API::Vector4* vector4);

		//Set parameter float
		static void setParameterFloat(MonoObject* this_ptr, MonoString* name, float flt);

		//Set parameter int
		static void setParameterInt(MonoObject* this_ptr, MonoString* name, int val);

		//Set parameter bool
		static void setParameterBool(MonoObject* this_ptr, MonoString* name, bool val);

		//Set parameter color
		static void setParameterColor(MonoObject* this_ptr, MonoString* name, API::Color* color);

		//Set parameter texture
		static void setParameterTexture(MonoObject* this_ptr, MonoString* name, MonoObject* texture);
	};
}