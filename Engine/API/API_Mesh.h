#pragma once

#include "API.h"

namespace GX
{
	class API_Mesh
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Mesh::.ctor", (void*)ctor);
			mono_add_internal_call("FalcoEngine.Mesh::get_subMeshCount", (void*)getSubMeshCount);
			mono_add_internal_call("FalcoEngine.Mesh::GetSubMesh", (void*)getSubMesh);
			mono_add_internal_call("FalcoEngine.Mesh::AddSubMesh", (void*)addSubMesh);
			mono_add_internal_call("FalcoEngine.Mesh::RemoveSubMesh(SubMesh)", (void*)removeSubMesh);
			mono_add_internal_call("FalcoEngine.Mesh::RemoveSubMesh(int)", (void*)removeSubMeshByIndex);
			mono_add_internal_call("FalcoEngine.Mesh::RecalculateBounds", (void*)recalculateBounds);
			mono_add_internal_call("FalcoEngine.Mesh::Commit", (void*)commit);
			mono_add_internal_call("FalcoEngine.Mesh::Destroy", (void*)destroy);
		}

		static void ctor(MonoObject* this_ptr);
		static int getSubMeshCount(MonoObject* this_ptr);
		static MonoObject* getSubMesh(MonoObject* this_ptr, int index);
		static void addSubMesh(MonoObject* this_ptr, MonoObject* value);
		static void removeSubMesh(MonoObject* this_ptr, MonoObject* value);
		static void removeSubMeshByIndex(MonoObject* this_ptr, int index);
		static void recalculateBounds(MonoObject* this_ptr);
		static void commit(MonoObject* this_ptr);
		static void destroy(MonoObject* this_ptr);
	};
}