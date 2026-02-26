#include "Button.h"

#include <imgui.h>

#include "../Core/APIManager.h"

#include "../Assets/Texture.h"
#include "../Components/Canvas.h"
#include "../Core/Engine.h"

namespace GX
{
	std::string Button::COMPONENT_TYPE = "Button";

	Button::Button() : UIElement(APIManager::getSingleton()->button_class)
	{
	}

	Button::~Button()
	{
	}

	std::string Button::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* Button::onClone()
	{
		Button* newComponent = new Button();
		newComponent->enabled = enabled;
		newComponent->anchor = anchor;
		newComponent->color = color;
		newComponent->size = size;
		newComponent->horizontalAlignment = horizontalAlignment;
		newComponent->verticalAlignment = verticalAlignment;
		newComponent->colHover = colHover;
		newComponent->colActive = colActive;
		newComponent->colDisabled = colDisabled;
		newComponent->interactable = interactable;
		newComponent->texNormal = texNormal;
		newComponent->texHover = texHover;
		newComponent->texActive = texActive;
		newComponent->texDisabled = texDisabled;
		newComponent->imageType = imageType;
		newComponent->raycastTarget = raycastTarget;

		return newComponent;
	}

	void Button::onRender(ImDrawList* drawList)
	{
		UIElement::onRender(drawList);

		glm::vec4 rect = getRect();
		
		float zoom = 1.0f;

		if (canvas != nullptr)
			zoom = canvas->getZoom();

		render(drawList, rect, texNormal, texHover, texActive, texDisabled, color, colHover, colActive, colDisabled, imageType, interactable, props.currentState, zoom);
	}

