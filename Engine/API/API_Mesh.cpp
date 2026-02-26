#include "API_Mesh.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Classes/GUIDGenerator.h"

#include "../Assets/Mesh.h"

namespace GX
{
	void API_Mesh::ctor(MonoObject* this_ptr)
	{
		Mesh* mesh = new Mesh();
		mesh->setLocation(Engine::getSingleton()->getAssetsPath());
		mesh->setName("Runtime/Mesh/" + GUIDGenerator::genGuid());
		mesh->load();

		mesh->setManagedObject(this_ptr);
	}

	int API_Mesh::getSubMeshCount(MonoObject* this_ptr)
	{
		Mesh* mesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, &mesh);

		if (mesh != nullptr)
			return mesh->getSubMeshCount();

		return 0;
	}

	MonoObject* API_Mesh::getSubMesh(MonoObject* this_ptr, int index)
	{
		Mesh* mesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, &mesh);

		if (mesh != nullptr)
		{
			if (index < mesh->getSubMeshCount())
			{
				SubMesh* sm = mesh->getSubMesh(index);
				if (sm != nullptr)
					return sm->getManagedObject();
			}
		}

		return nullptr;
	}

	void API_Mesh::addSubMesh(MonoObject* this_ptr, MonoObject* value)
	{
		Mesh* mesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, &mesh);

		if (mesh != nullptr)
		{
			SubMesh* subMesh = nullptr;
			mono_field_get_value(value, APIManager::getSingleton()->object_ptr_field, &subMesh);

			if (subMesh != nullptr)
				mesh->addSubMesh(subMesh);
		}
	}

	void API_Mesh::removeSubMesh(MonoObject* this_ptr, MonoObject* value)
	{
		Mesh* mesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, &mesh);

		if (mesh != nullptr)
		{
			SubMesh* subMesh = nullptr;
			mono_field_get_value(value, APIManager::getSingleton()->object_ptr_field, &subMesh);

			mesh->removeSubMesh(subMesh);
		}
	}

	void API_Mesh::removeSubMeshByIndex(MonoObject* this_ptr, int index)
	{
		Mesh* mesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, &mesh);

		if (mesh != nullptr)
		{
			mesh->removeSubMesh(index);
		}
	}

	void API_Mesh::recalculateBounds(MonoObject* this_ptr)
	{
		Mesh* mesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, &mesh);

		if (mesh != nullptr)
		{
			mesh->recalculateBounds();
		}
	}

	void API_Mesh::commit(MonoObject* this_ptr)
	{
		Mesh* mesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, &mesh);

		if (mesh != nullptr)
		{
			mesh->commit();
		}
	}

	void API_Mesh::destroy(MonoObject* this_ptr)
	{
		Mesh* mesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, &mesh);

		if (mesh != nullptr)
		{
			delete mesh;
		}
	}
}