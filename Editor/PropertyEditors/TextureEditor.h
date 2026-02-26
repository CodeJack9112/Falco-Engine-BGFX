#pragma once

#include "PropertyEditor.h"

#include "../../Engine/glm/vec2.hpp"

namespace GX
{
	class TreeNode;
	class Texture;

	class TextureEditor : public PropertyEditor
	{
	private:
		std::vector<Texture*> textures;

	public:
		TextureEditor();
		~TextureEditor();

		virtual void init(std::vector<Texture*> values);
		virtual void update();
		virtual void updateEditor();

	private:
		void onChangeGenMipMaps(bool value);
		void onChangeFilterMode(int value);
		void onChangeWrapMode(int value);
		void onChangeCompressionMethod(int value);
		void onChangeCompressionQuality(int value);
		void onChangeMaxResolution(int value);
		void onChangeBorderLT(glm::vec2 value);
		void onChangeBorderRB(glm::vec2 value);

		void onUpdatePreview();
	};
}