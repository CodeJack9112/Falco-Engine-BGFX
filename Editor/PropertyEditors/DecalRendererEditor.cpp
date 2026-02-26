#include "DecalRendererEditor.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Components/DecalRenderer.h"
#include "../Engine/Assets/Asset.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Assets/Texture.h"

#include "../Classes/Undo.h"
#include "../Engine/Classes/IO.h"
#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"
#include "../Windows/AssetsWindow.h"

#include "MaterialEditor.h"
#include "PropEditorHost.h"

#include "PropVector3.h"
#include "PropAsset.h"
#include "PropColorPicker.h"

namespace GX
{
	DecalRendererEditor::DecalRendererEditor()
	{
		setEditorName("DecalRendererEditor");
	}

	DecalRendererEditor::~DecalRendererEditor()
	{
	}

	void DecalRendererEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		DecalRenderer* decal = (DecalRenderer*)comps[0];

		Texture* matIcon = MainWindow::loadEditorIcon("Assets/material.png");

		PropAsset* material = new PropAsset(this, "Material", decal->getMaterial());
		material->setSupportedFormats({ "material" });
		material->setImage(matIcon);
		material->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onChangeMaterial(prop, from); });
		material->setOnClearCallback([=](Property* prop) { onClearMaterial(prop); });
		material->setOnClickCallback([=](Property* prop) {
			if (decal->getMaterial() != nullptr)
				MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(decal->getMaterial()->getName());
			});

		addProperty(material);

		if (decal->getMaterial() == nullptr)
			return;

		Material* mat = decal->getMaterial();

		std::string matLoc = mat->getLocation();
		std::string matName = IO::GetFileNameWithExt(mat->getName());

		if (matLoc == Engine::getSingleton()->getBuiltinResourcesPath())
			return;

		if (matLoc.find("system/materials/") != std::string::npos)
			return;

		MaterialEditor* materialEditor = new MaterialEditor();
		materialEditor->init(mat);
		PropEditorHost* mat_prop = new PropEditorHost(this, matName, materialEditor);
		mat_prop->icon = matIcon;
		addProperty(mat_prop);
	}

	void DecalRendererEditor::onChangeMaterial(TreeNode* prop, TreeNode* from)
	{
		std::string fullPath = from->getPath();
		Material* material = Material::load(Engine::getSingleton()->getAssetsPath(), fullPath);

		((PropAsset*)prop)->setValue(material);

		//Undo
		UndoData* undoData = Undo::addUndo("Change decal material");
		undoData->object2Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->object2Data[0])
			{
				DecalRenderer* comp = (DecalRenderer*)d.first;
				comp->setMaterial((Material*)d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->object2Data[1])
			{
				DecalRenderer* comp = (DecalRenderer*)d.first;
				comp->setMaterial((Material*)d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			DecalRenderer* comp = (DecalRenderer*)(*it);

			undoData->object2Data[0][comp] = comp->getMaterial();
			undoData->object2Data[1][comp] = material;

			comp->setMaterial(material);
		}

		MainWindow::getInspectorWindow()->updateCurrentEditor();
	}

	void DecalRendererEditor::onClearMaterial(Property* prop)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Remove decal material");
		undoData->object2Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->object2Data[0])
			{
				DecalRenderer* comp = (DecalRenderer*)d.first;
				comp->setMaterial((Material*)d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->object2Data[1])
			{
				DecalRenderer* comp = (DecalRenderer*)d.first;
				comp->setMaterial((Material*)d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		((PropAsset*)prop)->setValue(nullptr);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			DecalRenderer* comp = (DecalRenderer*)(*it);

			undoData->object2Data[0][comp] = comp->getMaterial();
			undoData->object2Data[1][comp] = nullptr;

			comp->setMaterial(nullptr);
		}

		MainWindow::getInspectorWindow()->updateCurrentEditor();
	}
}