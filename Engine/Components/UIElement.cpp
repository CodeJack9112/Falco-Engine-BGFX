#include "UIElement.h"

#include <algorithm>

#include <imgui.h>
#include <imgui_internal.h>
#include <mono/metadata/debug-helpers.h>

#include "../Renderer/Renderer.h"
#include "../Core/GameObject.h"
#include "../Components/Transform.h"
#include "../Components/Canvas.h"
#include "../Components/Text.h"
#include "../Components/Mask.h"

#include "../Core/Engine.h"
#include "../Core/InputManager.h"
#include "../Core/APIManager.h"

namespace GX
{
	UIElement::UIElement(MonoClass* monoClass) : Component(monoClass)
	{
		
	}

	UIElement::~UIElement()
	{
		clearClipRects();
	}

	void UIElement::clearClipRects()
	{
		props.clipRects.clear();
	}

	glm::vec3 UIElement::getPosition()
	{
		return transform->getPosition();
	}

	void UIElement::setPosition(glm::vec3 value)
	{
		transform->setPosition(value);
	}

	void UIElement::onRender(ImDrawList* drawList)
	{
		
	}

	void UIElement::onAttach()
	{
		Component::onAttach();

		transform = getGameObject()->getTransform();

		Renderer::getSingleton()->uiElements.push_back(this);

		if (canvas != nullptr)
		{
			auto it = std::find(canvas->uiElements.begin(), canvas->uiElements.end(), this);

			if (it == canvas->uiElements.end())
				canvas->uiElements.push_back(this);
		}
	}

	void UIElement::onDetach()
	{
		Component::onDetach();

		std::vector<UIElement*>& uiElements = Renderer::getSingleton()->uiElements;

		auto it = std::find(uiElements.begin(), uiElements.end(), this);

		if (it != uiElements.end())
			uiElements.erase(it);

		if (canvas != nullptr)
		{
			auto it = std::find(canvas->uiElements.begin(), canvas->uiElements.end(), this);

			if (it != canvas->uiElements.end())
				canvas->uiElements.erase(it);
		}

		clearClipRects();
	}

	void UIElement::onChangeParent(Transform* prevParent)
	{
		if (canvas != nullptr)
		{
			auto it = std::find(canvas->uiElements.begin(), canvas->uiElements.end(), this);

			if (it != canvas->uiElements.end())
				canvas->uiElements.erase(it);
		}

		Transform* parent = getGameObject()->getTransform()->getParent();
		if (parent != nullptr)
		{
			canvas = (Canvas*)parent->getGameObject()->getComponent(Canvas::COMPONENT_TYPE);
			while (parent != nullptr && canvas == nullptr)
			{
				parent = parent->getParent();
				if (parent != nullptr)
					canvas = (Canvas*)parent->getGameObject()->getComponent(Canvas::COMPONENT_TYPE);
			}
		}
		else
			canvas = nullptr;

		if (canvas != nullptr)
			canvas->uiElements.push_back(this);

		cachedParent = nullptr;

		props.prevCanvasSize = glm::vec2(FLT_MAX);
	}

