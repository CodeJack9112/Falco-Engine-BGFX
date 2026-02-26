#include "CSGBrushEditor.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Components/Component.h"
#include "../Engine/Components/CSGBrush.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Classes/StringConverter.h"
#include "../Engine/Renderer/CSGGeometry.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Renderer/Primitives.h"
#include "../Engine/Math/Mathf.h"

#include "PropBool.h"
#include "PropInt.h"
#include "PropFloat.h"
#include "PropComboBox.h"
#include "PropAsset.h"
#include "PropVector2.h"

#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/HierarchyWindow.h"

#include "../Classes/Undo.h"

namespace GX
{
	CSGBrushEditor::CSGBrushEditor()
	{
		setEditorName("CSGBrushEditor");

		cbId = Renderer::getSingleton()->addRenderCallback([=](int viewId, int viewLayer, Camera* camera) { renderGizmos(viewId, viewLayer, camera); });
	}

	CSGBrushEditor::~CSGBrushEditor()
	{
		Renderer::getSingleton()->removeRenderCallback(cbId);

		faceProps.clear();
	}

	void CSGBrushEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		CSGBrush* comp = (CSGBrush*)comps[0];
		auto& faces = comp->getFaces();

		Texture* matIcon = MainWindow::loadEditorIcon("Assets/material.png");

		PropComboBox* brushType = new PropComboBox(this, "Type", { "Cube", "Sphere", "Cone", "Cylinder" });
		brushType->setCurrentItem(static_cast<int>(comp->getBrushType()));
		brushType->setOnChangeCallback([=](Property* prop, int val) { onChangeBrushType(prop, val); });
		addProperty(brushType);

		PropComboBox* operation = new PropComboBox(this, "Operation", { "Add", "Subtract" });
		operation->setCurrentItem(static_cast<int>(comp->getBrushOperation()));
		operation->setOnChangeCallback([=](Property* prop, int val) { onChangeBrushOperation(prop, val); });
		addProperty(operation);

		PropBool* castShadows = new PropBool(this, "Cast shadows", comp->getCastShadows());
		castShadows->setOnChangeCallback([=](Property* prop, bool val) { onChangeCastShadows(prop, val); });
		addProperty(castShadows);

		if (comp->getBrushType() == CSGBrush::BrushType::Sphere ||
			comp->getBrushType() == CSGBrush::BrushType::Cone ||
			comp->getBrushType() == CSGBrush::BrushType::Cylinder)
		{
			PropInt* numSegments = new PropInt(this, "Segments", comp->getNumSegments());
			numSegments->setOnChangeCallback([=](Property* prop, int val) { onChangeNumSegments(prop, val); });
			addProperty(numSegments);
		}

		if (comp->getBrushType() == CSGBrush::BrushType::Sphere)
		{
			PropInt* numStacks = new PropInt(this, "Stacks", comp->getNumStacks());
			numStacks->setOnChangeCallback([=](Property* prop, int val) { onChangeNumStacks(prop, val); });
			addProperty(numStacks);
		}

		Property* brushProp = new Property(this, "Brush");

		Material* mmat = nullptr;
		glm::vec2 mtexCoordsScale = glm::vec2(1.0f);
		glm::vec2 mtexCoordsOffset = glm::vec2(0.0f);
		float mtexCoordsRotation = 0.0f;
		bool msmoothNormals = false;

		if (faces.size() > 0)
		{
			mmat = faces[0].material;
			mtexCoordsScale = faces[0].texCoordsScale;
			mtexCoordsOffset = faces[0].texCoordsOffset;
			mtexCoordsRotation = faces[0].texCoordsRotation;
			msmoothNormals = faces[0].smoothNormals;
		}

