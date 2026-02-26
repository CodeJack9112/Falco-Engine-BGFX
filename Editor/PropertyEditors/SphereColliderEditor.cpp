#include "SphereColliderEditor.h"

#include "../Engine/Components/SphereCollider.h"
#include "../Engine/Core/GameObject.h"

#include "PropBool.h"
#include "PropFloat.h"
#include "PropVector3.h"

#include "../Classes/Undo.h"
#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	SphereColliderEditor::SphereColliderEditor()
	{
		setEditorName("SphereColliderEditor");
	}

	SphereColliderEditor::~SphereColliderEditor()
	{
	}

	void SphereColliderEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Component* component = comps[0];
		SphereCollider* comp = (SphereCollider*)component;

		PropFloat* radius = new PropFloat(this, "Radius", comp->getRadius());
		radius->setOnChangeCallback([=](Property* prop, float val) { onChangeRadius(prop, val); });

		PropVector3* offsetPosition = new PropVector3(this, "Offset Position", comp->getOffset());
		offsetPosition->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeOffsetPosition(prop, val); });

		PropBool* isTrigger = new PropBool(this, "Is Trigger", comp->getIsTrigger());
		isTrigger->setOnChangeCallback([=](Property* prop, bool val) { onChangeIsTrigger(prop, val); });

		addProperty(isTrigger);
		addProperty(radius);
		addProperty(offsetPosition);
	}

	void SphereColliderEditor::onChangeRadius(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change sphere collider radius");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				SphereCollider* comp = (SphereCollider*)d.first;
				comp->setRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				SphereCollider* comp = (SphereCollider*)d.first;
				comp->setRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			SphereCollider* comp = (SphereCollider*)*it;

			undoData->floatData[0][comp] = comp->getRadius();
			undoData->floatData[1][comp] = val;

			comp->setRadius(val);
		}
	}

	void SphereColliderEditor::onChangeOffsetPosition(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change sphere collider position");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				SphereCollider* comp = (SphereCollider*)d.first;
				comp->setOffset(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				SphereCollider* comp = (SphereCollider*)d.first;
				comp->setOffset(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			SphereCollider* comp = (SphereCollider*)*it;

			undoData->vec3Data[0][comp] = comp->getOffset();
			undoData->vec3Data[1][comp] = val;

			comp->setOffset(val);
		}
	}

	void SphereColliderEditor::onChangeIsTrigger(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change sphere collider is trigger");
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