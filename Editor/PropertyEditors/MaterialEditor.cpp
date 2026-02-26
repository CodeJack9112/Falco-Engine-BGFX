#include "MaterialEditor.h"

#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"

#include "../Windows/MainWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/InspectorWindow.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/Cubemap.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Assets/Shader.h"
#include "../Engine/Assets/Mesh.h"
#include "../Engine/Assets/Model3DLoader.h"
#include "../Engine/Renderer/RenderTexture.h"
#include "../Engine/Components/Camera.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Renderer/Primitives.h"
#include "../Engine/Math/Mathf.h"
#include "../Engine/Core/InputManager.h"

#include "PropInt.h"
#include "PropFloat.h"
#include "PropBool.h"
#include "PropVector2.h"
#include "PropVector3.h"
#include "PropVector4.h"
#include "PropSampler.h"
#include "PropColorPicker.h"
#include "PropComboBox.h"

#include "../Classes/Undo.h"

#include "../Engine/glm/gtc/type_ptr.hpp"

namespace GX
{
	MaterialEditor::MaterialEditor()
	{
		setEditorName("MaterialEditor");
	}

	MaterialEditor::~MaterialEditor()
	{
		if (rt != nullptr)
			delete rt;

		if (bgfx::isValid(gbuffer))
			bgfx::destroy(gbuffer);

		if (bgfx::isValid(lightBuffer))
			bgfx::destroy(lightBuffer);
	}

	void MaterialEditor::init(Material* mat, bool showPreview)
	{
		material = mat;

		Shader* shader = mat->getShader();

		if (isShaderEditable)
		{
			std::vector<Shader*>& shaderList = MainWindow::getSingleton()->getAssetsWindow()->getShaderList();
			std::vector<std::string> shaderNames;

			for (auto it = shaderList.begin(); it != shaderList.end(); ++it)
				shaderNames.push_back((*it)->getAlias());

			PropComboBox* shaderProp = new PropComboBox(this, "Shader", shaderNames);
			shaderProp->setIsNested(true);
			shaderProp->setOnChangeCallback([=](Property* prop, std::string val) { onChangeShader(prop, val); });
			addProperty(shaderProp);

			if (shader != nullptr)
				shaderProp->setCurrentItem(shader->getAlias());
		}

		if (shader != nullptr)
		{
			std::vector<Uniform>& uniforms = material->getUniforms();

			for (auto it = uniforms.begin(); it != uniforms.end(); ++it)
			{
				Uniform& uniform = *it;
				const UniformVariant* sinf = shader->getUniformVariant(uniform.getName(), material->getDefinesString());

				if (sinf == nullptr)
					continue;

				if (sinf->isHidden)
					continue;

				std::string varName = uniform.getName();
				bool isDefine = material->getUniformIsDefine(varName);

				if (uniform.getType() == UniformType::Int)
				{
					PropInt* uInt = new PropInt(this, sinf->bindName, uniform.getValue<int>());
					if (sinf->min != FLT_MIN) uInt->setMinValue(sinf->min);
					if (sinf->max != FLT_MAX) uInt->setMaxValue(sinf->max);
					uInt->setOnChangeCallback([=](Property* prop, int val) { onChangeInt(prop, varName, val, isDefine);  });

					addProperty(uInt);
				}

				if (uniform.getType() == UniformType::Float)
				{
					PropFloat* uFloat = new PropFloat(this, sinf->bindName, uniform.getValue<float>());
					if (sinf->min != FLT_MIN) uFloat->setMinValue(sinf->min);
					if (sinf->max != FLT_MAX) uFloat->setMaxValue(sinf->max);
					uFloat->setOnChangeCallback([=](Property* prop, float val) { onChangeFloat(prop, varName, val, isDefine); });

					addProperty(uFloat);
				}

				if (uniform.getType() == UniformType::Bool)
				{
					PropBool* uBool = new PropBool(this, sinf->bindName, uniform.getValue<bool>());
					uBool->setOnChangeCallback([=](Property* prop, bool val) { onChangeBool(prop, varName, val, isDefine); });

					addProperty(uBool);
				}

				if (uniform.getType() == UniformType::Sampler2D)
				{
					Sampler2DDef smp = uniform.getValue<Sampler2DDef>();
					PropSampler* uSampler2D = new PropSampler(this, sinf->bindName, smp.second);
					uSampler2D->setSupportedFormats(Engine::getImageFileFormats());
					uSampler2D->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onChangeSampler2D(prop, from, varName, smp.first, isDefine); });
					uSampler2D->setOnClearCallback([=](Property* prop) { onClearSampler2D(prop, varName, smp.first, isDefine); });
					uSampler2D->setOnClickCallback([=](Property* prop) {
						if (smp.second != nullptr)
							MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(smp.second->getName());
					});

					addProperty(uSampler2D);
				}

				if (uniform.getType() == UniformType::SamplerCube)
				{
					SamplerCubeDef smp = uniform.getValue<SamplerCubeDef>();
					Texture* tex = nullptr;
					if (smp.second != nullptr)
					{
						for (int i = 0; i < 6; ++i)
						{
							tex = smp.second->getTexture(i);
							if (tex != nullptr) break;
						}
					}
					PropSampler* uSamplerCube = new PropSampler(this, sinf->bindName, tex);
					uSamplerCube->setSupportedFormats({ "cubemap" });
					uSamplerCube->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onChangeSamplerCube(prop, from, varName, smp.first, isDefine); });
					uSamplerCube->setOnClearCallback([=](Property* prop) { onClearSamplerCube(prop, varName, smp.first, isDefine); });
					uSamplerCube->setOnClickCallback([=](Property* prop) {
						if (smp.second != nullptr)
							MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(smp.second->getName());
					});

