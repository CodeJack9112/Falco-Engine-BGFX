#include "CubemapEditor.h"

#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"

#include "../Windows/MainWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/InspectorWindow.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/Cubemap.h"

#include "../Engine/Renderer/RenderTexture.h"
#include "../Engine/Components/Camera.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Renderer/Primitives.h"
#include "../Engine/Math/Mathf.h"
#include "../Engine/Core/InputManager.h"
#include "../Engine/glm/gtc/type_ptr.hpp"

#include "PropSampler.h"

#include "../Classes/Undo.h"

namespace GX
{
	glm::vec2 CubemapEditor::prevSize = glm::vec2(0.0f);

	CubemapEditor::CubemapEditor()
	{
		setEditorName("CubemapEditor");
	}

	CubemapEditor::~CubemapEditor()
	{
		if (rt != nullptr)
			delete rt;
	}

	void CubemapEditor::init(Cubemap* cube)
	{
		cubemap = cube;

		for (int i = 0; i < 6; ++i)
		{
			std::string texName = "Left (-X)";
			if (i == 1) texName = "Right (+X)";
			if (i == 2) texName = "Bottom (-Y)";
			if (i == 3) texName = "Top (+Y)";
			if (i == 4) texName = "Back (-Z)";
			if (i == 5) texName = "Front (+Z)";

			PropSampler* texture = new PropSampler(this, texName, cubemap->getTexture(i));
			texture->setSupportedFormats(Engine::getImageFileFormats());
			texture->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropTexture(prop, from, i); });
			texture->setOnClearCallback([=](Property* prop) { onClearTexture(prop, i); });
			texture->setOnClickCallback([=](Property* prop)
				{
					Texture* tex = cubemap->getTexture(i);
					if (tex != nullptr)
						MainWindow::getAssetsWindow()->focusOnFile(tex->getName());
				}
			);

