#pragma once

#include <vector>
#include <map>
#include <bgfx/bgfx.h>

#include "Component.h"

#include "../glm/vec2.hpp"

struct ImGuiWindow;

namespace GX
{
	class UIElement;
	
	enum class CanvasMode
	{
		ScaleToScreenSize,
		MatchScreenSize
	};

	class Canvas : public Component
	{
		friend class UIElement;
		friend class Renderer;

	protected:
		CanvasMode mode = CanvasMode::ScaleToScreenSize;

		glm::vec2 refScreenSize = glm::vec2(1920, 1080);
		std::map<void*, glm::vec2> realScreenSize;

		float screenMatchSide = 1.0f;

		std::vector<UIElement*> uiElements;

		float zoom = 1.0f;
		glm::vec2 offset = glm::vec2(0.0f);

		ImGuiWindow* window = nullptr;

	public:
		Canvas();
		virtual ~Canvas();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();

		virtual Component* onClone();

		virtual void onAttach();
		virtual void onDetach();

		glm::vec2 getRefScreenSize() { return refScreenSize; }
		void setRefScreenSize(glm::vec2 value);

		glm::vec2 getRealScreenSize(void* win);
		void setRealScreenSize(glm::vec2 value, void* win);

		int getRefScreenWidth() { return (int)refScreenSize.x; }
		int getRefScreenHeight() { return (int)refScreenSize.y; }

		float getScreenMatchSide() { return screenMatchSide; }
		/// <summary>
		/// 0 - width ... 1 - height
		/// </summary>
		void setScreenMatchSide(float value) { screenMatchSide = value; }

		CanvasMode getMode() { return mode; }
		void setMode(CanvasMode value) { mode = value; }

		std::vector<UIElement*>& getUIElements() { return uiElements; }

		void setZoom(float value) { zoom = value; }
		void setOffset(glm::vec2 value) { offset = value; }
		void setWindow(ImGuiWindow* value) { window = value; }

		float getZoom() { return zoom; }
		glm::vec2 getOffset() { return offset; }
		ImGuiWindow* getWindow() { return window; }
	};
}