					addProperty(uSamplerCube);
				}

				if (uniform.getType() == UniformType::Vec2)
				{
					std::string varName = uniform.getName();
					PropVector2* uVector2 = new PropVector2(this, sinf->bindName, uniform.getValue<glm::vec2>());
					uVector2->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeVec2(prop, varName, val, isDefine); });

					addProperty(uVector2);
				}

				if (uniform.getType() == UniformType::Vec3)
				{
					std::string varName = uniform.getName();
					PropVector3* uVector3 = new PropVector3(this, sinf->bindName, uniform.getValue<glm::vec3>());
					uVector3->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeVec3(prop, varName, val, isDefine); });

					addProperty(uVector3);
				}

				if (uniform.getType() == UniformType::Vec4)
				{
					std::string varName = uniform.getName();
					PropVector4* uVector4 = new PropVector4(this, sinf->bindName, uniform.getValue<glm::vec4>());
					uVector4->setOnChangeCallback([=](Property* prop, glm::vec4 val) { onChangeVec4(prop, varName, val, isDefine); });

					addProperty(uVector4);
				}

				if (uniform.getType() == UniformType::Color)
				{
					std::string varName = uniform.getName();
					PropColorPicker* uColor = new PropColorPicker(this, sinf->bindName, uniform.getValue<Color>());
					uColor->setOnChangeCallback([=](Property* prop, Color val) { onChangeColor(prop, varName, val, isDefine); });

					addProperty(uColor);
				}
			}
		}

		if (showPreview)
		{
			meshTransform = glm::identity<glm::mat4x4>();
			meshTransform = glm::translate(meshTransform, glm::vec3(0, 0, 0));
			meshTransform = glm::scale(meshTransform, glm::vec3(40.0f));
			meshTransform = glm::mat4_cast(Mathf::toQuaternion(glm::vec3(-90, 0, 0))) * meshTransform;

			auto meshes = Model3DLoader::load3DModelMeshes(Engine::getSingleton()->getBuiltinResourcesPath(), "Primitives/Sphere.fbx");
			sphereMesh = meshes[0].mesh;

			resetFrameBuffers(previewHeight, previewHeight);

			setPreviewFunction([=]() { onUpdatePreview(); });
		}
	}

	void MaterialEditor::update()
	{
		PropertyEditor::update();
	}

	void MaterialEditor::updateEditor()
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

				init(material);

				MainWindow::getSingleton()->getInspectorWindow()->loadCollapsedProperties();
				MainWindow::getSingleton()->getInspectorWindow()->setScrollPos(sp);
			}
		);
	}

	void MaterialEditor::onChangeInt(Property* prop, std::string varName, int val, bool isDefine)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change material property " + varName);
		undoData->stringData.resize(3);
		undoData->intData.resize(2);
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<int>(data->stringData[2][nullptr], data->intData[0][nullptr], data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<int>(data->stringData[2][nullptr], data->intData[1][nullptr], data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->stringData[0][nullptr] = material->getLocation();
		undoData->stringData[1][nullptr] = material->getName();
		undoData->stringData[2][nullptr] = varName;
		undoData->intData[0][nullptr] = material->getUniform<int>(varName);
		undoData->intData[1][nullptr] = val;
		undoData->boolData[0][nullptr] = isDefine;
		undoData->boolData[1][nullptr] = saveOnEdit;
		//

		material->setUniform<int>(varName, val, isDefine);
		if (saveOnEdit)
			material->save();
	}

	void MaterialEditor::onChangeFloat(Property* prop, std::string varName, float val, bool isDefine)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change material property " + varName);
		undoData->stringData.resize(3);
		undoData->floatData.resize(2);
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<float>(data->stringData[2][nullptr], data->floatData[0][nullptr], data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<float>(data->stringData[2][nullptr], data->floatData[1][nullptr], data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->stringData[0][nullptr] = material->getLocation();
		undoData->stringData[1][nullptr] = material->getName();
		undoData->stringData[2][nullptr] = varName;
		undoData->floatData[0][nullptr] = material->getUniform<float>(varName);
		undoData->floatData[1][nullptr] = val;
		undoData->boolData[0][nullptr] = isDefine;
		undoData->boolData[1][nullptr] = saveOnEdit;
		//

		material->setUniform<float>(varName, val, isDefine);
		if (saveOnEdit)
			material->save();
	}

	void MaterialEditor::onChangeBool(Property* prop, std::string varName, bool val, bool isDefine)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change material property " + varName);
		undoData->stringData.resize(3);
		undoData->boolData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<bool>(data->stringData[2][nullptr], data->boolData[2][nullptr], data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<bool>(data->stringData[2][nullptr], data->boolData[3][nullptr], data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->stringData[0][nullptr] = material->getLocation();
		undoData->stringData[1][nullptr] = material->getName();
		undoData->stringData[2][nullptr] = varName;
		undoData->boolData[2][nullptr] = material->getUniform<bool>(varName);
		undoData->boolData[3][nullptr] = val;
		undoData->boolData[0][nullptr] = isDefine;
		undoData->boolData[1][nullptr] = saveOnEdit;
		//

		material->setUniform<bool>(varName, val, isDefine);
		if (saveOnEdit)
			material->save();

		if (isDefine) updateEditor();
	}

	void MaterialEditor::onChangeVec2(Property* prop, std::string varName, glm::vec2 val, bool isDefine)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change material property " + varName);
		undoData->stringData.resize(3);
		undoData->vec2Data.resize(2);
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<glm::vec2>(data->stringData[2][nullptr], data->vec2Data[0][nullptr], data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<glm::vec2>(data->stringData[2][nullptr], data->vec2Data[1][nullptr], data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->stringData[0][nullptr] = material->getLocation();
		undoData->stringData[1][nullptr] = material->getName();
		undoData->stringData[2][nullptr] = varName;
		undoData->vec2Data[0][nullptr] = material->getUniform<glm::vec2>(varName);
		undoData->vec2Data[1][nullptr] = val;
		undoData->boolData[0][nullptr] = isDefine;
		undoData->boolData[1][nullptr] = saveOnEdit;
		//

		material->setUniform<glm::vec2>(varName, val, isDefine);
		if (saveOnEdit)
			material->save();
	}

	void MaterialEditor::onChangeVec3(Property* prop, std::string varName, glm::vec3 val, bool isDefine)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change material property " + varName);
		undoData->stringData.resize(3);
		undoData->vec3Data.resize(2);
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<glm::vec3>(data->stringData[2][nullptr], data->vec3Data[0][nullptr], data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<glm::vec3>(data->stringData[2][nullptr], data->vec3Data[1][nullptr], data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->stringData[0][nullptr] = material->getLocation();
		undoData->stringData[1][nullptr] = material->getName();
		undoData->stringData[2][nullptr] = varName;
		undoData->vec3Data[0][nullptr] = material->getUniform<glm::vec3>(varName);
		undoData->vec3Data[1][nullptr] = val;
		undoData->boolData[0][nullptr] = isDefine;
		undoData->boolData[1][nullptr] = saveOnEdit;
		//

		material->setUniform<glm::vec3>(varName, val, isDefine);
		if (saveOnEdit)
			material->save();
	}

	void MaterialEditor::onChangeVec4(Property* prop, std::string varName, glm::vec4 val, bool isDefine)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change material property " + varName);
		undoData->stringData.resize(3);
		undoData->vec4Data.resize(2);
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<glm::vec4>(data->stringData[2][nullptr], data->vec4Data[0][nullptr], data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<glm::vec4>(data->stringData[2][nullptr], data->vec4Data[1][nullptr], data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->stringData[0][nullptr] = material->getLocation();
		undoData->stringData[1][nullptr] = material->getName();
		undoData->stringData[2][nullptr] = varName;
		undoData->vec4Data[0][nullptr] = material->getUniform<glm::vec4>(varName);
		undoData->vec4Data[1][nullptr] = val;
		undoData->boolData[0][nullptr] = isDefine;
		undoData->boolData[1][nullptr] = saveOnEdit;
		//

		material->setUniform<glm::vec4>(varName, val, isDefine);
		if (saveOnEdit)
			material->save();
	}

	void MaterialEditor::onChangeColor(Property* prop, std::string varName, Color val, bool isDefine)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change material property " + varName);
		undoData->stringData.resize(3);
		undoData->vec4Data.resize(2);
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				glm::vec4 v4 = data->vec4Data[0][nullptr];
				Color col = Color(v4.x, v4.y, v4.z, v4.w);
				mat->setUniform<Color>(data->stringData[2][nullptr], col, data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				glm::vec4 v4 = data->vec4Data[1][nullptr];
				Color col = Color(v4.x, v4.y, v4.z, v4.w);
				mat->setUniform<Color>(data->stringData[2][nullptr], col, data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		Color col = material->getUniform<Color>(varName);
		glm::vec4 v4 = glm::vec4(col[0], col[1], col[2], col[3]);

		undoData->stringData[0][nullptr] = material->getLocation();
		undoData->stringData[1][nullptr] = material->getName();
		undoData->stringData[2][nullptr] = varName;
		undoData->vec4Data[0][nullptr] = v4;
		undoData->vec4Data[1][nullptr] = glm::vec4(val[0], val[1], val[2], val[3]);
		undoData->boolData[0][nullptr] = isDefine;
		undoData->boolData[1][nullptr] = saveOnEdit;
		//

		material->setUniform<Color>(varName, val, isDefine);
		if (saveOnEdit)
			material->save();
	}

	void MaterialEditor::onChangeSampler2D(TreeNode* prop, TreeNode* from, std::string varName, int val, bool isDefine)
	{
		std::string fullPath = from->getPath();
		Texture* texture = Texture::load(Engine::getSingleton()->getAssetsPath(), fullPath, true, Texture::CompressionMethod::Default);
		((PropSampler*)prop)->setValue(texture);

		//Undo
		UndoData* undoData = Undo::addUndo("Change material property " + varName);
		undoData->stringData.resize(7);
		undoData->boolData.resize(2);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				Texture* tex = Texture::load(data->stringData[3][nullptr], data->stringData[4][nullptr]);
				mat->setUniform<Sampler2DDef>(data->stringData[2][nullptr], { data->intData[0][nullptr], tex }, data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				Texture* tex = Texture::load(data->stringData[5][nullptr], data->stringData[6][nullptr]);
				mat->setUniform<Sampler2DDef>(data->stringData[2][nullptr], { data->intData[0][nullptr], tex }, data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		auto smp = material->getUniform<Sampler2DDef>(varName);

		undoData->stringData[0][nullptr] = material->getLocation();
		undoData->stringData[1][nullptr] = material->getName();
		undoData->stringData[2][nullptr] = varName;
		undoData->stringData[3][nullptr] = smp.second != nullptr ? smp.second->getLocation() : "";
		undoData->stringData[4][nullptr] = smp.second != nullptr ? smp.second->getName() : "";
		undoData->stringData[5][nullptr] = texture != nullptr ? texture->getLocation() : "";
		undoData->stringData[6][nullptr] = texture != nullptr ? texture->getName() : "";
		undoData->intData[0][nullptr] = val;
		undoData->boolData[0][nullptr] = isDefine;
		undoData->boolData[1][nullptr] = saveOnEdit;
		//

		material->setUniform<Sampler2DDef>(varName, std::make_pair(val, texture), isDefine);
		if (saveOnEdit)
			material->save();
	}

	void MaterialEditor::onClearSampler2D(Property* prop, std::string varName, int val, bool isDefine)
	{
		((PropSampler*)prop)->setValue(nullptr);

		//Undo
		UndoData* undoData = Undo::addUndo("Change material property " + varName);
		undoData->stringData.resize(5);
		undoData->boolData.resize(2);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				Texture* tex = Texture::load(data->stringData[3][nullptr], data->stringData[4][nullptr]);
				mat->setUniform<Sampler2DDef>(data->stringData[2][nullptr], { data->intData[0][nullptr], tex }, data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<Sampler2DDef>(data->stringData[2][nullptr], { data->intData[0][nullptr], nullptr }, data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		auto smp = material->getUniform<Sampler2DDef>(varName);

		undoData->stringData[0][nullptr] = material->getLocation();
		undoData->stringData[1][nullptr] = material->getName();
		undoData->stringData[2][nullptr] = varName;
		undoData->stringData[3][nullptr] = smp.second != nullptr ? smp.second->getLocation() : "";
		undoData->stringData[4][nullptr] = smp.second != nullptr ? smp.second->getName() : "";
		undoData->intData[0][nullptr] = val;
		undoData->boolData[0][nullptr] = isDefine;
		undoData->boolData[1][nullptr] = saveOnEdit;
		//

		material->setUniform<Sampler2DDef>(varName, std::make_pair(val, nullptr), isDefine);
		if (saveOnEdit)
			material->save();
	}

	void MaterialEditor::onChangeSamplerCube(TreeNode* prop, TreeNode* from, std::string varName, int val, bool isDefine)
	{
		std::string fullPath = from->getPath();
		Cubemap* cubemap = Cubemap::load(Engine::getSingleton()->getAssetsPath(), fullPath);
		Texture* tex1 = nullptr;
		if (cubemap != nullptr)
		{
			for (int i = 0; i < 6; ++i)
			{
				tex1 = cubemap->getTexture(i);
				if (tex1 != nullptr) break;
			}
		}
		((PropSampler*)prop)->setValue(tex1);

		//Undo
		UndoData* undoData = Undo::addUndo("Change material property " + varName);
		undoData->stringData.resize(7);
		undoData->boolData.resize(2);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				Cubemap* cub = Cubemap::load(data->stringData[3][nullptr], data->stringData[4][nullptr]);
				mat->setUniform<SamplerCubeDef>(data->stringData[2][nullptr], { data->intData[0][nullptr], cub }, data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				Cubemap* cub = Cubemap::load(data->stringData[5][nullptr], data->stringData[6][nullptr]);
				mat->setUniform<SamplerCubeDef>(data->stringData[2][nullptr], { data->intData[0][nullptr], cub }, data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		auto smp = material->getUniform<SamplerCubeDef>(varName);

		undoData->stringData[0][nullptr] = material->getLocation();
		undoData->stringData[1][nullptr] = material->getName();
		undoData->stringData[2][nullptr] = varName;
		undoData->stringData[3][nullptr] = smp.second != nullptr ? smp.second->getLocation() : "";
		undoData->stringData[4][nullptr] = smp.second != nullptr ? smp.second->getName() : "";
		undoData->stringData[5][nullptr] = cubemap != nullptr ? cubemap->getLocation() : "";
		undoData->stringData[6][nullptr] = cubemap != nullptr ? cubemap->getName() : "";
		undoData->intData[0][nullptr] = val;
		undoData->boolData[0][nullptr] = isDefine;
		undoData->boolData[1][nullptr] = saveOnEdit;
		//

		material->setUniform<SamplerCubeDef>(varName, std::make_pair(val, cubemap), isDefine);
		if (saveOnEdit)
			material->save();
	}

	void MaterialEditor::onClearSamplerCube(Property* prop, std::string varName, int val, bool isDefine)
	{
		((PropSampler*)prop)->setValue(nullptr);

		//Undo
		UndoData* undoData = Undo::addUndo("Change material property " + varName);
		undoData->stringData.resize(5);
		undoData->boolData.resize(2);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				Cubemap* cub = Cubemap::load(data->stringData[3][nullptr], data->stringData[4][nullptr]);
				mat->setUniform<SamplerCubeDef>(data->stringData[2][nullptr], { data->intData[0][nullptr], cub }, data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				mat->setUniform<SamplerCubeDef>(data->stringData[2][nullptr], { data->intData[0][nullptr], nullptr }, data->boolData[0][nullptr]);
				if (data->boolData[1][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		auto smp = material->getUniform<SamplerCubeDef>(varName);

		undoData->stringData[0][nullptr] = material->getLocation();
		undoData->stringData[1][nullptr] = material->getName();
		undoData->stringData[2][nullptr] = varName;
		undoData->stringData[3][nullptr] = smp.second != nullptr ? smp.second->getLocation() : "";
		undoData->stringData[4][nullptr] = smp.second != nullptr ? smp.second->getName() : "";
		undoData->intData[0][nullptr] = val;
		undoData->boolData[0][nullptr] = isDefine;
		undoData->boolData[1][nullptr] = saveOnEdit;
		//

		material->setUniform<SamplerCubeDef>(varName, std::make_pair(val, nullptr), isDefine);
		if (saveOnEdit)
			material->save();
	}

	void MaterialEditor::onChangeShader(Property* prop, std::string value)
	{
		std::vector<Shader*>& shaderList = MainWindow::getSingleton()->getAssetsWindow()->getShaderList();

		Shader* shader = nullptr;

		auto it = std::find_if(shaderList.begin(), shaderList.end(), [=](Shader* s) -> bool { return s->getAlias() == value; });
		if (it != shaderList.end())
			shader = *it;

		//Undo
		UndoData* undoData = Undo::addUndo("Change material shader");
		undoData->stringData.resize(6);
		undoData->boolData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				Shader* sh = Shader::load(data->stringData[2][nullptr], data->stringData[3][nullptr]);
				mat->setShader(sh);
				if (data->boolData[0][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (mat != nullptr)
			{
				Shader* sh = Shader::load(data->stringData[4][nullptr], data->stringData[5][nullptr]);
				mat->setShader(sh);
				if (data->boolData[0][nullptr])
					mat->save();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		Shader* prevShader = material->getShader();

		undoData->stringData[0][nullptr] = material->getLocation();
		undoData->stringData[1][nullptr] = material->getName();
		undoData->stringData[2][nullptr] = prevShader != nullptr ? prevShader->getLocation() : "";
		undoData->stringData[3][nullptr] = prevShader != nullptr ? prevShader->getName() : "";
		undoData->stringData[4][nullptr] = shader != nullptr ? shader->getLocation() : "";
		undoData->stringData[5][nullptr] = shader != nullptr ? shader->getName() : "";
		undoData->boolData[0][nullptr] = saveOnEdit;
		//

		material->setShader(shader);
		if (saveOnEdit)
			material->save();

		updateEditor();
	}

	void MaterialEditor::resetFrameBuffers(float w, float h)
	{
		if (rt == nullptr)
			rt = new RenderTexture(w, h);
		else
			rt->reset(w, h);

		if (material->getShader() != nullptr)
		{
			if (material->getShader()->getRenderMode() == RenderMode::Deferred)
			{
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
		}
	}

	void MaterialEditor::submitLightUniforms()
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

	void MaterialEditor::onUpdatePreview()
	{
		if (material != nullptr && rt != nullptr)
		{
			ImVec2 sz = ImGui::GetContentRegionAvail();
			float x = sz.x;

			if (sz.x > previewHeight)
			{
				if (prevHeight != previewHeight)
				{
					if (previewHeight > 0)
						resetFrameBuffers(previewHeight, previewHeight);
				}
			}
			else
			{
				if (prevWidth != sz.x)
				{
					if (sz.x > 0)
						resetFrameBuffers(sz.x, sz.x);
				}
			}

			prevWidth = sz.x;
			prevHeight = previewHeight;

			glm::vec2 mousePos = glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
			glm::vec2 dxy = Mathf::fDeg2Rad * mousePos;

			if (isPreviewHovered())
			{
				if (InputManager::getSingleton()->getMouseButtonDown(0))
					mousePressed = true;
			}

			if (InputManager::getSingleton()->getMouseButtonUp(0))
			{
				mousePressed = false;
			}

			//Render sphere with material
			float _aspect = (float)rt->getWidth() / (float)rt->getHeight();

			if (mousePressed && ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				glm::vec2 dxy = Mathf::fDeg2Rad * (mousePos - prevMousePos);

				rotX = glm::angleAxis(-dxy.x, glm::vec3(0, 1, 0));
				rotY = glm::angleAxis(-dxy.y, glm::vec3(0, 0, 1));

				meshTransform = glm::mat4_cast(rotX * rotY) * meshTransform;
			}
			
			prevMousePos = mousePos;

			glm::mat4x4 view = Camera::makeViewMatrix(camPos, glm::vec3(0, 0, 0), glm::vec3(0, -1, 0));
			glm::mat4x4 proj = Camera::makeProjectionMatrix(60.0f, _aspect, 0.1f, 100.0f);

			int viewId = Renderer::getSingleton()->getNumViewsUsed() + 3;

			bgfx::setViewClear(viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0.0f);
			bgfx::setViewRect(viewId, 0, 0, rt->getWidth(), rt->getHeight());
			bgfx::setViewTransform(viewId, glm::value_ptr(view), glm::value_ptr(proj));
			if (material->getShader() != nullptr && material->getShader()->getRenderMode() == RenderMode::Deferred)
				bgfx::setViewFrameBuffer(viewId, gbuffer);
			else
				bgfx::setViewFrameBuffer(viewId, rt->getFrameBufferHandle());
			//----
			
			uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LEQUAL | BGFX_STATE_CULL_CW;

			Primitives::mesh(meshTransform, sphereMesh, material, viewId,
				state,
				nullptr,
				[=]()
				{
					bgfx::setUniform(Renderer::getNormalMatrixUniform(), glm::value_ptr(glm::mat3(meshTransform)), 1);
					bgfx::setUniform(Renderer::getGpuSkinningUniform(), glm::value_ptr(glm::vec4(0.0, 0.0, 0.0, 0.0)), 1);
					bgfx::setUniform(Renderer::getInvModelUniform(), glm::value_ptr(glm::inverse(meshTransform)), 1);
					bgfx::setUniform(Renderer::getHasLightmapUniform(), glm::value_ptr(glm::vec4(0, 0, 0, 0)), 1);

					submitLightUniforms();

					bgfx::setUniform(Renderer::getCameraPositionUniform(), glm::value_ptr(glm::vec4(camPos, 0.0f)), 1);
					bgfx::setUniform(Renderer::getCameraClearColorUniform(), Color(0, 0, 0, 1.0).ptr(), 1);
					bgfx::setUniform(Renderer::getCameraClippingPlanesUniform(), glm::value_ptr(glm::vec4(0.1f, 100.0f, 0.0f, 0.0f)), 1);
					bgfx::setUniform(Renderer::getScreenParamsUniform(), glm::value_ptr(glm::vec4(rt->getWidth(), rt->getHeight(), 1.0f + 1.0f / rt->getWidth(), 1.0f + 1.0f / rt->getHeight())), 1);
				}
			);

			if (material->getShader() != nullptr)
			{
				if (material->getShader()->getRenderMode() == RenderMode::Deferred)
				{
					glm::mat4x4 geomProj = glm::orthoRH(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f);
					bgfx::setViewClear(viewId + 1, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0.0f);
					bgfx::setViewRect(viewId + 1, 0, 0, rt->getWidth(), rt->getHeight());
					bgfx::setViewFrameBuffer(viewId + 1, lightBuffer);
					bgfx::setViewTransform(viewId + 1, NULL, glm::value_ptr(geomProj));

					bgfx::setViewClear(viewId + 2, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0.0f);
					bgfx::setViewRect(viewId + 2, 0, 0, rt->getWidth(), rt->getHeight());
					bgfx::setViewFrameBuffer(viewId + 2, rt->getFrameBufferHandle());
					bgfx::setViewTransform(viewId + 2, NULL, glm::value_ptr(geomProj));

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
				}
			}

			//Renderer::getSingleton()->frame();

			float w = rt->getWidth();
			float h = rt->getHeight();
			float aspect = h / w;
			float aspectW = w / h;

			sz.y = sz.x * aspect;

			if (previewHeight < sz.y)
			{
				sz.y = previewHeight - 5;
				sz.x = sz.y * aspectW;
			}

			if (w < x)
				ImGui::SetCursorPosX(x / 2 - w / 2);
			if (h < prevHeight)
				ImGui::SetCursorPosY(prevHeight / 2 - h / 2);

			ImGui::Image((void*)rt->getColorTextureHandle().idx, sz, ImVec2(0, 1), ImVec2(1, 0));
		}
	}
}