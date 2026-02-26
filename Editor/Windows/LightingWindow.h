#pragma once

#include <string>
#include <vector>

#include "../Engine/Renderer/Color.h"

namespace GX
{
	class DialogProgress;
	class GameObject;
	class Material;
	class Texture;

	class LightingWindow
	{
	private:
		bool opened = true;

		int defaultLightmapSize = 0;
		int regionLightmapSize = 0;
		float lightmapBlurRadius = 0;
		DialogProgress* progressDialog = nullptr;

		std::vector<int> lightmapSizes = { 64, 128, 256, 512, 1024, 2048, 4096 };
		std::vector<int> regionLightmapSizes = { 512, 1024, 2048, 4096 };

		Texture* materialTexture = nullptr;

		Color ambientColor = Color::White;
		Color fogColor = Color::White;
		float fogDensity = 0;
		float realtimeGIIntensity = 0;
		float bakedGIIntensity = 0;

		void clearBakedData();
		void bakeLighting();

		void onDropSkyboxMaterial(Material* newMat);
		void onClearSkyboxMaterial();
		void onChangeAmbientColor(Color prevColor, Color newColor);
		void onChangeLightmapSize(int prevSize, int newSize);
		void onChangeSkyModel(int prevValue, int newValue);

		void onChangeRealtimeGI(bool prevValue, bool newValue);
		void onChangeRealtimeGIIntensity(float prevValue, float newValue);
		void onChangeGIBake(bool prevValue, bool newValue);
		void onChangeGIBounces(int prevValue, int newValue);
		void onChangeGIIntensity(float prevValue, float newValue);
		void onChangeGIQuality(int prevValue, int newValue);

		void onChangeFogEnabled(bool prevValue, bool newValue);
		void onChangeFogIncludeSkybox(bool prevValue, bool newValue);
		void onChangeFogStartDistance(float prevValue, float newValue);
		void onChangeFogEndDistance(float prevValue, float newValue);
		void onChangeFogDensity(float prevValue, float newValue);
		void onChangeFogColor(Color prevValue, Color newValue);
		void onChangeFogType(int prevValue, int newValue);

	public:
		LightingWindow();
		~LightingWindow();

		void init();
		void update();
		void show(bool show) { opened = show; }
		bool getVisible() { return opened; }

		void onSceneLoaded();
	};
}