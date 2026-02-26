#include "AudioListenerEditor.h"

#include "../Engine/Components/Component.h"
#include "../Engine/Components/AudioListener.h"
#include "../Engine/Core/GameObject.h"

#include "PropBool.h"
#include "PropFloat.h"

#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

#include "../Classes/Undo.h"

namespace GX
{
	AudioListenerEditor::AudioListenerEditor()
	{
		setEditorName("AudioListenerEditor");
	}

	AudioListenerEditor::~AudioListenerEditor()
	{
	}

	void AudioListenerEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		AudioListener* comp = (AudioListener*)comps[0];

		PropFloat* volume = new PropFloat(this, "Volume", comp->getVolume());
		volume->setOnChangeCallback([=](Property* prop, float val) { onChangeVolume(prop, val); });
		volume->setMinValue(0.0f);
		volume->setMaxValue(1.0f);

		addProperty(volume);
	}

	void AudioListenerEditor::onChangeVolume(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change audio listener volume");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				AudioListener* comp = (AudioListener*)d.first;
				comp->setVolume(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				AudioListener* comp = (AudioListener*)d.first;
				comp->setVolume(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			AudioListener* comp = (AudioListener*)*it;

			undoData->floatData[0][comp] = comp->getVolume();
			undoData->floatData[1][comp] = val;

			comp->setVolume(val);
		}
	}
}