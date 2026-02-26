#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;

	class PropEditorHost : public Property
	{
	public:
		PropEditorHost(PropertyEditor* ed, std::string name, PropertyEditor* hosted = nullptr);
		~PropEditorHost();

		virtual void update(bool opened) override;

		void setHostedEditor(PropertyEditor* hosted);
		PropertyEditor* getHostedEditor() { return hostedEditor; }

	private:
		PropertyEditor* hostedEditor = nullptr;
	};
}