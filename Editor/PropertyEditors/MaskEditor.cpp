#include "MaskEditor.h"

#include "PropColorPicker.h"

namespace GX
{
	MaskEditor::MaskEditor()
	{
		setEditorName("MaskEditor");
	}

	void MaskEditor::init(std::vector<Component*> comps)
	{
		UIElementEditor::init(comps);

		color->setVisible(false);
	}
}