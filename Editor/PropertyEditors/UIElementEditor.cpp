#include "UIElementEditor.h"

#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/UIElement.h"
#include "../Engine/Components/Transform.h"

#include "PropVector2.h"
#include "PropColorPicker.h"
#include "PropComboBox.h"
#include "PropBool.h"
#include "PropButton.h"

#include "ObjectEditor.h"

#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"
#include "../Classes/Undo.h"

namespace GX
{
	UIElementEditor::UIElementEditor()
	{
		setEditorName("UIElementEditor");
	}

	void UIElementEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		UIElement* element = (UIElement*)comps[0];

		size = new PropVector2(this, "Size", element->getSize());
		size->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeSize(prop, val); });

		PropButton* fitSize = new PropButton(this, "", "Fit parent");
		fitSize->setOnClickCallback([=](Property* prop) { onFitSize(prop); });

		anchor = new PropVector2(this, "Anchor", element->getAnchor());
		anchor->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeAnchor(prop, val); });

		PropComboBox* hAlign = new PropComboBox(this, "Horizontal alignment", { "Left", "Center", "Right", "Stretch" });
		hAlign->setCurrentItem(static_cast<int>(element->getHorizontalAlignment()));
		hAlign->setOnChangeCallback([=](Property* prop, int val) { onChangeHAlign(prop, val); });

		PropComboBox* vAlign = new PropComboBox(this, "Vertical alignment", { "Top", "Middle", "Bottom", "Stretch" });
		vAlign->setCurrentItem(static_cast<int>(element->getVerticalAlignment()));
		vAlign->setOnChangeCallback([=](Property* prop, int val) { onChangeVAlign(prop, val); });

		PropBool* raycastTarget = new PropBool(this, "Raycast target", element->getRaycastTarget());
		raycastTarget->setOnChangeCallback([=](Property* prop, bool val) { onChangeRaycastTarget(prop, val); });

		color = new PropColorPicker(this, "Color", element->getColor());
		color->setOnChangeCallback([=](Property* prop, Color val) { onChangeColor(prop, val); });

		addProperty(size);
		addProperty(fitSize);
		addProperty(anchor);
		addProperty(hAlign);
		addProperty(vAlign);
		addProperty(raycastTarget);
		addProperty(color);
	}

	void UIElementEditor::updateValues()
	{
		if (components.size() > 0)
		{
			UIElement* element = (UIElement*)components[0];
			size->setValue(element->getSize());
			anchor->setValue(element->getAnchor());
		}
	}

	void UIElementEditor::onChangeColor(Property* prop, Color value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change UI element color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[0])
			{
				UIElement* element = (UIElement*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColor(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[1])
			{
				UIElement* element = (UIElement*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColor(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			UIElement* element = (UIElement*)*it;

			Color col = element->getColor();
			undoData->vec4Data[0][element] = glm::vec4(col[0], col[1], col[2], col[3]);
			undoData->vec4Data[1][element] = glm::vec4(value[0], value[1], value[2], value[3]);

			element->setColor(value);
		}
	}

	void UIElementEditor::onChangeSize(Property* prop, glm::vec2 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change UI element size");
		undoData->vec2Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[0])
			{
				UIElement* element = (UIElement*)d.first;
				element->setSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[1])
			{
				UIElement* element = (UIElement*)d.first;
				element->setSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			UIElement* element = (UIElement*)*it;

			undoData->vec2Data[0][element] = element->getSize();
			undoData->vec2Data[1][element] = value;

			element->setSize(value);
		}

		((ObjectEditor*)getParentEditor())->updateTransform();
	}

	void UIElementEditor::onChangeAnchor(Property* prop, glm::vec2 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change UI element anchor");
		undoData->vec2Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[0])
			{
				UIElement* element = (UIElement*)d.first;
				element->setAnchor(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[1])
			{
				UIElement* element = (UIElement*)d.first;
				element->setAnchor(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			UIElement* element = (UIElement*)*it;

			undoData->vec2Data[0][element] = element->getAnchor();
			undoData->vec2Data[1][element] = value;

			element->setAnchor(value);
		}

		((ObjectEditor*)getParentEditor())->updateTransform();
	}

	void UIElementEditor::onChangeHAlign(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change UI element horizontal alignment");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				UIElement* element = (UIElement*)d.first;
				element->setHorizontalAlignment(static_cast<CanvasHorizontalAlignment>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				UIElement* element = (UIElement*)d.first;
				element->setHorizontalAlignment(static_cast<CanvasHorizontalAlignment>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			UIElement* element = (UIElement*)*it;

			undoData->intData[0][element] = static_cast<int>(element->getHorizontalAlignment());
			undoData->intData[1][element] = value;

			element->setHorizontalAlignment(static_cast<CanvasHorizontalAlignment>(value));
		}

		((ObjectEditor*)getParentEditor())->updateTransform();
	}

	void UIElementEditor::onChangeVAlign(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change UI element vertical alignment");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				UIElement* element = (UIElement*)d.first;
				element->setVerticalAlignment(static_cast<CanvasVerticalAlignment>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				UIElement* element = (UIElement*)d.first;
				element->setVerticalAlignment(static_cast<CanvasVerticalAlignment>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			UIElement* element = (UIElement*)*it;

			undoData->intData[0][element] = static_cast<int>(element->getVerticalAlignment());
			undoData->intData[1][element] = value;

			element->setVerticalAlignment(static_cast<CanvasVerticalAlignment>(value));
		}

		((ObjectEditor*)getParentEditor())->updateTransform();
	}

	void UIElementEditor::onChangeRaycastTarget(Property* prop, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change UI element raycast target");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				UIElement* element = (UIElement*)d.first;
				element->setRaycastTarget(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				UIElement* element = (UIElement*)d.first;
				element->setRaycastTarget(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			UIElement* element = (UIElement*)*it;

			undoData->boolData[0][element] = element->getRaycastTarget();
			undoData->boolData[1][element] = value;

			element->setRaycastTarget(value);
		}
	}

	void UIElementEditor::fitSize(UIElement* element)
	{
		UIElement* parent = nullptr;

		Transform* parentTransform = element->getTransform()->getParent();
		while (parentTransform != nullptr)
		{
			GameObject* obj = parentTransform->getGameObject();

			for (auto& it : obj->getComponents())
			{
				if (it->isUiComponent())
				{
					parent = (UIElement*)it;
					break;
				}
			}

			if (parent != nullptr)
				break;

			parentTransform = parentTransform->getParent();
		}

		if (parent != nullptr)
		{
			element->setSize(parent->getSize());

			glm::vec2 adjPos = glm::vec2(parent->getTransform()->getPosition()) + (element->getSize() * element->getAnchor()) - (parent->getSize() * parent->getAnchor());
			element->getTransform()->setPosition(glm::vec3(adjPos, 0.0f));
		}
	}

	void UIElementEditor::onFitSize(Property* prop)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Fit UI element size");
		undoData->vec2Data.resize(1);
		undoData->vec3Data.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[0])
			{
				UIElement* element = (UIElement*)d.first;
				element->setSize(d.second);
				element->getTransform()->setPosition(data->vec3Data[0][element]);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[0])
			{
				UIElement* element = (UIElement*)d.first;
				fitSize(element);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			UIElement* element = (UIElement*)*it;

			undoData->vec2Data[0][element] = element->getSize();
			undoData->vec3Data[0][element] = element->getTransform()->getPosition();

			fitSize(element);
		}

		MainWindow::getInspectorWindow()->updateCurrentEditor();
	}
}