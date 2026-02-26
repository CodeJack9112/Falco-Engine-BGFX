#include "API_MeshRenderer.h"

#include "../Core/APIManager.h"
#include "../Components/MeshRenderer.h"
#include "../Assets/Material.h"
#include "../Assets/Mesh.h"

namespace GX
{
	int API_MeshRenderer::getMaterialCount(MonoObject* this_ptr)
	{
		MeshRenderer* comp = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&comp));

		if (comp != nullptr)
			return comp->getSharedMaterialsCount();

		return 0;
	}

	int API_MeshRenderer::getSharedMaterialCount(MonoObject* this_ptr)
	{
		MeshRenderer* comp = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&comp));

		if (comp != nullptr)
			return comp->getSharedMaterialsCount();

		return 0;
	}

	MonoObject* API_MeshRenderer::getMaterial(MonoObject* this_ptr, int index)
	{
		MeshRenderer* comp = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&comp));

		if (comp != nullptr)
		{
			Material* mat = comp->getMaterial(index);
			if (mat != nullptr)
				return mat->getManagedObject();
		}

		return nullptr;
	}

	MonoObject* API_MeshRenderer::getSharedMaterial(MonoObject* this_ptr, int index)
	{
		MeshRenderer* comp = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&comp));

		if (comp != nullptr)
		{
			Material* mat = comp->getSharedMaterial(index);
			if (mat != nullptr)
				return mat->getManagedObject();
		}

		return nullptr;
	}

	void API_MeshRenderer::setSharedMaterial(MonoObject* this_ptr, int index, MonoObject* material)
	{
		MeshRenderer* comp = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&comp));

		if (comp != nullptr)
		{
			if (material != nullptr)
			{
				Material* asset = nullptr;
				mono_field_get_value(material, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&asset));
				comp->setSharedMaterial(index, asset);
			}
		}
	}

	MonoObject* API_MeshRenderer::getMesh(MonoObject* this_ptr)
	{
		MeshRenderer* comp = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&comp));

		if (comp != nullptr)
		{
			Mesh* mesh = comp->getMesh();

			if (mesh != nullptr)
				return mesh->getManagedObject();
		}

		return nullptr;
	}

	void API_MeshRenderer::setMesh(MonoObject* this_ptr, MonoObject* value)
	{
		MeshRenderer* comp = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&comp));

		if (comp != nullptr)
		{
			Mesh* mesh = nullptr;
			mono_field_get_value(value, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mesh));

			comp->setMesh(mesh);
		}
	}
}