#include "NavMeshAgent.h"

#include "../glm/gtc/type_ptr.hpp"

#include "../Core/Engine.h"
#include "../Renderer/Renderer.h"
#include "../Core/GameObject.h"
#include "../Components/Transform.h"
#include "../Core/NavigationManager.h"
#include "../Core/APIManager.h"
#include "../Core/Time.h"

#include "Navigation/InputGeom.h"
#include "Navigation/Detour/Include/DetourNavMeshQuery.h"
#include "Navigation/DebugUtils/Include/RecastDump.h"
#include "Navigation/DetourCrowd/Include/DetourCrowd.h"
#include "Navigation/Detour/Include/DetourCommon.h"

#include "../Math/Mathf.h"

namespace GX
{
	std::string NavMeshAgent::COMPONENT_TYPE = "NavMeshAgent";

	/* NavMesh Agent*/

	NavMeshAgent::NavMeshAgent() : Component(APIManager::getSingleton()->navmeshagent_class)
	{
		NavigationManager::getSingleton()->addAgent(this);
	}

	NavMeshAgent::~NavMeshAgent()
	{
		NavigationManager::getSingleton()->removeAgent(this);
	}

	std::string NavMeshAgent::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* NavMeshAgent::onClone()
	{
		NavMeshAgent* newComponent = new NavMeshAgent();
		newComponent->enabled = enabled;
		newComponent->radius = radius;
		newComponent->height = height;
		newComponent->speed = speed;
		newComponent->acceleration = acceleration;
		newComponent->rotationSpeed = rotationSpeed;
		newComponent->targetPosition = targetPosition;

		return newComponent;
	}

	void NavMeshAgent::update()
	{
		if (!getEnabled() || !getGameObject()->getActive())
			return;

		float dt = Time::getDeltaTime();
		float ts = Time::getTimeScale();
		GameObject* sceneNode = getGameObject();
		Transform* t = sceneNode->getTransform();
		glm::vec3 currentPos = t->getPosition();

		if (currentPath.VertCount > 0)
		{
			//***--------FIND PATH--------***//

			glm::vec3 targetPos = glm::vec3(currentPath.PosX[1], currentPath.PosY[1], currentPath.PosZ[1]);
			glm::vec3 dir = glm::normalize(targetPos - currentPos);
			glm::vec3 newPos = currentPos + ((dir * speed * 2.0f) * dt * ts);

			//***--------HANDLE COLLISIONS WITH NEIGHBOURS--------***//

			std::vector<NavMeshAgent*> & agents = NavigationManager::getSingleton()->getAgentList();
			static const float COLLISION_RESOLVE_FACTOR = 0.7f;
			glm::vec3 vel = dir;

			auto _idx = find(agents.begin(), agents.end(), this);
			int idx0 = distance(agents.begin(), _idx);

			for (int iter = 0; iter < 4; ++iter)
			{
				glm::vec3 disp = glm::vec3(0);

				float w = 0;

				for (auto it = agents.begin(); it != agents.end(); ++it)
				{
					NavMeshAgent* nei = *it;
					if (nei == this)
						continue;

					GameObject* neiNode = nei->getGameObject();
					Transform* nt = neiNode->getTransform();
					glm::vec3 neiPos = nt->getPosition();
					float neiRadius = nei->getRadius();

					if (glm::distance(neiPos, t->getPosition()) <= neiRadius + radius)
					{
						int idx1 = distance(agents.begin(), it);

						float diff[3];
						dtVsub(diff, glm::value_ptr(newPos), glm::value_ptr(neiPos));
						diff[1] = 0;

						float dist = dtVlenSqr(diff);
						if (dist > dtSqr(radius + neiRadius))
							continue;
						dist = dtMathSqrtf(dist);
						float pen = (radius + neiRadius) - dist;
						if (dist < 0.0001f)
						{
							// Agents on top of each other, try to choose diverging separation directions.
							if (idx0 > idx1)
								dtVset(diff, -vel.z, 0, vel.x);
							else
								dtVset(diff, vel.z, 0, -vel.x);
							pen = 0.01f;
						}
						else
						{
							pen = (1.0f / dist) * (pen * 0.5f) * COLLISION_RESOLVE_FACTOR;
						}

						dtVmad(glm::value_ptr(disp), glm::value_ptr(disp), diff, pen);

						w += 1.0f;
					}
				}

				if (w > 0.0001f)
				{
					const float iw = 1.0f / w;
					dtVscale(glm::value_ptr(disp), glm::value_ptr(disp), iw);
				}

				newPos = newPos + disp;
				//newPos = Mathf::lerp(newPos, newPos + disp, COLLISION_RESOLVE_FACTOR * 10.0f * dt);
			}

			//***--------SET POSITION AND ROTATION--------***//

			glm::vec3 right(dir.z, 0, -dir.x);
			right = glm::normalize(right);
			glm::vec3 up = glm::cross(dir, right);
			glm::highp_quat quat = glm::quatLookAt(-dir, up);

			t->setPosition(newPos);

			glm::highp_quat _r = t->getRotation();
			glm::highp_quat r = glm::slerp(_r, quat, (rotationSpeed * 2.0f) * dt * ts);

			if (!glm::isnan(r.x) && !glm::isnan(r.y) && !glm::isnan(r.z) && !glm::isnan(r.w))
				t->setRotation(r);
		}
	}

