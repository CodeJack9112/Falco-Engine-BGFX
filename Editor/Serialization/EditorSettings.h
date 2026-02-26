#pragma once

#include <string>
#include <vector>

#include "../Engine/Serialization/Data/SVector.h"
#include "../Engine/Serialization/Data/SQuaternion.h"
#include "../Engine/Serialization/Serializers/BinarySerializer.h"

namespace GX
{
	class PropertyState : public Archive
	{
	public:
		PropertyState() = default;
		~PropertyState() = default;

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(expanded);
			data(path);
		}

		bool expanded = true;
		std::string path = "";
	};

	class PropertyEditorData : public Archive
	{
	public:
		PropertyEditorData() = default;
		~PropertyEditorData() = default;

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(editorName);
			data(properties);
		}

		std::string editorName = "PropertyEditor";
		std::vector<PropertyState> properties;
	};

	class EditorSettings : public Archive
	{
	public:
		EditorSettings();
		~EditorSettings();

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(lastOpenedScene);
			data(showGrid);
			data(showNavMesh);
			data(snapToGrid);
			data(snapToGridMoveSize);
			data(snapToGridRotateSize);
			data(snapToGridScaleSize);
			data(sceneWindowVisible);
			data(consoleWindowVisible);
			data(inspectorWindowVisible);
			data(hierarchyWindowVisible);
			data(assetsWindowVisible);
			data(lightingWindowVisible);
			data(navigationWindowVisible);
			data(uiEditorWindowVisible);
			data(cameraPosition);
			data(cameraRotation);
			data(cameraSpeed);
			data(cameraSpeed2);
			data(cameraSpeed3);
			data(cameraSpeedPreset);
			data(cameraFOV);
			data(shadowsEnabled);
			data(cameraFarClipPlane);
			data(gizmoCenterBase);
			data(runScreenWidth);
			data(runScreenHeight);
			data(runFullscreen);
			data(showStats);
			data(gizmoIconsScale);
			data(gizmoShowBounds);
			data(saveOnPlay);
			data(standalone);
			data(gizmoType);
			data(gizmoTransformSpace);
			data(gizmoShowTerrainGrassBounds);
			data(gizmoShowTerrainTreesBounds);
			data(gizmoShowTerrainDetailMeshesBounds);
			data(snapToGridUIMoveSize);
			data(snapToGridUIRotateSize);
			data(clearConsoleOnPlay);
			data(animationEditorWindowVisible);
			data(occlusionCulling);
			data(giBake);
			data(giBounces);
			data(giIntensity);
			data(giQuality);
			data(lightmapSize);
			data(propertyEditorsData);
			data(consoleShowErrors);
			data(consoleShowWarnings);
			data(consoleShowInfos);
		}

		std::string lastOpenedScene = "";
		std::vector<PropertyEditorData> propertyEditorsData;

		bool showGrid = true;
		bool showNavMesh = false;
		bool snapToGrid = false;
		bool shadowsEnabled = true;
		
		float snapToGridMoveSize = 0.25f;
		float snapToGridRotateSize = 5.0f;
		float snapToGridScaleSize = 0.1f;

		float snapToGridUIMoveSize = 16.0f;
		float snapToGridUIRotateSize = 5.0f;

		int gizmoCenterBase = 0;
		int gizmoType = 1;
		int gizmoTransformSpace = 1;

		float cameraSpeed = 4.0f;
		float cameraSpeed2 = 10.0f;
		float cameraSpeed3 = 15.0f;
		float cameraFarClipPlane = 5500.0f;
		int cameraSpeedPreset = 0;
		float cameraFOV = 75.0f;

		bool showStats = false;
		float gizmoIconsScale = 0.35f;
		bool occlusionCulling = false;
		
		bool gizmoShowBounds = false;
		bool gizmoShowTerrainGrassBounds = false;
		bool gizmoShowTerrainTreesBounds = false;
		bool gizmoShowTerrainDetailMeshesBounds = false;

		bool sceneWindowVisible = true;
		bool consoleWindowVisible = true;
		bool inspectorWindowVisible = true;
		bool hierarchyWindowVisible = true;
		bool assetsWindowVisible = true;
		bool lightingWindowVisible = true;
		bool navigationWindowVisible = true;
		bool terrainWindowVisible = false;
		bool previewWindowVisible = true;
		bool uiEditorWindowVisible = true;
		bool animationEditorWindowVisible = false;

		int runScreenWidth = 1280;
		int runScreenHeight = 800;
		bool runFullscreen = false;
		bool saveOnPlay = true;
		bool standalone = false;
		bool clearConsoleOnPlay = true;

		bool giBake = true;
		int giBounces = 1;
		float giIntensity = 1.0f;
		int giQuality = 1;
		int lightmapSize = 256;

		bool consoleShowErrors = true;
		bool consoleShowWarnings = true;
		bool consoleShowInfos = true;

		SVector3 cameraPosition = SVector3(0, 2, 0);
		SQuaternion cameraRotation = SQuaternion(0, 0, 0, 1);

	private:
		std::string loadedPath = "";

	public:
		void load(std::string path);
		void save(std::string path);
		void save();
	};
}