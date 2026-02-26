#pragma once

#include "UIElement.h"

struct ImDrawList;

namespace GX
{
	class Texture;

	class Image : public UIElement
	{
	public:
		enum class ImageType
		{
			Simple,
			Sliced
		};

	private:
		Texture* texture = nullptr;

		ImageType imageType = ImageType::Simple;

	public:
		Image();
		virtual ~Image();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();
		virtual void onRender(ImDrawList* drawList);

		static void render(ImDrawList* drawList,
			glm::vec4 rect,
			Texture* texture,
			Color color,
			ImageType imageType,
			float zoom
		);

		void setTexture(Texture* value) { texture = value; }
		Texture* getTexture() { return texture; }

		ImageType getImageType() { return imageType; }
		void setImageType(ImageType value) { imageType = value; }
	};
}