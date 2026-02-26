#pragma once

#include "API.h"

namespace GX
{
	class API_MeshRenderer
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.MeshRenderer::get_materialsCount", (void*)getMaterialCount);
			mono_add_internal_call("FalcoEngine.MeshRenderer::get_sharedMaterialsCount", (void*)getSharedMaterialCount);
			mono_add_internal_call("FalcoEngine.MeshRenderer::GetMaterial", (void*)getMaterial);
			mono_add_internal_call("FalcoEngine.MeshRenderer::GetSharedMaterial", (void*)getSharedMaterial);
			mono_add_internal_call("FalcoEngine.MeshRenderer::SetSharedMaterial", (void*)setSharedMaterial);
			mono_add_internal_call("FalcoEngine.MeshRenderer::get_mesh", (void*)getMesh);
			mono_add_internal_call("FalcoEngine.MeshRenderer::set_mesh", (void*)setMesh);
		}

	private:
		static int getMaterialCount(MonoObject* this_ptr);
		static int getSharedMaterialCount(MonoObject* this_ptr);

		static MonoObject* getMaterial(MonoObject* this_ptr, int index);
		static MonoObject* getSharedMaterial(MonoObject* this_ptr, int index);

		static void setSharedMaterial(MonoObject* this_ptr, int index, MonoObject* material);

		static MonoObject* getMesh(MonoObject* this_ptr);
		static void setMesh(MonoObject* this_ptr, MonoObject* value);
	};
}