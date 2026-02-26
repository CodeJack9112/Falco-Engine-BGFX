#pragma once

#include "UIElementEditor.h"

namespace GX
{
	class MaskEditor : public UIElementEditor
	{
	public:
		MaskEditor();
		~MaskEditor() {}

		virtual void init(std::vector<Component*> comps);		
	};
}