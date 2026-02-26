#include "HingeJointEditor.h"

#include "../Engine/Components/HingeJoint.h"
#include "../Engine/Core/Engine.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Math/Mathf.h"

#include "PropBool.h"
#include "PropFloat.h"
#include "PropVector3.h"
#include "PropVector2.h"
#include "PropGameObject.h"
#include "PropString.h"
#include "PropButton.h"

#include "../Classes/Undo.h"
#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	HingeJointEditor::HingeJointEditor()
	{
		setEditorName("HingeJointEditor");
	}

	HingeJointEditor::~HingeJointEditor()
	{
	}

	void HingeJointEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Component* component = comps[0];
		HingeJoint* comp = (HingeJoint*)component;

		GameObject* connectedObj = nullptr;
		if (!comp->getConnectedObjectGuid().empty())
		{
			connectedObj = Engine::getSingleton()->getGameObject(comp->getConnectedObjectGuid());
		}

		PropGameObject* connectedObject = new PropGameObject(this, "Connected object", connectedObj);
		connectedObject->setSupportedFormats({ "::GameObject" });
		connectedObject->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropConnectedBody(prop, from); });
		connectedObject->setImage(MainWindow::loadEditorIcon("Inspector/rigidbody.png"));

		PropBool* linkedBodiesCollision = new PropBool(this, "Linked bodies collision", comp->getLinkedBodiesCollision());
		linkedBodiesCollision->setOnChangeCallback([=](Property* prop, bool val) { onChangeLinkedBodiesCollision(prop, val); });

		PropVector3* anchor = new PropVector3(this, "Anchor", comp->getAnchor());
		anchor->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeAnchor(prop, val); });

		PropVector3* connectedAnchor = new PropVector3(this, "Connected anchor", comp->getConnectedAnchor());
		connectedAnchor->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeConnectedAnchor(prop, val); });

		PropButton* autoConfigure = new PropButton(this, "", "Auto");
		autoConfigure->setOnClickCallback([=](Property* prop) { onClickAutoConfigure(prop); });

		PropVector3* axis = new PropVector3(this, "Axis", comp->getAxis());
		axis->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeAxis(prop, val); });

		PropVector2* limits = new PropVector2(this, "Limits", glm::vec2(comp->getLimitMin(), comp->getLimitMax()) * Mathf::fRad2Deg);
		limits->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeLimits(prop, val); });

		addProperty(connectedObject);
		addProperty(linkedBodiesCollision);
		addProperty(anchor);
		addProperty(connectedAnchor);
		addProperty(autoConfigure);
		addProperty(axis);
		addProperty(limits);
	}

	void HingeJointEditor::onChangeLinkedBodiesCollision(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change joint bodies collision");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setLinkedBodiesCollision(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setLinkedBodiesCollision(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			HingeJoint* comp = (HingeJoint*)*it;

			undoData->boolData[0][comp] = comp->getLinkedBodiesCollision();
			undoData->boolData[1][comp] = val;

			comp->setLinkedBodiesCollision(val);
		}
	}

	void HingeJointEditor::onChangeAnchor(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change joint anchor");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setAnchor(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setAnchor(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			HingeJoint* comp = (HingeJoint*)*it;

			undoData->vec3Data[0][comp] = comp->getAnchor();
			undoData->vec3Data[1][comp] = val;

			comp->setAnchor(val);
		}
	}

	void HingeJointEditor::onChangeConnectedAnchor(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change joint connected anchor");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setConnectedAnchor(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setConnectedAnchor(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			HingeJoint* comp = (HingeJoint*)*it;

			undoData->vec3Data[0][comp] = comp->getConnectedAnchor();
			undoData->vec3Data[1][comp] = val;

			comp->setConnectedAnchor(val);
		}
	}

	void HingeJointEditor::onChangeAxis(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change joint axis");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setAxis(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setAxis(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			HingeJoint* comp = (HingeJoint*)*it;

			undoData->vec3Data[0][comp] = comp->getAxis();
			undoData->vec3Data[1][comp] = val;

			comp->setAxis(val);
		}
	}

	void HingeJointEditor::onChangeLimits(Property* prop, glm::vec2 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change joint limits");
		undoData->vec2Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[0])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setLimitMin(d.second.x);
				comp->setLimitMax(d.second.y);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[1])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setLimitMin(d.second.x);
				comp->setLimitMax(d.second.y);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			HingeJoint* comp = (HingeJoint*)*it;

			undoData->vec2Data[0][comp] = glm::vec2(comp->getLimitMin(), comp->getLimitMax()) * Mathf::fDeg2Rad;
			undoData->vec2Data[1][comp] = val;

			comp->setLimitMin(val.x * Mathf::fDeg2Rad);
			comp->setLimitMax(val.y * Mathf::fDeg2Rad);
		}
	}

	void HingeJointEditor::onDropConnectedBody(TreeNode* prop, TreeNode* from)
	{
		GameObject* node = Engine::getSingleton()->getGameObject(from->name);
		((PropGameObject*)prop)->setValue(node);

		//Undo
		UndoData* undoData = Undo::addUndo("Change joint connected object");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setConnectedObjectGuid(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[1])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setConnectedObjectGuid(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			HingeJoint* comp = (HingeJoint*)*it;

			undoData->stringData[0][comp] = comp->getConnectedObjectGuid();
			undoData->stringData[1][comp] = from->name;

			comp->setConnectedObjectGuid(from->name);
		}
	}

	void HingeJointEditor::onClickAutoConfigure(Property* prop)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Joint auto configure");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->setConnectedAnchor(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				HingeJoint* comp = (HingeJoint*)d.first;
				comp->autoConfigureAnchors();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			HingeJoint* comp = (HingeJoint*)*it;

			undoData->vec3Data[0][comp] = comp->getConnectedAnchor();
			undoData->vec3Data[1][comp] = glm::vec3(0.0f);

			comp->autoConfigureAnchors();
		}

		updateEditor();
	}
}