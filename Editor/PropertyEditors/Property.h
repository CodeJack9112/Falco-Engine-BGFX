#pragma once

#include <string>
#include <functional>

#include "../Classes/TreeNode.h"

namespace GX
{
	class PropertyEditor;

	class Property : public TreeNode
	{
	public:
		Property(PropertyEditor* ed, std::string name);
		virtual ~Property();

		void update(bool opened) override;

		PropertyEditor* getEditor() { return editor; }

		void setOnClearCallback(std::function<void(Property* prop)> callback) { clearCallback = callback; }
		void callClearCallback();

	private:
		PropertyEditor* editor = nullptr;

		std::function<void(Property * prop)> clearCallback = nullptr;
	};
}