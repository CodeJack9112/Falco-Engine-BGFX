#include "GameWindow.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <sstream>

#include "MainWindow.h"
#include "UIEditorWindow.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/APIManager.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Renderer/RenderTexture.h"
#include "../Engine/Core/InputManager.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Components/Canvas.h"

#include <SDL2/SDL.h>

namespace GX
{
	GameWindow::GameWindow()
	{
		
	}

	GameWindow::~GameWindow()
	{
		delete renderTarget;
	}

	void GameWindow::init()
	{
		int width = MainWindow::getSingleton()->getWidth();
		int height = MainWindow::getSingleton()->getHeight();
		renderTarget = new RenderTexture(width, height);
	}

	void GameWindow::update()
	{
		bool lastHovered = false;

		if (opened)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			bool begin = ImGui::Begin("Game", &opened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::PopStyleVar();

			if (begin)
			{
				focused = ImGui::IsWindowFocused();

				position = glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
				size = glm::vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

				float mx = ImGui::GetMousePos().x;
				float my = ImGui::GetMousePos().y;

				lastHovered = hovered;
				mouseOver = (mx > position.x && mx < position.x + size.x && my > position.y && my < position.y + size.y);

				if (size != oldSize)
				{
					if (size.x > 0 && size.y > 0)
						onResize();

					oldSize = size;
				}

				ImVec2 sz = ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 22);

				if (sz.x > 0 && sz.y > 0)
				{
					if (bgfx::isValid(renderTarget->getColorTextureHandle()))
						ImGui::Image((void*)renderTarget->getColorTextureHandle().idx, sz, ImVec2(0, 1), ImVec2(1, 0));

					ImVec2 cp = ImGui::GetCursorPos();
					std::vector<Canvas*>& canvases = Renderer::getSingleton()->getCanvases();
					ImGuiWindow* win = ImGui::GetCurrentWindow();
					for (auto canv = canvases.begin(); canv != canvases.end(); ++canv)
					{
						Canvas* canvas = *canv;
						if (!canvas->getEnabled() || !canvas->getGameObject()->getActive())
							continue;

						canvas->setOffset(glm::vec2(position.x + 1.0f, position.y + 20.0f));
					}

					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
					ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));

					if (ImGui::BeginChild("##GameUI", ImVec2(size.x, size.y)))
					{
						bool uiWndActive = MainWindow::getUIEditorWindow()->isMousePressed();

						auto gameObjects = Engine::getSingleton()->getGameObjects();
						Renderer::getSingleton()->renderUI(gameObjects, glm::vec2(-1.0f, -20.0f), (hovered || mouseWasPressed) && !uiWndActive);
						gameObjects.clear();

						ImGui::SetCursorPos(ImVec2(0, 0));

						APIManager::getSingleton()->execute("OnGUI");
					}

					ImGui::EndChild();
					ImGui::PopStyleVar(3);

					ImGui::SetCursorPos(cp);
				}

				hovered = mouseOver && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

				if (hovered)
				{
					if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
						mouseWasPressed = true;
				}

				//if (!wasHovered && hovered && (ImGui::IsMouseDragging(0) || ImGui::IsMouseDragging(1)))
				//	wasHovered = hovered;
			}

			ImGui::End();

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				mouseWasPressed = false;

			if (!ImGui::GetIO().WantCaptureKeyboard && !ImGui::GetIO().WantTextInput)
			{
				ctrlPressed = InputManager::getSingleton()->getKey(SDL_SCANCODE_LCTRL);
				shiftPressed = InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT);

				if (MainWindow::getIsPlaying())
				{
					if (InputManager::getSingleton()->getKey(SDL_SCANCODE_F5))
					{
						if (shiftPressed)
						{
							MainWindow::getSingleton()->onPlay();
						}
						else
						{
							InputManager::getSingleton()->setCursorLocked(false);
							InputManager::getSingleton()->setCursorVisible(true);
						}
					}
				}
			}

			if (!opened)
			{
				if (MainWindow::getIsPlaying())
					MainWindow::getSingleton()->onPlay();
			}
		}
		else
		{
			mouseOver = false;
		}
	}

	glm::vec2 GameWindow::getPosition()
	{
		return position;
	}

	glm::vec2 GameWindow::getSize()
	{
		return size;
	}

	bool GameWindow::isFocused()
	{
		return focused;
	}

	bool GameWindow::isHovered()
	{
		return hovered;
	}

	void GameWindow::onResize()
	{
		MainWindow::getSingleton()->addOnEndUpdateCallback([=]()
			{
				renderTarget->reset(getSize().x, getSize().y);
				Renderer::getSingleton()->resetFrameBuffers();

				if (Engine::getSingleton()->getIsRuntimeMode())
				{
					int w = getSize().x;
					int h = getSize().y;

					void* args[2] = { &w, &h };
					APIManager::getSingleton()->execute("OnWindowResized", args, "int,int");
				}
			}
		);
	}
}