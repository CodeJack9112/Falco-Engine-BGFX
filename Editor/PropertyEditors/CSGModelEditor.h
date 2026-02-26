#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec2.hpp"

namespace GX
{
	class CSGModelEditor : public ComponentEditor
	{
	public:
		CSGModelEditor();
		virtual ~CSGModelEditor() {}

		virtual void init(std::vector<Component*> comps);

	private:
		
	};
}