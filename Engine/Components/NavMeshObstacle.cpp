#include "NavMeshObstacle.h"

#include <glm/gtc/type_ptr.hpp>

#include "../Core/Engine.h"
#include "../Core/GameObject.h"
#include "../Components/Transform.h"
#include "../Core/NavigationManager.h"
#include "../Core/APIManager.h"
#include "../Math/AxisAlignedBox.h"

#include "../Navigation/DetourTileCache/Include/DetourTileCache.h"

namespace GX
{
	std::string NavMeshObstacle::COMPONENT_TYPE = "NavMeshObstacle";

	NavMeshObstacle::NavMeshObstacle() : Component(APIManager::getSingleton()->navmeshobstacle_class)
	{
		
	}

	NavMeshObstacle::~NavMeshObstacle()
	{
		
	}

	std::string NavMeshObstacle::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	void NavMeshObstacle::onAttach()
	{
		Component::onAttach();

		NavigationManager::getSingleton()->addObstacle(this);

		if (getEnabled() && getGameObject()->getActive())
			addObstacle();
	}

	void NavMeshObstacle::onDetach()
	{
		Component::onDetach();

		NavigationManager::getSingleton()->removeObstacle(this);
		removeObstacle();
	}

	void NavMeshObstacle::onSceneLoaded()
	{
		
	}

	void NavMeshObstacle::onStateChanged()
	{
		if (getGameObject() == nullptr)
			return;

		if (getGameObject()->getActive() && getEnabled())
			addObstacle();
		else
			removeObstacle();
	}

	Component* NavMeshObstacle::onClone()
	{
		NavMeshObstacle* newComponent = new NavMeshObstacle();
		newComponent->enabled = enabled;
		newComponent->size = size;
		newComponent->offset = offset;

		return newComponent;
	}

	void NavMeshObstacle::update()
	{
		if (!getEnabled() || !getGameObject()->getActive())
			return;

		GameObject* obj = getGameObject();
		Transform* t = obj->getTransform();

		glm::vec3 pos = t->getPosition();
		glm::vec3 scale = t->getScale();
		glm::highp_quat rot = t->getRotation();

		if (oldSize != size)
		{
			oldSize = size;
			updateObstacle();
		}

		if (oldOffset != offset)
		{
			oldOffset = offset;
			updateObstacle();
		}

		if (oldPos != pos)
		{
			oldPos = pos;
			updateObstacle();
		}

		if (oldRot != rot)
		{
			oldRot = rot;
			updateObstacle();
		}

		if (oldScale != scale)
		{
			oldScale = scale;
			updateObstacle();
		}
	}

	void NavMeshObstacle::addObstacle()
	{
		if (obstacleRef == 0)
		{
			NavigationManager* mgr = NavigationManager::getSingleton();
			dtTileCache* cache = mgr->getTileCache();

			if (cache == nullptr)
				return;

			GameObject* obj = getGameObject();
			Transform* t = obj->getTransform();

			glm::mat4x4 ft = t->getTransformMatrix();
			ft = glm::translate(ft, offset);
			ft = glm::scale(ft, size);

			AxisAlignedBox aab = AxisAlignedBox::BOX_NULL;
			
			aab.setExtents(glm::vec3(-1.0f), glm::vec3(1.0f));
			aab.transform(ft);

			cache->addBoxObstacle(glm::value_ptr(aab.getMinimum()), glm::value_ptr(aab.getMaximum()), &obstacleRef);

			cache->update(0.0f, mgr->getNavMesh());
		}
	}

	void NavMeshObstacle::removeObstacle()
	{
		if (obstacleRef != 0)
		{
			NavigationManager* mgr = NavigationManager::getSingleton();
			dtTileCache* cache = mgr->getTileCache();
			cache->removeObstacle(obstacleRef);
			obstacleRef = 0;

			cache->update(0.0f, mgr->getNavMesh());
		}
	}

	void NavMeshObstacle::updateObstacle()
	{
		removeObstacle();
		addObstacle();

		if (!Engine::getSingleton()->getIsRuntimeMode())
			NavigationManager::getSingleton()->setNavMeshIsDirty();
	}
}