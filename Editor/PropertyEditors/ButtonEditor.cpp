#include "ButtonEditor.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Components/Button.h"
#include "../Engine/Assets/Texture.h"

#include "../Windows/MainWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/InspectorWindow.h"

#include "PropSampler.h"
#include "PropColorPicker.h"
#include "PropBool.h"
#include "PropComboBox.h"

#include "../Classes/Undo.h"

namespace GX
{
	ButtonEditor::ButtonEditor()
	{
		setEditorName("ButtonEditor");
	}

	void ButtonEditor::init(std::vector<Component*> comps)
	{
		UIElementEditor::init(comps);

		Button* button = (Button*)comps[0];

		PropComboBox* imageType = new PropComboBox(this, "Image type", { "Simple", "Sliced" });
		imageType->setCurrentItem(static_cast<int>(button->getImageType()));
		imageType->setOnChangeCallback([=](Property* prop, int val) { onChangeImageType(prop, val); });

		PropSampler* texNormal = new PropSampler(this, "Texture", button->getTextureNormal());
		texNormal->setSupportedFormats(Engine::getImageFileFormats());
		texNormal->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onChangeNormalTexture(prop, from); });
		texNormal->setOnClearCallback([=](Property* prop) { onClearNormalTexture(prop); });
		texNormal->setOnClickCallback([=](Property* prop)
			{
				Texture* tex = button->getTextureNormal();
				if (tex != nullptr)
					MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(tex->getName());
			}
		);

		PropSampler* texHover = new PropSampler(this, "Hover texture", button->getTextureHover());
		texHover->setSupportedFormats(Engine::getImageFileFormats());
		texHover->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onChangeHoverTexture(prop, from); });
		texHover->setOnClearCallback([=](Property* prop) { onClearHoverTexture(prop); });
		texHover->setOnClickCallback([=](Property* prop)
			{
				Texture* tex = button->getTextureHover();
				if (tex != nullptr)
					MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(tex->getName());
			}
		);

		PropSampler* texActive = new PropSampler(this, "Active texture", button->getTextureActive());
		texActive->setSupportedFormats(Engine::getImageFileFormats());
		texActive->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onChangeActiveTexture(prop, from); });
		texActive->setOnClearCallback([=](Property* prop) { onClearActiveTexture(prop); });
		texActive->setOnClickCallback([=](Property* prop)
			{
				Texture* tex = button->getTextureActive();
				if (tex != nullptr)
					MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(tex->getName());
			}
		);

		PropSampler* texDisabled = new PropSampler(this, "Disabled texture", button->getTextureDisabled());
		texDisabled->setSupportedFormats(Engine::getImageFileFormats());
		texDisabled->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onChangeDisabledTexture(prop, from); });
		texDisabled->setOnClearCallback([=](Property* prop) { onClearDisabledTexture(prop); });
		texDisabled->setOnClickCallback([=](Property* prop)
			{
				Texture* tex = button->getTextureDisabled();
				if (tex != nullptr)
					MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(tex->getName());
			}
		);

		PropColorPicker* colorHover = new PropColorPicker(this, "Hover color", button->getColorHover());
		colorHover->setOnChangeCallback([=](Property* prop, Color col) { onChangeHoverColor(prop, col); });

		PropColorPicker* colorActive = new PropColorPicker(this, "Active color", button->getColorActive());
		colorActive->setOnChangeCallback([=](Property* prop, Color col) { onChangeActiveColor(prop, col); });

		PropColorPicker* colorDisabled = new PropColorPicker(this, "Disabled color", button->getColorDisabled());
		colorDisabled->setOnChangeCallback([=](Property* prop, Color col) { onChangeDisabledColor(prop, col); });

		PropBool* interactable = new PropBool(this, "Interactable", button->getInteractable());
		interactable->setOnChangeCallback([=](Property* prop, bool val) { onChangeInteractable(prop, val); });

		addProperty(colorHover);
		addProperty(colorActive);
		addProperty(colorDisabled);
		addProperty(imageType);
		addProperty(texNormal);
		addProperty(texHover);
		addProperty(texActive);
		addProperty(texDisabled);
		addProperty(interactable);
	}

	void ButtonEditor::onChangeNormalTexture(TreeNode* prop, TreeNode* from)
	{
		std::string path = from->getPath();
		Texture* tex = Texture::load(Engine::getSingleton()->getAssetsPath(), path, false, Texture::CompressionMethod::None);
		((PropSampler*)prop)->setValue(tex);

		//Undo
		UndoData* undoData = Undo::addUndo("Change button texture");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Button* element = (Button*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureNormal(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[2])
			{
				Button* element = (Button*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[3][element], false, Texture::CompressionMethod::None);
				element->setTextureNormal(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			Texture* texture = button->getTextureNormal();
			undoData->stringData[0][button] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][button] = texture != nullptr ? texture->getName() : "";
			undoData->stringData[2][button] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][button] = path;

			button->setTextureNormal(tex);
		}
	}

	void ButtonEditor::onClearNormalTexture(TreeNode* prop)
	{
		((PropSampler*)prop)->setValue(nullptr);

		//Undo
		UndoData* undoData = Undo::addUndo("Clear button texture");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Button* element = (Button*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureNormal(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Button* element = (Button*)d.first;
				element->setTextureNormal(nullptr);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			Texture* texture = button->getTextureNormal();
			undoData->stringData[0][button] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][button] = texture != nullptr ? texture->getName() : "";

			button->setTextureNormal(nullptr);
		}
	}

	void ButtonEditor::onChangeHoverTexture(TreeNode* prop, TreeNode* from)
	{
		std::string path = from->getPath();
		Texture* tex = Texture::load(Engine::getSingleton()->getAssetsPath(), path, false, Texture::CompressionMethod::None);
		((PropSampler*)prop)->setValue(tex);

		//Undo
		UndoData* undoData = Undo::addUndo("Change button hovered texture");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Button* element = (Button*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureHover(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[2])
			{
				Button* element = (Button*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[3][element], false, Texture::CompressionMethod::None);
				element->setTextureHover(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			Texture* texture = button->getTextureHover();
			undoData->stringData[0][button] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][button] = texture != nullptr ? texture->getName() : "";
			undoData->stringData[2][button] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][button] = path;

			button->setTextureHover(tex);
		}
	}

	void ButtonEditor::onClearHoverTexture(TreeNode* prop)
	{
		((PropSampler*)prop)->setValue(nullptr);

		//Undo
		UndoData* undoData = Undo::addUndo("Clear button hovered texture");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Button* element = (Button*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureHover(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Button* element = (Button*)d.first;
				element->setTextureHover(nullptr);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			Texture* texture = button->getTextureHover();
			undoData->stringData[0][button] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][button] = texture != nullptr ? texture->getName() : "";

			button->setTextureHover(nullptr);
		}
	}

	void ButtonEditor::onChangeActiveTexture(TreeNode* prop, TreeNode* from)
	{
		std::string path = from->getPath();
		Texture* tex = Texture::load(Engine::getSingleton()->getAssetsPath(), path, false, Texture::CompressionMethod::None);
		((PropSampler*)prop)->setValue(tex);

		//Undo
		UndoData* undoData = Undo::addUndo("Change button active texture");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Button* element = (Button*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureActive(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[2])
			{
				Button* element = (Button*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[3][element], false, Texture::CompressionMethod::None);
				element->setTextureActive(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			Texture* texture = button->getTextureActive();
			undoData->stringData[0][button] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][button] = texture != nullptr ? texture->getName() : "";
			undoData->stringData[2][button] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][button] = path;

			button->setTextureActive(tex);
		}
	}

	void ButtonEditor::onClearActiveTexture(TreeNode* prop)
	{
		((PropSampler*)prop)->setValue(nullptr);

		//Undo
		UndoData* undoData = Undo::addUndo("Clear button active texture");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Button* element = (Button*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureActive(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Button* element = (Button*)d.first;
				element->setTextureActive(nullptr);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			Texture* texture = button->getTextureActive();
			undoData->stringData[0][button] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][button] = texture != nullptr ? texture->getName() : "";

			button->setTextureActive(nullptr);
		}
	}

	void ButtonEditor::onChangeDisabledTexture(TreeNode* prop, TreeNode* from)
	{
		std::string path = from->getPath();
		Texture* tex = Texture::load(Engine::getSingleton()->getAssetsPath(), path, false, Texture::CompressionMethod::None);
		((PropSampler*)prop)->setValue(tex);

		//Undo
		UndoData* undoData = Undo::addUndo("Change button disabled texture");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Button* element = (Button*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureDisabled(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[2])
			{
				Button* element = (Button*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[3][element], false, Texture::CompressionMethod::None);
				element->setTextureDisabled(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			Texture* texture = button->getTextureDisabled();
			undoData->stringData[0][button] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][button] = texture != nullptr ? texture->getName() : "";
			undoData->stringData[2][button] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][button] = path;

			button->setTextureDisabled(tex);
		}
	}

	void ButtonEditor::onClearDisabledTexture(TreeNode* prop)
	{
		((PropSampler*)prop)->setValue(nullptr);

		//Undo
		UndoData* undoData = Undo::addUndo("Clear button disabled texture");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Button* element = (Button*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureDisabled(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Button* element = (Button*)d.first;
				element->setTextureDisabled(nullptr);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			Texture* texture = button->getTextureDisabled();
			undoData->stringData[0][button] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][button] = texture != nullptr ? texture->getName() : "";

			button->setTextureDisabled(nullptr);
		}
	}

	void ButtonEditor::onChangeHoverColor(Property* prop, Color value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change button hovered color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[0])
			{
				Button* element = (Button*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorHover(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[1])
			{
				Button* element = (Button*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorHover(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			Color col = button->getColorHover();
			undoData->vec4Data[0][button] = glm::vec4(col[0], col[1], col[2], col[3]);
			undoData->vec4Data[1][button] = glm::vec4(value[0], value[1], value[2], value[3]);

			button->setColorHover(value);
		}
	}

	void ButtonEditor::onChangeActiveColor(Property* prop, Color value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change button active color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[0])
			{
				Button* element = (Button*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorActive(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[1])
			{
				Button* element = (Button*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorActive(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			Color col = button->getColorActive();
			undoData->vec4Data[0][button] = glm::vec4(col[0], col[1], col[2], col[3]);
			undoData->vec4Data[1][button] = glm::vec4(value[0], value[1], value[2], value[3]);

			button->setColorActive(value);
		}
	}

	void ButtonEditor::onChangeDisabledColor(Property* prop, Color value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change button disabled color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[0])
			{
				Button* element = (Button*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorDisabled(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[1])
			{
				Button* element = (Button*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorDisabled(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			Color col = button->getColorDisabled();
			undoData->vec4Data[0][button] = glm::vec4(col[0], col[1], col[2], col[3]);
			undoData->vec4Data[1][button] = glm::vec4(value[0], value[1], value[2], value[3]);

			button->setColorDisabled(value);
		}
	}

	void ButtonEditor::onChangeInteractable(Property* prop, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change button interactable");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Button* element = (Button*)d.first;
				element->setInteractable(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Button* element = (Button*)d.first;
				element->setInteractable(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			undoData->boolData[0][button] = button->getInteractable();
			undoData->boolData[1][button] = value;

			button->setInteractable(value);
		}
	}

	void ButtonEditor::onChangeImageType(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change button image type");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Button* element = (Button*)d.first;
				element->setImageType(static_cast<Button::ImageType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Button* element = (Button*)d.first;
				element->setImageType(static_cast<Button::ImageType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Button* button = (Button*)*it;

			undoData->intData[0][button] = static_cast<int>(button->getImageType());
			undoData->intData[1][button] = value;

			button->setImageType(static_cast<Button::ImageType>(value));
		}
	}
}