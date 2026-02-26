#include <string>
#include <vector>
#include <map>

#include <imgui.h>

#include "../Engine/glm/glm.hpp"

namespace GX
{
	class Canvas;
	class UIElement;
	class UndoData;

	class UIEditorWindow
	{
	private:
		enum class ResizeDirection
		{
			None,
			TopLeft,
			TopRight,
			BottomLeft,
			BottomRight,
			Top,
			Bottom,
			Left,
			Right
		};

		bool opened = true;
		bool focused = false;
		bool hovered = false;

		ImVec4 bgColor = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
		glm::vec2 camPos = glm::vec2(0, 0);
		float zoom = 0.5f;
		glm::vec2 prevMousePosition = glm::vec2(0.0f);
		std::map<UIElement*, glm::vec3> roundedPositions;
		std::map<UIElement*, glm::vec2> roundedSizes;

		bool mmbPressed = false;
		bool lmbPressed = false;
		bool checkDragging = true;
		bool isDragging = false;
		bool isDraggingElements = false;
		bool isResizingElements = false;
		bool isAnySelectedElementWasHovered = false;
		ResizeDirection resizeDirection = ResizeDirection::None;

		glm::vec2 transformVector(glm::vec2 vec, Canvas* canvas);
		glm::vec2 transformVector(glm::vec2 vec, float width);

		std::vector<UIElement*> selectedElements;
		bool isElementSelected(UIElement* value);

		UndoData* moveUndo = nullptr;
		UndoData* sizeUndo = nullptr;

		void onStartMove();
		void onEndMove();

		void onStartResize();
		void onEndResize();

	public:
		UIEditorWindow();
		~UIEditorWindow();

		void init();
		void update();

		void show(bool show) { opened = show; }
		bool getVisible() { return opened; }
		bool isFocused() { return focused; }
		bool isHovered() { return hovered; }
		bool isMousePressed() { return lmbPressed; }

		void selectObject(UIElement* value, bool multiple, bool callCallback = true);
		void selectObjects(std::vector<UIElement*> value, bool callCallback = true);
		void clearSelection();

		std::vector<UIElement*>& getSelectedObjects() { return selectedElements; }
	};
}