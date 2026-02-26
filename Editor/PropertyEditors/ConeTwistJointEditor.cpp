#include "ConeTwistJointEditor.h"

#include "../Engine/Components/ConeTwistJoint.h"
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
	ConeTwistJointEditor::ConeTwistJointEditor()
	{
		setEditorName("ConeTwistJointEditor");
	}

	ConeTwistJointEditor::~ConeTwistJointEditor()
	{
	}

	void ConeTwistJointEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Component* component = comps[0];
		ConeTwistJoint* comp = (ConeTwistJoint*)component;

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

		PropVector3* limits = new PropVector3(this, "Limits", comp->getLimits() * Mathf::fRad2Deg);
		limits->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeLimits(prop, val); });

		addProperty(connectedObject);
		addProperty(linkedBodiesCollision);
		addProperty(anchor);
		addProperty(connectedAnchor);
		addProperty(autoConfigure);
		addProperty(limits);
	}

	void ConeTwistJointEditor::onChangeLinkedBodiesCollision(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change joint bodies collision");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				ConeTwistJoint* comp = (ConeTwistJoint*)d.first;
				comp->setLinkedBodiesCollision(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				ConeTwistJoint* comp = (ConeTwistJoint*)d.first;
				comp->setLinkedBodiesCollision(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			ConeTwistJoint* comp = (ConeTwistJoint*)*it;

			undoData->boolData[0][comp] = comp->getLinkedBodiesCollision();
			undoData->boolData[1][comp] = val;

			comp->setLinkedBodiesCollision(val);
		}
	}

	void ConeTwistJointEditor::onChangeAnchor(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change joint anchor");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				ConeTwistJoint* comp = (ConeTwistJoint*)d.first;
				comp->setAnchor(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				ConeTwistJoint* comp = (ConeTwistJoint*)d.first;
				comp->setAnchor(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			ConeTwistJoint* comp = (ConeTwistJoint*)*it;

			undoData->vec3Data[0][comp] = comp->getAnchor();
			undoData->vec3Data[1][comp] = val;

			comp->setAnchor(val);
		}
	}

	void ConeTwistJointEditor::onChangeConnectedAnchor(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change joint connected anchor");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				ConeTwistJoint* comp = (ConeTwistJoint*)d.first;
				comp->setConnectedAnchor(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				ConeTwistJoint* comp = (ConeTwistJoint*)d.first;
				comp->setConnectedAnchor(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			ConeTwistJoint* comp = (ConeTwistJoint*)*it;

			undoData->vec3Data[0][comp] = comp->getConnectedAnchor();
			undoData->vec3Data[1][comp] = val;

			comp->setConnectedAnchor(val);
		}
	}

	void ConeTwistJointEditor::onChangeLimits(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change joint limits");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				ConeTwistJoint* comp = (ConeTwistJoint*)d.first;
				comp->setLimits(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				ConeTwistJoint* comp = (ConeTwistJoint*)d.first;
				comp->setLimits(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			ConeTwistJoint* comp = (ConeTwistJoint*)*it;

			undoData->vec3Data[0][comp] = comp->getLimits();
			undoData->vec3Data[1][comp] = val;

			comp->setLimits(val * Mathf::fDeg2Rad);
		}
	}

	void ConeTwistJointEditor::onDropConnectedBody(TreeNode* prop, TreeNode* from)
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
				ConeTwistJoint* comp = (ConeTwistJoint*)d.first;
				comp->setConnectedObjectGuid(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[1])
			{
				ConeTwistJoint* comp = (ConeTwistJoint*)d.first;
				comp->setConnectedObjectGuid(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			ConeTwistJoint* comp = (ConeTwistJoint*)*it;

			undoData->stringData[0][comp] = comp->getConnectedObjectGuid();
			undoData->stringData[1][comp] = from->name;

			comp->setConnectedObjectGuid(from->name);
		}
	}

	void ConeTwistJointEditor::onClickAutoConfigure(Property* prop)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Joint auto configure");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				ConeTwistJoint* comp = (ConeTwistJoint*)d.first;
				comp->setConnectedAnchor(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				ConeTwistJoint* comp = (ConeTwistJoint*)d.first;
				comp->autoConfigureAnchors();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			ConeTwistJoint* comp = (ConeTwistJoint*)*it;

			undoData->vec3Data[0][comp] = comp->getConnectedAnchor();
			undoData->vec3Data[1][comp] = glm::vec3(0.0f);

			comp->autoConfigureAnchors();
		}

		updateEditor();
	}
}