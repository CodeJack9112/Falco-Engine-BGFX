#include "Model3DEditor.h"

#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"

#include "../Windows/MainWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/InspectorWindow.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Assets/Shader.h"
#include "../Engine/Assets/Mesh.h"
#include "../Engine/Assets/Model3DLoader.h"
#include "../Engine/Renderer/RenderTexture.h"
#include "../Engine/Components/Camera.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Renderer/Primitives.h"
#include "../Engine/Renderer/BatchedGeometry.h"
#include "../Engine/Math/Mathf.h"
#include "../Engine/Math/AxisAlignedBox.h"
#include "../Engine/Core/InputManager.h"

#include "PropInt.h"
#include "PropFloat.h"
#include "PropBool.h"
#include "PropComboBox.h"
#include "PropButton.h"

#include "../Engine/Serialization/Meta/SModel3DMeta.h"

#include "../Engine/glm/gtc/type_ptr.hpp"

namespace GX
{
	glm::vec2 Model3DEditor::prevSize = glm::vec2(0.0f);
	int Model3DEditor::lodLevel = 0;

	Model3DEditor::Model3DEditor()
	{
		setEditorName("Model3DEditor");
	}

	Model3DEditor::~Model3DEditor()
	{
		if (rt != nullptr)
			delete rt;

		for (auto& meta : metas)
			delete meta;

		metas.clear();

		if (bgfx::isValid(gbuffer))
			bgfx::destroy(gbuffer);

		if (bgfx::isValid(lightBuffer))
			bgfx::destroy(lightBuffer);

		meshes.clear();
	}

	void Model3DEditor::init(std::vector<std::string> paths)
	{
		modelPaths = paths;
		std::string path = paths[0];
		std::string ext = IO::GetFileExtension(path);

		for (auto& p : paths)
		{
			if (IO::GetFileExtension(p) != "prefab")
			{
				SModel3DMeta meta;
				std::string metaFilePath = Engine::getSingleton()->getLibraryPath() + p + ".meta";
				std::string d = IO::GetFilePath(metaFilePath);
				if (!IO::DirExists(d))
					IO::CreateDir(d, true);

				if (!IO::FileExists(metaFilePath))
					meta.save(metaFilePath);
				
				meta.load(metaFilePath);

				SModel3DMeta* m = new SModel3DMeta();
				*m = meta;
				metas.push_back(m);
			}
		}

		if (ext != "prefab")
		{
			//Props
			SModel3DMeta meta;
			std::string metaFilePath = Engine::getSingleton()->getLibraryPath() + path + ".meta";
			if (IO::FileExists(metaFilePath))
				meta.load(metaFilePath);

			PropFloat* scale = new PropFloat(this, "Scale factor", meta.scale);
			scale->setOnChangeCallback([=](Property* prop, float val) { onChangeScale(prop, val); });
			addProperty(scale);

			PropBool* optimizeMeshes = new PropBool(this, "Optimize meshes", meta.optimizeMeshes);
			optimizeMeshes->setOnChangeCallback([=](Property* prop, bool val) { onChangeOptimizeMeshes(prop, val); });
			addProperty(optimizeMeshes);

			PropBool* generateLightmapUVs = new PropBool(this, "Generate lightmap UVs", meta.generateLightmapUVs);
			generateLightmapUVs->setOnChangeCallback([=](Property* prop, bool val) { onChangeGenerateLightmapUVs(prop, val); });
			addProperty(generateLightmapUVs);

			PropComboBox* calcNormals = new PropComboBox(this, "Normals", { "Import", "Calculate" });
			calcNormals->setCurrentItem(meta.calculateNormals);
			calcNormals->setOnChangeCallback([=](Property* prop, bool val) { onChangeCalculateNormals(prop, val); });
			addProperty(calcNormals);

			maxSmoothingAngle = new PropFloat(this, "Max smoothing angle", meta.maxSmoothingAngle);
			maxSmoothingAngle->setOnChangeCallback([=](Property* prop, float val) { onChangeMaxSmoothingAngle(prop, val); });
			maxSmoothingAngle->setVisible(meta.calculateNormals);
			addProperty(maxSmoothingAngle);

			Property* lod = new Property(this, "LOD");
			
			PropBool* lodGenerate = new PropBool(this, "Generate LOD", meta.generateLod);
			lodGenerate->setOnChangeCallback([=](Property* prop, bool val) { onChangeLodGenerate(prop, val); });
			lod->addChild(lodGenerate);

			lodLevels = new PropInt(this, "LOD levels", meta.lodLevels);
			lodLevels->setOnChangeCallback([=](Property* prop, int val) { onChangeLodLevels(prop, val); });
			lodLevels->setMinValue(1);
			lodLevels->setMaxValue(3);
			lodLevels->setVisible(meta.generateLod);
			lod->addChild(lodLevels);

			lodError = new PropFloat(this, "LOD error", meta.lodError);
			lodError->setOnChangeCallback([=](Property* prop, float val) { onChangeLodError(prop, val); });
			lodError->setMinValue(0.0f);
			lodError->setMaxValue(1.0f);
			lodError->setVisible(meta.generateLod);
			lod->addChild(lodError);

			lodPreserveTopology = new PropBool(this, "Preserve mesh topology", meta.lodPreserveMeshTopology);
			lodPreserveTopology->setOnChangeCallback([=](Property* prop, bool val) { onChangeLodPreserveTopology(prop, val); });
			lodPreserveTopology->setVisible(meta.generateLod);
			lod->addChild(lodPreserveTopology);
			
			addProperty(lod);

			saveMeta = new PropButton(this, "", "Apply");
			saveMeta->setOnClickCallback([=](Property* prop) { onClickApply(prop); });
			addProperty(saveMeta);
			saveMeta->setVisible(false);
		}

		//Load preview
		meshes.clear();

		AxisAlignedBox bbox = AxisAlignedBox::BOX_NULL;

		std::vector<Model3DLoader::ModelMeshData> _meshes;
		if (ext != "prefab")
			_meshes = Model3DLoader::load3DModelMeshes(Engine::getSingleton()->getAssetsPath(), path);
		else
			_meshes = Model3DLoader::load3DModelMeshesFromPrefab(Engine::getSingleton()->getAssetsPath(), path);

		for (auto& m : _meshes)
		{
			ModelMeshData data;
			data.materials = m.materials;
			data.mesh = m.mesh;
			data.transform = m.transform;
			meshes.push_back(data);

			AxisAlignedBox localBbox = m.mesh->getBounds();
			localBbox.transform(m.transform);
			bbox.merge(localBbox);
		}

		if (bbox == AxisAlignedBox::BOX_NULL)
			bbox = AxisAlignedBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f);

