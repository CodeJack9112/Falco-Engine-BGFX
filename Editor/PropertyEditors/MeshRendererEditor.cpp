#include "MeshRendererEditor.h"

#include "../Windows/MainWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/HierarchyWindow.h"
#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Components/MeshRenderer.h"
#include "../Engine/Renderer/BatchedGeometry.h"

#include "MaterialEditor.h"
#include "Property.h"
#include "PropEditorHost.h"
#include "PropButton.h"
#include "PropBool.h"
#include "PropComboBox.h"
#include "PropFloat.h"

#include "../Classes/Undo.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	MeshRendererEditor::MeshRendererEditor()
	{
		setEditorName("MeshRendererEditor");
	}

	MeshRendererEditor::~MeshRendererEditor()
	{
	}

	void MeshRendererEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		MeshRenderer* meshRenderer = (MeshRenderer*)comps[0];

		PropBool* castShadows = new PropBool(this, "Cast shadows", meshRenderer->getCastShadows());
		castShadows->setOnChangeCallback([=](Property* prop, bool val) { onChangeCastShadows(prop, val); });

		addProperty(castShadows);

		PropComboBox* lightmapSize = new PropComboBox(this, "Lightmap size", { "Default", "64", "128", "256", "512", "1024", "2048", "4096" });
		lightmapSize->setCurrentItem(meshRenderer->getLightmapSize());
		lightmapSize->setOnChangeCallback([=](Property* prop, int val) { onChangeLightmapSize(prop, val); });

		addProperty(lightmapSize);

		PropFloat* lodBias = new PropFloat(this, "LOD max distance", meshRenderer->getLodMaxDistance());
		lodBias->setMinValue(1.0f);
		lodBias->setMaxValue(200.0f);
		lodBias->setOnChangeCallback([=](Property* prop, float val) { onChangeLodMaxDistance(prop, val); });

		addProperty(lodBias);

		PropBool* cullOverMaxDistance = new PropBool(this, "Cull over max distance", meshRenderer->getCullOverMaxDistance());
		cullOverMaxDistance->setOnChangeCallback([=](Property* prop, bool val) { onChangeCullOverMaxDistance(prop, val); });

		addProperty(cullOverMaxDistance);

		//Bones
		if (meshRenderer->isSkinned())
		{
			Property* propSkin = new Property(this, "Skin");
			propSkin->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/skin.png", false, Texture::CompressionMethod::None, true);

			std::string root = "None";
			if (meshRenderer->getRootObject() != nullptr)
				root = meshRenderer->getRootObject()->getName();

			PropButton* skinRoot = new PropButton(this, "Root", root);
			skinRoot->setSupportedFormats({ "::GameObject" });
			skinRoot->setOnClickCallback([=](TreeNode* prop) { onClickSkinRoot(prop); });
			skinRoot->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropSkinRoot(prop, from); });
			skinRoot->setImage(MainWindow::loadEditorIcon("Hierarchy/empty.png"));

			propSkin->addChild(skinRoot);
			addProperty(propSkin);
		}

		if (isMaterialsIdentical())
			listMaterials();
	}

	bool MeshRendererEditor::isMaterialsIdentical()
	{
		bool result = true;

		MeshRenderer* entity = (MeshRenderer*)components[0];
		std::vector<Material*> materials;
		
		for (int i = 0; i < entity->getSharedMaterialsCount(); ++i)
			materials.push_back(entity->getSharedMaterial(i));

		auto end = std::unique(materials.begin(), materials.end(), [](Material* l, Material* r) {
				return l->getOrigin() == r->getOrigin();
		});

		materials.erase(end, materials.end());

		if (result)
		{
			for (auto it = components.begin(); it != components.end(); ++it)
			{
				if (!result)
					break;

				MeshRenderer* currEntity = (MeshRenderer*)*it;
				std::vector<Material*> currMaterials;
				for (int i = 0; i < currEntity->getSharedMaterialsCount(); ++i)
					currMaterials.push_back(currEntity->getSharedMaterial(i));

				auto _end = std::unique(currMaterials.begin(), currMaterials.end(), [](Material* l, Material* r) {
						return l->getOrigin() == r->getOrigin();
					});

				currMaterials.erase(_end, currMaterials.end());

				if (materials.size() != currMaterials.size())
				{
					result = false;
					currMaterials.clear();
					break;
				}

				int i = 0;
				for (std::vector<Material*>::iterator _it = materials.begin(); _it != materials.end(); ++_it, ++i)
				{
					Material* currMaterial = *_it;

					if (currMaterial != nullptr)
					{
						if (currMaterial->getOrigin() != currMaterials.at(i)->getOrigin())
						{
							result = false;
							currMaterials.clear();
							break;
						}
					}
					else
					{
						if (currMaterials.at(i) != nullptr)
						{
							result = false;
							currMaterials.clear();
							break;
						}
					}
				}

				currMaterials.clear();
			}
		}

		materials.clear();

		return result;
	}

	void MeshRendererEditor::listMaterials()
	{
		Property* materialsProp = new Property(this, "Materials");
		materialsProp->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/materials.png", false, Texture::CompressionMethod::None, true);
		addProperty(materialsProp);

		MeshRenderer* meshRenderer = (MeshRenderer*)components[0];

		//Materials
		int i = 0;
		std::vector<Material*> materials;
		for (int i = 0; i < meshRenderer->getSharedMaterialsCount(); ++i)
			materials.push_back(meshRenderer->getSharedMaterial(i));

		for (auto it = materials.begin(); it < materials.end(); ++it, ++i)
		{
			Material* mat = *it;

			std::string matName = "None";

			if (mat != nullptr)
				matName = IO::GetFileNameWithExt(mat->getName());

			PropButton* matBtn = new PropButton(this, "Material " + std::to_string(i), matName);
			matBtn->setSupportedFormats({ "material" });
			matBtn->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropMaterial(prop, from, i); });
			matBtn->setOnClickCallback([=](Property* prop)
				{
					if (mat != nullptr)
						MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(mat->getName());
				}
			);
			matBtn->setOnClearCallback([=](Property* prop)
				{
					onDropMaterial(prop, nullptr, i);
				}
			);
			matBtn->setImage(MainWindow::loadEditorIcon("Assets/material.png"));
			materialsProp->addChild(matBtn);
		}

		std::vector<Material*> _materials = materials;
		auto end = std::unique(_materials.begin(), _materials.end(), [](Material* l, Material* r) {
			return l->getOrigin() == r->getOrigin();
		});
		_materials.erase(end, _materials.end());

		for (auto it = _materials.begin(); it < _materials.end(); ++it)
		{
			Material* mat = *it;

			if (mat == nullptr)
				continue;

			std::string matLoc = mat->getLocation();
			std::string matName = IO::GetFileNameWithExt(mat->getName());

			if (matLoc == Engine::getSingleton()->getBuiltinResourcesPath())
				continue;

			if (matLoc.find("system/materials/") != std::string::npos)
				continue;

			MaterialEditor* materialEditor = new MaterialEditor();
			materialEditor->init(mat);
			PropEditorHost* mat_prop = new PropEditorHost(this, matName, materialEditor);
			mat_prop->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Assets/material.png", false, Texture::CompressionMethod::None, true);
			addProperty(mat_prop);
		}

		_materials.clear();
	}

	void MeshRendererEditor::onClickSkinRoot(TreeNode* prop)
	{
		MeshRenderer* meshRenderer = (MeshRenderer*)components[0];

		TreeView* tree = MainWindow::getSingleton()->getHierarchyWindow()->getTreeView();
		if (meshRenderer->getRootObject() != nullptr)
		{
			MainWindow::addOnEndUpdateCallback([=]()
				{
					TreeNode* node = tree->getNodeByName(meshRenderer->getRootObject()->getGuid(), tree->getRootNode());
					if (node != nullptr)
					{
						tree->selectNodes({ node->name }, false);
						tree->focusOnNode(node);
					}
				}
			);
		}
	}

	void MeshRendererEditor::onChangeCastShadows(Property* prop, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change mesh renderer cast shadows");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			bool updBatches = false;

			for (auto& d : data->boolData[0])
			{
				MeshRenderer* comp = (MeshRenderer*)d.first;
				comp->setCastShadows(d.second);

				if (comp->getGameObject()->getBatchingStatic())
					updBatches = true;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
		};

		undoData->redoAction = [=](UndoData* data)
		{
			bool updBatches = false;

			for (auto& d : data->boolData[1])
			{
				MeshRenderer* comp = (MeshRenderer*)d.first;
				comp->setCastShadows(d.second);

				if (comp->getGameObject()->getBatchingStatic())
					updBatches = true;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
		};
		//

		bool updBatches = false;

		for (auto& comp : components)
		{
			MeshRenderer* meshRenderer = ((MeshRenderer*)comp);

			undoData->boolData[0][meshRenderer] = meshRenderer->getCastShadows();
			undoData->boolData[1][meshRenderer] = value;

			meshRenderer->setCastShadows(value);

			if (meshRenderer->getGameObject()->getBatchingStatic())
				updBatches = true;
		}

		if (updBatches)
			MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
	}

	void MeshRendererEditor::onChangeLightmapSize(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change mesh renderer lightmap size");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				MeshRenderer* comp = (MeshRenderer*)d.first;
				comp->setLightmapSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				MeshRenderer* comp = (MeshRenderer*)d.first;
				comp->setLightmapSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto& comp : components)
		{
			MeshRenderer* meshRenderer = ((MeshRenderer*)comp);

			undoData->intData[0][meshRenderer] = meshRenderer->getLightmapSize();
			undoData->intData[1][meshRenderer] = value;

			meshRenderer->setLightmapSize(value);
		}
	}

	void MeshRendererEditor::onChangeLodMaxDistance(Property* prop, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change mesh renderer LOD max distance");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				MeshRenderer* comp = (MeshRenderer*)d.first;
				comp->setLodMaxDistance(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				MeshRenderer* comp = (MeshRenderer*)d.first;
				comp->setLodMaxDistance(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto& comp : components)
		{
			MeshRenderer* meshRenderer = ((MeshRenderer*)comp);

			undoData->floatData[0][meshRenderer] = meshRenderer->getLodMaxDistance();
			undoData->floatData[1][meshRenderer] = value;

			meshRenderer->setLodMaxDistance(value);
		}
	}

	void MeshRendererEditor::onChangeCullOverMaxDistance(Property* prop, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change mesh renderer cull over max distance");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				MeshRenderer* comp = (MeshRenderer*)d.first;
				comp->setCullOverMaxDistance(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				MeshRenderer* comp = (MeshRenderer*)d.first;
				comp->setCullOverMaxDistance(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto& comp : components)
		{
			MeshRenderer* meshRenderer = ((MeshRenderer*)comp);

			undoData->boolData[0][meshRenderer] = meshRenderer->getCullOverMaxDistance();
			undoData->boolData[1][meshRenderer] = value;

			meshRenderer->setCullOverMaxDistance(value);
		}
	}

	void MeshRendererEditor::onDropSkinRoot(TreeNode* prop, TreeNode* from)
	{
		((PropButton*)prop)->setValue(from->alias);

		//Undo
		UndoData* undoData = Undo::addUndo("Change mesh renderer skin root");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				MeshRenderer* comp = (MeshRenderer*)d.first;
				GameObject* obj = Engine::getSingleton()->getGameObject(d.second);
				comp->setRootObject(obj);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[1])
			{
				MeshRenderer* comp = (MeshRenderer*)d.first;
				GameObject* obj = Engine::getSingleton()->getGameObject(d.second);
				comp->setRootObject(obj);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto& comp : components)
		{
			MeshRenderer* meshRenderer = ((MeshRenderer*)comp);

			undoData->stringData[0][meshRenderer] = meshRenderer->getRootObjectGuid();
			undoData->stringData[1][meshRenderer] = from->name;

			GameObject* obj = Engine::getSingleton()->getGameObject(from->name);
			meshRenderer->setRootObject(obj);
		}
	}

	void MeshRendererEditor::onDropMaterial(TreeNode* prop, TreeNode* from, int index)
	{
		Material* mt = nullptr;
		std::string path = "";

		if (from != nullptr)
		{
			path = from->getPath();
			mt = Material::load(Engine::getSingleton()->getAssetsPath(), path);
			((PropButton*)prop)->setValue(IO::GetFileNameWithExt(mt->getName()));
		}
		else
		{
			((PropButton*)prop)->setValue("None");
		}

		//Undo
		UndoData* undoData = Undo::addUndo("Set mesh renderer material");
		undoData->stringData.resize(4);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			
			bool updBatches = false;

			for (auto& d : data->stringData[0])
			{
				MeshRenderer* rend = (MeshRenderer*)d.first;
				Material* mat = Material::load(data->stringData[0][rend], data->stringData[1][rend]);
				rend->setSharedMaterial(idx, mat);

				if (rend->getGameObject()->getBatchingStatic())
					updBatches = true;
			}

			MainWindow::getSingleton()->getInspectorWindow()->updateCurrentEditor();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			
			bool updBatches = false;

			for (auto& d : data->stringData[2])
			{
				MeshRenderer* rend = (MeshRenderer*)d.first;
				Material* mat = Material::load(data->stringData[2][rend], data->stringData[3][rend]);
				rend->setSharedMaterial(idx, mat);

				if (rend->getGameObject()->getBatchingStatic())
					updBatches = true;
			}

			MainWindow::getSingleton()->getInspectorWindow()->updateCurrentEditor();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
		};
		//

		undoData->intData[0][nullptr] = index;

		bool updBatches = false;

		for (auto& comp : components)
		{
			MeshRenderer* meshRenderer = (MeshRenderer*)comp;

			Material* prevMat = meshRenderer->getSharedMaterial(index);

			undoData->stringData[0][meshRenderer] = prevMat != nullptr ? prevMat->getLocation() : "";
			undoData->stringData[1][meshRenderer] = prevMat != nullptr ? prevMat->getName() : "";

			undoData->stringData[2][meshRenderer] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][meshRenderer] = path;

			meshRenderer->setSharedMaterial(index, mt);

			if (comp->getGameObject()->getBatchingStatic())
				updBatches = true;
		}

		updateEditor();

		if (updBatches)
			MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
	}
}