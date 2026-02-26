#pragma once

#include "Component.h"

#include "../glm/vec2.hpp"
#include "../glm/vec3.hpp"
#include "../glm/vec4.hpp"
#include "../Renderer/Color.h"

#include <cfloat>
#include <vector>

struct ImDrawList;

namespace GX
{
	class Canvas;
	
	enum class CanvasHorizontalAlignment
	{
		Left,
		Center,
		Right,
		Stretch
	};

	enum class CanvasVerticalAlignment
	{
		Top,
		Middle,
		Bottom,
		Stretch
	};

	class UIElement : public Component
	{
	public:
		enum class State
		{
			Normal,
			Hover,
			Active
		};

		struct Properties
		{
			State currentState = State::Normal;
			bool isHovered = false;
			bool isHoveredGlobal = false;
			bool isPressed[3] = { false, false, false };
			std::vector<glm::vec4> clipRects;
			bool focused = false;
			bool isEditor = false;
			glm::vec2 prevCanvasSize = glm::vec2(FLT_MAX);
		};

	protected:
		Transform* transform = nullptr;
		Canvas* canvas = nullptr;

		CanvasHorizontalAlignment horizontalAlignment = CanvasHorizontalAlignment::Center;
		CanvasVerticalAlignment verticalAlignment = CanvasVerticalAlignment::Middle;

		glm::vec2 size = glm::vec2(64, 64);
		glm::vec2 anchor = glm::vec2(0.5f, 0.5f);
		Color color = Color::White;
		bool raycastTarget = true;
		bool visible = true;

		Properties props;

	private:
		UIElement* cachedParent = nullptr;
		glm::vec2 getParentSize();
		glm::vec2 getParentAnchor();

	public:
		UIElement(MonoClass* monoClass);
		virtual ~UIElement();

		virtual void onRender(ImDrawList* drawList);
		virtual void onAttach();
		virtual void onDetach();
		virtual void onChangeParent(Transform* prevParent);
		virtual void onUpdate(float deltaTime);
		virtual bool isUiComponent() { return true; }

		virtual void onMouseEnter(glm::vec2 cursorPos);
		virtual void onMouseExit(glm::vec2 cursorPos);
		virtual void onMouseMove(glm::vec2 cursorPos);
		virtual void onMouseDown(int btn, glm::vec2 cursorPos);
		virtual void onMouseUp(int btn, glm::vec2 cursorPos);

		Canvas* getCanvas() { return canvas; }

		CanvasHorizontalAlignment getHorizontalAlignment() { return horizontalAlignment; }
		void setHorizontalAlignment(CanvasHorizontalAlignment value) { horizontalAlignment = value; }

		CanvasVerticalAlignment getVerticalAlignment() { return verticalAlignment; }
		void setVerticalAlignment(CanvasVerticalAlignment value) { verticalAlignment = value; }

		glm::vec2 getSize() { return size; }
		void setSize(glm::vec2 value) { size = value; }
		float getWidth() { return size.x; }
		void setWidth(float value) { size.x = value; }
		float getHeight() { return size.y; }
		void setHeight(float value) { size.y = value; }

		void setColor(Color value) { color = value; }
		Color getColor() { return color; }

		glm::vec2 getAnchor() { return anchor; }
		void setAnchor(glm::vec2 value);

		bool getRaycastTarget() { return raycastTarget; }
		void setRaycastTarget(bool value) { raycastTarget = value; }

		Transform* getTransform() { return transform; }

		glm::vec4 getRect();
		
		Properties& getProperties() { return props; }

		void clearClipRects();

		glm::vec3 getPosition();
		void setPosition(glm::vec3 value);
	};
}