		meshTransform = glm::identity<glm::mat4x4>();
		meshTransform = glm::mat4_cast(Mathf::toQuaternion(glm::vec3(0, 90, 180))) * meshTransform;

		bbox.transform(meshTransform);

		glm::vec3 mmax = bbox.getMaximum();
		float maxSide = std::max(std::max(mmax.x, mmax.y), mmax.z);
		camPos = (glm::vec3(-maxSide, 0, 0) * 5.0f) * 3.0f;
		modelScale = maxSide;

		meshTransform = glm::scale(meshTransform, glm::vec3(5.0f));
		meshTransform = glm::translate(meshTransform, glm::vec3(0, bbox.getCenter().y, 0));

		resetFrameBuffers(prevSize.x > 0 ? prevSize.x : previewHeight, prevSize.y > 0 ? prevSize.y : previewHeight);

		setPreviewFunction([=]() { onUpdatePreview(); });
	}

	void Model3DEditor::update()
	{
		PropertyEditor::update();
	}

	void Model3DEditor::updateEditor()
	{
		if (rt != nullptr)
			resetFrameBuffers(rt->getWidth(), rt->getHeight());

		MainWindow::addOnEndUpdateCallback([=]()
			{
				float sp = MainWindow::getSingleton()->getInspectorWindow()->getScrollPos();
				MainWindow::getSingleton()->getInspectorWindow()->saveCollapsedProperties();

				auto props = getTreeView()->getRootNode()->children;

				for (auto it = props.begin(); it != props.end(); ++it)
					removeProperty((Property*)*it);

				init(modelPaths);

				MainWindow::getSingleton()->getInspectorWindow()->loadCollapsedProperties();
				MainWindow::getSingleton()->getInspectorWindow()->setScrollPos(sp);
			}
		);
	}

	void Model3DEditor::resetFrameBuffers(float w, float h)
	{
		if (rt == nullptr)
			rt = new RenderTexture(w, h);
		else
			rt->reset(w, h);

		if (bgfx::isValid(gbuffer))
			bgfx::destroy(gbuffer);

		if (bgfx::isValid(lightBuffer))
			bgfx::destroy(lightBuffer);

		const uint64_t tsFlags = 0
			| BGFX_SAMPLER_MIN_POINT
			| BGFX_SAMPLER_MAG_POINT
			| BGFX_SAMPLER_MIP_POINT
			| BGFX_SAMPLER_U_CLAMP
			| BGFX_SAMPLER_V_CLAMP
			;

		bgfx::Attachment gbufferAt[5];

		//GBuffer
		gbufferTex[0] = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
		gbufferTex[1] = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
		gbufferTex[2] = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
		gbufferTex[3] = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
		gbufferTex[4] = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT | tsFlags);

		gbufferAt[0].init(gbufferTex[0]);
		gbufferAt[1].init(gbufferTex[1]);
		gbufferAt[2].init(gbufferTex[2]);
		gbufferAt[3].init(gbufferTex[3]);
		gbufferAt[4].init(gbufferTex[4]);

		gbuffer = bgfx::createFrameBuffer(BX_COUNTOF(gbufferAt), gbufferAt, true);

		//Light buffer
		lightBufferTex = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
		lightBuffer = bgfx::createFrameBuffer(1, &lightBufferTex, true);
	}

	void Model3DEditor::reloadModel()
	{
		for (auto& path : modelPaths)
		{
			for (auto& asset : Asset::getLoadedInstances())
			{
				if (asset.second->getAssetType() == Mesh::ASSET_TYPE)
				{
					Mesh* mesh = (Mesh*)asset.second;
					if (mesh->getSourceFile() == path)
					{
						if (IO::FileExists(mesh->getOrigin()))
							IO::FileDelete(mesh->getOrigin());

						mesh->unload();
					}
				}
			}

			Model3DLoader::cache3DModel(Engine::getSingleton()->getAssetsPath(), path);
		}

		MainWindow::addOnEndUpdateCallback([=]()
			{
				BatchedGeometry::getSingleton()->rebuild();
			}
		);
	}

	void Model3DEditor::onChangeScale(Property* prop, float value)
	{
		for (auto& meta : metas)
			meta->scale = value;

		saveMeta->setVisible(true);
	}

	void Model3DEditor::onChangeOptimizeMeshes(Property* prop, bool value)
	{
		for (auto& meta : metas)
			meta->optimizeMeshes = value;

		saveMeta->setVisible(true);
	}

	void Model3DEditor::onChangeCalculateNormals(Property* prop, bool value)
	{
		for (auto& meta : metas)
			meta->calculateNormals = value;

		maxSmoothingAngle->setVisible(value);
		saveMeta->setVisible(true);
	}

	void Model3DEditor::onChangeMaxSmoothingAngle(Property* prop, float value)
	{
		for (auto& meta : metas)
			meta->maxSmoothingAngle = value;

		saveMeta->setVisible(true);
	}

	void Model3DEditor::onChangeGenerateLightmapUVs(Property* prop, bool value)
	{
		for (auto& meta : metas)
			meta->generateLightmapUVs = value;

		saveMeta->setVisible(true);
	}

	void Model3DEditor::onChangeLodGenerate(Property* prop, bool value)
	{
		for (auto& meta : metas)
			meta->generateLod = value;

		lodLevels->setVisible(value);
		lodError->setVisible(value);
		lodPreserveTopology->setVisible(value);

		saveMeta->setVisible(true);
	}

	void Model3DEditor::onChangeLodLevels(Property* prop, int value)
	{
		for (auto& meta : metas)
			meta->lodLevels = value;

		saveMeta->setVisible(true);
	}

	void Model3DEditor::onChangeLodError(Property* prop, float value)
	{
		for (auto& meta : metas)
			meta->lodError = value;

		saveMeta->setVisible(true);
	}

	void Model3DEditor::onChangeLodPreserveTopology(Property* prop, bool value)
	{
		for (auto& meta : metas)
			meta->lodPreserveMeshTopology = value;

		saveMeta->setVisible(true);
	}

	void Model3DEditor::onClickApply(Property* prop)
	{
		for (auto& meta : metas)
			meta->save(meta->filePath);

		reloadModel();
		updateEditor();
	}

	void Model3DEditor::submitLightUniforms()
	{
		bgfx::setUniform(Renderer::getLightPositionUniform(), glm::value_ptr(glm::vec4(-1, -1, 0, 0)), 1);
		bgfx::setUniform(Renderer::getLightDirectionUniform(), glm::value_ptr(glm::vec4(1, 1, 0, 0)), 1);
		bgfx::setUniform(Renderer::getLightTypeUniform(), glm::value_ptr(glm::vec4(2, 0, 0, 0)), 1);
		bgfx::setUniform(Renderer::getLightRenderModeUniform(), glm::value_ptr(glm::vec4(0, 0, 0, 0)), 1);
		bgfx::setUniform(Renderer::getLightRadiusUniform(), glm::value_ptr(glm::vec4(1, 0, 0, 0)), 1);
		bgfx::setUniform(Renderer::getLightColorUniform(), glm::value_ptr(glm::vec4(1, 1, 1, 1)), 1);
		bgfx::setUniform(Renderer::getLightIntensityUniform(), glm::value_ptr(glm::vec4(1, 0, 0, 0)), 1);
		bgfx::setUniform(Renderer::getLightCastShadowsUniform(), glm::value_ptr(glm::vec4(0, 0, 0, 0)), 1);
	}

	void Model3DEditor::onUpdatePreview()
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

			if (isPreviewHovered() && !controlsHovered)
			{
				if (InputManager::getSingleton()->getMouseButtonDown(0))
					mousePressed = true;
				
				if (InputManager::getSingleton()->getMouseButtonDown(2))
					mouseMPressed = true;

				if (InputManager::getSingleton()->getMouseWheel().second > 0)
					camPos += glm::vec3(0.5f * modelScale, 0.0f, 0.0f);

				if (InputManager::getSingleton()->getMouseWheel().second < 0)
					camPos += glm::vec3(-0.5f * modelScale, 0.0f, 0.0f);
			}

			if (InputManager::getSingleton()->getMouseButtonUp(0))
				mousePressed = false;
			
			if (InputManager::getSingleton()->getMouseButtonUp(2))
				mouseMPressed = false;

			//Render meshes
			float _aspect = (float)rt->getWidth() / (float)rt->getHeight();

			if (mousePressed)
			{
				glm::vec2 dxy = Mathf::fDeg2Rad * (mousePos - prevMousePos);

				rotX = glm::angleAxis(-dxy.x, glm::vec3(0, 1, 0));
				rotY = glm::angleAxis(-dxy.y, glm::vec3(0, 0, 1));

				meshTransform = glm::mat4_cast(rotX * rotY) * meshTransform;
			}

			if (mouseMPressed)
			{
				glm::vec2 offset = (mousePos - prevMousePos) * 0.02f;
				camPos += glm::vec3(0, -offset.y, offset.x) * modelScale;
			}

			prevMousePos = mousePos;

			glm::mat4x4 view = Camera::makeViewMatrix(camPos, camPos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
			glm::mat4x4 proj = Camera::makeProjectionMatrix(60.0f, _aspect, 0.01f, 100.0f * modelScale);

			int viewId = Renderer::getSingleton()->getNumViewsUsed() + 3;

			//GBuffer
			bgfx::setViewClear(viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0.0f);
			bgfx::setViewRect(viewId, 0, 0, rt->getWidth(), rt->getHeight());
			bgfx::setViewTransform(viewId, glm::value_ptr(view), glm::value_ptr(proj));
			bgfx::setViewFrameBuffer(viewId, gbuffer);

			//Light
			glm::mat4x4 geomProj = glm::orthoRH(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f);
			bgfx::setViewClear(viewId + 1, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0.0f);
			bgfx::setViewRect(viewId + 1, 0, 0, rt->getWidth(), rt->getHeight());
			bgfx::setViewFrameBuffer(viewId + 1, lightBuffer);
			bgfx::setViewTransform(viewId + 1, NULL, glm::value_ptr(geomProj));

			//Compose deferred
			bgfx::setViewClear(viewId + 2, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0.0f);
			bgfx::setViewRect(viewId + 2, 0, 0, rt->getWidth(), rt->getHeight());
			bgfx::setViewFrameBuffer(viewId + 2, rt->getFrameBufferHandle());
			bgfx::setViewTransform(viewId + 2, NULL, glm::value_ptr(geomProj));

			//Forward
			bgfx::setViewClear(viewId + 3, 0, 0x00000000, 1.0f, 0.0f);
			bgfx::setViewRect(viewId + 3, 0, 0, rt->getWidth(), rt->getHeight());
			bgfx::setViewFrameBuffer(viewId + 3, rt->getFrameBufferHandle());
			bgfx::setViewTransform(viewId + 3, glm::value_ptr(view), glm::value_ptr(proj));
			//----

			bgfx::touch(viewId);
			bgfx::touch(viewId + 1);
			bgfx::touch(viewId + 2);
			bgfx::touch(viewId + 3);

			uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LEQUAL | BGFX_STATE_CULL_CW;

			for (auto& mesh : meshes)
			{
				glm::mat4x4 mtx = meshTransform * mesh.transform;

				//Deferred
				Primitives::mesh(mtx, mesh.mesh, mesh.materials, viewId,
					state,
					nullptr,
					static_cast<int>(RenderMode::Deferred),
					[=]()
					{
						bgfx::setUniform(Renderer::getNormalMatrixUniform(), glm::value_ptr(glm::mat3(mtx)), 1);
						bgfx::setUniform(Renderer::getGpuSkinningUniform(), glm::value_ptr(glm::vec4(0.0, 0.0, 0.0, 0.0)), 1);
						bgfx::setUniform(Renderer::getInvModelUniform(), glm::value_ptr(glm::inverse(mtx)), 1);
						bgfx::setUniform(Renderer::getHasLightmapUniform(), glm::value_ptr(glm::vec4(0, 0, 0, 0)), 1);

						submitLightUniforms();

						bgfx::setUniform(Renderer::getCameraPositionUniform(), glm::value_ptr(glm::vec4(camPos, 0.0f)), 1);
						bgfx::setUniform(Renderer::getCameraClearColorUniform(), Color(0, 0, 0, 1.0).ptr(), 1);
						bgfx::setUniform(Renderer::getCameraClippingPlanesUniform(), glm::value_ptr(glm::vec4(0.1f, 100.0f, 0.0f, 0.0f)), 1);
						bgfx::setUniform(Renderer::getScreenParamsUniform(), glm::value_ptr(glm::vec4(rt->getWidth(), rt->getHeight(), 1.0f + 1.0f / rt->getWidth(), 1.0f + 1.0f / rt->getHeight())), 1);
					},
					lodLevel - 1
				);
			}

			//Light
			Renderer::getSingleton()->setSystemUniforms(nullptr);
			bgfx::setTexture(0, Renderer::getSingleton()->getAlbedoMapUniform(), gbufferTex[0]);
			bgfx::setTexture(1, Renderer::getSingleton()->getNormalMapUniform(), gbufferTex[1]);
			bgfx::setTexture(2, Renderer::getSingleton()->getMRAMapUniform(), gbufferTex[2]);
			bgfx::setTexture(3, Renderer::getSingleton()->getLightmapUniform(), gbufferTex[3]);
			bgfx::setTexture(4, Renderer::getSingleton()->getDepthMapUniform(), gbufferTex[4]);
			bgfx::setUniform(Renderer::getSingleton()->getInvViewProjUniform(), glm::value_ptr(glm::inverse(proj * view)), 1);

			submitLightUniforms();
			bgfx::setUniform(Renderer::getLightIntensityUniform(), glm::value_ptr(glm::vec4(1, 0, 0, 0)), 1);

			bgfx::setUniform(Renderer::getCameraPositionUniform(), glm::value_ptr(glm::vec4(camPos, 0.0f)), 1);
			bgfx::setUniform(Renderer::getCameraClearColorUniform(), Color(0, 0, 0, 1.0).ptr(), 1);
			bgfx::setUniform(Renderer::getCameraClippingPlanesUniform(), glm::value_ptr(glm::vec4(0.1f, 100.0f, 0.0f, 0.0f)), 1);

			Primitives::screenSpaceQuad();
			bgfx::submit(viewId + 1, Renderer::getSingleton()->getLightProgram());

			//Compose
			Renderer::getSingleton()->setSystemUniforms(nullptr);
			bgfx::setUniform(Renderer::getSingleton()->getAmbientColorUniform(), glm::value_ptr(glm::vec4(0.25f, 0.25f, 0.25f, 1.0f)), 1);

			bgfx::setTexture(0, Renderer::getSingleton()->getAlbedoMapUniform(), gbufferTex[0]);
			bgfx::setTexture(1, Renderer::getSingleton()->getDepthMapUniform(), gbufferTex[4]);
			bgfx::setTexture(2, Renderer::getSingleton()->getLightColorMapUniform(), lightBufferTex);
			bgfx::setTexture(3, Renderer::getSingleton()->getLightmapUniform(), gbufferTex[3]);
			bgfx::setState(0
				| BGFX_STATE_WRITE_RGB
				| BGFX_STATE_WRITE_A
				| BGFX_STATE_WRITE_Z);
			Primitives::screenSpaceQuad();
			bgfx::submit(viewId + 2, Renderer::getSingleton()->getCombineProgram());

			//Forward
			for (auto& mesh : meshes)
			{
				glm::mat4x4 mtx = meshTransform * mesh.transform;

				Primitives::mesh(mtx, mesh.mesh, mesh.materials, viewId + 3,
					state,
					nullptr,
					static_cast<int>(RenderMode::Forward),
					[=]()
					{
						bgfx::setUniform(Renderer::getNormalMatrixUniform(), glm::value_ptr(glm::mat3(mtx)), 1);
						bgfx::setUniform(Renderer::getGpuSkinningUniform(), glm::value_ptr(glm::vec4(0.0, 0.0, 0.0, 0.0)), 1);
						bgfx::setUniform(Renderer::getInvModelUniform(), glm::value_ptr(glm::inverse(mtx)), 1);
						bgfx::setUniform(Renderer::getHasLightmapUniform(), glm::value_ptr(glm::vec4(0, 0, 0, 0)), 1);

						submitLightUniforms();

						bgfx::setUniform(Renderer::getCameraPositionUniform(), glm::value_ptr(glm::vec4(camPos, 0.0f)), 1);
						bgfx::setUniform(Renderer::getCameraClearColorUniform(), Color(0, 0, 0, 1.0).ptr(), 1);
						bgfx::setUniform(Renderer::getCameraClippingPlanesUniform(), glm::value_ptr(glm::vec4(0.1f, 100.0f, 0.0f, 0.0f)), 1);
						bgfx::setUniform(Renderer::getScreenParamsUniform(), glm::value_ptr(glm::vec4(rt->getWidth(), rt->getHeight(), 1.0f + 1.0f / rt->getWidth(), 1.0f + 1.0f / rt->getHeight())), 1);
					},
					lodLevel - 1
				);
			}

			//Renderer::getSingleton()->frame();

			float w = rt->getWidth();
			float h = rt->getHeight();

			ImVec2 cur = ImGui::GetCursorPos();
			ImGui::Image((void*)rt->getColorTextureHandle().idx, ImVec2(w, h), ImVec2(0, 1), ImVec2(1, 0));

			ImGui::SetCursorPosX(cur.x + 5);
			ImGui::SetCursorPosY(cur.y + 5);

			int maxLod = 0;
			int vertexCount = 0;

			if (meshes.size() > 0)
			{
				if (meshes[0].mesh->getSubMeshCount() > 0)
					maxLod = meshes[0].mesh->getSubMesh(0)->getLodLevelsCount();
			}

			if (maxLod > 0)
			{
				if (lodLevel - 1 > -1)
				{
					for (auto& mesh : meshes)
					{
						for (int i = 0; i < mesh.mesh->getSubMeshCount(); ++i)
						{
							SubMesh* subMesh = mesh.mesh->getSubMesh(i);
							vertexCount += subMesh->getLodIndexBuffer(lodLevel - 1).size();
						}
					}
				}
				else
				{
					for (auto& mesh : meshes)
					{
						for (int i = 0; i < mesh.mesh->getSubMeshCount(); ++i)
						{
							SubMesh* subMesh = mesh.mesh->getSubMesh(i);
							vertexCount += subMesh->getIndexBuffer().size();
						}
					}
				}

				ImGui::Text("LOD Level: ");
				ImGui::SameLine();
				ImGui::SliderInt("##lod_level", &lodLevel, 0, maxLod);
				controlsHovered = ImGui::IsItemHovered();

				ImGui::SetCursorPosX(cur.x + 5);

				std::string vcnt = "Vertex count: " + std::to_string(vertexCount);
				ImGui::Text(vcnt.c_str());
			}
			else
			{
				for (auto& mesh : meshes)
				{
					for (int i = 0; i < mesh.mesh->getSubMeshCount(); ++i)
					{
						SubMesh* subMesh = mesh.mesh->getSubMesh(i);
						vertexCount += subMesh->getIndexBuffer().size();
					}
				}

				std::string vcnt = "Vertex count: " + std::to_string(vertexCount);
				ImGui::Text(vcnt.c_str());
			}
		}
	}
}