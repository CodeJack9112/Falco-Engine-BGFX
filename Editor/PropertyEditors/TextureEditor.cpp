#include "TextureEditor.h"

#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"

#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

#include "../Engine/Assets/Texture.h"

#include "PropBool.h"
#include "PropComboBox.h"
#include "PropVector2.h"

#include <imgui_internal.h>

namespace GX
{
	TextureEditor::TextureEditor()
	{
		setEditorName("TextureEditor");
	}

	TextureEditor::~TextureEditor()
	{
	}

	void TextureEditor::init(std::vector<Texture*> values)
	{
		if (values.size() == 0)
			return;

		textures = values;
		values.clear();

		Texture* texture = textures[0];

		PropBool* genMips = new PropBool(this, "Generate Mip Maps", texture->getGenMipMaps());
		genMips->setOnChangeCallback([=](Property* prop, bool val) { onChangeGenMipMaps(val); });

		addProperty(genMips);

		PropComboBox* wrapMode = new PropComboBox(this, "Wrap mode", { "Repeat", "Clamp" });
		wrapMode->setCurrentItem(static_cast<int>(texture->getWrapMode()));
		wrapMode->setOnChangeCallback([=](Property* prop, int val) { onChangeWrapMode(val); });

		addProperty(wrapMode);

		PropComboBox* filterMode = new PropComboBox(this, "Filter mode", { "Point", "Linear", "Anisotropic" });
		filterMode->setCurrentItem(static_cast<int>(texture->getFilterMode()));
		filterMode->setOnChangeCallback([=](Property* prop, int val) { onChangeFilterMode(val); });

		addProperty(filterMode);

		PropComboBox* compressionMethod = new PropComboBox(this, "Compression", { "Default", "None", "BC7" });
		compressionMethod->setCurrentItem(static_cast<int>(texture->getCompressionMethod()));
		compressionMethod->setOnChangeCallback([=](Property* prop, int val) { onChangeCompressionMethod(val); });

		addProperty(compressionMethod);

		if (texture->getCompressionMethod() != Texture::CompressionMethod::None)
		{
			PropComboBox* compressionQuality = new PropComboBox(this, "Compression quality", { "Default", "Low", "Normal", "High", "Very High" });
			compressionQuality->setCurrentItem(texture->getCompressionQuality());
			compressionQuality->setOnChangeCallback([=](Property* prop, int val) { onChangeCompressionQuality(val); });

			addProperty(compressionQuality);
		}

		std::vector<std::string> resolutions = { "Default", "8192", "4096", "2048", "1024", "512", "256", "128", "64", "32", "16" };
		PropComboBox* maxResolution = new PropComboBox(this, "Max resolution", resolutions);
		maxResolution->setCurrentItem(std::to_string(texture->getMaxResolution()));
		maxResolution->setOnChangeCallback([=](Property* prop, int val) { onChangeMaxResolution(val); });

		addProperty(maxResolution);

		Rect border = texture->getBorder();
		PropVector2* borderLT = new PropVector2(this, "Border (LT)", glm::vec2(border.left, border.top));
		borderLT->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeBorderLT(val); });
		borderLT->setIsDraggable(false);
		borderLT->setValueType(PropVector2::ValueType::Integer);

		addProperty(borderLT);

		PropVector2* borderRB = new PropVector2(this, "Border (RB)", glm::vec2(border.right, border.bottom));
		borderRB->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeBorderRB(val); });
		borderRB->setIsDraggable(false);
		borderRB->setValueType(PropVector2::ValueType::Integer);

		addProperty(borderRB);

		setPreviewFunction([=]() { onUpdatePreview(); });
	}

	void TextureEditor::update()
	{
		PropertyEditor::update();
	}

	void TextureEditor::updateEditor()
	{
		MainWindow::addOnEndUpdateCallback([=]()
			{
				float sp = MainWindow::getSingleton()->getInspectorWindow()->getScrollPos();
				MainWindow::getSingleton()->getInspectorWindow()->saveCollapsedProperties();

				auto props = getTreeView()->getRootNode()->children;

				for (auto it = props.begin(); it != props.end(); ++it)
					removeProperty((Property*)*it);

				init(textures);

				MainWindow::getSingleton()->getInspectorWindow()->loadCollapsedProperties();
				MainWindow::getSingleton()->getInspectorWindow()->setScrollPos(sp);
			}
		);
	}

	void TextureEditor::onChangeGenMipMaps(bool value)
	{
		for (auto& t : textures)
			t->setGenMipMaps(value);
	}

	void TextureEditor::onChangeFilterMode(int value)
	{
		for (auto& t : textures)
			t->setFilterMode(static_cast<Texture::FilterMode>(value));
	}

	void TextureEditor::onChangeWrapMode(int value)
	{
		for (auto& t : textures)
			t->setWrapMode(static_cast<Texture::WrapMode>(value));
	}

	void TextureEditor::onChangeCompressionMethod(int value)
	{
		for (auto& t : textures)
			t->setCompressionMethod(static_cast<Texture::CompressionMethod>(value));
	}

	void TextureEditor::onChangeCompressionQuality(int value)
	{
		for (auto& t : textures)
			t->setCompressionQuality(value);
	}

	void TextureEditor::onChangeMaxResolution(int value)
	{
		std::vector<std::string> resolutions = { "0", "8192", "4096", "2048", "1024", "512", "256", "128", "64", "32", "16" };
		int resolution = std::atoi(resolutions[value].c_str());

		for (auto& t : textures)
			t->setMaxResolution(resolution);
	}

	void TextureEditor::onChangeBorderLT(glm::vec2 value)
	{
		for (auto& t : textures)
		{
			Rect border = t->getBorder();
			t->setBorder(Rect(value.x, value.y, border.right, border.bottom));
			t->reload();
		}
	}

	void TextureEditor::onChangeBorderRB(glm::vec2 value)
	{
		for (auto& t : textures)
		{
			Rect border = t->getBorder();
			t->setBorder(Rect(border.left, border.top, value.x, value.y));
			t->reload();
		}
	}

	void TextureEditor::onUpdatePreview()
	{
		if (textures.size() > 0)
		{
			ImVec2 sz = ImGui::GetContentRegionAvail();
			sz.x -= 1;
			float x = sz.x;
			
			float w = textures[0]->getOriginalWidth();
			float h = textures[0]->getOriginalHeight();
			float aspect = h / w;
			float aspectW = w / h;

			sz.y = sz.x * aspect;
			
			if (previewHeight < sz.y)
			{
				sz.y = previewHeight - 5;
				sz.x = sz.y * aspectW;
			}

			ImGui::SetCursorPosX(x / 2.0f - sz.x / 2.0f);
			ImGui::SetCursorPosY(previewHeight / 2.0f - sz.y / 2.0f);

			ImGui::Image((void*)textures[0]->getHandle().idx, sz, ImVec2(0, 1), ImVec2(1, 0));

			float dw = sz.x / w;
			float dh = sz.y / h;

			Rect border = textures[0]->getBorder();

			border.left *= dw;
			border.right *= dw;
			border.top *= dh;
			border.bottom *= dh;

			if (border.left > 0 || border.top > 0 || border.right > 0 || border.bottom > 0)
			{
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				ImGuiWindow* win = ImGui::GetCurrentWindow();
				ImGui::SetCursorPosX(x / 2.0f - sz.x / 2.0f);
				ImGui::SetCursorPosY(previewHeight / 2.0f - sz.y / 2.0f);
				ImVec2 sp = win->DC.CursorPos;

				drawList->AddLine(ImVec2(sp.x + border.left, sp.y), ImVec2(sp.x + border.left, sp.y + sz.y), 0xff00ff00);
				drawList->AddLine(ImVec2(sp.x + sz.x - border.right, sp.y), ImVec2(sp.x + sz.x - border.right, sp.y + sz.y), 0xff00ff00);
				drawList->AddLine(ImVec2(sp.x, sp.y + border.top), ImVec2(sp.x + sz.x, sp.y + border.top), 0xff00ff00);
				drawList->AddLine(ImVec2(sp.x, sp.y + sz.y - border.bottom), ImVec2(sp.x + sz.x, sp.y + sz.y - border.bottom), 0xff00ff00);
			}

			float y = previewHeight - 20 + ImGui::GetScrollY();
			ImVec2 cursor = ImGui::GetCursorPos();
			std::string str = std::to_string(textures[0]->getWidth()) + "x" + std::to_string(textures[0]->getHeight());
			ImVec2 textSize = ImGui::CalcTextSize(str.c_str());
			ImGui::SetCursorPos(ImVec2(x / 2 - textSize.x / 2, y));
			ImGui::Text(str.c_str());
		}
	}
}