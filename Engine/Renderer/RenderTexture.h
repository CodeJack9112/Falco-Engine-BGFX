#pragma once

#include <vector>
#include <bgfx/bgfx.h>

#include "../Core/Object.h"

namespace GX
{
	class Texture;

	class RenderTexture : public Object
	{
	public:
		enum class TextureType
		{
			ColorWithDepth,
			ColorOnly
		};

	private:
		bgfx::FrameBufferHandle frameBufferHandle = { bgfx::kInvalidHandle };
		bgfx::TextureHandle colorTextureHandle = { bgfx::kInvalidHandle };
		bgfx::TextureHandle depthTextureHandle = { bgfx::kInvalidHandle };

		int width = 0;
		int height = 0;

		Texture* colorTexture = nullptr;
		Texture* depthTexture = nullptr;

		TextureType textureType = TextureType::ColorWithDepth;

	public:
		RenderTexture(int w, int h, TextureType type = TextureType::ColorWithDepth);
		virtual ~RenderTexture();

		void reset(int w, int h);

		int getWidth() { return width; }
		int getHeight() { return height; }

		bgfx::TextureHandle getColorTextureHandle() { return colorTextureHandle; }
		bgfx::TextureHandle getDepthTextureHandle() { return depthTextureHandle; }
		bgfx::FrameBufferHandle getFrameBufferHandle() { return frameBufferHandle; }

		Texture* getColorTexture() { return colorTexture; }
		Texture* getDepthTexture() { return depthTexture; }
	};
}