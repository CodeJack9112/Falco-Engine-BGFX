#pragma once

#include <string>
#include <vector>

#include <bgfx/bgfx.h>

#include "Asset.h"

#include "../glm/vec2.hpp"
#include "../glm/vec4.hpp"

namespace GX
{
	class Texture;

	class Cubemap : public Asset
	{
	private:
		Texture* textures[6];
		static Cubemap* nullCubemap;

		bgfx::TextureHandle textureHandle = { bgfx::kInvalidHandle };
		unsigned char* data = nullptr;
		static void releaseDataCallback(void* _ptr, void* _userData);

	public:
		Cubemap();
		virtual ~Cubemap();

		static std::string ASSET_TYPE;

		virtual void load();
		virtual void unload();
		static void unloadAll();
		virtual void reload();
		virtual std::string getAssetType() { return ASSET_TYPE; }

		static Cubemap* create(std::string location, std::string name);
		static Cubemap* load(std::string location, std::string name);
		void save();

		void setTexture(int index, Texture* texture, bool update = true);
		Texture* getTexture(int index) { return textures[index]; }

		void updateCubemap();

		static Cubemap* getNullCubemap() { return nullCubemap; }
		static void createNullCubemap();
		static void destroyNullCubemap();

		bgfx::TextureHandle getHandle() { return textureHandle; }
	};
}