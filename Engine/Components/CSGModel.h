#pragma once

#include "Component.h"

namespace GX
{
	class CSGModel : public Component
	{
	public:
		CSGModel();
		virtual ~CSGModel() {}

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType() { return COMPONENT_TYPE; }

		virtual Component* onClone();
	};
}