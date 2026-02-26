#include "API_Camera.h"

#include "../Core/Engine.h"
#include "../Renderer/Renderer.h"
#include "../Core/APIManager.h"
#include "../Components/Camera.h"
#include "../Components/MeshRenderer.h"
#include "../Components/Terrain.h"
#include "../Components/Water.h"
#include "../Components/ParticleSystem.h"
#include "../Components/Transform.h"
#include "../Renderer/RenderTexture.h"
#include "../Assets/Texture.h"
#include "../Core/GameObject.h"

namespace GX
{
	void API_Camera::getViewMatrix(MonoObject* this_ptr, bool ownFrustumOnly, API::Matrix4* matrix)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		glm::mat4x4 mat = cam->getViewMatrix();

		matrix->m00 = mat[0][0];
		matrix->m10 = mat[1][0];
		matrix->m20 = mat[2][0];
		matrix->m30 = mat[3][0];

		matrix->m01 = mat[0][1];
		matrix->m11 = mat[1][1];
		matrix->m21 = mat[2][1];
		matrix->m31 = mat[3][1];

		matrix->m02 = mat[0][2];
		matrix->m12 = mat[1][2];
		matrix->m22 = mat[2][2];
		matrix->m32 = mat[3][2];

		matrix->m03 = mat[0][3];
		matrix->m13 = mat[1][3];
		matrix->m23 = mat[2][3];
		matrix->m33 = mat[3][3];
	}

	void API_Camera::getProjectionMatrix(MonoObject* this_ptr, API::Matrix4* matrix)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		glm::mat4x4 mat = cam->getProjectionMatrix();

		matrix->m00 = mat[0][0];
		matrix->m10 = mat[1][0];
		matrix->m20 = mat[2][0];
		matrix->m30 = mat[3][0];

		matrix->m01 = mat[0][1];
		matrix->m11 = mat[1][1];
		matrix->m21 = mat[2][1];
		matrix->m31 = mat[3][1];

		matrix->m02 = mat[0][2];
		matrix->m12 = mat[1][2];
		matrix->m22 = mat[2][2];
		matrix->m32 = mat[3][2];

		matrix->m03 = mat[0][3];
		matrix->m13 = mat[1][3];
		matrix->m23 = mat[2][3];
		matrix->m33 = mat[3][3];
	}

	void API_Camera::worldToScreenPoint(MonoObject* this_ptr, API::Vector3* ref_world, API::Vector3* out_screen)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		glm::vec3 screenSpacePoint = cam->worldToScreenPoint(glm::vec3(ref_world->x, ref_world->y, ref_world->z));

		out_screen->x = screenSpacePoint.x;
		out_screen->y = screenSpacePoint.y;
		out_screen->z = screenSpacePoint.z;
	}

	void API_Camera::screenToWorldPoint(MonoObject* this_ptr, API::Vector3* ref_screen, API::Vector3* out_world)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		glm::vec3 vect = cam->screenToWorldPoint(glm::vec3(ref_screen->x, ref_screen->y, ref_screen->z));
		
		out_world->x = vect.x;
		out_world->y = vect.y;
		out_world->z = vect.z;
	}

	float API_Camera::getNearClipDistance(MonoObject* this_ptr)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		return cam->getNear();
	}

	void API_Camera::setNearClipDistance(MonoObject* this_ptr, float value)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		cam->setNear(value);
	}

	float API_Camera::getFarClipDistance(MonoObject* this_ptr)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		return cam->getFar();
	}

	void API_Camera::setFarClipDistance(MonoObject* this_ptr, float value)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		cam->setFar(value);
	}

	bool API_Camera::isObjectVisible(MonoObject* this_ptr, MonoObject* obj)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		GameObject* object = nullptr;
		mono_field_get_value(obj, APIManager::getSingleton()->gameobject_ptr_field, &object);

		AxisAlignedBox aab = AxisAlignedBox::Extent::EXTENT_INFINITE;

		MeshRenderer* rend = (MeshRenderer*)object->getComponent(MeshRenderer::COMPONENT_TYPE);
		if (rend != nullptr)
		{
			aab = rend->getBounds();
		}

		if (aab.isInfinite())
		{
			ParticleSystem* rend2 = (ParticleSystem*)object->getComponent(ParticleSystem::COMPONENT_TYPE);
			if (rend2 != nullptr)
			{
				for (auto& em : rend2->getEmitters())
					aab.merge(em->getBounds());
			}
		}

		if (aab.isInfinite())
		{
			Water* rend3 = (Water*)object->getComponent(Water::COMPONENT_TYPE);
			if (rend3 != nullptr)
			{
				aab = rend3->getBounds();
			}
		}

		if (aab.isInfinite())
		{
			Terrain* rend4 = (Terrain*)object->getComponent(Terrain::COMPONENT_TYPE);
			if (rend4 != nullptr)
				aab = rend4->getBounds();
		}

		bool visible = false;

		if (aab.isFinite())
		{
			visible = cam->isVisible(aab);
		}
		else
		{
			Transform* trans = object->getTransform();
			glm::vec3 screenPos = cam->worldToScreenPoint(trans->getPosition());
			if (screenPos.z > 0.0f &&
				screenPos.x >= 0 &&
				screenPos.x <= Renderer::getSingleton()->getWidth() &&
				screenPos.y >= 0 &&
				screenPos.y <= Renderer::getSingleton()->getHeight())
			{
				visible = true;
			}
		}

		return visible;
	}

	MonoObject* API_Camera::getRenderTarget(MonoObject* this_ptr)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		RenderTexture* rt = cam->getRenderTarget();

		return rt->getManagedObject();
	}

	void API_Camera::setRenderTarget(MonoObject* this_ptr, MonoObject* value)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		RenderTexture* rt = nullptr;
		if (value != nullptr)
			mono_field_get_value(value, APIManager::getSingleton()->object_ptr_field, reinterpret_cast<void*>(&rt));

		if (cam != nullptr)
			cam->setRenderTarget(rt);
	}

	MonoObject* API_Camera::getGBufferTexture(MonoObject* this_ptr, int value)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		Texture* tex = nullptr;

		switch (value)
		{
		case 0:
			tex = cam->getGBufferDiffuseTexture();
			break;
		case 1:
			tex = cam->getGBufferNormalTexture();
			break;
		case 2:
			tex = cam->getGBufferMRASTexture();
			break;
		case 3:
			tex = cam->getGBufferLightmapTexture();
			break;
		case 4:
			tex = cam->getGBufferDepthTexture();
			break;
		}

		if (tex != nullptr)
			return tex->getManagedObject();

		return nullptr;
	}

	float API_Camera::getFov(MonoObject* this_ptr)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		if (cam == nullptr)
			return 0.0f;

		return cam->getFOVy();
	}

	void API_Camera::setFov(MonoObject* this_ptr, float value)
	{
		Camera* cam = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&cam));

		if (cam == nullptr)
			return;

		cam->setFOVy(value);
	}
}