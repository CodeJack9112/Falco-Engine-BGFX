#include "AudioSourceEditor.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Components/Component.h"
#include "../Engine/Components/AudioSource.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Assets/AudioClip.h"
#include "../Engine/Classes/StringConverter.h"

#include "PropVector2.h"
#include "PropVector3.h"
#include "PropVector4.h"
#include "PropBool.h"
#include "PropInt.h"
#include "PropFloat.h"
#include "PropButton.h"

#include "../Windows/MainWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/InspectorWindow.h"

#include "../Classes/Undo.h"

namespace GX
{
	AudioSourceEditor::AudioSourceEditor()
	{
		setEditorName("AudioSourceEditor");
	}

	AudioSourceEditor::~AudioSourceEditor()
	{
	}

	void AudioSourceEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		AudioSource* comp = (AudioSource*)comps[0];

		std::vector<std::string> audioFormats = Engine::getAudioFileFormats();

		std::string clipName = "None";

		AudioClip* audioClip = comp->getAudioClip();
		if (audioClip != nullptr && audioClip->isLoaded())
			clipName = IO::GetFileNameWithExt(audioClip->getName());

		PropButton* fileName = new PropButton(this, "Audio clip", CP_UNI(clipName));
		fileName->setSupportedFormats(audioFormats);
		fileName->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropAudioClip(prop, from); });
		fileName->setOnClickCallback([=](Property* prop) { onClickAudioClip(prop); });
		fileName->setImage(MainWindow::loadEditorIcon("Assets/audio_clip.png"));

		PropBool* playOnStart = new PropBool(this, "Play on start", comp->getPlayOnStart());
		playOnStart->setOnChangeCallback([=](Property* prop, bool val) { onChangePlayOnStart(prop, val); });

		PropBool* loop = new PropBool(this, "Loop", comp->getLooped());
		loop->setOnChangeCallback([=](Property* prop, bool val) { onChangeLoop(prop, val); });

		PropBool* is2D = new PropBool(this, "2D", comp->getIs2D());
		is2D->setOnChangeCallback([=](Property* prop, bool val) { onChangeIs2D(prop, val); });

		PropFloat* volume = new PropFloat(this, "Volume", comp->getVolume());
		volume->setOnChangeCallback([=](Property* prop, float val) { onChangeVolume(prop, val); });
		volume->setMinValue(0.0f);
		volume->setMaxValue(1.0f);

		PropFloat* pitch = new PropFloat(this, "Pitch", comp->getPitch());
		pitch->setOnChangeCallback([=](Property* prop, float val) { onChangePitch(prop, val); });

		addProperty(fileName);
		addProperty(playOnStart);
		addProperty(loop);
		addProperty(is2D);
		addProperty(pitch);
		addProperty(volume);
		
		if (!comp->getIs2D())
		{
			PropFloat* minDistance = new PropFloat(this, "Min distance", comp->getMinDistance());
			minDistance->setOnChangeCallback([=](Property* prop, float val) { onChangeMinDistance(prop, val); });
			addProperty(minDistance);

			PropFloat* maxDistance = new PropFloat(this, "Max distance", comp->getMaxDistance());
			maxDistance->setOnChangeCallback([=](Property* prop, float val) { onChangeMaxDistance(prop, val); });
			addProperty(maxDistance);
		}
	}

	void AudioSourceEditor::onChangePlayOnStart(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change audio source play on start");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setPlayOnStart(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setPlayOnStart(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			AudioSource* comp = (AudioSource*)(*it);

			undoData->boolData[0][comp] = comp->getPlayOnStart();
			undoData->boolData[1][comp] = val;

			comp->setPlayOnStart(val);
		}
	}

	void AudioSourceEditor::onChangeLoop(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change audio source loop");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setLoop(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setLoop(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			AudioSource* comp = (AudioSource*)(*it);

			undoData->boolData[0][comp] = comp->getLooped();
			undoData->boolData[1][comp] = val;

			comp->setLoop(val);
		}
	}

	void AudioSourceEditor::onChangeVolume(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change audio source volume");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setVolume(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setVolume(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			AudioSource* comp = (AudioSource*)(*it);

			undoData->floatData[0][comp] = comp->getVolume();
			undoData->floatData[1][comp] = val;

			comp->setVolume(val);
		}
	}

	void AudioSourceEditor::onChangeMinDistance(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change audio source min distance");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setMinDistance(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setMinDistance(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			AudioSource* comp = (AudioSource*)(*it);

			undoData->floatData[0][comp] = comp->getMinDistance();
			undoData->floatData[1][comp] = val;

			comp->setMinDistance(val);
		}
	}

	void AudioSourceEditor::onChangeMaxDistance(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change audio source max distance");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setMaxDistance(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setMaxDistance(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			AudioSource* comp = (AudioSource*)(*it);
			
			undoData->floatData[0][comp] = comp->getMaxDistance();
			undoData->floatData[1][comp] = val;

			comp->setMaxDistance(val);
		}
	}

	void AudioSourceEditor::onChangeIs2D(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change audio source is 2D");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setIs2D(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setIs2D(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			AudioSource* comp = (AudioSource*)(*it);

			undoData->boolData[0][comp] = comp->getIs2D();
			undoData->boolData[1][comp] = val;

			comp->setIs2D(val);
		}

		updateEditor();
	}

	void AudioSourceEditor::onChangePitch(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change audio source pitch");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setPitch(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				AudioSource* comp = (AudioSource*)d.first;
				comp->setPitch(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			AudioSource* comp = (AudioSource*)(*it);

			undoData->floatData[0][comp] = comp->getPitch();
			undoData->floatData[1][comp] = val;

			comp->setPitch(val);
		}
	}

	void AudioSourceEditor::onDropAudioClip(TreeNode* prop, TreeNode* from)
	{
		std::string fullPath = CP_SYS(from->getPath());
		((PropButton*)prop)->setValue(IO::GetFileNameWithExt(fullPath));

		//Undo
		UndoData* undoData = Undo::addUndo("Change audio source file");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				AudioClip* _audioClip = nullptr;

				if (!d.second.empty())
					_audioClip = AudioClip::load(Engine::getSingleton()->getAssetsPath(), d.second);

				AudioSource* comp = (AudioSource*)d.first;
				comp->setAudioClip(_audioClip);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[1])
			{
				AudioClip* _audioClip = nullptr;

				if (!d.second.empty())
					_audioClip = AudioClip::load(Engine::getSingleton()->getAssetsPath(), d.second);

				AudioSource* comp = (AudioSource*)d.first;
				comp->setAudioClip(_audioClip);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		AudioClip* audioClip = AudioClip::load(Engine::getSingleton()->getAssetsPath(), fullPath);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			AudioSource* comp = (AudioSource*)(*it);

			std::string clipPath = "";
			if (comp->getAudioClip() != nullptr)
				clipPath = comp->getAudioClip()->getName();

			undoData->stringData[0][comp] = clipPath;
			undoData->stringData[1][comp] = fullPath;

			comp->setAudioClip(audioClip);
		}
	}

	void AudioSourceEditor::onClickAudioClip(Property* prop)
	{
		AudioSource* comp = nullptr;
		comp = (AudioSource*)components[0];

		AudioClip* audioClip = comp->getAudioClip();
		if (audioClip != nullptr && audioClip->isLoaded())
			MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(audioClip->getName());
	}
}