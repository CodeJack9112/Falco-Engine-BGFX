#pragma once

#include "Collider.h"

namespace GX
{
	class TerrainCollider : public Collider
	{
	private:
		float* pTerrainHeightDataConvert = nullptr;

		void rebuild();

	public:
		TerrainCollider();
		virtual ~TerrainCollider();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType() { return COMPONENT_TYPE; }
		virtual Component* onClone();
		virtual void onAttach();
		virtual void onSceneLoaded();
	};
}