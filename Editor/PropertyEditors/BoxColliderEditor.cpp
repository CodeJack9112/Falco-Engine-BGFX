#include "BoxColliderEditor.h"

#include "../Engine/Components/BoxCollider.h"
#include "../Engine/Math/Mathf.h"
#include "../Engine/Core/GameObject.h"

#include "PropBool.h"
#include "PropFloat.h"
#include "PropVector3.h"

#include "../Classes/Undo.h"
#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	BoxColliderEditor::BoxColliderEditor()
	{
		setEditorName("BoxColliderEditor");
	}

	BoxColliderEditor::~BoxColliderEditor()
	{
	}

	void BoxColliderEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Component* component = comps[0];
		BoxCollider* comp = (BoxCollider*)component;

		PropVector3* size = new PropVector3(this, "Size", comp->getBoxSize());
		size->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeSize(prop, val); });

		PropVector3* offsetPosition = new PropVector3(this, "Offset Position", comp->getOffset());
		offsetPosition->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeOffsetPosition(prop, val); });

		glm::vec3 eulerAngles = Mathf::toEuler(comp->getRotation()) * Mathf::fRad2Deg;

		PropVector3* offsetRotation = new PropVector3(this, "Offset Rotation", eulerAngles);
		offsetRotation->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeOffsetRotation(prop, val); });

		PropBool* isTrigger = new PropBool(this, "Is Trigger", comp->getIsTrigger());
		isTrigger->setOnChangeCallback([=](Property* prop, bool val) { onChangeIsTrigger(prop, val); });

		addProperty(isTrigger);
		addProperty(size);
		addProperty(offsetPosition);
		addProperty(offsetRotation);
	}

	void BoxColliderEditor::onChangeSize(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change box collider size");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				BoxCollider* comp = (BoxCollider*)d.first;
				comp->setBoxSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				BoxCollider* comp = (BoxCollider*)d.first;
				comp->setBoxSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Component* component = (*it);
			BoxCollider* comp = (BoxCollider*)component;

			undoData->vec3Data[0][comp] = comp->getBoxSize();
			undoData->vec3Data[1][comp] = val;

			comp->setBoxSize(val);
		}
	}

	void BoxColliderEditor::onChangeOffsetPosition(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change box collider position");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				BoxCollider* comp = (BoxCollider*)d.first;
				comp->setOffset(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				BoxCollider* comp = (BoxCollider*)d.first;
				comp->setOffset(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Component* component = (*it);
			BoxCollider* comp = (BoxCollider*)component;

			undoData->vec3Data[0][comp] = comp->getOffset();
			undoData->vec3Data[1][comp] = val;

			comp->setOffset(val);
		}
	}

	void BoxColliderEditor::onChangeOffsetRotation(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change box collider rotation");
		undoData->quatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->quatData[0])
			{
				BoxCollider* comp = (BoxCollider*)d.first;
				comp->setRotation(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->quatData[1])
			{
				BoxCollider* comp = (BoxCollider*)d.first;
				comp->setRotation(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Component* component = (*it);
			BoxCollider* comp = (BoxCollider*)component;
			glm::highp_quat rotation = Mathf::toQuaternion(val);

			undoData->quatData[0][comp] = comp->getRotation();
			undoData->quatData[1][comp] = rotation;

			comp->setRotation(rotation);
		}
	}

	void BoxColliderEditor::onChangeIsTrigger(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change box collider is trigger");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				BoxCollider* comp = (BoxCollider*)d.first;
				comp->setIsTrigger(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				BoxCollider* comp = (BoxCollider*)d.first;
				comp->setIsTrigger(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Component* component = (*it);
			Collider* comp = (Collider*)component;

			undoData->boolData[0][comp] = comp->getIsTrigger();
			undoData->boolData[1][comp] = val;

			comp->setIsTrigger(val);
		}
	}
}