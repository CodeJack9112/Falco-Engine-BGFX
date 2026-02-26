#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec2.hpp"
#include "../Engine/glm/vec3.hpp"
#include "../Engine/Renderer/Color.h"

namespace GX
{
	class Property;
	class Texture;
	class Camera;
	class Terrain;
	class UndoData;

	class TerrainEditor : public ComponentEditor
	{
	public:
		TerrainEditor();
		~TerrainEditor();

		virtual void init(std::vector<Component*> comps);

		static bool isEditModeActive();

	private:
		enum class TerrainEditMode
		{
			NONE,
			RAISE,
			LOWER,
			SMOOTH,
			FLATTEN,
			PAINT_TEXTURE,
			PAINT_TREES,
			PAINT_DETAIL_MESHES,
			PAINT_GRASS,
			SETTINGS
		};

		Texture* raiseIcon = nullptr;
		Texture* lowerIcon = nullptr;
		Texture* smoothIcon = nullptr;
		Texture* flattenIcon = nullptr;
		Texture* paintIcon = nullptr;
		Texture* treeIcon = nullptr;
		Texture* meshIcon = nullptr;
		Texture* grassIcon = nullptr;
		Texture* settingsIcon = nullptr;

		Texture* terrainIcon = nullptr;
		Texture* materialIcon = nullptr;

		float brushSize = 5.0f;
		float brushStrength = 1.0f;
		float flattenHeight = 0.0f;

		int editTexture = -1;
		int editGrass = -1;
		int editTree = -1;
		int editMesh = -1;

		int selectedTexture = 0;
		int selectedGrass = 0;
		int selectedTree = 0;
		int selectedMesh = 0;

		float prevScreenX = 0.0f;
		float prevScreenY = 0.0f;
		bool clickInWindow = false;

		bool lmbDown = false;
		bool grassVisible = false;
		bool treesVisible = false;
		bool meshesVisible = false;

		glm::vec3 lastHitPos = glm::vec3(0.0f);
		glm::vec3 lastHitPosWorld = glm::vec3(0.0f);

		std::vector<int> terrainSizes = { 64, 128, 256, 512 };
		std::vector<int> terrainWorldSizes = { 64, 128, 256, 512, 1024, 2048, 4096 };

		static TerrainEditMode editMode;
		std::string renderCallbackId = "";

		bool massPlaceTreesWnd = false;
		bool massPlaceKeepTrees = false;
		int massPlaceNumTrees = 10000;

		bool treesModified = false;
		bool meshesModified = false;
		bool grassModified = false;

		UndoData* undoDataHeightmap = nullptr;
		UndoData* undoDataTextures = nullptr;
		UndoData* undoDataTrees = nullptr;
		UndoData* undoDataGrass = nullptr;
		UndoData* undoDataMeshes = nullptr;

		void drawCircle(unsigned long long primitiveType, int viewId, Camera* camera, Color color, float radius);
		void drawBrushes(int viewId, int viewLayer, Camera* camera);

		void onUpdateTerrainEditor(Property* prop);

		void paintTerrain();
		void mousePickTerrain();
		void onEditTerrain();

		void textureEditorWindow();
		void grassEditorWindow();
		void treeEditorWindow();
		void meshEditorWindow();
		void massPlaceTreesWindow();

		void addTerrainTexture();
		void removeTerrainTexture(int index);

		void addTerrainGrass();
		void removeTerrainGrass(int index);

		void addTerrainTree();
		void removeTerrainTree(int index);

		void addTerrainMesh();
		void removeTerrainMesh(int index);

		void setTerrainFilePath(std::string path);
		void setTerrainMaterial(std::string path);
		void setTerrainSize(int value);
		void setTerrainWorldSize(int value);
		void setTerrainTreeImpostorStartDistance(int value);
		void setTerrainTreeDrawDistance(int value);
		void setTerrainDetailMeshesDrawDistance(int value);
		void setTerrainGrassDrawDistance(int value);
		void setTerrainCastShadows(bool value);
		void setTerrainTreesCastShadows(bool value);
		void setTerrainDetailMeshesCastShadows(bool value);
		void setTerrainGrassCastShadows(bool value);
		void setTerrainDrawTrees(bool value);
		void setTerrainDrawDetailMeshes(bool value);
		void setTerrainDrawGrass(bool value);
	};
}