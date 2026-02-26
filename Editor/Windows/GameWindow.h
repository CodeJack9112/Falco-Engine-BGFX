#pragma once

#include <string>
#include <vector>
#include <functional>

#include "../Engine/glm/vec2.hpp"

namespace GX
{
	class RenderTexture;
	
	class GameWindow
	{
		friend class MainWindow;
		friend class SceneWindow;

	protected:
		RenderTexture* renderTarget = nullptr;

		void onResize();

	private:
		bool opened = false;
		
		bool ctrlPressed = false;
		bool shiftPressed = false;

		glm::vec2 position = glm::vec2(0, 0);
		glm::vec2 size = glm::vec2(0, 0);
		glm::vec2 oldSize = glm::vec2(0, 0);
		glm::vec2 prevMousePos = glm::vec2(0, 0);

		bool focused = false;
		bool hovered = false;
		//bool wasHovered = false;
		bool mouseOver = false;
		bool mouseWasPressed = false;

	public:
		GameWindow();
		~GameWindow();

		void init();
		void update();
		void show(bool show) { opened = show; }
		bool getVisible() { return opened; }
		bool isMousePressed() { return mouseWasPressed; }
		
		glm::vec2 getPosition();
		glm::vec2 getSize();
		bool isFocused();
		bool isHovered();
	};
}