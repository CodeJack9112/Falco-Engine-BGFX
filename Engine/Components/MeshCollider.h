#pragma once

#include "Collider.h"

class btTriangleMesh;

namespace GX
{
	class MeshCollider : public Collider
	{
	private:
		bool convex = false;
		btTriangleMesh* colMesh = nullptr;

		void rebuild();

	public:
		MeshCollider();
		virtual ~MeshCollider();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();

		bool getConvex() { return convex; }
		void setConvex(bool c);

		virtual void onAttach();
		virtual void onSceneLoaded();
	};
}