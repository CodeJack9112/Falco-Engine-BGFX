#include "MeshColliderEditor.h"

#include "../Engine/Components/MeshCollider.h"
#include "../Engine/Core/GameObject.h"

#include "PropBool.h"
#include "PropFloat.h"

#include "../Classes/Undo.h"
#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	MeshColliderEditor::MeshColliderEditor()
	{
		setEditorName("MeshColliderEditor");
	}

	MeshColliderEditor::~MeshColliderEditor()
	{
	}

	void MeshColliderEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Component* component = comps[0];
		MeshCollider* comp = (MeshCollider*)component;

		PropBool* convex = new PropBool(this, "Convex", comp->getConvex());
		convex->setOnChangeCallback([=](Property* prop, bool val) { onChangeConvex(prop, val); });

		PropBool* isTrigger = new PropBool(this, "Is Trigger", comp->getIsTrigger());
		isTrigger->setOnChangeCallback([=](Property* prop, bool val) { onChangeIsTrigger(prop, val); });

		addProperty(isTrigger);
		addProperty(convex);
	}

	void MeshColliderEditor::onChangeConvex(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change mesh collider is convex");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				MeshCollider* comp = (MeshCollider*)d.first;
				comp->setConvex(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				MeshCollider* comp = (MeshCollider*)d.first;
				comp->setConvex(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			MeshCollider* comp = (MeshCollider*)*it;

			undoData->boolData[0][comp] = comp->getConvex();
			undoData->boolData[1][comp] = val;

			comp->setConvex(val);
		}
	}

	void MeshColliderEditor::onChangeIsTrigger(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change mesh collider is trigger");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				MeshCollider* comp = (MeshCollider*)d.first;
				comp->setIsTrigger(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				MeshCollider* comp = (MeshCollider*)d.first;
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