#include "CSGModelEditor.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Components/Component.h"
#include "../Engine/Components/CSGModel.h"

namespace GX
{
	CSGModelEditor::CSGModelEditor()
	{
		setEditorName("CSGModelEditor");
	}

	void CSGModelEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		CSGModel* comp = (CSGModel*)comps[0];
	}
}