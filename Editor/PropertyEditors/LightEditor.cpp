#include "LightEditor.h"

#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"
#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"

#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/Light.h"

#include "PropColorPicker.h"
#include "PropFloat.h"
#include "PropBool.h"
#include "PropComboBox.h"

#include "../Classes/Undo.h"

namespace GX
{
	LightEditor::LightEditor()
	{
		setEditorName("LightEditor");
	}

	LightEditor::~LightEditor()
	{
	}

	void LightEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Light* light = (Light*)comps[0];

		PropComboBox* lightType = new PropComboBox(this, "Type", { "Point", "Spot", "Directional" });
		lightType->setCurrentItem(static_cast<int>(light->getLightType()));
		lightType->setOnChangeCallback([=](Property* prop, int val) { onChangeType(prop, val); });

		PropComboBox* lightRenderMode = new PropComboBox(this, "Render mode", { "Realtime", "Mixed", "Baked" });
		lightRenderMode->setCurrentItem(static_cast<int>(light->getLightRenderMode()));
		lightRenderMode->setOnChangeCallback([=](Property* prop, int val) { onChangeRenderMode(prop, val); });

		PropColorPicker* color = new PropColorPicker(this, "Color", light->getColor());
		color->setOnChangeCallback([=](Property* prop, Color val) { onChangeColor(prop, val); });

		PropFloat* intensity = new PropFloat(this, "Intensity", light->getIntensity());
		intensity->setOnChangeCallback([=](Property* prop, float val) { onChangeIntensity(prop, val); });

		addProperty(lightType);
		addProperty(lightRenderMode);
		addProperty(color);
		addProperty(intensity);

		if (light->getLightType() == LightType::Spot)
		{
			PropFloat* range = new PropFloat(this, "Range", light->getRadius());
			range->setOnChangeCallback([=](Property* prop, float val) { onChangeRadius(prop, val); });

			outerRadius = new PropFloat(this, "Outer radius", light->getOuterRadius());
			outerRadius->setOnChangeCallback([=](Property* prop, float val) { onChangeOuterRadius(prop, val); });
			outerRadius->setMinValue(0.0f);
			outerRadius->setMaxValue(179.0f);

			innerRadius = new PropFloat(this, "Inner radius", light->getInnerRadius());
			innerRadius->setOnChangeCallback([=](Property* prop, float val) { onChangeInnerRadius(prop, val); });
			innerRadius->setMinValue(0.0f);
			innerRadius->setMaxValue(179.0f);
			addProperty(outerRadius);
			addProperty(innerRadius);
			addProperty(range);
		}
		else if (light->getLightType() == LightType::Point)
		{
			PropFloat* radius = new PropFloat(this, "Radius", light->getRadius());
			radius->setOnChangeCallback([=](Property* prop, float val) { onChangeRadius(prop, val); });
			addProperty(radius);
		}

		PropBool* castShadows = new PropBool(this, "Cast shadows", light->getCastShadows());
		castShadows->setOnChangeCallback([=](Property* prop, bool val) { onChangeCastShadows(prop, val); });
		addProperty(castShadows);

		if (light->getCastShadows())
		{
			PropFloat* bias = new PropFloat(this, "Shadow bias", light->getBias());
			bias->setOnChangeCallback([=](Property* prop, float val) { onChangeBias(prop, val); });
			addProperty(bias);
		}
	}

	void LightEditor::onChangeType(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change light type");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Light* comp = (Light*)d.first;
				comp->setLightType(static_cast<LightType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Light* comp = (Light*)d.first;
				comp->setLightType(static_cast<LightType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Light* light = (Light*)(*it);

			undoData->intData[0][light] = static_cast<int>(light->getLightType());
			undoData->intData[1][light] = value;

			light->setLightType(static_cast<LightType>(value));
		}

		updateEditor();
	}

	void LightEditor::onChangeRenderMode(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change light render mode");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Light* comp = (Light*)d.first;
				comp->setLightRenderMode(static_cast<LightRenderMode>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Light* comp = (Light*)d.first;
				comp->setLightRenderMode(static_cast<LightRenderMode>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Light* light = (Light*)(*it);

			undoData->intData[0][light] = static_cast<int>(light->getLightRenderMode());
			undoData->intData[1][light] = value;

			light->setLightRenderMode(static_cast<LightRenderMode>(value));
		}
	}

	void LightEditor::onChangeColor(Property* prop, Color value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change light color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[0])
			{
				Light* comp = (Light*)d.first;
				comp->setColor(Color(d.second.x, d.second.y, d.second.z, d.second.w));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[1])
			{
				Light* comp = (Light*)d.first;
				comp->setColor(Color(d.second.x, d.second.y, d.second.z, d.second.w));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Light* light = (Light*)(*it);

			Color c = light->getColor();
			undoData->vec4Data[0][light] = glm::vec4(c.r(), c.g(), c.b(), c.a());
			undoData->vec4Data[1][light] = glm::vec4(value.r(), value.g(), value.b(), value.a());

			light->setColor(value);
		}
	}

	void LightEditor::onChangeRadius(Property* prop, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change light radius");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				Light* comp = (Light*)d.first;
				comp->setRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				Light* comp = (Light*)d.first;
				comp->setRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Light* light = (Light*)(*it);

			undoData->floatData[0][light] = light->getRadius();
			undoData->floatData[1][light] = value;

			light->setRadius(value);
		}
	}

	void LightEditor::onChangeInnerRadius(Property* prop, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change light inner radius");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				Light* comp = (Light*)d.first;
				comp->setInnerRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				Light* comp = (Light*)d.first;
				comp->setInnerRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Light* light = (Light*)(*it);

			undoData->floatData[0][light] = light->getInnerRadius();
			undoData->floatData[1][light] = value;

			light->setInnerRadius(value);
		}

		if (innerRadius->getValue() > outerRadius->getValue()) outerRadius->setValue(innerRadius->getValue());
	}

	void LightEditor::onChangeOuterRadius(Property* prop, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change light outer radius");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				Light* comp = (Light*)d.first;
				comp->setOuterRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				Light* comp = (Light*)d.first;
				comp->setOuterRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Light* light = (Light*)(*it);
			
			undoData->floatData[0][light] = light->getOuterRadius();
			undoData->floatData[1][light] = value;

			light->setOuterRadius(value);
		}

		if (outerRadius->getValue() < innerRadius->getValue()) innerRadius->setValue(outerRadius->getValue());
	}

	void LightEditor::onChangeIntensity(Property* prop, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change light intensity");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				Light* comp = (Light*)d.first;
				comp->setIntensity(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				Light* comp = (Light*)d.first;
				comp->setIntensity(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Light* light = (Light*)(*it);

			undoData->floatData[0][light] = light->getIntensity();
			undoData->floatData[1][light] = value;

			light->setIntensity(value);
		}
	}

	void LightEditor::onChangeBias(Property* prop, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change light shadow bias");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				Light* comp = (Light*)d.first;
				comp->setBias(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				Light* comp = (Light*)d.first;
				comp->setBias(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Light* light = (Light*)(*it);

			undoData->floatData[0][light] = light->getBias();
			undoData->floatData[1][light] = value;

			light->setBias(value);
		}
	}

	void LightEditor::onChangeCastShadows(Property* prop, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change light cast shadows");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Light* comp = (Light*)d.first;
				comp->setCastShadows(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Light* comp = (Light*)d.first;
				comp->setCastShadows(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Light* light = (Light*)(*it);

			undoData->boolData[0][light] = light->getCastShadows();
			undoData->boolData[1][light] = value;

			light->setCastShadows(value);
		}

		updateEditor();
	}
}