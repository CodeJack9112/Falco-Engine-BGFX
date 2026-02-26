#include "API_UIElement.h"

#include "../Core/Engine.h"
#include "../Renderer/Renderer.h"
#include "../Core/APIManager.h"

#include "../Components/UIElement.h"
#include "../Components/Canvas.h"

namespace GX
{
	void API_UIElement::getColor(MonoObject* this_ptr, API::Color* out_color)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		Color _color = uiElement->getColor();

		out_color->r = _color.r();
		out_color->g = _color.g();
		out_color->b = _color.b();
		out_color->a = _color.a();
	}

	void API_UIElement::setColor(MonoObject* this_ptr, API::Color* ref_color)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		uiElement->setColor(Color(ref_color->r, ref_color->g, ref_color->b, ref_color->a));
	}

	void API_UIElement::getAnchor(MonoObject* this_ptr, API::Vector2* out_value)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		glm::vec2 _anchor = uiElement->getAnchor();

		out_value->x = _anchor.x;
		out_value->y = _anchor.y;
	}

	void API_UIElement::setAnchor(MonoObject* this_ptr, API::Vector2* ref_value)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		uiElement->setAnchor(glm::vec2(ref_value->x, ref_value->y));
	}

	int API_UIElement::getHorizontalAlignment(MonoObject* this_ptr)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		return static_cast<int>(uiElement->getHorizontalAlignment());
	}

	void API_UIElement::setHorizontalAlignment(MonoObject* this_ptr, int value)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		uiElement->setHorizontalAlignment(static_cast<CanvasHorizontalAlignment>(value));
	}

	int API_UIElement::getVerticalAlignment(MonoObject* this_ptr)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		return static_cast<int>(uiElement->getVerticalAlignment());
	}

	void API_UIElement::setVerticalAlignment(MonoObject* this_ptr, int value)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		uiElement->setVerticalAlignment(static_cast<CanvasVerticalAlignment>(value));
	}

	void API_UIElement::getSize(MonoObject* this_ptr, API::Vector2* out_value)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		glm::vec2 _value = uiElement->getSize();

		out_value->x = _value.x;
		out_value->y = _value.y;
	}

	void API_UIElement::setSize(MonoObject* this_ptr, API::Vector2* ref_value)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		uiElement->setSize(glm::vec2(ref_value->x, ref_value->y));
	}

	bool API_UIElement::getHovered(MonoObject* this_ptr)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		UIElement::Properties props = uiElement->getProperties();

		return props.isHovered;
	}

	MonoObject* API_UIElement::getCanvas(MonoObject* this_ptr)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		Canvas* canvas = uiElement->getCanvas();

		return canvas->getManagedObject();
	}

	void API_UIElement::getPosition(MonoObject* this_ptr, API::Vector3* out_value)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		glm::vec3 _value = uiElement->getPosition();

		out_value->x = _value.x;
		out_value->y = _value.y;
		out_value->z = _value.z;
	}

	void API_UIElement::setPosition(MonoObject* this_ptr, API::Vector3* ref_value)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		uiElement->setPosition(glm::vec3(ref_value->x, ref_value->y, ref_value->z));
	}

	void API_UIElement::getRect(MonoObject* this_ptr, API::Rect* out_value)
	{
		UIElement* uiElement = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uiElement));

		glm::vec4 _value = uiElement->getRect();

		out_value->m_XMin = _value.x;
		out_value->m_YMin = _value.y;
		out_value->m_Width = _value.z;
		out_value->m_Height = _value.w;
	}
}