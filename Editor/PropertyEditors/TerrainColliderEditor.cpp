#include "TerrainColliderEditor.h"

#include "../Engine/Components/TerrainCollider.h"
#include "../Engine/Core/GameObject.h"

#include "PropBool.h"

#include "../Classes/Undo.h"
#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	TerrainColliderEditor::TerrainColliderEditor()
	{
		setEditorName("TerrainColliderEditor");
	}

	TerrainColliderEditor::~TerrainColliderEditor()
	{
	}

	void TerrainColliderEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Component* component = comps[0];
		TerrainCollider* comp = (TerrainCollider*)component;

		PropBool* isTrigger = new PropBool(this, "Is Trigger", comp->getIsTrigger());
		isTrigger->setOnChangeCallback([=](Property* prop, bool val) { onChangeIsTrigger(prop, val); });

		addProperty(isTrigger);
	}

	void TerrainColliderEditor::onChangeIsTrigger(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change terrain collider is trigger");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Collider* comp = (Collider*)d.first;
				comp->setIsTrigger(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Collider* comp = (Collider*)d.first;
				comp->setIsTrigger(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Collider* comp = (Collider*)*it;

			undoData->boolData[0][comp] = comp->getIsTrigger();
			undoData->boolData[1][comp] = val;

			comp->setIsTrigger(val);
		}
	}
}