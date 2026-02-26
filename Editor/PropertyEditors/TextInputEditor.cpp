#include "TextInputEditor.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Components/TextInput.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/Font.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Classes/Helpers.h"

#include "../Windows/MainWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/InspectorWindow.h"

#include "PropSampler.h"
#include "PropColorPicker.h"
#include "PropBool.h"
#include "PropComboBox.h"
#include "PropString.h"
#include "PropFloat.h"
#include "PropInt.h"
#include "PropButton.h"
#include "PropCustom.h"
#include "PropVector4.h"

#include "../Classes/Undo.h"

namespace GX
{
	TextInputEditor::TextInputEditor()
	{
		setEditorName("TextInputEditor");
	}

	void TextInputEditor::init(std::vector<Component*> comps)
	{
		UIElementEditor::init(comps);

		TextInput* textInput = (TextInput*)comps[0];

		PropComboBox* imageType = new PropComboBox(this, "Image type", { "Simple", "Sliced" });
		imageType->setCurrentItem(static_cast<int>(textInput->getImageType()));
		imageType->setOnChangeCallback([=](Property* prop, int val) { onChangeImageType(prop, val); });

		PropSampler* texNormal = new PropSampler(this, "Texture", textInput->getTextureNormal());
		texNormal->setSupportedFormats(Engine::getImageFileFormats());
		texNormal->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onChangeNormalTexture(prop, from); });
		texNormal->setOnClearCallback([=](Property* prop) { onClearNormalTexture(prop); });
		texNormal->setOnClickCallback([=](Property* prop)
			{
				Texture* tex = textInput->getTextureNormal();
				if (tex != nullptr)
					MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(tex->getName());
			}
		);

		PropSampler* texHover = new PropSampler(this, "Hover texture", textInput->getTextureHover());
		texHover->setSupportedFormats(Engine::getImageFileFormats());
		texHover->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onChangeHoverTexture(prop, from); });
		texHover->setOnClearCallback([=](Property* prop) { onClearHoverTexture(prop); });
		texHover->setOnClickCallback([=](Property* prop)
			{
				Texture* tex = textInput->getTextureHover();
				if (tex != nullptr)
					MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(tex->getName());
			}
		);

		PropSampler* texActive = new PropSampler(this, "Active texture", textInput->getTextureActive());
		texActive->setSupportedFormats(Engine::getImageFileFormats());
		texActive->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onChangeActiveTexture(prop, from); });
		texActive->setOnClearCallback([=](Property* prop) { onClearActiveTexture(prop); });
		texActive->setOnClickCallback([=](Property* prop)
			{
				Texture* tex = textInput->getTextureActive();
				if (tex != nullptr)
					MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(tex->getName());
			}
		);

		PropSampler* texDisabled = new PropSampler(this, "Disabled texture", textInput->getTextureDisabled());
		texDisabled->setSupportedFormats(Engine::getImageFileFormats());
		texDisabled->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onChangeDisabledTexture(prop, from); });
		texDisabled->setOnClearCallback([=](Property* prop) { onClearDisabledTexture(prop); });
		texDisabled->setOnClickCallback([=](Property* prop)
			{
				Texture* tex = textInput->getTextureDisabled();
				if (tex != nullptr)
					MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(tex->getName());
			}
		);

		PropColorPicker* colorHover = new PropColorPicker(this, "Hover color", textInput->getColorHover());
		colorHover->setOnChangeCallback([=](Property* prop, Color col) { onChangeHoverColor(prop, col); });

		PropColorPicker* colorActive = new PropColorPicker(this, "Active color", textInput->getColorActive());
		colorActive->setOnChangeCallback([=](Property* prop, Color col) { onChangeActiveColor(prop, col); });

		PropColorPicker* colorDisabled = new PropColorPicker(this, "Disabled color", textInput->getColorDisabled());
		colorDisabled->setOnChangeCallback([=](Property* prop, Color col) { onChangeDisabledColor(prop, col); });

		PropColorPicker* colorText = new PropColorPicker(this, "Text color", textInput->getColorText());
		colorText->setOnChangeCallback([=](Property* prop, Color col) { onChangeTextColor(prop, col); });

		PropColorPicker* colorPlaceholder = new PropColorPicker(this, "Placeholder color", textInput->getColorPlaceholder());
		colorPlaceholder->setOnChangeCallback([=](Property* prop, Color col) { onChangePlaceholderColor(prop, col); });

		std::string fontVal = "None";
		if (textInput->getFont() != nullptr && textInput->getFont()->isLoaded())
			fontVal = IO::GetFileNameWithExt(textInput->getFont()->getName());

		PropButton* font = new PropButton(this, "Font", fontVal);
		font->setSupportedFormats({ "ttf", "otf" });
		font->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropFont(prop, from); });
		font->setOnClickCallback([=](Property* prop)
			{
				Font* fnt = textInput->getFont();
				if (fnt != nullptr)
					MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(fnt->getName());
			}
		);
		font->setOnClearCallback([=](Property* prop) { onClearFont(prop); });
		font->setImage(MainWindow::loadEditorIcon("Assets/font.png"));

		PropFloat* fontSize = new PropFloat(this, "Font size", textInput->getFontSize());
		fontSize->setOnChangeCallback([=](Property* prop, float value) { onChangeFontSize(prop, value); });

		PropInt* fontResolution = new PropInt(this, "Font resolution", textInput->getFontResolution());
		fontResolution->setOnChangeCallback([=](Property* prop, int val) { onChangeFontResolution(prop, val); });
		fontResolution->setMinValue(1);
		fontResolution->setMaxValue(128);

		PropFloat* lineSpacing = new PropFloat(this, "Line spacing", textInput->getLineSpacing());
		lineSpacing->setOnChangeCallback([=](Property* prop, float val) { onChangeLineSpacing(prop, val); });

		PropVector4* padding = new PropVector4(this, "Padding", textInput->getPadding());
		padding->setOnChangeCallback([=](Property* prop, glm::vec4 val) { onChangePadding(prop, val); });

		Texture* hAlignL = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/text_align_left.png", false, Texture::CompressionMethod::None, true);
		Texture* hAlignC = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/text_align_center.png", false, Texture::CompressionMethod::None, true);
		Texture* hAlignR = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/text_align_right.png", false, Texture::CompressionMethod::None, true);

		Texture* hAlignT = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/text_align_top.png", false, Texture::CompressionMethod::None, true);
		Texture* hAlignM = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/text_align_middle.png", false, Texture::CompressionMethod::None, true);
		Texture* hAlignB = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/text_align_bottom.png", false, Texture::CompressionMethod::None, true);

		PropCustom* hAlignButtons = new PropCustom(this, "Text horizontal alignment");
		hAlignButtons->setUseColumns(true);
		hAlignButtons->setOnUpdateCallback([=](Property* prop)
			{
				ImVec4 col1 = ImVec4(1, 1, 1, 1);
				ImVec4 col2 = ImVec4(1, 1, 1, 1);
				ImVec4 col3 = ImVec4(1, 1, 1, 1);

				if (textInput->getHorizontalTextAlignment() == TextInput::TextHorizontalAlignment::Left)
				{
					col1 = ImVec4(1, 1, 1, 1);
					col2 = ImVec4(1, 1, 1, 0.2f);
					col3 = ImVec4(1, 1, 1, 0.2f);
				}
				else if (textInput->getHorizontalTextAlignment() == TextInput::TextHorizontalAlignment::Center)
				{
					col1 = ImVec4(1, 1, 1, 0.2f);
					col2 = ImVec4(1, 1, 1, 1);
					col3 = ImVec4(1, 1, 1, 0.2f);
				}
				else if (textInput->getHorizontalTextAlignment() == TextInput::TextHorizontalAlignment::Right)
				{
					col1 = ImVec4(1, 1, 1, 0.2f);
					col2 = ImVec4(1, 1, 1, 0.2f);
					col3 = ImVec4(1, 1, 1, 1);
				}

				if (ImGui::ImageButton((void*)hAlignL->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col1))
					onChangeHTextAlign(prop, 0);

				MainWindow::HelpMarker("Left");

				ImGui::SameLine();

				if (ImGui::ImageButton((void*)hAlignC->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col2))
					onChangeHTextAlign(prop, 1);

				MainWindow::HelpMarker("Center");

				ImGui::SameLine();

				if (ImGui::ImageButton((void*)hAlignR->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col3))
					onChangeHTextAlign(prop, 2);

				MainWindow::HelpMarker("Right");
			}
		);

		PropCustom* vAlignButtons = new PropCustom(this, "Text vertical alignment");
		vAlignButtons->setUseColumns(true);
		vAlignButtons->setOnUpdateCallback([=](Property* prop)
			{
				ImVec4 col1 = ImVec4(1, 1, 1, 1);
				ImVec4 col2 = ImVec4(1, 1, 1, 1);
				ImVec4 col3 = ImVec4(1, 1, 1, 1);

				if (textInput->getVerticalTextAlignment() == TextInput::TextVerticalAlignment::Top)
				{
					col1 = ImVec4(1, 1, 1, 1);
					col2 = ImVec4(1, 1, 1, 0.2f);
					col3 = ImVec4(1, 1, 1, 0.2f);
				}
				else if (textInput->getVerticalTextAlignment() == TextInput::TextVerticalAlignment::Middle)
				{
					col1 = ImVec4(1, 1, 1, 0.2f);
					col2 = ImVec4(1, 1, 1, 1);
					col3 = ImVec4(1, 1, 1, 0.2f);
				}
				else if (textInput->getVerticalTextAlignment() == TextInput::TextVerticalAlignment::Bottom)
				{
					col1 = ImVec4(1, 1, 1, 0.2f);
					col2 = ImVec4(1, 1, 1, 0.2f);
					col3 = ImVec4(1, 1, 1, 1);
				}

				if (ImGui::ImageButton((void*)hAlignT->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col1))
					onChangeVTextAlign(prop, 0);

				MainWindow::HelpMarker("Top");

				ImGui::SameLine();

				if (ImGui::ImageButton((void*)hAlignM->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col2))
					onChangeVTextAlign(prop, 1);

				MainWindow::HelpMarker("Middle");

				ImGui::SameLine();

				if (ImGui::ImageButton((void*)hAlignB->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col3))
					onChangeVTextAlign(prop, 2);

				MainWindow::HelpMarker("Bottom");
			}
		);

		PropComboBox* wordWrap = new PropComboBox(this, "Word wrap", { "None", "Word wrap", "Break word" });
		wordWrap->setCurrentItem(static_cast<int>(textInput->getWordWrap()));
		wordWrap->setOnChangeCallback([=](Property* prop, int val) { onChangeWordWrap(prop, val); });

		PropBool* multiline = new PropBool(this, "Multiline", textInput->getMultiline());
		multiline->setOnChangeCallback([=](Property* prop, bool value) { onChangeMultiline(prop, value); });

		PropString* text = new PropString(this, "Text", textInput->getText());
		text->setMultiline(textInput->getMultiline());
		text->setOnChangeCallback([=](Property* prop, std::string value) { onChangeText(prop, value); });

		PropString* placeholder = new PropString(this, "Placeholder", textInput->getPlaceholder());
		placeholder->setMultiline(textInput->getMultiline());
		placeholder->setOnChangeCallback([=](Property* prop, std::string value) { onChangePlaceholder(prop, value); });

		PropBool* interactable = new PropBool(this, "Interactable", textInput->getInteractable());
		interactable->setOnChangeCallback([=](Property* prop, bool val) { onChangeInteractable(prop, val); });

		addProperty(colorHover);
		addProperty(colorActive);
		addProperty(colorDisabled);
		addProperty(imageType);
		addProperty(texNormal);
		addProperty(texHover);
		addProperty(texActive);
		addProperty(texDisabled);
		addProperty(colorText);
		addProperty(colorPlaceholder);
		addProperty(font);
		addProperty(fontSize);
		addProperty(fontResolution);
		addProperty(lineSpacing);
		addProperty(padding);
		addProperty(hAlignButtons);
		addProperty(vAlignButtons);
		addProperty(wordWrap);
		addProperty(multiline);
		addProperty(text);
		addProperty(placeholder);
		addProperty(interactable);
	}

	void TextInputEditor::onChangeNormalTexture(TreeNode* prop, TreeNode* from)
	{
		std::string path = from->getPath();
		Texture* tex = Texture::load(Engine::getSingleton()->getAssetsPath(), path, false, Texture::CompressionMethod::None);
		((PropSampler*)prop)->setValue(tex);

		//Undo
		UndoData* undoData = Undo::addUndo("Change text input texture");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureNormal(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[2])
			{
				TextInput* element = (TextInput*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[3][element], false, Texture::CompressionMethod::None);
				element->setTextureNormal(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Texture* texture = element->getTextureNormal();
			undoData->stringData[0][element] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][element] = texture != nullptr ? texture->getName() : "";
			undoData->stringData[2][element] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][element] = path;

			element->setTextureNormal(tex);
		}
	}

	void TextInputEditor::onClearNormalTexture(TreeNode* prop)
	{
		((PropSampler*)prop)->setValue(nullptr);

		//Undo
		UndoData* undoData = Undo::addUndo("Clear text input texture");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureNormal(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setTextureNormal(nullptr);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Texture* texture = element->getTextureNormal();
			undoData->stringData[0][element] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][element] = texture != nullptr ? texture->getName() : "";

			element->setTextureNormal(nullptr);
		}
	}

	void TextInputEditor::onChangeHoverTexture(TreeNode* prop, TreeNode* from)
	{
		std::string path = from->getPath();
		Texture* tex = Texture::load(Engine::getSingleton()->getAssetsPath(), path, false, Texture::CompressionMethod::None);
		((PropSampler*)prop)->setValue(tex);

		//Undo
		UndoData* undoData = Undo::addUndo("Change text input hovered texture");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureHover(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[2])
			{
				TextInput* element = (TextInput*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[3][element], false, Texture::CompressionMethod::None);
				element->setTextureHover(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Texture* texture = element->getTextureHover();
			undoData->stringData[0][element] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][element] = texture != nullptr ? texture->getName() : "";
			undoData->stringData[2][element] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][element] = path;

			element->setTextureHover(tex);
		}
	}

	void TextInputEditor::onClearHoverTexture(TreeNode* prop)
	{
		((PropSampler*)prop)->setValue(nullptr);

		//Undo
		UndoData* undoData = Undo::addUndo("Clear text input hovered texture");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureHover(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setTextureHover(nullptr);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Texture* texture = element->getTextureHover();
			undoData->stringData[0][element] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][element] = texture != nullptr ? texture->getName() : "";

			element->setTextureHover(nullptr);
		}
	}

	void TextInputEditor::onChangeActiveTexture(TreeNode* prop, TreeNode* from)
	{
		std::string path = from->getPath();
		Texture* tex = Texture::load(Engine::getSingleton()->getAssetsPath(), path, false, Texture::CompressionMethod::None);
		((PropSampler*)prop)->setValue(tex);

		//Undo
		UndoData* undoData = Undo::addUndo("Change text input active texture");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureActive(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[2])
			{
				TextInput* element = (TextInput*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[3][element], false, Texture::CompressionMethod::None);
				element->setTextureActive(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Texture* texture = element->getTextureActive();
			undoData->stringData[0][element] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][element] = texture != nullptr ? texture->getName() : "";
			undoData->stringData[2][element] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][element] = path;

			element->setTextureActive(tex);
		}
	}

	void TextInputEditor::onClearActiveTexture(TreeNode* prop)
	{
		((PropSampler*)prop)->setValue(nullptr);

		//Undo
		UndoData* undoData = Undo::addUndo("Clear text input active texture");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureActive(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setTextureActive(nullptr);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Texture* texture = element->getTextureActive();
			undoData->stringData[0][element] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][element] = texture != nullptr ? texture->getName() : "";

			element->setTextureActive(nullptr);
		}
	}

	void TextInputEditor::onChangeDisabledTexture(TreeNode* prop, TreeNode* from)
	{
		std::string path = from->getPath();
		Texture* tex = Texture::load(Engine::getSingleton()->getAssetsPath(), path, false, Texture::CompressionMethod::None);
		((PropSampler*)prop)->setValue(tex);

		//Undo
		UndoData* undoData = Undo::addUndo("Change text input disabled texture");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureDisabled(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[2])
			{
				TextInput* element = (TextInput*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[3][element], false, Texture::CompressionMethod::None);
				element->setTextureDisabled(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Texture* texture = element->getTextureDisabled();
			undoData->stringData[0][element] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][element] = texture != nullptr ? texture->getName() : "";
			undoData->stringData[2][element] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][element] = path;

			element->setTextureDisabled(tex);
		}
	}

	void TextInputEditor::onClearDisabledTexture(TreeNode* prop)
	{
		((PropSampler*)prop)->setValue(nullptr);

		//Undo
		UndoData* undoData = Undo::addUndo("Clear text input disabled texture");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTextureDisabled(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setTextureDisabled(nullptr);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Texture* texture = element->getTextureDisabled();
			undoData->stringData[0][element] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][element] = texture != nullptr ? texture->getName() : "";

			element->setTextureDisabled(nullptr);
		}
	}

	void TextInputEditor::onChangeHoverColor(Property* prop, Color value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text input hovered color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[0])
			{
				TextInput* element = (TextInput*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorHover(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[1])
			{
				TextInput* element = (TextInput*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorHover(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Color col = element->getColorHover();
			undoData->vec4Data[0][element] = glm::vec4(col[0], col[1], col[2], col[3]);
			undoData->vec4Data[1][element] = glm::vec4(value[0], value[1], value[2], value[3]);

			element->setColorHover(value);
		}
	}

	void TextInputEditor::onChangeActiveColor(Property* prop, Color value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text input active color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[0])
			{
				TextInput* element = (TextInput*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorActive(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[1])
			{
				TextInput* element = (TextInput*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorActive(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Color col = element->getColorActive();
			undoData->vec4Data[0][element] = glm::vec4(col[0], col[1], col[2], col[3]);
			undoData->vec4Data[1][element] = glm::vec4(value[0], value[1], value[2], value[3]);

			element->setColorActive(value);
		}
	}

	void TextInputEditor::onChangeDisabledColor(Property* prop, Color value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text input disabled color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[0])
			{
				TextInput* element = (TextInput*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorDisabled(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[1])
			{
				TextInput* element = (TextInput*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorDisabled(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Color col = element->getColorDisabled();
			undoData->vec4Data[0][element] = glm::vec4(col[0], col[1], col[2], col[3]);
			undoData->vec4Data[1][element] = glm::vec4(value[0], value[1], value[2], value[3]);

			element->setColorDisabled(value);
		}
	}

	void TextInputEditor::onChangeTextColor(Property* prop, Color value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text input text color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[0])
			{
				TextInput* element = (TextInput*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorText(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[1])
			{
				TextInput* element = (TextInput*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorText(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Color col = element->getColorText();
			undoData->vec4Data[0][element] = glm::vec4(col[0], col[1], col[2], col[3]);
			undoData->vec4Data[1][element] = glm::vec4(value[0], value[1], value[2], value[3]);

			element->setColorText(value);
		}
	}

	void TextInputEditor::onChangePlaceholderColor(Property* prop, Color value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text input placeholder color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[0])
			{
				TextInput* element = (TextInput*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorPlaceholder(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[1])
			{
				TextInput* element = (TextInput*)d.first;

				Color col = Color(d.second.x, d.second.y, d.second.z, d.second.w);
				element->setColorPlaceholder(col);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			Color col = element->getColorPlaceholder();
			undoData->vec4Data[0][element] = glm::vec4(col[0], col[1], col[2], col[3]);
			undoData->vec4Data[1][element] = glm::vec4(value[0], value[1], value[2], value[3]);

			element->setColorPlaceholder(value);
		}
	}

	void TextInputEditor::onChangeFontSize(Property* prop, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text input font size");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setFontSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				TextInput* element = (TextInput*)d.first;
				element->setFontSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* text = (TextInput*)*it;

			undoData->floatData[0][text] = text->getFontSize();
			undoData->floatData[1][text] = value;

			text->setFontSize(value);
		}
	}

	void TextInputEditor::onChangeFontResolution(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text font resolution");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setFontResolution(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				TextInput* element = (TextInput*)d.first;
				element->setFontResolution(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* text = (TextInput*)*it;

			undoData->intData[0][text] = text->getFontResolution();
			undoData->intData[1][text] = value;

			text->setFontResolution(value);
		}
	}

	void TextInputEditor::onChangeLineSpacing(Property* prop, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text line spacing");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setLineSpacing(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				TextInput* element = (TextInput*)d.first;
				element->setLineSpacing(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* text = (TextInput*)*it;

			undoData->floatData[0][text] = text->getLineSpacing();
			undoData->floatData[1][text] = value;

			text->setLineSpacing(value);
		}
	}

	void TextInputEditor::onChangePadding(Property* prop, glm::vec4 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text line spacing");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setPadding(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[1])
			{
				TextInput* element = (TextInput*)d.first;
				element->setPadding(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* text = (TextInput*)*it;

			undoData->vec4Data[0][text] = text->getPadding();
			undoData->vec4Data[1][text] = value;

			text->setPadding(value);
		}
	}

	void TextInputEditor::onChangeHTextAlign(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text horizontal alignment");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setHorizontalTextAlignment(static_cast<TextInput::TextHorizontalAlignment>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				TextInput* element = (TextInput*)d.first;
				element->setHorizontalTextAlignment(static_cast<TextInput::TextHorizontalAlignment>(value));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* text = (TextInput*)*it;

			undoData->intData[0][text] = static_cast<int>(text->getHorizontalTextAlignment());
			undoData->intData[1][text] = value;

			text->setHorizontalTextAlignment(static_cast<TextInput::TextHorizontalAlignment>(value));
		}
	}

	void TextInputEditor::onChangeVTextAlign(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text vertical alignment");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setVerticalTextAlignment(static_cast<TextInput::TextVerticalAlignment>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				TextInput* element = (TextInput*)d.first;
				element->setVerticalTextAlignment(static_cast<TextInput::TextVerticalAlignment>(value));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* text = (TextInput*)*it;

			undoData->intData[0][text] = static_cast<int>(text->getVerticalTextAlignment());
			undoData->intData[1][text] = value;

			text->setVerticalTextAlignment(static_cast<TextInput::TextVerticalAlignment>(value));
		}
	}

	void TextInputEditor::onChangeWordWrap(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text word wrap");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setWordWrap(static_cast<TextInput::TextWordWrap>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				TextInput* element = (TextInput*)d.first;
				element->setWordWrap(static_cast<TextInput::TextWordWrap>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* text = (TextInput*)*it;

			undoData->intData[0][text] = static_cast<int>(text->getWordWrap());
			undoData->intData[1][text] = value;

			text->setWordWrap(static_cast<TextInput::TextWordWrap>(value));
		}
	}

	void TextInputEditor::onChangeMultiline(Property* prop, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text input multiline");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setMultiline(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				TextInput* element = (TextInput*)d.first;
				element->setMultiline(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			undoData->boolData[0][element] = element->getMultiline();
			undoData->boolData[1][element] = value;

			element->setMultiline(value);
		}

		MainWindow::getInspectorWindow()->updateCurrentEditor();
	}

	void TextInputEditor::onChangeText(Property* prop, std::string value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text input text");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setText(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[1])
			{
				TextInput* element = (TextInput*)d.first;
				element->setText(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* text = (TextInput*)*it;

			undoData->stringData[0][text] = text->getText();
			undoData->stringData[1][text] = value;

			text->setText(value);
		}
	}

	void TextInputEditor::onChangePlaceholder(Property* prop, std::string value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text input placeholder");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setPlaceholder(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[1])
			{
				TextInput* element = (TextInput*)d.first;
				element->setPlaceholder(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* text = (TextInput*)*it;

			undoData->stringData[0][text] = text->getPlaceholder();
			undoData->stringData[1][text] = value;

			text->setPlaceholder(value);
		}
	}

	void TextInputEditor::onChangeInteractable(Property* prop, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text input interactable");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setInteractable(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				TextInput* element = (TextInput*)d.first;
				element->setInteractable(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			undoData->boolData[0][element] = element->getInteractable();
			undoData->boolData[1][element] = value;

			element->setInteractable(value);
		}
	}

	void TextInputEditor::onChangeImageType(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text input image type");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setImageType(static_cast<TextInput::ImageType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				TextInput* element = (TextInput*)d.first;
				element->setImageType(static_cast<TextInput::ImageType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* element = (TextInput*)*it;

			undoData->intData[0][element] = static_cast<int>(element->getImageType());
			undoData->intData[1][element] = value;

			element->setImageType(static_cast<TextInput::ImageType>(value));
		}
	}

	void TextInputEditor::onDropFont(TreeNode* node, TreeNode* from)
	{
		((PropButton*)node)->setValue(from->alias);
		std::string path = from->getPath();
		Font* font = Font::load(Engine::getSingleton()->getAssetsPath(), path);

		//Undo
		UndoData* undoData = Undo::addUndo("Change text input font");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;
				Font* fnt = Font::load(d.second, data->stringData[1][element]);
				element->setFont(fnt);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[2])
			{
				TextInput* element = (TextInput*)d.first;
				Font* fnt = Font::load(d.second, data->stringData[3][element]);
				element->setFont(fnt);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* text = (TextInput*)*it;

			Font* fnt = text->getFont();
			undoData->stringData[0][text] = fnt != nullptr ? fnt->getLocation() : "";
			undoData->stringData[1][text] = fnt != nullptr ? fnt->getName() : "";
			undoData->stringData[2][text] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][text] = path;

			text->setFont(font);
		}
	}

	void TextInputEditor::onClearFont(TreeNode* prop)
	{
		((PropButton*)prop)->setValue("None");

		//Undo
		UndoData* undoData = Undo::addUndo("Clear text input font");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;

				Font* fnt = Font::load(d.second, data->stringData[1][element]);
				element->setFont(fnt);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				TextInput* element = (TextInput*)d.first;
				element->setFont(nullptr);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			TextInput* text = (TextInput*)*it;

			Font* fnt = text->getFont();
			undoData->stringData[0][text] = fnt != nullptr ? fnt->getLocation() : "";
			undoData->stringData[1][text] = fnt != nullptr ? fnt->getName() : "";

			text->setFont(nullptr);
		}
	}
}