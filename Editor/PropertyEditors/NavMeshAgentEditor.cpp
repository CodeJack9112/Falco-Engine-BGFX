#include "NavMeshAgentEditor.h"

#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/Component.h"
#include "../Engine/Components/NavMeshAgent.h"

#include "PropBool.h"
#include "PropFloat.h"

#include "../Classes/Undo.h"

#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	NavMeshAgentEditor::NavMeshAgentEditor()
	{
		setEditorName("NavMeshAgentEditor");
	}

	NavMeshAgentEditor::~NavMeshAgentEditor()
	{
	}

	void NavMeshAgentEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Component* component = comps[0];
		NavMeshAgent* comp = (NavMeshAgent*)component;

		PropFloat* radius = new PropFloat(this, "Radius", comp->getRadius());
		radius->setOnChangeCallback([=](Property* prop, float val) { onChangeRadius(prop, val); });

		PropFloat* height = new PropFloat(this, "Height", comp->getHeight());
		height->setOnChangeCallback([=](Property* prop, float val) { onChangeHeight(prop, val); });

		PropFloat* speed = new PropFloat(this, "Speed", comp->getSpeed());
		speed->setOnChangeCallback([=](Property* prop, float val) { onChangeSpeed(prop, val); });

		PropFloat* rotationSpeed = new PropFloat(this, "Rotation speed", comp->getRotationSpeed());
		rotationSpeed->setOnChangeCallback([=](Property* prop, float val) { onChangeRotationSpeed(prop, val); });

		PropFloat* acceleration = new PropFloat(this, "Acceleration", comp->getAcceleration());
		acceleration->setOnChangeCallback([=](Property* prop, float val) { onChangeAcceleration(prop, val); });

		addProperty(radius);
		addProperty(height);
		addProperty(speed);
		addProperty(rotationSpeed);
		addProperty(acceleration);
	}

	void NavMeshAgentEditor::onChangeRadius(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navmesh agent radius");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				NavMeshAgent* comp = (NavMeshAgent*)d.first;
				comp->setRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				NavMeshAgent* comp = (NavMeshAgent*)d.first;
				comp->setRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			NavMeshAgent* comp = (NavMeshAgent*)*it;
			
			undoData->floatData[0][comp] = comp->getRadius();
			undoData->floatData[1][comp] = val;

			comp->setRadius(val);
		}
	}

	void NavMeshAgentEditor::onChangeHeight(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navmesh agent height");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				NavMeshAgent* comp = (NavMeshAgent*)d.first;
				comp->setHeight(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				NavMeshAgent* comp = (NavMeshAgent*)d.first;
				comp->setHeight(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			NavMeshAgent* comp = (NavMeshAgent*)*it;

			undoData->floatData[0][comp] = comp->getHeight();
			undoData->floatData[1][comp] = val;

			comp->setHeight(val);
		}
	}

	void NavMeshAgentEditor::onChangeSpeed(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navmesh agent speed");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				NavMeshAgent* comp = (NavMeshAgent*)d.first;
				comp->setSpeed(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				NavMeshAgent* comp = (NavMeshAgent*)d.first;
				comp->setSpeed(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			NavMeshAgent* comp = (NavMeshAgent*)*it;

			undoData->floatData[0][comp] = comp->getSpeed();
			undoData->floatData[1][comp] = val;

			comp->setSpeed(val);
		}
	}

	void NavMeshAgentEditor::onChangeAcceleration(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navmesh agent acceleration");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				NavMeshAgent* comp = (NavMeshAgent*)d.first;
				comp->setAcceleration(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				NavMeshAgent* comp = (NavMeshAgent*)d.first;
				comp->setAcceleration(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			NavMeshAgent* comp = (NavMeshAgent*)*it;

			undoData->floatData[0][comp] = comp->getAcceleration();
			undoData->floatData[1][comp] = val;

			comp->setAcceleration(val);
		}
	}

	void NavMeshAgentEditor::onChangeRotationSpeed(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navmesh agent rotation speed");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				NavMeshAgent* comp = (NavMeshAgent*)d.first;
				comp->setRotationSpeed(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				NavMeshAgent* comp = (NavMeshAgent*)d.first;
				comp->setRotationSpeed(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			NavMeshAgent* comp = (NavMeshAgent*)*it;

			undoData->floatData[0][comp] = comp->getRotationSpeed();
			undoData->floatData[1][comp] = val;

			comp->setRotationSpeed(val);
		}
	}
}