	void Button::render(ImDrawList* drawList,
		glm::vec4 rect,
		Texture* texNormal,
		Texture* texHover,
		Texture* texActive,
		Texture* texDisabled,
		Color color,
		Color colHover,
		Color colActive,
		Color colDisabled,
		ImageType imageType,
		bool interactable,
		State currentState,
		float zoom)
	{
		ImU32 elemColor = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r(), color.g(), color.b(), color.a()));
		Texture* currentTexture = nullptr;

		if (!interactable)
		{
			currentTexture = texDisabled;
			elemColor = ImGui::ColorConvertFloat4ToU32(ImVec4(colDisabled.r(), colDisabled.g(), colDisabled.b(), colDisabled.a()));
		}
		else
		{
			if (currentState == State::Normal)
			{
				currentTexture = texNormal;
				elemColor = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r(), color.g(), color.b(), color.a()));
			}
			if (currentState == State::Hover)
			{
				currentTexture = texHover;
				elemColor = ImGui::ColorConvertFloat4ToU32(ImVec4(colHover.r(), colHover.g(), colHover.b(), colHover.a()));
			}
			if (currentState == State::Active)
			{
				currentTexture = texActive;
				elemColor = ImGui::ColorConvertFloat4ToU32(ImVec4(colActive.r(), colActive.g(), colActive.b(), colActive.a()));
			}
		}

		if (currentTexture == nullptr)
			drawList->AddRectFilled(ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w), elemColor, 0.0f);
		else
		{
			if (imageType == ImageType::Simple)
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w), ImVec2(0, 1), ImVec2(1, 0), elemColor);
			else
			{
				float w = currentTexture->getOriginalWidth() * zoom;
				float h = currentTexture->getOriginalHeight() * zoom;

				Rect border = currentTexture->getBorder() * zoom;

				//Top Left
				ImVec2 tx11 = ImVec2(0.0f, 1.0f);
				ImVec2 tx12 = ImVec2(1.0f / w * border.left, 1.0f - (1.0f / h * border.top));
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x, rect.y), ImVec2(rect.x + border.left, rect.y + border.top), tx11, tx12, elemColor);

				//Top right
				ImVec2 tx21 = ImVec2(1.0f - (1.0f / w * border.right), 1.0f);
				ImVec2 tx22 = ImVec2(1.0f, 1.0f - (1.0f / h * border.top));
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.z - border.right, rect.y), ImVec2(rect.z, rect.y + border.top), tx21, tx22, elemColor);

				//Top
				ImVec2 tx31 = ImVec2(1.0f / w * border.left, 1.0f);
				ImVec2 tx32 = ImVec2(1.0f - (1.0f / w * border.right), 1.0f - (1.0f / h * border.top));
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x + border.left, rect.y), ImVec2(rect.z - border.right, rect.y + border.top), tx31, tx32, elemColor);

				//Left
				ImVec2 tx41 = ImVec2(0.0f, 1.0f - (1.0f / h * border.top));
				ImVec2 tx42 = ImVec2(1.0f / w * border.left, 1.0f / h * border.bottom);
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x, rect.y + border.top), ImVec2(rect.x + border.left, rect.w - border.bottom), tx41, tx42, elemColor);

				//Center
				ImVec2 tx51 = ImVec2(1.0f / w * border.left, 1.0f - (1.0f / h * border.top));
				ImVec2 tx52 = ImVec2(1.0f - (1.0f / w * border.right), 1.0f / h * border.bottom);
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x + border.left, rect.y + border.top), ImVec2(rect.z - border.right, rect.w - border.bottom), tx51, tx52, elemColor);

				//Right
				ImVec2 tx61 = ImVec2(1.0f - (1.0f / w * border.right), 1.0f - (1.0f / h * border.top));
				ImVec2 tx62 = ImVec2(1.0f, 1.0f / h * border.bottom);
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.z - border.right, rect.y + border.top), ImVec2(rect.z, rect.w - border.bottom), tx61, tx62, elemColor);

				//Bottom Left
				ImVec2 tx71 = ImVec2(0.0f, 1.0f / h * border.bottom);
				ImVec2 tx72 = ImVec2(1.0f / w * border.left, 0.0f);
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x, rect.w - border.bottom), ImVec2(rect.x + border.left, rect.w), tx71, tx72, elemColor);

				//Bottom Right
				ImVec2 tx81 = ImVec2(1.0f - (1.0f / w * border.right), 1.0f / h * border.bottom);
				ImVec2 tx82 = ImVec2(1.0f, 0.0f);
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.z - border.right, rect.w - border.bottom), ImVec2(rect.z, rect.w), tx81, tx82, elemColor);

				//Bottom
				ImVec2 tx91 = ImVec2(1.0f / w * border.left, 1.0f / h * border.bottom);
				ImVec2 tx92 = ImVec2(1.0f - (1.0f / w * border.right), 0.0f);
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x + border.left, rect.w - border.bottom), ImVec2(rect.z - border.right, rect.w), tx91, tx92, elemColor);
			}
		}
	}

	void Button::onMouseEnter(glm::vec2 cursorPos)
	{
		if (!props.isEditor)
		{
			if (interactable)
				UIElement::onMouseEnter(cursorPos);
		}
		else
			UIElement::onMouseEnter(cursorPos);
	}

	void Button::onMouseExit(glm::vec2 cursorPos)
	{
		if (!props.isEditor)
		{
			if (interactable)
				UIElement::onMouseExit(cursorPos);
		}
		else
			UIElement::onMouseExit(cursorPos);
	}

	void Button::onMouseMove(glm::vec2 cursorPos)
	{
		if (!props.isEditor)
		{
			if (interactable)
				UIElement::onMouseMove(cursorPos);
		}
		else
			UIElement::onMouseMove(cursorPos);
	}

	void Button::onMouseDown(int btn, glm::vec2 cursorPos)
	{
		if (!props.isEditor)
		{
			if (interactable)
				UIElement::onMouseDown(btn, cursorPos);
		}
		else
			UIElement::onMouseDown(btn, cursorPos);
	}

	void Button::onMouseUp(int btn, glm::vec2 cursorPos)
	{
		if (!props.isEditor)
		{
			if (interactable)
				UIElement::onMouseUp(btn, cursorPos);
		}
		else
			UIElement::onMouseUp(btn, cursorPos);
	}
}