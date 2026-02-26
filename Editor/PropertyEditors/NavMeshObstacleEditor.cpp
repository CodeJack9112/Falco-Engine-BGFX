#include "NavMeshObstacleEditor.h"

#include "../Engine/Components/Component.h"
#include "../Engine/Components/NavMeshObstacle.h"
#include "../Engine/Core/Engine.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/NavigationManager.h"

#include "PropBool.h"
#include "PropVector3.h"

#include "../Classes/Undo.h"

#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	NavMeshObstacleEditor::NavMeshObstacleEditor()
	{
		setEditorName("NavMeshObstacleEditor");
	}

	NavMeshObstacleEditor::~NavMeshObstacleEditor()
	{
	}

	void NavMeshObstacleEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Component* component = comps[0];
		NavMeshObstacle* comp = (NavMeshObstacle*)component;

		PropVector3* size = new PropVector3(this, "Size", comp->getSize());
		size->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeSize(prop, val); });

		PropVector3* offset = new PropVector3(this, "Offset", comp->getOffset());
		offset->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeOffset(prop, val); });

		addProperty(size);
		addProperty(offset);
	}

	void NavMeshObstacleEditor::onChangeSize(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navmesh obstacle size");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				NavMeshObstacle* comp = (NavMeshObstacle*)d.first;
				comp->setSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				NavMeshObstacle* comp = (NavMeshObstacle*)d.first;
				comp->setSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			NavMeshObstacle* comp = (NavMeshObstacle*)*it;

			undoData->vec3Data[0][comp] = comp->getSize();
			undoData->vec3Data[1][comp] = val;

			comp->setSize(val);
		}

		NavigationManager::getSingleton()->setNavMeshIsDirty();
	}

	void NavMeshObstacleEditor::onChangeOffset(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navmesh obstacle offset");
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				NavMeshObstacle* comp = (NavMeshObstacle*)d.first;
				comp->setOffset(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				NavMeshObstacle* comp = (NavMeshObstacle*)d.first;
				comp->setOffset(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			NavMeshObstacle* comp = (NavMeshObstacle*)*it;

			undoData->vec3Data[0][comp] = comp->getOffset();
			undoData->vec3Data[1][comp] = val;

			comp->setOffset(val);
		}

		NavigationManager::getSingleton()->setNavMeshIsDirty();
	}
}