			addProperty(texture);
		}

		meshTransform = glm::identity<glm::mat4x4>();
		meshTransform = glm::scale(meshTransform, glm::vec3(100, 100, 100));
		resetFrameBuffers(prevSize.x > 0 ? prevSize.x : previewHeight, prevSize.y > 0 ? prevSize.y : previewHeight);
		setPreviewFunction([=]() { onUpdatePreview(); });
	}

	void CubemapEditor::update()
	{
		PropertyEditor::update();
	}

	void CubemapEditor::updateEditor()
	{
		if (rt != nullptr)
			resetFrameBuffers(rt->getWidth(), rt->getHeight());

		MainWindow::addOnEndUpdateCallback([=]() {
			float sp = MainWindow::getSingleton()->getInspectorWindow()->getScrollPos();
			MainWindow::getSingleton()->getInspectorWindow()->saveCollapsedProperties();

			auto props = getTreeView()->getRootNode()->children;

			for (auto it = props.begin(); it != props.end(); ++it)
				removeProperty((Property*)*it);

			init(cubemap);

			MainWindow::getSingleton()->getInspectorWindow()->loadCollapsedProperties();
			MainWindow::getSingleton()->getInspectorWindow()->setScrollPos(sp);
		});
	}

	void CubemapEditor::onDropTexture(TreeNode* prop, TreeNode* from, int index)
	{
		std::string path = from->getPath();

		Texture* texture = Texture::load(Engine::getSingleton()->getAssetsPath(), path, true, Texture::CompressionMethod::Default);
		((PropSampler*)prop)->setValue(texture);

		//Undo
		UndoData* undoData = Undo::addUndo("Change cubemap texture");
		undoData->stringData.resize(6);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			Cubemap* cub = Cubemap::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (cub != nullptr)
			{
				Texture* tex = Texture::load(data->stringData[2][nullptr], data->stringData[3][nullptr]);
				cub->setTexture(data->intData[0][nullptr], tex);
				cub->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Cubemap* cub = Cubemap::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (cub != nullptr)
			{
				Texture* tex = Texture::load(data->stringData[4][nullptr], data->stringData[5][nullptr]);
				cub->setTexture(data->intData[0][nullptr], tex);
				cub->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		Texture* tex = cubemap->getTexture(index);

		undoData->stringData[0][nullptr] = cubemap->getLocation();
		undoData->stringData[1][nullptr] = cubemap->getName();
		undoData->stringData[2][nullptr] = tex != nullptr ? tex->getLocation() : "";
		undoData->stringData[3][nullptr] = tex != nullptr ? tex->getName() : "";
		undoData->stringData[4][nullptr] = texture != nullptr ? texture->getLocation() : "";
		undoData->stringData[5][nullptr] = texture != nullptr ? texture->getName() : "";
		undoData->intData[0][nullptr] = index;
		//

		cubemap->setTexture(index, texture);
		cubemap->save();
	}

	void CubemapEditor::onClearTexture(Property* prop, int index)
	{
		((PropSampler*)prop)->setValue(nullptr);

		//Undo
		UndoData* undoData = Undo::addUndo("Clear cubemap texture");
		undoData->stringData.resize(4);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			Cubemap* cub = Cubemap::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (cub != nullptr)
			{
				Texture* tex = Texture::load(data->stringData[2][nullptr], data->stringData[3][nullptr]);
				cub->setTexture(data->intData[0][nullptr], tex);
				cub->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Cubemap* cub = Cubemap::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (cub != nullptr)
			{
				cub->setTexture(data->intData[0][nullptr], nullptr);
				cub->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		Texture* tex = cubemap->getTexture(index);

		undoData->stringData[0][nullptr] = cubemap->getLocation();
		undoData->stringData[1][nullptr] = cubemap->getName();
		undoData->stringData[2][nullptr] = tex != nullptr ? tex->getLocation() : "";
		undoData->stringData[3][nullptr] = tex != nullptr ? tex->getName() : "";
		undoData->intData[0][nullptr] = index;
		//

		cubemap->setTexture(index, nullptr);
		cubemap->save();
	}

	void CubemapEditor::resetFrameBuffers(float w, float h)
	{
		if (rt == nullptr)
			rt = new RenderTexture(w, h);
		else
			rt->reset(w, h);
	}

	void CubemapEditor::onUpdatePreview()
	{
		if (rt != nullptr)
		{
			ImVec2 sz = ImGui::GetContentRegionAvail();
			glm::vec2 size = glm::vec2(sz.x, previewHeight);

			if (prevSize != size)
			{
				if (sz.x > 0 && previewHeight > 0)
					resetFrameBuffers(sz.x, previewHeight);

				prevSize = size;
			}

			glm::vec2 mousePos = glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
			glm::vec2 dxy = Mathf::fDeg2Rad * mousePos;

			if (isPreviewHovered())
			{
				if (InputManager::getSingleton()->getMouseButtonDown(0))
					mousePressed = true;
			}

			if (InputManager::getSingleton()->getMouseButtonUp(0))
				mousePressed = false;

			//Render meshes
			float _aspect = (float)rt->getWidth() / (float)rt->getHeight();

			if (mousePressed)
			{
				glm::vec2 dxy = Mathf::fDeg2Rad * (mousePos - prevMousePos);

				rotX = glm::angleAxis(dxy.x, glm::vec3(0, 1, 0));
				rotY = glm::angleAxis(dxy.y, glm::vec3(0, 0, 1));

				meshTransform = glm::mat4_cast(rotX * rotY) * meshTransform;
			}

			prevMousePos = mousePos;

			glm::mat4x4 view = Camera::makeViewMatrix(camPos, camPos + glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
			glm::mat4x4 proj = Camera::makeProjectionMatrix(60.0f, _aspect, 0.01f, 1000.0f);

			int viewId = Renderer::getSingleton()->getNumViewsUsed() + 3;

			//View
			bgfx::setViewClear(viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0.0f);
			bgfx::setViewRect(viewId, 0, 0, rt->getWidth(), rt->getHeight());
			bgfx::setViewTransform(viewId, glm::value_ptr(view), glm::value_ptr(proj));
			bgfx::setViewFrameBuffer(viewId, rt->getFrameBufferHandle());

			//bgfx::touch(viewId);

			bgfx::setTexture(0, Renderer::getSingleton()->getAlbedoMapUniform(), cubemap->getHandle());

			Primitives::skybox(meshTransform, viewId, BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_Z, Renderer::getSingleton()->getSkyboxProgram(), nullptr);

			float w = rt->getWidth();
			float h = rt->getHeight();

			ImGui::Image((void*)rt->getColorTextureHandle().idx, ImVec2(w, h), ImVec2(0, 1), ImVec2(1, 0));
		}
	}
}