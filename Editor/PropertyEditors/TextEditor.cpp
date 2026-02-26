#include "TextEditor.h"

#include "../Engine/Components/Text.h"

#include "PropString.h"
#include "PropInt.h"
#include "PropFloat.h"
#include "PropComboBox.h"
#include "PropButton.h"
#include "PropCustom.h"

#include "../Engine/Classes/Helpers.h"
#include "../Engine/Core/Engine.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Assets/Font.h"
#include "../Engine/Assets/Texture.h"

#include "../Windows/MainWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/InspectorWindow.h"
#include "../Classes/Undo.h"

#include <dear-imgui/imgui_internal.h>

namespace GX
{
	TextEditor::TextEditor()
	{
		setEditorName("TextEditor");
	}

	void TextEditor::init(std::vector<Component*> comps)
	{
		UIElementEditor::init(comps);

		Text* uitext = (Text*)comps[0];

		PropString* text = new PropString(this, "Text", uitext->getText());
		text->setMultiline(true);
		text->setOnChangeCallback([=](Property* prop, std::string val) { onChangeText(prop, val); });

		std::string fontVal = "None";
		if (uitext->getFont() != nullptr && uitext->getFont()->isLoaded())
			fontVal = IO::GetFileNameWithExt(uitext->getFont()->getName());

		PropButton* font = new PropButton(this, "Font", fontVal);
		font->setSupportedFormats({ "ttf", "otf" });
		font->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropFont(prop, from); });
		font->setOnClickCallback([=](Property* prop)
			{
				Font* fnt = uitext->getFont();
				if (fnt != nullptr)
					MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(fnt->getName());
			}
		);
		font->setOnClearCallback([=](Property* prop) { onClearFont(prop); });
		font->setImage(MainWindow::loadEditorIcon("Assets/font.png"));

		PropFloat* fontSize = new PropFloat(this, "Font size", uitext->getFontSize());
		fontSize->setOnChangeCallback([=](Property* prop, float val) { onChangeFontSize(prop, val); });

		PropInt* fontResolution = new PropInt(this, "Font resolution", uitext->getFontResolution());
		fontResolution->setOnChangeCallback([=](Property* prop, int val) { onChangeFontResolution(prop, val); });
		fontResolution->setMinValue(1);
		fontResolution->setMaxValue(128);

		PropFloat* lineSpacing = new PropFloat(this, "Line spacing", uitext->getLineSpacing());
		lineSpacing->setOnChangeCallback([=](Property* prop, float val) { onChangeLineSpacing(prop, val); });

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

				if (uitext->getHorizontalTextAlignment() == Text::TextHorizontalAlignment::Left)
				{
					col1 = ImVec4(1, 1, 1, 1);
					col2 = ImVec4(1, 1, 1, 0.2f);
					col3 = ImVec4(1, 1, 1, 0.2f);
				}
				else if (uitext->getHorizontalTextAlignment() == Text::TextHorizontalAlignment::Center)
				{
					col1 = ImVec4(1, 1, 1, 0.2f);
					col2 = ImVec4(1, 1, 1, 1);
					col3 = ImVec4(1, 1, 1, 0.2f);
				}
				else if (uitext->getHorizontalTextAlignment() == Text::TextHorizontalAlignment::Right)
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

				if (uitext->getVerticalTextAlignment() == Text::TextVerticalAlignment::Top)
				{
					col1 = ImVec4(1, 1, 1, 1);
					col2 = ImVec4(1, 1, 1, 0.2f);
					col3 = ImVec4(1, 1, 1, 0.2f);
				}
				else if (uitext->getVerticalTextAlignment() == Text::TextVerticalAlignment::Middle)
				{
					col1 = ImVec4(1, 1, 1, 0.2f);
					col2 = ImVec4(1, 1, 1, 1);
					col3 = ImVec4(1, 1, 1, 0.2f);
				}
				else if (uitext->getVerticalTextAlignment() == Text::TextVerticalAlignment::Bottom)
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
		wordWrap->setCurrentItem(static_cast<int>(uitext->getWordWrap()));
		wordWrap->setOnChangeCallback([=](Property* prop, int val) { onChangeWordWrap(prop, val); });

		addProperty(font);
		addProperty(fontSize);
		addProperty(fontResolution);
		addProperty(lineSpacing);
		addProperty(hAlignButtons);
		addProperty(vAlignButtons);
		addProperty(wordWrap);
		addProperty(text);
	}

	void TextEditor::onChangeText(Property* prop, std::string value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text label");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Text* element = (Text*)d.first;
				element->setText(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[1])
			{
				Text* element = (Text*)d.first;
				element->setText(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Text* text = (Text*)*it;

			undoData->stringData[0][text] = text->getText();
			undoData->stringData[1][text] = value;

			text->setText(value);
		}
	}

	void TextEditor::onChangeFontSize(Property* prop, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text font size");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				Text* element = (Text*)d.first;
				element->setFontSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				Text* element = (Text*)d.first;
				element->setFontSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Text* text = (Text*)*it;

			undoData->floatData[0][text] = text->getFontSize();
			undoData->floatData[1][text] = value;

			text->setFontSize(value);
		}
	}

	void TextEditor::onChangeFontResolution(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text font resolution");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Text* element = (Text*)d.first;
				element->setFontResolution(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Text* element = (Text*)d.first;
				element->setFontResolution(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Text* text = (Text*)*it;

			undoData->intData[0][text] = text->getFontResolution();
			undoData->intData[1][text] = value;

			text->setFontResolution(value);
		}
	}

	void TextEditor::onChangeLineSpacing(Property* prop, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text line spacing");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				Text* element = (Text*)d.first;
				element->setLineSpacing(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				Text* element = (Text*)d.first;
				element->setLineSpacing(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Text* text = (Text*)*it;

			undoData->floatData[0][text] = text->getLineSpacing();
			undoData->floatData[1][text] = value;

			text->setLineSpacing(value);
		}
	}

	void TextEditor::onChangeHTextAlign(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text horizontal alignment");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Text* element = (Text*)d.first;
				element->setHorizontalTextAlignment(static_cast<Text::TextHorizontalAlignment>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Text* element = (Text*)d.first;
				element->setHorizontalTextAlignment(static_cast<Text::TextHorizontalAlignment>(value));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Text* text = (Text*)*it;

			undoData->intData[0][text] = static_cast<int>(text->getHorizontalTextAlignment());
			undoData->intData[1][text] = value;

			text->setHorizontalTextAlignment(static_cast<Text::TextHorizontalAlignment>(value));
		}
	}

	void TextEditor::onChangeVTextAlign(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text vertical alignment");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Text* element = (Text*)d.first;
				element->setVerticalTextAlignment(static_cast<Text::TextVerticalAlignment>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Text* element = (Text*)d.first;
				element->setVerticalTextAlignment(static_cast<Text::TextVerticalAlignment>(value));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Text* text = (Text*)*it;

			undoData->intData[0][text] = static_cast<int>(text->getVerticalTextAlignment());
			undoData->intData[1][text] = value;

			text->setVerticalTextAlignment(static_cast<Text::TextVerticalAlignment>(value));
		}
	}

	void TextEditor::onChangeWordWrap(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change text word wrap");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Text* element = (Text*)d.first;
				element->setWordWrap(static_cast<Text::TextWordWrap>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Text* element = (Text*)d.first;
				element->setWordWrap(static_cast<Text::TextWordWrap>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Text* text = (Text*)*it;

			undoData->intData[0][text] = static_cast<int>(text->getWordWrap());
			undoData->intData[1][text] = value;

			text->setWordWrap(static_cast<Text::TextWordWrap>(value));
		}
	}

	void TextEditor::onDropFont(TreeNode* node, TreeNode* from)
	{
		((PropButton*)node)->setValue(from->alias);
		std::string path = from->getPath();
		Font* font = Font::load(Engine::getSingleton()->getAssetsPath(), path);

		//Undo
		UndoData* undoData = Undo::addUndo("Change text font");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Text* element = (Text*)d.first;
				Font* fnt = Font::load(d.second, data->stringData[1][element]);
				element->setFont(fnt);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[2])
			{
				Text* element = (Text*)d.first;
				Font* fnt = Font::load(d.second, data->stringData[3][element]);
				element->setFont(fnt);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Text* text = (Text*)*it;

			Font* fnt = text->getFont();
			undoData->stringData[0][text] = fnt != nullptr ? fnt->getLocation() : "";
			undoData->stringData[1][text] = fnt != nullptr ? fnt->getName() : "";
			undoData->stringData[2][text] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][text] = path;

			text->setFont(font);
		}
	}

	void TextEditor::onClearFont(TreeNode* prop)
	{
		((PropButton*)prop)->setValue("None");

		//Undo
		UndoData* undoData = Undo::addUndo("Clear text font");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Text* element = (Text*)d.first;

				Font* fnt = Font::load(d.second, data->stringData[1][element]);
				element->setFont(fnt);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Text* element = (Text*)d.first;
				element->setFont(nullptr);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Text* text = (Text*)*it;

			Font* fnt = text->getFont();
			undoData->stringData[0][text] = fnt != nullptr ? fnt->getLocation() : "";
			undoData->stringData[1][text] = fnt != nullptr ? fnt->getName() : "";

			text->setFont(nullptr);
		}
	}
}