#include "RigidBodyEditor.h"

#include "../Engine/Components/RigidBody.h"
#include "../Engine/Core/GameObject.h"

#include "PropVector2.h"
#include "PropVector3.h"
#include "PropVector4.h"
#include "PropString.h"
#include "PropBool.h"
#include "PropBool3.h"
#include "PropInt.h"
#include "PropFloat.h"

#include "../Classes/Undo.h"
#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	RigidBodyEditor::RigidBodyEditor()
	{
		setEditorName("RigidBodyEditor");
	}

	RigidBodyEditor::~RigidBodyEditor()
	{
	}

	void RigidBodyEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Component* component = comps[0];
		RigidBody* body = (RigidBody*)component;

		PropBool* isStatic = new PropBool(this, "Static", body->getIsStatic());
		isStatic->setOnChangeCallback([=](Property* prop, bool val) { onChangeStatic(prop, val); });
		addProperty(isStatic);

		PropBool* kinematic = new PropBool(this, "Kinematic", body->getIsKinematic());
		kinematic->setOnChangeCallback([=](Property* prop, bool val) { onChangeKinematic(prop, val); });
		addProperty(kinematic);

		PropBool* useOwnGravity = new PropBool(this, "Use own gravity", body->getUseOwnGravity());
		useOwnGravity->setOnChangeCallback([=](Property* prop, bool val) { onChangeUseOwnGravity(prop, val); });
		addProperty(useOwnGravity);

		if (body->getUseOwnGravity())
		{
			PropVector3* gravity = new PropVector3(this, "Gravity", body->getGravity());
			gravity->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeGravity(prop, val); });
			addProperty(gravity);
		}

		PropFloat* friction = new PropFloat(this, "Friction", body->getFriction());
		friction->setOnChangeCallback([=](Property* prop, float val) { onChangeFriction(prop, val); });

		PropFloat* bounciness = new PropFloat(this, "Bounciness", body->getBounciness());
		bounciness->setOnChangeCallback([=](Property* prop, float val) { onChangeBounciness(prop, val); });

		PropFloat* linearDamping = new PropFloat(this, "Linear damping", body->getLinearDamping());
		linearDamping->setOnChangeCallback([=](Property* prop, float val) { onChangeLinearDamping(prop, val); });

		PropFloat* angularDamping = new PropFloat(this, "Angular damping", body->getAngularDamping());
		angularDamping->setOnChangeCallback([=](Property* prop, float val) { onChangeAngularDamping(prop, val); });

		PropFloat* mass = new PropFloat(this, "Mass", body->getMass());
		mass->setOnChangeCallback([=](Property* prop, float val) { onChangeMass(prop, val); });

		bool freezePos[3], freezeRot[3];
		freezePos[0] = body->getFreezePositionX();
		freezePos[1] = body->getFreezePositionY();
		freezePos[2] = body->getFreezePositionZ();

		freezeRot[0] = body->getFreezeRotationX();
		freezeRot[1] = body->getFreezeRotationY();
		freezeRot[2] = body->getFreezeRotationZ();

		PropBool3* freezePosition = new PropBool3(this, "Freeze Position", "X", "Y", "Z", freezePos);
		PropBool3* freezeRotation = new PropBool3(this, "Freeze Rotation", "X", "Y", "Z", freezeRot);

		freezePosition->setOnChangeCallback([=](Property* prop, bool val[3]) { onChangeFreezePosition(prop, val); });
		freezeRotation->setOnChangeCallback([=](Property* prop, bool val[3]) { onChangeFreezeRotation(prop, val); });

		addProperty(friction);
		addProperty(bounciness);
		addProperty(linearDamping);
		addProperty(angularDamping);
		addProperty(mass);
		addProperty(freezePosition);
		addProperty(freezeRotation);
	}

	void RigidBodyEditor::onChangeMass(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rigidbody mass");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setMass(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setMass(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			RigidBody* body = (RigidBody*)*it;

			undoData->floatData[0][body] = body->getMass();
			undoData->floatData[1][body] = val;

			body->setMass(val);
		}
	}

	void RigidBodyEditor::onChangeFreezePosition(Property* prop, bool val[3])
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rigidbody freeze position");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setFreezePositionX((int)d.second.x);
				comp->setFreezePositionY((int)d.second.y);
				comp->setFreezePositionZ((int)d.second.z);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setFreezePositionX((int)d.second.x);
				comp->setFreezePositionY((int)d.second.y);
				comp->setFreezePositionZ((int)d.second.z);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			RigidBody* body = (RigidBody*)*it;

			undoData->vec3Data[0][body] = glm::vec3(body->getFreezePositionX(), body->getFreezePositionY(), body->getFreezePositionZ());
			undoData->vec3Data[1][body] = glm::vec3(val[0], val[1], val[2]);

			body->setFreezePositionX(val[0]);
			body->setFreezePositionY(val[1]);
			body->setFreezePositionZ(val[2]);
		}
	}

	void RigidBodyEditor::onChangeFreezeRotation(Property* prop, bool val[3])
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rigidbody freeze rotation");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setFreezeRotationX((int)d.second.x);
				comp->setFreezeRotationY((int)d.second.y);
				comp->setFreezeRotationZ((int)d.second.z);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setFreezeRotationX((int)d.second.x);
				comp->setFreezeRotationY((int)d.second.y);
				comp->setFreezeRotationZ((int)d.second.z);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			RigidBody* body = (RigidBody*)*it;

			undoData->vec3Data[0][body] = glm::vec3(body->getFreezeRotationX(), body->getFreezeRotationY(), body->getFreezeRotationZ());
			undoData->vec3Data[1][body] = glm::vec3(val[0], val[1], val[2]);

			body->setFreezeRotationX(val[0]);
			body->setFreezeRotationY(val[1]);
			body->setFreezeRotationZ(val[2]);
		}
	}

	void RigidBodyEditor::onChangeKinematic(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rigidbody is kinematic");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setIsKinematic(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setIsKinematic(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			RigidBody* body = (RigidBody*)*it;

			undoData->boolData[0][body] = body->getIsKinematic();
			undoData->boolData[1][body] = val;

			body->setIsKinematic(val);
		}
	}

	void RigidBodyEditor::onChangeStatic(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rigidbody is static");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setIsStatic(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setIsStatic(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			RigidBody* body = (RigidBody*)*it;

			undoData->boolData[0][body] = body->getIsStatic();
			undoData->boolData[1][body] = val;

			body->setIsStatic(val);
		}
	}

	void RigidBodyEditor::onChangeUseOwnGravity(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rigidbody use own gravity");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setUseOwnGravity(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setUseOwnGravity(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			RigidBody* body = (RigidBody*)*it;

			undoData->boolData[0][body] = body->getUseOwnGravity();
			undoData->boolData[1][body] = val;

			body->setUseOwnGravity(val);
		}

		updateEditor();
	}

	void RigidBodyEditor::onChangeGravity(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rigidbody gravity");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setGravity(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setGravity(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			RigidBody* body = (RigidBody*)*it;

			undoData->vec3Data[0][body] = body->getGravity();
			undoData->vec3Data[1][body] = val;

			body->setGravity(val);
		}
	}

	void RigidBodyEditor::onChangeFriction(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rigidbody friction");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setFriction(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setFriction(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			RigidBody* body = (RigidBody*)*it;

			undoData->floatData[0][body] = body->getFriction();
			undoData->floatData[1][body] = val;

			body->setFriction(val);
		}
	}

	void RigidBodyEditor::onChangeBounciness(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rigidbody bounciness");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setBounciness(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setBounciness(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			RigidBody* body = (RigidBody*)*it;

			undoData->floatData[0][body] = body->getBounciness();
			undoData->floatData[1][body] = val;

			body->setBounciness(val);
		}
	}

	void RigidBodyEditor::onChangeLinearDamping(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rigidbody linear damping");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setLinearDamping(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setLinearDamping(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			RigidBody* body = (RigidBody*)*it;

			undoData->floatData[0][body] = body->getLinearDamping();
			undoData->floatData[1][body] = val;

			body->setLinearDamping(val);
		}
	}

	void RigidBodyEditor::onChangeAngularDamping(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rigidbody angular damping");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setAngularDamping(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				RigidBody* comp = (RigidBody*)d.first;
				comp->setAngularDamping(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			RigidBody* body = (RigidBody*)*it;

			undoData->floatData[0][body] = body->getAngularDamping();
			undoData->floatData[1][body] = val;

			body->setAngularDamping(val);
		}
	}
}