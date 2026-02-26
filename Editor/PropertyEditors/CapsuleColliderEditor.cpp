#include "CapsuleColliderEditor.h"

#include "../Engine/Components/CapsuleCollider.h"
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
	CapsuleColliderEditor::CapsuleColliderEditor()
	{
		setEditorName("CapsuleColliderEditor");
	}

	CapsuleColliderEditor::~CapsuleColliderEditor()
	{
	}

	void CapsuleColliderEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Component* component = comps[0];
		CapsuleCollider* comp = (CapsuleCollider*)component;

		PropFloat* radius = new PropFloat(this, "Radius", comp->getRadius());
		radius->setOnChangeCallback([=](Property* prop, float val) { onChangeRadius(prop, val); });

		PropFloat* height = new PropFloat(this, "Height", comp->getHeight());
		height->setOnChangeCallback([=](Property* prop, float val) { onChangeHeight(prop, val); });

		PropVector3* offsetPosition = new PropVector3(this, "Offset Position", comp->getOffset());
		offsetPosition->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeOffsetPosition(prop, val); });

		glm::vec3 eulerAngles = Mathf::toEuler(comp->getRotation());

		PropVector3* offsetRotation = new PropVector3(this, "Offset Rotation", eulerAngles);
		offsetRotation->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeOffsetRotation(prop, val); });

		PropBool* isTrigger = new PropBool(this, "Is Trigger", comp->getIsTrigger());
		isTrigger->setOnChangeCallback([=](Property* prop, bool val) { onChangeIsTrigger(prop, val); });

		addProperty(isTrigger);
		addProperty(radius);
		addProperty(height);
		addProperty(offsetPosition);
		addProperty(offsetRotation);
	}

	void CapsuleColliderEditor::onChangeRadius(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change capsule collider radius");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				CapsuleCollider* comp = (CapsuleCollider*)d.first;
				comp->setRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				CapsuleCollider* comp = (CapsuleCollider*)d.first;
				comp->setRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CapsuleCollider* comp = (CapsuleCollider*)*it;

			undoData->floatData[0][comp] = comp->getRadius();
			undoData->floatData[1][comp] = val;

			comp->setRadius(val);
		}
	}

	void CapsuleColliderEditor::onChangeHeight(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change capsule collider height");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				CapsuleCollider* comp = (CapsuleCollider*)d.first;
				comp->setHeight(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				CapsuleCollider* comp = (CapsuleCollider*)d.first;
				comp->setHeight(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CapsuleCollider* comp = (CapsuleCollider*)*it;

			undoData->floatData[0][comp] = comp->getHeight();
			undoData->floatData[1][comp] = val;

			comp->setHeight(val);
		}
	}

	void CapsuleColliderEditor::onChangeOffsetPosition(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change capsule collider position");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				CapsuleCollider* comp = (CapsuleCollider*)d.first;
				comp->setOffset(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				CapsuleCollider* comp = (CapsuleCollider*)d.first;
				comp->setOffset(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CapsuleCollider* comp = (CapsuleCollider*)*it;

			undoData->vec3Data[0][comp] = comp->getOffset();
			undoData->vec3Data[1][comp] = val;

			comp->setOffset(val);
		}
	}

	void CapsuleColliderEditor::onChangeOffsetRotation(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change capsule collider rotation");
		undoData->quatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->quatData[0])
			{
				CapsuleCollider* comp = (CapsuleCollider*)d.first;
				comp->setRotation(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->quatData[1])
			{
				CapsuleCollider* comp = (CapsuleCollider*)d.first;
				comp->setRotation(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CapsuleCollider* comp = (CapsuleCollider*)*it;
			glm::highp_quat rotation = Mathf::toQuaternion(val);

			undoData->quatData[0][comp] = comp->getRotation();
			undoData->quatData[1][comp] = rotation;

			comp->setRotation(rotation);
		}
	}

	void CapsuleColliderEditor::onChangeIsTrigger(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change capsule collider is trigger");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				CapsuleCollider* comp = (CapsuleCollider*)d.first;
				comp->setIsTrigger(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				CapsuleCollider* comp = (CapsuleCollider*)d.first;
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