#include "RenderTexture.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Assets/Texture.h"
#include "../Classes/GUIDGenerator.h"

namespace GX
{
	RenderTexture::RenderTexture(int w, int h, TextureType type) : Object(APIManager::getSingleton()->rendertexture_class)
	{
		textureType = type;

		colorTexture = new Texture();

		if (type == TextureType::ColorWithDepth)
			depthTexture = new Texture();

		reset(w, h);
	}

	RenderTexture::~RenderTexture()
	{
		if (bgfx::isValid(frameBufferHandle))
			bgfx::destroy(frameBufferHandle);

		if (colorTexture != nullptr)
			delete colorTexture;

		if (depthTexture != nullptr)
			delete depthTexture;

		colorTexture = nullptr;
		depthTexture = nullptr;
	}

	void RenderTexture::reset(int w, int h)
	{
		if (bgfx::isValid(frameBufferHandle))
			bgfx::destroy(frameBufferHandle);

		width = w;
		height = h;

		if (width < 8) width = 8;
		if (height < 8) height = 8;

		colorTextureHandle = bgfx::createTexture2D(uint16_t(width), uint16_t(height), false, 1, bgfx::TextureFormat::BGRA8,
			BGFX_TEXTURE_RT
			| BGFX_SAMPLER_MIN_ANISOTROPIC
			| BGFX_SAMPLER_MAG_ANISOTROPIC
			//| BGFX_SAMPLER_MIP_POINT
			| BGFX_SAMPLER_U_CLAMP
			| BGFX_SAMPLER_V_CLAMP);

		colorTexture->textureHandle = colorTextureHandle;
		colorTexture->width = width;
		colorTexture->originalWidth = width;
		colorTexture->height = height;
		colorTexture->originalHeight = height;
		colorTexture->format = bgfx::TextureFormat::BGRA8;

		if (textureType == TextureType::ColorWithDepth)
		{
			depthTextureHandle = bgfx::createTexture2D(uint16_t(width), uint16_t(height), false, 1, bgfx::TextureFormat::D24S8,
				BGFX_TEXTURE_RT
				| BGFX_SAMPLER_MIN_POINT
				| BGFX_SAMPLER_MAG_POINT
				| BGFX_SAMPLER_MIP_POINT
				| BGFX_SAMPLER_U_CLAMP
				| BGFX_SAMPLER_V_CLAMP);

			bgfx::Attachment at[2];
			at[0].init(colorTextureHandle);
			at[1].init(depthTextureHandle);

			frameBufferHandle = bgfx::createFrameBuffer(2, at, true);

			depthTexture->textureHandle = depthTextureHandle;
			depthTexture->width = width;
			depthTexture->originalWidth = width;
			depthTexture->height = height;
			depthTexture->originalHeight = height;
			depthTexture->format = bgfx::TextureFormat::D24S8;
		}
		else
		{
			bgfx::Attachment at[1];
			at[0].init(colorTextureHandle);

			frameBufferHandle = bgfx::createFrameBuffer(1, at, true);
		}
	}
}