#include "VehicleEditor.h"

#include "../Engine/Components/Vehicle.h"
#include "../Engine/Core/Engine.h"
#include "../Engine/Core/GameObject.h"

#include "PropBool.h"
#include "PropFloat.h"
#include "PropVector3.h"
#include "PropVector2.h"
#include "PropGameObject.h"
#include "PropString.h"
#include "PropButton.h"
#include "PropComboBox.h"

#include "../Classes/Undo.h"
#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	VehicleEditor::VehicleEditor()
	{
		setEditorName("VehicleEditor");
	}

	VehicleEditor::~VehicleEditor()
	{
	}

	void VehicleEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Component* component = comps[0];
		Vehicle* comp = (Vehicle*)component;

		PropBool* invertForward = new PropBool(this, "Invert forward", comp->getInvertForward());
		invertForward->setOnChangeCallback([=](Property* prop, bool val) { onChangeInvertForward(prop, val); });

		PropComboBox* axis = new PropComboBox(this, "Axis", { "XYZ", "XZY", "ZXY", "ZYX", "YXZ", "YZX" });
		
		if (comp->getAxis() == glm::vec3(0, 1, 2)) axis->setCurrentItem(0);
		if (comp->getAxis() == glm::vec3(0, 2, 1)) axis->setCurrentItem(1);
		if (comp->getAxis() == glm::vec3(2, 0, 1)) axis->setCurrentItem(2);
		if (comp->getAxis() == glm::vec3(2, 1, 0)) axis->setCurrentItem(3);
		if (comp->getAxis() == glm::vec3(1, 0, 2)) axis->setCurrentItem(4);
		if (comp->getAxis() == glm::vec3(1, 2, 0)) axis->setCurrentItem(5);

		axis->setOnChangeCallback([=](Property* prop, std::string val) { onChangeAxis(prop, val); });

		Property* wheelsGroup = new Property(this, "Wheels");
		wheelsGroup->setHasButton(true);
		wheelsGroup->setButtonText("+");
		wheelsGroup->setOnButtonClickCallback([=](TreeNode* prop) { onClickAddWheel(prop); });

		std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();

		for (auto it = wheels.begin(); it != wheels.end(); ++it)
		{
			Vehicle::WheelInfo inf = *it;

			GameObject* connectedObj = nullptr;
			if (!inf.m_connectedObjectGuid.empty())
			{
				connectedObj = Engine::getSingleton()->getGameObject(inf.m_connectedObjectGuid);
			}

			int index = std::distance(wheels.begin(), it);

			Property* wheel = new Property(this, "Wheel " + std::to_string(index + 1));
			wheel->setUserData(static_cast<void*>(new int(index)));
			wheel->setPopupMenu({ "Remove" }, [=](TreeNode* node, int val) { onClickWheelPopup(node, val); });

			PropGameObject* connectedObject = new PropGameObject(this, "Connected object", connectedObj);
			connectedObject->setSupportedFormats({ "::GameObject" });
			connectedObject->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropConnectedBody(prop, from); });
			connectedObject->setImage(MainWindow::loadEditorIcon("Hierarchy/empty.png"));

			PropBool* isFrontWheel = new PropBool(this, "Is Front Wheel", inf.m_isFrontWheel);
			isFrontWheel->setOnChangeCallback([=](Property* prop, bool val) { onChangeIsFrontWheel(prop, val); });

			PropVector3* connectionPoint = new PropVector3(this, "Connection Point", inf.m_connectionPoint);
			connectionPoint->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeConnectionPoint(prop, val); });

			PropButton* autoConfigure = new PropButton(this, "", "Auto");
			autoConfigure->setOnClickCallback([=](Property* prop) { onClickAutoConfigure(prop); });

			PropFloat* radius = new PropFloat(this, "Radius", inf.m_radius);
			radius->setOnChangeCallback([=](Property* prop, float val) { onChangeRadius(prop, val); });

			PropFloat* width = new PropFloat(this, "Width", inf.m_width);
			width->setOnChangeCallback([=](Property* prop, float val) { onChangeWidth(prop, val); });

			PropFloat* suspensionStiffness = new PropFloat(this, "Suspension Stiffness", inf.m_suspensionStiffness);
			suspensionStiffness->setOnChangeCallback([=](Property* prop, float val) { onChangeSuspensionStiffness(prop, val); });

			PropFloat* suspensionDamping = new PropFloat(this, "Suspension Damping", inf.m_suspensionDamping);
			suspensionDamping->setOnChangeCallback([=](Property* prop, float val) { onChangeSuspensionDamping(prop, val); });

			PropFloat* suspensionCompression = new PropFloat(this, "Suspension Compression", inf.m_suspensionCompression);
			suspensionCompression->setOnChangeCallback([=](Property* prop, float val) { onChangeSuspensionCompression(prop, val); });

			PropFloat* suspensionRestLength = new PropFloat(this, "Suspension Rest Length", inf.m_suspensionRestLength);
			suspensionRestLength->setOnChangeCallback([=](Property* prop, float val) { onChangeSuspensionRestLength(prop, val); });

			PropFloat* friction = new PropFloat(this, "Friction", inf.m_friction);
			friction->setOnChangeCallback([=](Property* prop, float val) { onChangeFriction(prop, val); });

			PropFloat* rollInfluence = new PropFloat(this, "Roll Influence", inf.m_rollInfluence);
			rollInfluence->setOnChangeCallback([=](Property* prop, float val) { onChangeRollInfluence(prop, val); });

			PropVector3* direction = new PropVector3(this, "Direction", inf.m_direction);
			direction->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeDirection(prop, val); });

			PropVector3* axle = new PropVector3(this, "Axle", inf.m_axle);
			axle->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeAxle(prop, val); });

			wheel->addChild(connectedObject);
			wheel->addChild(isFrontWheel);
			wheel->addChild(connectionPoint);
			wheel->addChild(autoConfigure);
			wheel->addChild(radius);
			wheel->addChild(width);
			wheel->addChild(suspensionStiffness);
			wheel->addChild(suspensionDamping);
			wheel->addChild(suspensionCompression);
			wheel->addChild(suspensionRestLength);
			wheel->addChild(friction);
			wheel->addChild(rollInfluence);
			wheel->addChild(direction);
			wheel->addChild(axle);

			wheelsGroup->addChild(wheel);
		}

		addProperty(invertForward);
		addProperty(axis);
		addProperty(wheelsGroup);
	}

	void VehicleEditor::onChangeAxis(Property* prop, std::string val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle axis");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				comp->setAxis(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				comp->setAxis(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* component = (Vehicle*)(*it);

			//"XYZ", "XZY", "ZXY", "ZYX", "YXZ", "YZX"
			glm::vec3 axis = glm::vec3(0.0);
			if (val == "XYZ") axis = glm::vec3(0, 1, 2);
			if (val == "XZY") axis = glm::vec3(0, 2, 1);
			if (val == "ZXY") axis = glm::vec3(2, 0, 1);
			if (val == "ZYX") axis = glm::vec3(2, 1, 0);
			if (val == "YXZ") axis = glm::vec3(1, 0, 2);
			if (val == "YZX") axis = glm::vec3(1, 2, 0);

			undoData->vec3Data[0][component] = component->getAxis();
			undoData->vec3Data[1][component] = axis;

			component->setAxis(axis);
		}
	}

	void VehicleEditor::onChangeInvertForward(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle invert forward");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				comp->setInvertForward(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				comp->setInvertForward(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* component = (Vehicle*)(*it);

			undoData->boolData[0][component] = component->getInvertForward();
			undoData->boolData[1][component] = val;

			component->setInvertForward(val);
		}
	}

	void VehicleEditor::onDropConnectedBody(TreeNode* prop, TreeNode* from)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		GameObject* node = Engine::getSingleton()->getGameObject(from->name);
		((PropGameObject*)prop)->setValue(node);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel object");
		undoData->stringData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->stringData[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_connectedObjectGuid = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->stringData[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_connectedObjectGuid = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->stringData[0][comp] = inf.m_connectedObjectGuid;
			undoData->stringData[1][comp] = from->name;

			inf.m_connectedObjectGuid = from->name;
		}
	}

	void VehicleEditor::onClickAutoConfigure(Property* prop)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Vehicle wheel auto configure");
		undoData->vec3Data.resize(1);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_connectionPoint = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				comp->autoConfigureAnchors(idx);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->vec3Data[0][comp] = inf.m_connectionPoint;

			comp->autoConfigureAnchors(wheelIndex);
		}

		updateEditor();
	}

	void VehicleEditor::onChangeRadius(TreeNode* prop, float value)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel radius");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_radius = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_radius = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;
			
			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->floatData[0][comp] = inf.m_radius;
			undoData->floatData[1][comp] = value;

			inf.m_radius = value;
		}
	}

	void VehicleEditor::onChangeWidth(TreeNode* prop, float value)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel width");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_width = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_width = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->floatData[0][comp] = inf.m_width;
			undoData->floatData[1][comp] = value;

			inf.m_width = value;
		}
	}

	void VehicleEditor::onChangeSuspensionStiffness(TreeNode* prop, float value)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel suspension stiffness");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_suspensionStiffness = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_suspensionStiffness = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->floatData[0][comp] = inf.m_suspensionStiffness;
			undoData->floatData[1][comp] = value;

			inf.m_suspensionStiffness = value;
		}
	}

	void VehicleEditor::onChangeSuspensionDamping(TreeNode* prop, float value)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel suspension damping");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_suspensionDamping = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_suspensionDamping = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->floatData[0][comp] = inf.m_suspensionDamping;
			undoData->floatData[1][comp] = value;

			inf.m_suspensionDamping = value;
		}
	}

	void VehicleEditor::onChangeSuspensionCompression(TreeNode* prop, float value)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel suspension compression");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_suspensionCompression = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_suspensionCompression = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->floatData[0][comp] = inf.m_suspensionCompression;
			undoData->floatData[1][comp] = value;

			inf.m_suspensionCompression = value;
		}
	}

	void VehicleEditor::onChangeSuspensionRestLength(TreeNode* prop, float value)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel suspension rest length");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_suspensionRestLength = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_suspensionRestLength = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->floatData[0][comp] = inf.m_suspensionRestLength;
			undoData->floatData[1][comp] = value;

			inf.m_suspensionRestLength = value;
		}
	}

	void VehicleEditor::onChangeFriction(TreeNode* prop, float value)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel friction");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_friction = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_friction = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->floatData[0][comp] = inf.m_friction;
			undoData->floatData[1][comp] = value;

			inf.m_friction = value;
		}
	}

	void VehicleEditor::onChangeRollInfluence(TreeNode* prop, float value)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel roll influence");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_rollInfluence = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_rollInfluence = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->floatData[0][comp] = inf.m_rollInfluence;
			undoData->floatData[1][comp] = value;

			inf.m_rollInfluence = value;
		}
	}

	void VehicleEditor::onChangeDirection(TreeNode* prop, glm::vec3 value)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel direction");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_direction = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_direction = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->vec3Data[0][comp] = inf.m_direction;
			undoData->vec3Data[1][comp] = value;

			inf.m_direction = value;
		}
	}

	void VehicleEditor::onChangeAxle(TreeNode* prop, glm::vec3 value)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel axle");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_axle = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_axle = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->vec3Data[0][comp] = inf.m_axle;
			undoData->vec3Data[1][comp] = value;

			inf.m_axle = value;
		}
	}

	void VehicleEditor::onChangeConnectionPoint(TreeNode* prop, glm::vec3 value)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel connection point");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_connectionPoint = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_connectionPoint = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->vec3Data[0][comp] = inf.m_connectionPoint;
			undoData->vec3Data[1][comp] = value;

			inf.m_connectionPoint = value;
		}
	}

	void VehicleEditor::onChangeIsFrontWheel(TreeNode* prop, bool value)
	{
		void* data = prop->parent->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		//Undo
		UndoData* undoData = Undo::addUndo("Change vehicle wheel is front wheel");
		undoData->boolData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = wheelIndex;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->boolData[0])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_isFrontWheel = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->boolData[1])
			{
				Vehicle* comp = (Vehicle*)d.first;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(idx);

				inf.m_isFrontWheel = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo> & wheels = comp->getWheels();
			Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

			undoData->boolData[0][comp] = inf.m_isFrontWheel;
			undoData->boolData[1][comp] = value;

			inf.m_isFrontWheel = value;
		}
	}

	void VehicleEditor::onClickAddWheel(TreeNode* prop)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Add vehicle wheel");
		undoData->objectData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Vehicle* comp = (Vehicle*)d;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				wheels.erase(wheels.begin() + wheels.size() - 1);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Vehicle* comp = (Vehicle*)d;
				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				if (wheels.size() == 0)
				{
					wheels.push_back(Vehicle::WheelInfo());
				}
				else
				{
					wheels.push_back(wheels[wheels.size() - 1]);
					wheels[wheels.size() - 1].m_connectedObjectGuid = "";
					wheels[wheels.size() - 1].m_connectedObjectRef = nullptr;
				}
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Vehicle* comp = (Vehicle*)*it;

			std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
			if (wheels.size() == 0)
			{
				wheels.push_back(Vehicle::WheelInfo());
			}
			else
			{
				wheels.push_back(wheels[wheels.size() - 1]);
				wheels[wheels.size() - 1].m_connectedObjectGuid = "";
				wheels[wheels.size() - 1].m_connectedObjectRef = nullptr;
			}

			undoData->objectData[0].push_back(comp);
		}

		updateEditor();
	}

	void VehicleEditor::onClickWheelPopup(TreeNode* prop, int index)
	{
		void* data = prop->getUserData();
		int wheelIndex = *static_cast<int*>(data);

		if (index == 0)
		{
			//Undo
			UndoData* undoData = Undo::addUndo("Remove vehicle wheel");
			undoData->objectData.resize(1);
			undoData->stringData.resize(1);
			undoData->boolData.resize(1);
			undoData->vec3Data.resize(3);
			undoData->floatData.resize(8);
			undoData->intData.resize(1);

			undoData->intData[0][nullptr] = wheelIndex;

			undoData->undoAction = [=](UndoData* data)
			{
				int idx = data->intData[0][nullptr];

				for (auto& d : data->objectData[0])
				{
					Vehicle* comp = (Vehicle*)d;
					std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
					wheels.insert(wheels.begin() + idx, Vehicle::WheelInfo());
					Vehicle::WheelInfo& inf = wheels.at(idx);

					inf.m_connectedObjectGuid = data->stringData[0][comp];
					inf.m_isFrontWheel = data->boolData[0][comp];
					inf.m_axle = data->vec3Data[0][comp];
					inf.m_connectionPoint = data->vec3Data[1][comp];
					inf.m_direction = data->vec3Data[2][comp];
					inf.m_friction = data->floatData[0][comp];
					inf.m_radius = data->floatData[1][comp];
					inf.m_rollInfluence = data->floatData[2][comp];
					inf.m_suspensionCompression = data->floatData[3][comp];
					inf.m_suspensionDamping = data->floatData[4][comp];
					inf.m_suspensionRestLength = data->floatData[5][comp];
					inf.m_suspensionStiffness = data->floatData[6][comp];
					inf.m_width = data->floatData[7][comp];
				}

				MainWindow::getInspectorWindow()->updateCurrentEditor();
			};

			undoData->redoAction = [=](UndoData* data)
			{
				int idx = data->intData[0][nullptr];

				for (auto& d : data->objectData[0])
				{
					Vehicle* comp = (Vehicle*)d;
					std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
					wheels.erase(wheels.begin() + idx);
				}

				MainWindow::getInspectorWindow()->updateCurrentEditor();
			};
			//

			for (auto it = components.begin(); it != components.end(); ++it)
			{
				Vehicle* comp = (Vehicle*)*it;

				std::vector<Vehicle::WheelInfo>& wheels = comp->getWheels();
				Vehicle::WheelInfo& inf = wheels.at(wheelIndex);

				undoData->stringData[0][comp] = inf.m_connectedObjectGuid;
				undoData->boolData[0][comp] = inf.m_isFrontWheel;
				undoData->vec3Data[0][comp] = inf.m_axle;
				undoData->vec3Data[1][comp] = inf.m_connectionPoint;
				undoData->vec3Data[2][comp] = inf.m_direction;
				undoData->floatData[0][comp] = inf.m_friction;
				undoData->floatData[1][comp] = inf.m_radius;
				undoData->floatData[2][comp] = inf.m_rollInfluence;
				undoData->floatData[3][comp] = inf.m_suspensionCompression;
				undoData->floatData[4][comp] = inf.m_suspensionDamping;
				undoData->floatData[5][comp] = inf.m_suspensionRestLength;
				undoData->floatData[6][comp] = inf.m_suspensionStiffness;
				undoData->floatData[7][comp] = inf.m_width;

				wheels.erase(wheels.begin() + wheelIndex);
			}
		}

		updateEditor();
	}
}