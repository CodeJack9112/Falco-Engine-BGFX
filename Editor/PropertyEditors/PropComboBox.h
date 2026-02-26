#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;

	class PropComboBox : public Property
	{
	private:
		struct Node
		{
		public:
			std::string value = "";
			Node* parent = nullptr;
			std::vector<Node*> children;
			std::string getPath();
		};

	public:
		PropComboBox(PropertyEditor* ed, std::string name, std::vector<std::string> val);
		~PropComboBox();

		virtual void update(bool opened) override;
		void setValue(std::vector<std::string> val);
		void setValue(int index, std::string val);
		void setValueEnabled(std::vector<bool> val);
		void setValueEnabled(int index, bool val);
		bool getValueEnabled(int index);
		void setLabel(std::string val) { label = val; }
		void setCurrentItem(int val) { item_current = val; }
		void setCurrentItem(std::string val);
		std::vector<std::string> getValue() { return value; }
		bool getIsNested() { return isNested; }
		void setIsNested(bool value);
		void setOnChangeCallback(std::function<void(Property * prop, std::string val)> callback) { onChangeCallback = callback; }
		void setOnChangeCallback(std::function<void(Property * prop, int val)> callback) { onChangeCallback2 = callback; }

	private:
		std::vector<std::string> value;
		std::vector<bool> valuesEnabled;
		int item_current = 0;
		std::string guid1 = "";
		std::string label = "";
		bool isNested = false;
		std::vector<Node*> nodes;

		void updateList(Node* node, const char*& current, int& n);
		void rebuildNestedList();

		std::function<void(Property * prop, std::string val)> onChangeCallback = nullptr;
		std::function<void(Property * prop, int val)> onChangeCallback2 = nullptr;
	};
}