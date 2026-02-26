#include "ComponentEditor.h"

#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"
#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	ComponentEditor::ComponentEditor()
	{
		getTreeView()->setOnEndUpdateCallback([=]() { onTreeViewEndUpdate(); });
	}

	void ComponentEditor::updateEditor()
	{
		updateState = true;
	}

	void ComponentEditor::onTreeViewEndUpdate()
	{
		if (updateState)
		{
			float sp = MainWindow::getSingleton()->getInspectorWindow()->getScrollPos();
			MainWindow::getSingleton()->getInspectorWindow()->saveCollapsedProperties();

			auto props = getTreeView()->getRootNode()->children;

			for (auto it = props.begin(); it != props.end(); ++it)
				removeProperty((Property*)*it);

			init(components);

			MainWindow::getSingleton()->getInspectorWindow()->loadCollapsedProperties();
			MainWindow::getSingleton()->getInspectorWindow()->setScrollPos(sp);

			updateState = false;
		}
	}
}