	void UIElement::onMouseEnter(glm::vec2 cursorPos)
	{
		if (!props.isHovered)
		{
			if (!props.isEditor)
			{
				if (!props.isPressed[0] && !props.isPressed[1] && !props.isPressed[2])
				{
					props.currentState = State::Hover;
				}

				if (Engine::getSingleton()->getIsRuntimeMode())
				{
					//API event call here (Mouse Hover)
					MonoMethodDesc* methodDesc = nullptr;
					MonoMethod* method = nullptr;
					MonoClass* mclass = APIManager::getSingleton()->uielement_class;
					std::string methodDescStr = std::string("UIElement:CallOnMouseEnter(UIElement,Vector2)");
					methodDesc = mono_method_desc_new(methodDescStr.c_str(), false);

					if (methodDesc)
					{
						method = mono_method_desc_search_in_class(methodDesc, mclass);
						mono_method_desc_free(methodDesc);

						if (method)
						{
							MonoObject* vec2 = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->vector2_class);
							mono_field_set_value(vec2, APIManager::getSingleton()->vector2_x, &cursorPos.x);
							mono_field_set_value(vec2, APIManager::getSingleton()->vector2_y, &cursorPos.y);

							void* _vec2 = mono_object_unbox(vec2);
							void* args[2] = { managedObject, _vec2 };
							mono_runtime_invoke(method, managedObject, args, nullptr);
						}
					}
				}
			}

			props.isHovered = true;
		}
	}

	void UIElement::onMouseExit(glm::vec2 cursorPos)
	{
		if (props.isHovered)
		{
			if (!props.isEditor)
			{
				if (!props.isPressed[0] && !props.isPressed[1] && !props.isPressed[2])
				{
					props.currentState = State::Normal;
				}

				if (Engine::getSingleton()->getIsRuntimeMode())
				{
					//API event call here (Mouse Exit)
					MonoMethodDesc* methodDesc = nullptr;
					MonoMethod* method = nullptr;
					MonoClass* mclass = APIManager::getSingleton()->uielement_class;
					std::string methodDescStr = std::string("UIElement:CallOnMouseExit(UIElement,Vector2)");
					methodDesc = mono_method_desc_new(methodDescStr.c_str(), false);

					if (methodDesc)
					{
						method = mono_method_desc_search_in_class(methodDesc, mclass);
						mono_method_desc_free(methodDesc);

						if (method)
						{
							MonoObject* vec2 = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->vector2_class);
							mono_field_set_value(vec2, APIManager::getSingleton()->vector2_x, &cursorPos.x);
							mono_field_set_value(vec2, APIManager::getSingleton()->vector2_y, &cursorPos.y);

							void* _vec2 = mono_object_unbox(vec2);
							void* args[2] = { managedObject, _vec2 };
							mono_runtime_invoke(method, managedObject, args, nullptr);
						}
					}
				}
			}

			props.isHovered = false;
		}
	}

	void UIElement::onMouseMove(glm::vec2 cursorPos)
	{
		if (!Engine::getSingleton()->getIsRuntimeMode())
			return;

		if (props.isEditor)
			return;

		MonoMethodDesc* methodDesc = nullptr;
		MonoMethod* method = nullptr;
		MonoClass* mclass = APIManager::getSingleton()->uielement_class;
		std::string methodDescStr = std::string("UIElement:CallOnMouseMove(UIElement,Vector2)");

		if (props.isPressed[0] || props.isPressed[1] || props.isPressed[2])
		{
			//API event call here (Mouse Move)
			methodDesc = mono_method_desc_new(methodDescStr.c_str(), false);

			if (methodDesc)
			{
				method = mono_method_desc_search_in_class(methodDesc, mclass);
				mono_method_desc_free(methodDesc);

				if (method)
				{
					MonoObject* vec2 = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->vector2_class);
					mono_field_set_value(vec2, APIManager::getSingleton()->vector2_x, &cursorPos.x);
					mono_field_set_value(vec2, APIManager::getSingleton()->vector2_y, &cursorPos.y);

					void* _vec2 = mono_object_unbox(vec2);
					void* args[2] = { managedObject, _vec2 };
					mono_runtime_invoke(method, managedObject, args, nullptr);
				}
			}
		}
		else
		{
			if (props.isHovered)
			{
				//API event call here (Mouse Move)
				methodDesc = mono_method_desc_new(methodDescStr.c_str(), false);

				if (methodDesc)
				{
					method = mono_method_desc_search_in_class(methodDesc, mclass);
					mono_method_desc_free(methodDesc);

					if (method)
					{
						MonoObject* vec2 = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->vector2_class);
						mono_field_set_value(vec2, APIManager::getSingleton()->vector2_x, &cursorPos.x);
						mono_field_set_value(vec2, APIManager::getSingleton()->vector2_y, &cursorPos.y);

						void* _vec2 = mono_object_unbox(vec2);
						void* args[2] = { managedObject, _vec2 };
						mono_runtime_invoke(method, managedObject, args, nullptr);
					}
				}
			}
		}
	}

	void UIElement::onMouseDown(int btn, glm::vec2 cursorPos)
	{
		if (props.isEditor)
			return;

		if (props.isHovered)
		{
			props.focused = true;

			if (!props.isPressed[btn])
			{
				if (btn == 0)
					props.currentState = State::Active;

				props.isPressed[btn] = true;

				if (Engine::getSingleton()->getIsRuntimeMode())
				{
					//API event call here (Mouse Down)
					MonoMethodDesc* methodDesc = nullptr;
					MonoMethod* method = nullptr;
					MonoClass* mclass = APIManager::getSingleton()->uielement_class;
					std::string methodDescStr = std::string("UIElement:CallOnMouseDown(UIElement,int,Vector2)");
					methodDesc = mono_method_desc_new(methodDescStr.c_str(), false);

					if (methodDesc)
					{
						method = mono_method_desc_search_in_class(methodDesc, mclass);
						mono_method_desc_free(methodDesc);

						if (method)
						{
							MonoObject* vec2 = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->vector2_class);
							mono_field_set_value(vec2, APIManager::getSingleton()->vector2_x, &cursorPos.x);
							mono_field_set_value(vec2, APIManager::getSingleton()->vector2_y, &cursorPos.y);

							void* _vec2 = mono_object_unbox(vec2);
							void* args[3] = { managedObject, &btn, _vec2 };
							mono_runtime_invoke(method, managedObject, args, nullptr);
						}
					}
				}
			}
		}
		else
		{
			props.focused = false;
		}
	}

	void UIElement::onMouseUp(int btn, glm::vec2 cursorPos)
	{
		if (props.isEditor)
			return;

		if (props.isPressed[btn])
		{
			if (props.isHovered)
			{
				if (btn == 0)
					props.currentState = State::Hover;

				if (Engine::getSingleton()->getIsRuntimeMode())
				{
					//API event call here (Mouse Up)
					MonoMethodDesc* methodDesc = nullptr;
					MonoMethod* method = nullptr;
					MonoClass* mclass = APIManager::getSingleton()->uielement_class;
					std::string methodDescStr = std::string("UIElement:CallOnMouseUp(UIElement,int,Vector2)");
					methodDesc = mono_method_desc_new(methodDescStr.c_str(), false);

					if (methodDesc)
					{
						method = mono_method_desc_search_in_class(methodDesc, mclass);
						mono_method_desc_free(methodDesc);

						if (method)
						{
							MonoObject* vec2 = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->vector2_class);
							mono_field_set_value(vec2, APIManager::getSingleton()->vector2_x, &cursorPos.x);
							mono_field_set_value(vec2, APIManager::getSingleton()->vector2_y, &cursorPos.y);

							void* _vec2 = mono_object_unbox(vec2);
							void* args[3] = { managedObject, &btn, _vec2 };
							mono_runtime_invoke(method, managedObject, args, nullptr);
						}
					}
				}
			}
			else
			{
				props.currentState = State::Normal;
			}

			props.isPressed[btn] = false;
		}
	}

	void UIElement::onUpdate(float deltaTime)
	{
		if (transform->getParent() != nullptr)
		{
			glm::vec2 canv = getParentSize();

			if (props.prevCanvasSize == glm::vec2(FLT_MAX))
				props.prevCanvasSize = canv;

			if (props.prevCanvasSize != canv)
			{
				glm::vec3 off = glm::vec3(canv - props.prevCanvasSize, 0.0f);
				glm::vec2 anch = getParentAnchor();

				if (horizontalAlignment == CanvasHorizontalAlignment::Left)
					transform->setPosition(glm::vec3((transform->getPosition() - off * anch.x).x, transform->getPosition().y, 0.0f));
				if (horizontalAlignment == CanvasHorizontalAlignment::Center)
					transform->setPosition(glm::vec3(transform->getPosition().x - (off.x * anch.x) * 0.5f + (off.x * (1.0f - anch.x)) * 0.5f, transform->getPosition().y, 0.0f));
				if (horizontalAlignment == CanvasHorizontalAlignment::Right)
					transform->setPosition(glm::vec3((transform->getPosition() + off * (1.0f - anch.x)).x, transform->getPosition().y, 0.0f));
				if (horizontalAlignment == CanvasHorizontalAlignment::Stretch)
				{
					glm::vec2 aa = anchor - anch;
					transform->setPosition(glm::vec3(transform->getPosition().x + (off.x * aa.x), transform->getPosition().y, 0.0f));
					setSize(glm::vec2(getSize().x + off.x, getSize().y));
				}

				if (verticalAlignment == CanvasVerticalAlignment::Top)
					transform->setPosition(glm::vec3(transform->getPosition().x, (transform->getPosition() - off * anch.y).y, 0.0f));
				if (verticalAlignment == CanvasVerticalAlignment::Middle)
					transform->setPosition(glm::vec3(transform->getPosition().x, transform->getPosition().y - (off.y * anch.y) * 0.5f + (off.y * (1.0f - anch.y)) * 0.5f, 0.0f));
				if (verticalAlignment == CanvasVerticalAlignment::Bottom)
					transform->setPosition(glm::vec3(transform->getPosition().x, (transform->getPosition() + off * (1.0f - anch.y)).y, 0.0f));
				if (verticalAlignment == CanvasVerticalAlignment::Stretch)
				{
					glm::vec2 aa = anchor - anch;
					transform->setPosition(glm::vec3(transform->getPosition().x, transform->getPosition().y + (off.y * aa.y), 0.0f));
					setSize(glm::vec2(getSize().x, getSize().y + off.y));
				}

				props.prevCanvasSize = canv;
			}
		}

		if (!getEnabled() || !getGameObject()->getActive())
			return;

		if (canvas != nullptr)
		{
			float zoom = canvas->getZoom();
			glm::vec2 offset = canvas->getOffset();

			InputManager* mgr = InputManager::getSingleton();
			auto cp = mgr->getMouseRelativePosition();
			glm::vec2 cursorPos = glm::vec2(cp.first, cp.second);

			std::vector<UIElement*> hovered;
			for (auto it = canvas->uiElements.begin(); it != canvas->uiElements.end(); ++it)
			{
				UIElement* elem = *it;
				Properties& elemProps = elem->getProperties();

				if (!elem->getEnabled() || !elem->getGameObject()->getActive())
					continue;

				if (elem != this)
				{
					if (!elemProps.isEditor)
					{
						if (!elem->getRaycastTarget())
							continue;
					}
				}

				glm::vec4 rect = elem->getRect();
				ImRect r1 = ImRect(rect.x, rect.y, rect.z, rect.w);

				for (int k = (int)elemProps.clipRects.size() - 1; k >= 0; k--)
				{
					glm::vec4 clipRect = elemProps.clipRects[k];
					ImRect r2 = ImRect(clipRect.x, clipRect.y, clipRect.z, clipRect.w);
					r1.ClipWithFull(r2);
				}

				rect = glm::vec4(r1.Min.x, r1.Min.y, r1.Max.x, r1.Max.y);

				if (cursorPos.x > rect.x && cursorPos.x < rect.z &&
					cursorPos.y > rect.y && cursorPos.y < rect.w)
				{
					hovered.push_back(elem);
				}
				else if (cursorPos.x > rect.z && cursorPos.x < rect.x &&
					cursorPos.y > rect.w && cursorPos.y < rect.y)
				{
					hovered.push_back(elem);
				}
				else if (cursorPos.x > rect.x && cursorPos.x < rect.z &&
					cursorPos.y > rect.w && cursorPos.y < rect.y)
				{
					hovered.push_back(elem);
				}
				else if (cursorPos.x > rect.z && cursorPos.x < rect.x &&
					cursorPos.y > rect.y && cursorPos.y < rect.w)
				{
					hovered.push_back(elem);
				}
			}

			glm::vec4 thisRect = getRect();

			if (cursorPos.x > thisRect.x && cursorPos.x < thisRect.z &&
				cursorPos.y > thisRect.y && cursorPos.y < thisRect.w)
			{
				props.isHoveredGlobal = true;
			}
			else if (cursorPos.x > thisRect.z && cursorPos.x < thisRect.x &&
				cursorPos.y > thisRect.w && cursorPos.y < thisRect.y)
			{
				props.isHoveredGlobal = true;
			}
			else if (cursorPos.x > thisRect.x && cursorPos.x < thisRect.z &&
				cursorPos.y > thisRect.w && cursorPos.y < thisRect.y)
			{
				props.isHoveredGlobal = true;
			}
			else if (cursorPos.x > thisRect.z && cursorPos.x < thisRect.x &&
				cursorPos.y > thisRect.y && cursorPos.y < thisRect.w)
			{
				props.isHoveredGlobal = true;
			}
			else
			{
				props.isHoveredGlobal = false;
			}

			UIElement* hoveredObj = nullptr;
			int idxMax = -INT_MAX;
			for (auto it = hovered.begin(); it != hovered.end(); ++it)
			{
				int idx = Engine::getSingleton()->getGameObjectIndex((*it)->getGameObject()) + (int)(*it)->getTransform()->getPosition().z;
				if (idx > idxMax)
				{
					hoveredObj = (*it);
					idxMax = idx;
				}
			}

			if (hoveredObj == this)
			{
				onMouseEnter(cursorPos);
			}

			auto it = std::find(hovered.begin(), hovered.end(), this);
			if (it == hovered.end() || hoveredObj != this)
			{
				onMouseExit(cursorPos);
			}

			for (int i = 0; i < 3; ++i)
			{
				if (mgr->getMouseButtonDown(i))
					onMouseDown(i, cursorPos);
				if (mgr->getMouseButtonUp(i))
					onMouseUp(i, cursorPos);
			}

			onMouseMove(cursorPos);

			hovered.clear();
		}
	}

	glm::vec2 UIElement::getParentSize()
	{
		if (canvas == nullptr)
			return glm::vec2(0.0f);

		if (transform->getParent() == nullptr)
			return glm::vec2(0.0f);

		glm::vec3 canv = glm::vec3(0);
		if (transform->getParent()->getGameObject() == canvas->getGameObject())
		{
			ImGuiWindow* win = canvas->getWindow();
			canv = glm::vec3(canvas->getRealScreenSize(win), 0.0f);
		}
		else
		{
			if (cachedParent != nullptr)
				canv = glm::vec3(cachedParent->getSize(), 0.0f);
			else
			{
				std::vector<Component*>& comps = transform->getParent()->getGameObject()->getComponents();
				for (auto it = comps.begin(); it != comps.end(); ++it)
				{
					if ((*it)->isUiComponent())
					{
						cachedParent = (UIElement*)*it;
						canv = glm::vec3(cachedParent->getSize(), 0.0f);
					}
				}
			}
		}

		return canv;
	}

	glm::vec2 UIElement::getParentAnchor()
	{
		if (transform->getParent() == nullptr)
			return glm::vec2(0.0f);

		glm::vec2 canv = glm::vec3(0);
		if (transform->getParent()->getGameObject() == canvas->getGameObject())
			canv = glm::vec2(0.0f);
		else
		{
			if (cachedParent != nullptr)
				canv = cachedParent->getAnchor();
			else
			{
				std::vector<Component*>& comps = transform->getParent()->getGameObject()->getComponents();
				for (auto it = comps.begin(); it != comps.end(); ++it)
				{
					if ((*it)->isUiComponent())
					{
						cachedParent = (UIElement*)*it;
						canv = cachedParent->getAnchor();
					}
				}
			}
		}

		return canv;
	}

	void UIElement::setAnchor(glm::vec2 value)
	{
		glm::vec2 posOffset = value - anchor;
		anchor = value;

		if (transform != nullptr)
		{
			transform->setPosition(glm::vec3(glm::vec2(transform->getPosition()) + posOffset * size, 0.0f));

			std::vector<Transform*>& children = transform->getChildren();
			for (auto it = children.begin(); it != children.end(); ++it)
			{
				Transform* child = *it;
				child->setPosition(glm::vec3(glm::vec2(child->getPosition()) - posOffset * size, 0.0f));
			}
		}
	}

	glm::vec4 UIElement::getRect()
	{
		float zoom = 1.0f;
		glm::vec2 offset = glm::vec2(0.0f);

		if (canvas != nullptr)
		{
			zoom = canvas->getZoom();
			offset = canvas->getOffset();
		}

		float x = transform->getPosition().x;
		float y = transform->getPosition().y;

		if (isnan(x)) x = 0;
		if (isnan(y)) y = 0;

		glm::vec2 posMin = (glm::vec2(x, y) - (getSize() * getAnchor())) * zoom + offset;
		glm::vec2 posMax = (glm::vec2(x, y) + getSize() - (getSize() * getAnchor())) * zoom + offset;

		return glm::vec4(posMin, posMax);
	}
}