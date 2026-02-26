#pragma once

#include <string>

namespace GX
{
	class Texture;
	class Material;
	class GameObject;
	class Transform;

	class DialogCSGStairBuilder
	{
	private:
		void clearUndo();
		void rebuild();

	public:
		DialogCSGStairBuilder();
		~DialogCSGStairBuilder();

		void show(GameObject* root);
		void update();

		int numSteps = 12;
		float width = 2.0f;
		float height = 3.0f;
		float length = 4.0f;
		float uvScale = 1.0f;
		bool closed = false;
		Material* material = nullptr;

		Texture* materialIcon = nullptr;

	private:
		bool visible = false;
		GameObject* rootObject = nullptr;
	};
}