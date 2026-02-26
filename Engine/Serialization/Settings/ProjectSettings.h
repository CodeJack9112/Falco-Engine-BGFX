#pragma once

#include <string>
#include <functional>

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	class ProjectSettings : public Archive
	{
	private:
		std::string startupScene;

		int screenWidth = 1280;
		int screenHeight = 800;
		bool useNativeResolution = true;
		bool fullscreen = true;
		bool exclusiveMode = false;
		bool vsync = true;
		bool fxaa = false;
		bool useDynamicResolution = false;
		bool autoDynamicResolution = false;
		int autoDynamicResolutionTargetFPS = 60;
		float dynamicResolution = 1.0f;

		int directionalShadowResolution = 2048;
		int spotShadowResolution = 1024;
		int pointShadowResolution = 2048;
		int shadowCascadesCount = 4;
		int shadowSamplesCount = 2;
		bool shadowCascadesBlending = true;
		float shadowDistance = 100.0f;
		bool shadowsEnabled = true;

		int textureCompression = 2;
		int textureCompressionQuality = 1;
		int textureMaxResolution = 4096;

		bool enableSteamAPI = false;
		int steamAppId = 0;

		bool collisionMatrix[32][32];

		std::vector<std::string> tags;
		std::vector<std::string> layers;
		std::vector<std::string> scenes;

	public:
		ProjectSettings();
		~ProjectSettings() = default;

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(startupScene);
			data(screenWidth);
			data(screenHeight);
			data(useNativeResolution);
			data(fullscreen);
			data(exclusiveMode);
			data(vsync);
			data(fxaa);
			data(useDynamicResolution);
			data(autoDynamicResolution);
			data(autoDynamicResolutionTargetFPS);
			data(dynamicResolution);
			data(directionalShadowResolution);
			data(spotShadowResolution);
			data(pointShadowResolution);
			data(shadowCascadesCount);
			data(shadowSamplesCount);
			data(shadowCascadesBlending);
			data(shadowsEnabled);
			data(shadowDistance);
			data(textureCompression);
			data(textureCompressionQuality);
			data(textureMaxResolution);
			data(enableSteamAPI);
			data(steamAppId);
			data(tags);
			data(layers);
			data(scenes);
			
			for (int i = 0; i < 32; ++i)
			{
				for (int j = 0; j < 32; ++j)
				{
					data(collisionMatrix[i][j]);
				}
			}
		}

		void save();
		void load();

		std::string getStartupScene() { return startupScene; }
		void setStartupScene(std::string value) { startupScene = value; }

		int getScreenWidth() { return screenWidth; }
		void setScreenWidth(int value) { screenWidth = value; }

		int getScreenHeight() { return screenHeight; }
		void setScreenHeight(int value) { screenHeight = value; }

		bool getUseNativeResolution() { return useNativeResolution; }
		void setUseNativeResolution(bool value) { useNativeResolution = value; }

		bool getFullScreen() { return fullscreen; }
		void setFullScreen(bool value) { fullscreen = value; }

		bool getExclusiveMode() { return exclusiveMode; }
		void setExclusiveMode(bool value) { exclusiveMode = value; }

		bool getVSync() { return vsync; }
		void setVSync(bool value);

		bool getFXAA() { return fxaa; }
		void setFXAA(bool value) { fxaa = value; }

		bool getUseDynamicResolution() { return useDynamicResolution; }
		void setUseDynamicResolution(bool value);

		bool getAutoDynamicResolution() { return autoDynamicResolution; }
		void setAutoDynamicResolution(bool value);

		int getAutoDynamicResolutionTargetFPS() { return autoDynamicResolutionTargetFPS; }
		void setAutoDynamicResolutionTargetFPS(int value) { autoDynamicResolutionTargetFPS = value; }
		
		float getDynamicResolution() { return dynamicResolution; }
		void setDynamicResolution(float value);

		int getDirectionalShadowResolution() { return directionalShadowResolution; }
		void setDirectionalShadowResolution(int value);

		int getSpotShadowResolution() { return spotShadowResolution; }
		void setSpotShadowResolution(int value);

		int getPointShadowResolution() { return pointShadowResolution; }
		void setPointShadowResolution(int value);

		int getShadowCascadesCount() { return shadowCascadesCount; }
		void setShadowCascadesCount(int value);

		int getShadowSamplesCount() { return shadowSamplesCount; }
		void setShadowSamplesCount(int value);

		bool getShadowCascadesBlending() { return shadowCascadesBlending; }
		void setShadowCascadesBlending(bool value);

		bool getShadowsEnabled() { return shadowsEnabled; }
		void setShadowsEnabled(bool value) { shadowsEnabled = value; }

		float getShadowDistance() { return shadowDistance; }
		void setShadowDistance(float value) { shadowDistance = value; }

		int getTextureCompression() { return textureCompression; }
		void setTextureCompression(int value, std::function<void(std::string status, int progress)> callback = nullptr);

		int getTextureCompressionQuality() { return textureCompressionQuality; }
		void setTextureCompressionQuality(int value, std::function<void(std::string status, int progress)> callback = nullptr);

		int getTextureMaxResolution() { return textureMaxResolution; }
		void setTextureMaxResolution(int value, std::function<void(std::string status, int progress)> callback = nullptr);

		bool getEnableSteamAPI() { return enableSteamAPI; }
		void setEnableSteamAPI(bool value) { enableSteamAPI = value; }

		int getSteamAppId() { return steamAppId; }
		void setSteamAppId(int value) { steamAppId = value; }

		bool getCollisionMask(int i, int j) { return collisionMatrix[i][j]; }
		void setCollisionMask(int i, int j, bool value);

		std::vector<std::string> & getTags() { return tags; }
		std::vector<std::string> & getLayers() { return layers; }
		std::vector<std::string> & getScenes() { return scenes; }
	};
}