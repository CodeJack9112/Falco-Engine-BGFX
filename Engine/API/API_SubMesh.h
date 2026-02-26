#pragma once

#include "API.h"

namespace GX
{
	class API_SubMesh
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.SubMesh::.ctor", (void*)ctor);

			mono_add_internal_call("FalcoEngine.SubMesh::get_parent", (void*)getParent);

			mono_add_internal_call("FalcoEngine.SubMesh::get_indices", (void*)getIndices);
			mono_add_internal_call("FalcoEngine.SubMesh::set_indices", (void*)setIndices);

			mono_add_internal_call("FalcoEngine.SubMesh::get_vertices", (void*)getVertices);
			mono_add_internal_call("FalcoEngine.SubMesh::set_vertices", (void*)setVertices);

			mono_add_internal_call("FalcoEngine.SubMesh::get_normals", (void*)getNormals);
			mono_add_internal_call("FalcoEngine.SubMesh::set_normals", (void*)setNormals);

			mono_add_internal_call("FalcoEngine.SubMesh::get_tangents", (void*)getTangents);
			mono_add_internal_call("FalcoEngine.SubMesh::set_tangents", (void*)setTangents);

			mono_add_internal_call("FalcoEngine.SubMesh::get_bitangents", (void*)getBitangents);
			mono_add_internal_call("FalcoEngine.SubMesh::set_bitangents", (void*)setBitangents);

			mono_add_internal_call("FalcoEngine.SubMesh::get_texcoords0", (void*)getTexcoords0);
			mono_add_internal_call("FalcoEngine.SubMesh::set_texcoords0", (void*)setTexcoords0);

			mono_add_internal_call("FalcoEngine.SubMesh::get_texcoords1", (void*)getTexcoords1);
			mono_add_internal_call("FalcoEngine.SubMesh::set_texcoords1", (void*)setTexcoords1);

			mono_add_internal_call("FalcoEngine.SubMesh::get_blendWeights", (void*)getBlendWeights);
			mono_add_internal_call("FalcoEngine.SubMesh::set_blendWeights", (void*)setBlendWeights);

			mono_add_internal_call("FalcoEngine.SubMesh::get_blendIndices", (void*)getBlendIndices);
			mono_add_internal_call("FalcoEngine.SubMesh::set_blendIndices", (void*)setBlendIndices);

			mono_add_internal_call("FalcoEngine.SubMesh::get_colors", (void*)getColors);
			mono_add_internal_call("FalcoEngine.SubMesh::set_colors", (void*)setColors);

			mono_add_internal_call("FalcoEngine.SubMesh::Commit", (void*)commit);

			mono_add_internal_call("FalcoEngine.SubMesh::Destroy", (void*)destroy);
		}

		static void ctor(MonoObject* this_ptr);

		static MonoObject* getParent(MonoObject* this_ptr);
		
		static MonoArray* getIndices(MonoObject* this_ptr);
		static void setIndices(MonoObject* this_ptr, MonoArray* value);

		static MonoArray* getVertices(MonoObject* this_ptr);
		static void setVertices(MonoObject* this_ptr, MonoArray* value);

		static MonoArray* getNormals(MonoObject* this_ptr);
		static void setNormals(MonoObject* this_ptr, MonoArray* value);

		static MonoArray* getTangents(MonoObject* this_ptr);
		static void setTangents(MonoObject* this_ptr, MonoArray* value);

		static MonoArray* getBitangents(MonoObject* this_ptr);
		static void setBitangents(MonoObject* this_ptr, MonoArray* value);

		static MonoArray* getTexcoords0(MonoObject* this_ptr);
		static void setTexcoords0(MonoObject* this_ptr, MonoArray* value);

		static MonoArray* getTexcoords1(MonoObject* this_ptr);
		static void setTexcoords1(MonoObject* this_ptr, MonoArray* value);

		static MonoArray* getBlendWeights(MonoObject* this_ptr);
		static void setBlendWeights(MonoObject* this_ptr, MonoArray* value);

		static MonoArray* getBlendIndices(MonoObject* this_ptr);
		static void setBlendIndices(MonoObject* this_ptr, MonoArray* value);

		static MonoArray* getColors(MonoObject* this_ptr);
		static void setColors(MonoObject* this_ptr, MonoArray* value);

		static void commit(MonoObject* this_ptr);

		static void destroy(MonoObject* this_ptr);
	};
}