		PropAsset* masterMaterial = new PropAsset(this, "Material", mmat);
		masterMaterial->setSupportedFormats({ "material" });
		masterMaterial->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropMasterMaterial(from); });
		masterMaterial->setOnClickCallback([=](Property* prop)
			{
				if (mmat != nullptr)
					MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(mmat->getName());
			}
		);
		masterMaterial->setOnClearCallback([=](Property* prop)
			{
				onDropMasterMaterial(nullptr);
			}
		);
		brushProp->addChild(masterMaterial);

		PropVector2* masterUvScale = new PropVector2(this, "UV Scale", mtexCoordsScale);
		masterUvScale->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeMasterUVScale(prop, val); });
		brushProp->addChild(masterUvScale);

		PropVector2* masterUvOffset = new PropVector2(this, "UV Offset", mtexCoordsOffset);
		masterUvOffset->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeMasterUVOffset(prop, val); });
		brushProp->addChild(masterUvOffset);

		PropFloat* masterUvRotation = new PropFloat(this, "UV Rotation", mtexCoordsRotation);
		masterUvRotation->setOnChangeCallback([=](Property* prop, float val) { onChangeMasterUVRotation(prop, val); });
		brushProp->addChild(masterUvRotation);

		if (comp->getBrushType() != CSGBrush::BrushType::Cube)
		{
			PropBool* masterSmoothNormals = new PropBool(this, "Smooth normals", msmoothNormals);
			masterSmoothNormals->setOnChangeCallback([=](Property* prop, bool val) { onChangeMasterSmoothNormals(prop, val); });
			brushProp->addChild(masterSmoothNormals);
		}

		addProperty(brushProp);

		updateFaceProps();
	}

	void CSGBrushEditor::updateFaceProps()
	{
		CSGBrush* comp = (CSGBrush*)components[0];
		auto& faces = comp->getFaces();

		faceProps.clear();
		if (facesProp != nullptr)
			removeProperty(facesProp);

		facesProp = nullptr;

		if (comp->getBrushType() == CSGBrush::BrushType::Cube)
		{
			facesProp = new Property(this, "Faces");

			int idx = 0;
			for (auto face : faces)
			{
				Material* mat = face.material;

				std::string name = "Face " + std::to_string(idx);

				Property* faceProp = new Property(this, name);

				PropAsset* material = new PropAsset(this, "Material", mat);
				material->setSupportedFormats({ "material" });
				material->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropMaterial(from, idx); });
				material->setOnClickCallback([=](Property* prop)
					{
						if (mat != nullptr)
							MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(mat->getName());
					}
				);
				material->setOnClearCallback([=](Property* prop)
					{
						onDropMaterial(nullptr, idx);
					}
				);

				faceProp->addChild(material);

				if (comp->getBrushType() == CSGBrush::BrushType::Custom)
				{
					Property* uvsProp = new Property(this, "Texture coordinates");

					for (int i = 0; i < face.texCoords.size(); ++i)
					{
						PropVector2* uv = new PropVector2(this, "UV " + std::to_string(i), face.texCoords[i]);
						uv->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeUV(prop, val, idx, i); });
						uvsProp->addChild(uv);
					}

					faceProp->addChild(uvsProp);
				}

				PropVector2* uvScale = new PropVector2(this, "UV Scale", face.texCoordsScale);
				uvScale->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeUVScale(prop, val, idx); });
				faceProp->addChild(uvScale);

				PropVector2* uvOffset = new PropVector2(this, "UV Offset", face.texCoordsOffset);
				uvOffset->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeUVOffset(prop, val, idx); });
				faceProp->addChild(uvOffset);

				PropFloat* uvRotation = new PropFloat(this, "UV Rotation", face.texCoordsRotation);
				uvRotation->setOnChangeCallback([=](Property* prop, float val) { onChangeUVRotation(prop, val, idx); });
				faceProp->addChild(uvRotation);

				facesProp->addChild(faceProp);

				faceProps.push_back(faceProp);

				++idx;
			}

			addProperty(facesProp);
		}
		else if (comp->getBrushType() == CSGBrush::BrushType::Cylinder)
		{
			facesProp = new Property(this, "Faces");

			int nSegs = comp->getNumSegments();
			int idx = nSegs * 2;

			Material* mat1 = faces[0].material;

			PropAsset* topCapMaterial = new PropAsset(this, "Top material", mat1);
			topCapMaterial->setSupportedFormats({ "material" });
			topCapMaterial->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropMaterialRange(from, 0, nSegs); });
			topCapMaterial->setOnClickCallback([=](Property* prop)
				{
					if (mat1 != nullptr)
						MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(mat1->getName());
				}
			);
			topCapMaterial->setOnClearCallback([=](Property* prop)
				{
					onDropMaterialRange(nullptr, 0, nSegs);
				}
			);

			facesProp->addChild(topCapMaterial);

			Material* mat2 = faces[idx].material;

			PropAsset* bottomCapMaterial = new PropAsset(this, "Bottom material", mat2);
			bottomCapMaterial->setSupportedFormats({ "material" });
			bottomCapMaterial->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropMaterialRange(from, idx, idx + nSegs); });
			bottomCapMaterial->setOnClickCallback([=](Property* prop)
				{
					if (mat2 != nullptr)
						MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(mat2->getName());
				}
			);
			bottomCapMaterial->setOnClearCallback([=](Property* prop)
				{
					onDropMaterialRange(nullptr, idx, idx + nSegs);
				}
			);

			facesProp->addChild(bottomCapMaterial);

			Material* mat3 = faces[nSegs].material;

			PropAsset* mainMaterial = new PropAsset(this, "Main material", mat3);
			mainMaterial->setSupportedFormats({ "material" });
			mainMaterial->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropMaterialRange(from, nSegs, idx); });
			mainMaterial->setOnClickCallback([=](Property* prop)
				{
					if (mat3 != nullptr)
						MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(mat3->getName());
				}
			);
			mainMaterial->setOnClearCallback([=](Property* prop)
				{
					onDropMaterialRange(nullptr, nSegs, idx);
				}
			);

			facesProp->addChild(mainMaterial);

			addProperty(facesProp);
		}
	}

	void CSGBrushEditor::onChangeBrushType(Property* prop, int val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush type");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->intData[0])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setBrushType(static_cast<CSGBrush::BrushType>(d.second));

				comps.push_back(brush);

				auto& faces = brush->getFaces();

				for (int i = 0; i < faces.size(); ++i)
				{
					std::string loc = data->string2Data[brush][i * 2];
					std::string name = data->string2Data[brush][i * 2 + 1];

					if (!loc.empty() && !name.empty())
					{
						Material* mat = Material::load(loc, name);
						brush->setMaterial(i, mat);
					}
					else
					{
						brush->setMaterial(i, nullptr);
					}

					brush->setUVOffset(i, data->vec22Data[brush][i * 2]);
					brush->setUVScale(i, data->vec22Data[brush][i * 2 + 1]);
					brush->setUVRotation(i, data->float2Data[brush][i]);
				}

				MainWindow::getHierarchyWindow()->updateNodeIcon(brush->getGameObject()->getTransform());
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->intData[1])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setBrushType(static_cast<CSGBrush::BrushType>(d.second));

				comps.push_back(brush);

				MainWindow::getHierarchyWindow()->updateNodeIcon(brush->getGameObject()->getTransform());
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CSGBrush* brush = (CSGBrush*)*it;

			auto& faces = brush->getFaces();

			undoData->string2Data[brush].resize(faces.size() * 2);
			undoData->vec22Data[brush].resize(faces.size() * 2);
			undoData->float2Data[brush].resize(faces.size());

			undoData->intData[0][brush] = static_cast<int>(brush->getBrushType());
			undoData->intData[1][brush] = val;

			for (int f = 0; f < faces.size(); ++f)
			{
				if (faces[f].material != nullptr)
				{
					undoData->string2Data[brush][f * 2] = faces[f].material->getLocation();
					undoData->string2Data[brush][f * 2 + 1] = faces[f].material->getName();
				}
				else
				{
					undoData->string2Data[brush][f * 2] = "";
					undoData->string2Data[brush][f * 2 + 1] = "";
				}

				undoData->vec22Data[brush][f * 2] = faces[f].texCoordsOffset;
				undoData->vec22Data[brush][f * 2 + 1] = faces[f].texCoordsScale;
				undoData->float2Data[brush][f] = faces[f].texCoordsRotation;
			}

			brush->setBrushType(static_cast<CSGBrush::BrushType>(val));

			MainWindow::getHierarchyWindow()->updateNodeIcon(brush->getGameObject()->getTransform());
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			MainWindow::getInspectorWindow()->updateCurrentEditor();
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onChangeBrushOperation(Property* prop, int val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush operation");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->intData[0])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setBrushOperation(static_cast<CSGBrush::BrushOperation>(d.second));

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->intData[1])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setBrushOperation(static_cast<CSGBrush::BrushOperation>(d.second));

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CSGBrush* brush = (CSGBrush*)*it;

			undoData->intData[0][brush] = static_cast<int>(brush->getBrushOperation());
			undoData->intData[1][brush] = val;

			brush->setBrushOperation(static_cast<CSGBrush::BrushOperation>(val));
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			MainWindow::getInspectorWindow()->updateCurrentEditor();
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onChangeCastShadows(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush cast shadows");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->boolData[0])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setCastShadows(d.second);

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->boolData[1])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setCastShadows(d.second);

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CSGBrush* brush = (CSGBrush*)*it;

			undoData->boolData[0][brush] = brush->getCastShadows();
			undoData->boolData[1][brush] = val;

			brush->setCastShadows(val);
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			MainWindow::getInspectorWindow()->updateCurrentEditor();
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onChangeNumSegments(Property* prop, int val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush segments count");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->intData[0])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setNumSegments(d.second);

				comps.push_back(brush);

				auto& faces = brush->getFaces();

				for (int i = 0; i < faces.size(); ++i)
				{
					std::string loc = data->string2Data[brush][i * 2];
					std::string name = data->string2Data[brush][i * 2 + 1];

					if (!loc.empty() && !name.empty())
					{
						Material* mat = Material::load(loc, name);
						brush->setMaterial(i, mat);
					}
					else
					{
						brush->setMaterial(i, nullptr);
					}

					brush->setUVOffset(i, data->vec22Data[brush][i * 2]);
					brush->setUVScale(i, data->vec22Data[brush][i * 2 + 1]);
					brush->setUVRotation(i, data->float2Data[brush][i]);
				}
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->intData[1])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setNumSegments(d.second);

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CSGBrush* brush = (CSGBrush*)*it;

			auto& faces = brush->getFaces();

			undoData->string2Data[brush].resize(faces.size() * 2);
			undoData->vec22Data[brush].resize(faces.size() * 2);
			undoData->float2Data[brush].resize(faces.size());

			undoData->intData[0][brush] = brush->getNumSegments();
			undoData->intData[1][brush] = val;

			for (int f = 0; f < faces.size(); ++f)
			{
				if (faces[f].material != nullptr)
				{
					undoData->string2Data[brush][f * 2] = faces[f].material->getLocation();
					undoData->string2Data[brush][f * 2 + 1] = faces[f].material->getName();
				}
				else
				{
					undoData->string2Data[brush][f * 2] = "";
					undoData->string2Data[brush][f * 2 + 1] = "";
				}

				undoData->vec22Data[brush][f * 2] = faces[f].texCoordsOffset;
				undoData->vec22Data[brush][f * 2 + 1] = faces[f].texCoordsScale;
				undoData->float2Data[brush][f] = faces[f].texCoordsRotation;
			}

			brush->setNumSegments(val);
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			MainWindow::getInspectorWindow()->updateCurrentEditor();
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onChangeNumStacks(Property* prop, int val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush stacks count");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->intData[0])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setNumStacks(d.second);

				comps.push_back(brush);

				auto& faces = brush->getFaces();

				for (int i = 0; i < faces.size(); ++i)
				{
					std::string loc = data->string2Data[brush][i * 2];
					std::string name = data->string2Data[brush][i * 2 + 1];

					if (!loc.empty() && !name.empty())
					{
						Material* mat = Material::load(loc, name);
						brush->setMaterial(i, mat);
					}
					else
					{
						brush->setMaterial(i, nullptr);
					}

					brush->setUVOffset(i, data->vec22Data[brush][i * 2]);
					brush->setUVScale(i, data->vec22Data[brush][i * 2 + 1]);
					brush->setUVRotation(i, data->float2Data[brush][i]);
				}
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->intData[1])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setNumStacks(d.second);

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CSGBrush* brush = (CSGBrush*)*it;

			auto& faces = brush->getFaces();

			undoData->string2Data[brush].resize(faces.size() * 2);
			undoData->vec22Data[brush].resize(faces.size() * 2);
			undoData->float2Data[brush].resize(faces.size());

			undoData->intData[0][brush] = brush->getNumStacks();
			undoData->intData[1][brush] = val;

			for (int f = 0; f < faces.size(); ++f)
			{
				if (faces[f].material != nullptr)
				{
					undoData->string2Data[brush][f * 2] = faces[f].material->getLocation();
					undoData->string2Data[brush][f * 2 + 1] = faces[f].material->getName();
				}
				else
				{
					undoData->string2Data[brush][f * 2] = "";
					undoData->string2Data[brush][f * 2 + 1] = "";
				}

				undoData->vec22Data[brush][f * 2] = faces[f].texCoordsOffset;
				undoData->vec22Data[brush][f * 2 + 1] = faces[f].texCoordsScale;
				undoData->float2Data[brush][f] = faces[f].texCoordsRotation;
			}

			brush->setNumStacks(val);
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			MainWindow::getInspectorWindow()->updateCurrentEditor();
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onChangeUV(Property* prop, glm::vec2 val, int faceId, int vertId)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush face texture coordinates");
		undoData->vec2Data.resize(2);
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->vec2Data[0])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setUV(data->intData[0][nullptr], data->intData[1][nullptr], d.second);

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->vec2Data[1])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setUV(data->intData[0][nullptr], data->intData[1][nullptr], d.second);

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		undoData->intData[0][nullptr] = faceId;
		undoData->intData[1][nullptr] = vertId;

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CSGBrush* brush = (CSGBrush*)*it;

			undoData->vec2Data[0][brush] = brush->getUV(faceId, vertId);
			undoData->vec2Data[1][brush] = val;

			brush->setUV(faceId, vertId, val);
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onChangeUVScale(Property* prop, glm::vec2 val, int faceId)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush face UV coords scale");
		undoData->vec2Data.resize(2);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->vec2Data[0])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setUVScale(data->intData[0][nullptr], d.second);

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->vec2Data[1])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setUVScale(data->intData[0][nullptr], d.second);

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		undoData->intData[0][nullptr] = faceId;

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CSGBrush* brush = (CSGBrush*)*it;

			undoData->vec2Data[0][brush] = brush->getUVScale(faceId);
			undoData->vec2Data[1][brush] = val;

			brush->setUVScale(faceId, val);
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onChangeUVOffset(Property* prop, glm::vec2 val, int faceId)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush face UV coords offset");
		undoData->vec2Data.resize(2);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->vec2Data[0])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setUVOffset(data->intData[0][nullptr], d.second);

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->vec2Data[1])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setUVOffset(data->intData[0][nullptr], d.second);

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		undoData->intData[0][nullptr] = faceId;

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CSGBrush* brush = (CSGBrush*)*it;

			undoData->vec2Data[0][brush] = brush->getUVOffset(faceId);
			undoData->vec2Data[1][brush] = val;

			brush->setUVOffset(faceId, val);
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onChangeUVRotation(Property* prop, float val, int faceId)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush face UV coords rotation");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->floatData[0])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setUVRotation(data->intData[0][nullptr], d.second);

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->floatData[1])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				brush->setUVRotation(data->intData[0][nullptr], d.second);

				comps.push_back(brush);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		undoData->intData[0][nullptr] = faceId;

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			CSGBrush* brush = (CSGBrush*)*it;

			undoData->floatData[0][brush] = brush->getUVRotation(faceId);
			undoData->floatData[1][brush] = val;

			brush->setUVRotation(faceId, val);
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onDropMaterial(TreeNode* from, int matIdx)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush material");
		undoData->stringData.resize(4);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->stringData[0])
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				comps.push_back(brush);

				if (!d.second.empty())
				{
					Material* mat = Material::load(data->stringData[2][brush], d.second);
					brush->setMaterial(data->intData[0][nullptr], mat);
				}
				else
				{
					brush->setMaterial(data->intData[0][nullptr], nullptr);
				}
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->stringData[1])
			{
				CSGBrush* brush = (CSGBrush*)d.first;

				comps.push_back(brush);
				
				if (!d.second.empty())
				{
					Material* mat = Material::load(data->stringData[3][brush], d.second);
					brush->setMaterial(data->intData[0][nullptr], mat);
				}
				else
				{
					brush->setMaterial(data->intData[0][nullptr], nullptr);
				}
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		std::string path = "";
		if (from != nullptr)
			path = from->getPath();

		Material* mat = Material::load(Engine::getSingleton()->getAssetsPath(), path);

		undoData->intData[0][nullptr] = matIdx;

		for (auto comp : components)
		{
			CSGBrush* brush = (CSGBrush*)comp;
			auto& faces = brush->getFaces();
			std::string prevMat = "";
			std::string prevLoc = "";

			if (faces[matIdx].material != nullptr)
			{
				prevLoc = faces[matIdx].material->getLocation();
				prevMat = faces[matIdx].material->getName();
			}

			undoData->stringData[0][brush] = prevMat;
			undoData->stringData[1][brush] = path;
			undoData->stringData[2][brush] = prevLoc;
			undoData->stringData[3][brush] = Engine::getSingleton()->getAssetsPath();

			brush->setMaterial(matIdx, mat);
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			MainWindow::getInspectorWindow()->updateCurrentEditor();
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onDropMasterMaterial(TreeNode* from)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush material");
		
		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->string2Data)
			{
				CSGBrush* brush = (CSGBrush*)d.first;

				comps.push_back(brush);

				auto& faces = brush->getFaces();

				for (int i = 0; i < faces.size(); ++i)
				{
					std::string loc = data->string2Data[brush][i * 2];
					std::string name = data->string2Data[brush][i * 2 + 1];

					if (!loc.empty() && !name.empty())
					{
						Material* mat = Material::load(loc, name);
						brush->setMaterial(i, mat);
					}
					else
					{
						brush->setMaterial(i, nullptr);
					}
				}
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->string2Data)
			{
				CSGBrush* brush = (CSGBrush*)d.first;

				comps.push_back(brush);

				if (!d.second.empty())
				{
					Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
					auto& faces = brush->getFaces();
					for (int i = 0; i < faces.size(); ++i)
						brush->setMaterial(i, mat);
				}
				else
				{
					auto& faces = brush->getFaces();
					for (int i = 0; i < faces.size(); ++i)
						brush->setMaterial(i, nullptr);
				}
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		std::string path = "";
		if (from != nullptr)
			path = from->getPath();

		Material* mat = Material::load(Engine::getSingleton()->getAssetsPath(), path);

		undoData->stringData.resize(2);

		for (auto comp : components)
		{
			CSGBrush* brush = (CSGBrush*)comp;

			auto& faces = brush->getFaces();

			undoData->string2Data[brush].resize(faces.size() * 2);

			undoData->stringData[0][nullptr] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[1][nullptr] = path;
			
			for (int f = 0; f < faces.size(); ++f)
			{
				if (faces[f].material != nullptr)
				{
					undoData->string2Data[brush][f * 2] = faces[f].material->getLocation();
					undoData->string2Data[brush][f * 2 + 1] = faces[f].material->getName();
				}
				else
				{
					undoData->string2Data[brush][f * 2] = "";
					undoData->string2Data[brush][f * 2 + 1] = "";
				}

				brush->setMaterial(f, mat);
			}
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			MainWindow::getInspectorWindow()->updateCurrentEditor();
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onDropMaterialRange(TreeNode* from, int start, int end)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush material");

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			int s = data->intData[0][nullptr];
			int e = data->intData[1][nullptr];

			for (auto& d : data->string2Data)
			{
				CSGBrush* brush = (CSGBrush*)d.first;

				comps.push_back(brush);

				auto& faces = brush->getFaces();

				for (int i = s; i < e; ++i)
				{
					std::string loc = data->string2Data[brush][i * 2];
					std::string name = data->string2Data[brush][i * 2 + 1];

					if (!loc.empty() && !name.empty())
					{
						Material* mat = Material::load(loc, name);
						brush->setMaterial(i, mat);
					}
					else
					{
						brush->setMaterial(i, nullptr);
					}
				}
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			int s = data->intData[0][nullptr];
			int e = data->intData[1][nullptr];

			for (auto& d : data->string2Data)
			{
				CSGBrush* brush = (CSGBrush*)d.first;

				comps.push_back(brush);

				if (!d.second.empty())
				{
					Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
					auto& faces = brush->getFaces();
					for (int i = s; i < e; ++i)
						brush->setMaterial(i, mat);
				}
				else
				{
					auto& faces = brush->getFaces();
					for (int i = s; i < e; ++i)
						brush->setMaterial(i, nullptr);
				}
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		std::string path = "";
		if (from != nullptr)
			path = from->getPath();

		Material* mat = Material::load(Engine::getSingleton()->getAssetsPath(), path);

		undoData->stringData.resize(2);
		undoData->intData.resize(2);

		undoData->stringData[0][nullptr] = Engine::getSingleton()->getAssetsPath();
		undoData->stringData[1][nullptr] = path;

		undoData->intData[0][nullptr] = start;
		undoData->intData[1][nullptr] = end;

		for (auto comp : components)
		{
			CSGBrush* brush = (CSGBrush*)comp;

			auto& faces = brush->getFaces();

			undoData->string2Data[brush].resize(faces.size() * 2);

			for (int f = start; f < end; ++f)
			{
				if (faces[f].material != nullptr)
				{
					undoData->string2Data[brush][f * 2] = faces[f].material->getLocation();
					undoData->string2Data[brush][f * 2 + 1] = faces[f].material->getName();
				}
				else
				{
					undoData->string2Data[brush][f * 2] = "";
					undoData->string2Data[brush][f * 2 + 1] = "";
				}

				brush->setMaterial(f, mat);
			}
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			MainWindow::getInspectorWindow()->updateCurrentEditor();
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onChangeMasterUVScale(Property* prop, glm::vec2 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush UV coords scale");

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->vec22Data)
			{
				CSGBrush* brush = (CSGBrush*)d.first;

				comps.push_back(brush);
				
				auto& faces = brush->getFaces();

				for (int i = 0; i < faces.size(); ++i)
					brush->setUVScale(i, data->vec22Data[brush][i]);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->vec22Data)
			{
				CSGBrush* brush = (CSGBrush*)d.first;
				comps.push_back(brush);

				auto& faces = brush->getFaces();

				for (int i = 0; i < faces.size(); ++i)
					brush->setUVScale(i, data->vec2Data[0][nullptr]);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		undoData->vec2Data.resize(1);
		undoData->vec2Data[0][nullptr] = val;

		for (auto comp : components)
		{
			CSGBrush* brush = (CSGBrush*)comp;

			auto& faces = brush->getFaces();
			undoData->vec22Data[brush].resize(faces.size());

			for (int f = 0; f < faces.size(); ++f)
			{
				undoData->vec22Data[brush][f] = faces[f].texCoordsScale;

				brush->setUVScale(f, val);
			}
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			updateFaceProps();
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onChangeMasterUVOffset(Property* prop, glm::vec2 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush UV coords offset");

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->vec22Data)
			{
				CSGBrush* brush = (CSGBrush*)d.first;

				comps.push_back(brush);

				auto& faces = brush->getFaces();

				for (int i = 0; i < faces.size(); ++i)
					brush->setUVOffset(i, data->vec22Data[brush][i]);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->vec22Data)
			{
				CSGBrush* brush = (CSGBrush*)d.first;

				comps.push_back(brush);

				auto& faces = brush->getFaces();

				for (int i = 0; i < faces.size(); ++i)
					brush->setUVOffset(i, data->vec2Data[0][nullptr]);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		undoData->vec2Data.resize(1);
		undoData->vec2Data[0][nullptr] = val;

		for (auto comp : components)
		{
			CSGBrush* brush = (CSGBrush*)comp;

			auto& faces = brush->getFaces();
			undoData->vec22Data[brush].resize(faces.size());

			for (int f = 0; f < faces.size(); ++f)
			{
				undoData->vec22Data[brush][f] = faces[f].texCoordsOffset;

				brush->setUVOffset(f, val);
			}
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			updateFaceProps();
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onChangeMasterUVRotation(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush UV coords rotation");

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->float2Data)
			{
				CSGBrush* brush = (CSGBrush*)d.first;

				comps.push_back(brush);

				auto& faces = brush->getFaces();

				for (int i = 0; i < faces.size(); ++i)
					brush->setUVRotation(i, data->float2Data[brush][i]);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->float2Data)
			{
				CSGBrush* brush = (CSGBrush*)d.first;

				comps.push_back(brush);

				auto& faces = brush->getFaces();

				for (int i = 0; i < faces.size(); ++i)
					brush->setUVRotation(i, data->floatData[0][nullptr]);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		undoData->floatData.resize(1);
		undoData->floatData[0][nullptr] = val;

		for (auto comp : components)
		{
			CSGBrush* brush = (CSGBrush*)comp;

			auto& faces = brush->getFaces();
			undoData->float2Data[brush].resize(faces.size());

			for (int f = 0; f < faces.size(); ++f)
			{
				undoData->float2Data[brush][f] = faces[f].texCoordsRotation;

				brush->setUVRotation(f, val);
			}
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			updateFaceProps();
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::onChangeMasterSmoothNormals(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change CSG brush smooth normals");

		undoData->undoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->bool2Data)
			{
				CSGBrush* brush = (CSGBrush*)d.first;

				comps.push_back(brush);

				auto& faces = brush->getFaces();

				for (int i = 0; i < faces.size(); ++i)
					brush->setSmoothNormals(i, data->bool2Data[brush][i]);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::vector<Component*> comps;

			for (auto& d : data->bool2Data)
			{
				CSGBrush* brush = (CSGBrush*)d.first;

				comps.push_back(brush);

				auto& faces = brush->getFaces();

				for (int i = 0; i < faces.size(); ++i)
					brush->setSmoothNormals(i, data->boolData[0][nullptr]);
			}

			MainWindow::addOnEndUpdateCallback([=]() {
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				CSGGeometry::getSingleton()->rebuild(comps);
				}
			);
		};
		//

		undoData->boolData.resize(1);
		undoData->boolData[0][nullptr] = val;

		for (auto comp : components)
		{
			CSGBrush* brush = (CSGBrush*)comp;

			auto& faces = brush->getFaces();
			undoData->bool2Data[brush].resize(faces.size());

			for (int f = 0; f < faces.size(); ++f)
			{
				undoData->bool2Data[brush][f] = faces[f].smoothNormals;

				brush->setSmoothNormals(f, val);
			}
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			updateFaceProps();
			CSGGeometry::getSingleton()->rebuild(components);
			}
		);
	}

	void CSGBrushEditor::renderGizmos(int viewId, int viewLayer, Camera* camera)
	{
		uint64_t renderState = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_ALWAYS
			| BGFX_STATE_BLEND_ALPHA;
			//| BGFX_STATE_CULL_CW;

		int overlayViewId = Renderer::getSingleton()->getOverlayViewId() + viewLayer;

		Color color = Color(1.0f, 1.0f, 0.0f, 0.5f);

		bool hovered = false;

		for (int i = 0; i < faceProps.size(); ++i)
		{
			if (faceProps[i]->getIsHovered())
			{
				hovered = true;
				break;
			}
		}

		if (hovered)
		{
			for (auto comp : components)
			{
				CSGBrush* brush = (CSGBrush*)comp;
				Transform* t = brush->getGameObject()->getTransform();

				glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
				glm::mat4x4 rotMat = glm::mat4_cast(t->getRotation());
				glm::vec3 offset = glm::vec3(0.0f);
				glm::vec3 scale = t->getScale();

				if (faceProps[0]->getIsHovered()) //+Z
					offset = t->getRotation() * (t->getScale() * glm::vec3(0.0f, 0.0f, 1.0f));

				if (faceProps[1]->getIsHovered()) //-Z
					offset = t->getRotation() * (t->getScale() * glm::vec3(0.0f, 0.0f, -1.0f));

				if (faceProps[2]->getIsHovered()) //+Y
				{
					glm::quat rot = Mathf::toQuaternion(glm::vec3(90.0f, 0.0f, 0.0f));
					offset = t->getRotation() * (t->getScale() * glm::vec3(0.0f, 1.0f, 0.0f));
					rotMat *= glm::mat4_cast(rot);
					scale = glm::vec3(scale.x, scale.z, scale.y);
				}

				if (faceProps[3]->getIsHovered()) //-Y
				{
					glm::quat rot = Mathf::toQuaternion(glm::vec3(90.0f, 0.0f, 0.0f));
					offset = t->getRotation() * (t->getScale() * glm::vec3(0.0f, -1.0f, 0.0f));
					rotMat *= glm::mat4_cast(rot);
					scale = glm::vec3(scale.x, scale.z, scale.y);
				}

				if (faceProps[4]->getIsHovered()) //+X
				{
					glm::quat rot = Mathf::toQuaternion(glm::vec3(0.0f, 90.0f, 0.0f));
					offset = t->getRotation() * (t->getScale() * glm::vec3(1.0f, 0.0f, 0.0f));
					rotMat *= glm::mat4_cast(rot);
					scale = glm::vec3(scale.z, scale.y, scale.x);
				}

				if (faceProps[5]->getIsHovered()) //-X
				{
					glm::quat rot = Mathf::toQuaternion(glm::vec3(0.0f, 90.0f, 0.0f));
					offset = t->getRotation() * (t->getScale() * glm::vec3(-1.0f, 0.0f, 0.0f));
					rotMat *= glm::mat4_cast(rot);
					scale = glm::vec3(scale.z, scale.y, scale.x);
				}

				mtx = glm::translate(mtx, t->getPosition() + offset);
				mtx *= rotMat;
				mtx = glm::scale(mtx, scale);

				Primitives::plane(mtx, 1.0f, 1.0f, color, overlayViewId, renderState, Renderer::getSingleton()->getSimpleProgram(), camera);
			}
		}
	}
}