	void NavMeshAgent::setRadius(float r)
	{
		radius = r;
	}

	void NavMeshAgent::setHeight(float h)
	{
		height = h;
	}

	void NavMeshAgent::setSpeed(float s)
	{
		speed = s;
	}

	void NavMeshAgent::setAcceleration(float a)
	{
		acceleration = a;
	}

	void NavMeshAgent::setTargetPosition(glm::vec3 pos)
	{
		targetPosition = pos;
		
		glm::vec3 currPos = getGameObject()->getTransform()->getPosition();
		dtNavMeshQuery * query = NavigationManager::getSingleton()->getNavMeshQuery();
		dtNavMesh* navMesh = NavigationManager::getSingleton()->getNavMesh();

		if (navMesh != nullptr)
			currentPath = findPath(currPos, pos);
	}

	void NavMeshAgent::setRotationSpeed(float s)
	{
		rotationSpeed = s;
	}

	PathData NavMeshAgent::findPath(glm::vec3 StartPos, glm::vec3 EndPos)
	{
		PathData m_PathStore;

		dtStatus status;
		const float pExtents[3] = { 4, 4, 4 }; // size of box around start/end points to look for nav polygons
		dtPolyRef StartPoly;
		float StartNearest[3];
		dtPolyRef EndPoly;
		float EndNearest[3];
		dtPolyRef PolyPath[MAX_PATHPOLY];
		int nPathCount = 0;
		float StraightPath[MAX_PATHVERT * 3];
		int nVertCount = 0;

		dtNavMeshQuery * m_navQuery = NavigationManager::getSingleton()->getNavMeshQuery();

		// setup the filter
		dtQueryFilter Filter;
		Filter.setIncludeFlags(SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED);
		Filter.setExcludeFlags(0);
		Filter.setAreaCost(SAMPLE_POLYAREA_GROUND, 1.0f);
		Filter.setAreaCost(SAMPLE_POLYAREA_WATER, 10.0f);
		Filter.setAreaCost(SAMPLE_POLYAREA_ROAD, 1.0f);
		Filter.setAreaCost(SAMPLE_POLYAREA_DOOR, 1.0f);
		Filter.setAreaCost(SAMPLE_POLYAREA_GRASS, 2.0f);
		Filter.setAreaCost(SAMPLE_POLYAREA_JUMP, 1.5f);

		// find the start polygon
		status = m_navQuery->findNearestPoly(glm::value_ptr(StartPos), pExtents, &Filter, &StartPoly, StartNearest);
		if ((status&DT_FAILURE) || (status&DT_STATUS_DETAIL_MASK))  m_PathStore.valid = false; // couldn't find a polygon

		// find the end polygon
		status = m_navQuery->findNearestPoly(glm::value_ptr(EndPos), pExtents, &Filter, &EndPoly, EndNearest);
		if ((status&DT_FAILURE) || (status&DT_STATUS_DETAIL_MASK))  m_PathStore.valid = false; // couldn't find a polygon

		status = m_navQuery->findPath(StartPoly, EndPoly, StartNearest, EndNearest, &Filter, PolyPath, &nPathCount, MAX_PATHPOLY);
		if ((status&DT_FAILURE) || (status&DT_STATUS_DETAIL_MASK)) m_PathStore.valid = false; // couldn't create a path
		if (nPathCount == 0) m_PathStore.valid = false; // couldn't find a path

		status = m_navQuery->findStraightPath(StartNearest, EndNearest, PolyPath, nPathCount, StraightPath, NULL, NULL, &nVertCount, MAX_PATHVERT);
		if ((status&DT_FAILURE) || (status&DT_STATUS_DETAIL_MASK))  m_PathStore.valid = false; // couldn't create a path
		if (nVertCount == 0) m_PathStore.valid = false; // couldn't find a path

		// At this point we have our path.  Copy it to the path store
		int nIndex = 0;
		for (int nVert = 0; nVert < nVertCount; nVert++)
		{
			m_PathStore.PosX[nVert] = StraightPath[nIndex++];
			m_PathStore.PosY[nVert] = StraightPath[nIndex++];
			m_PathStore.PosZ[nVert] = StraightPath[nIndex++];
		}

		m_PathStore.VertCount = nVertCount;

		return m_PathStore;

	}
}