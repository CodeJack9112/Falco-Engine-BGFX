#include "VideoPlayerEditor.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Components/Component.h"
#include "../Engine/Components/VideoPlayer.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Assets/VideoClip.h"
#include "../Engine/Classes/StringConverter.h"

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
	VideoPlayerEditor::VideoPlayerEditor()
	{
		setEditorName("VideoPlayerEditor");
	}

	VideoPlayerEditor::~VideoPlayerEditor()
	{
	}

	void VideoPlayerEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		VideoPlayer* comp = (VideoPlayer*)comps[0];

		std::vector<std::string> audioFormats = Engine::getVideoFileFormats();

		std::string clipName = "None";

		VideoClip* videoClip = comp->getVideoClip();
		if (videoClip != nullptr && videoClip->isLoaded())
			clipName = IO::GetFileNameWithExt(videoClip->getName());

		PropButton* fileName = new PropButton(this, "Video clip", CP_UNI(clipName));
		fileName->setSupportedFormats(audioFormats);
		fileName->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropVideoClip(prop, from); });
		fileName->setOnClickCallback([=](Property* prop) { onClickVideoClip(prop); });
		fileName->setImage(MainWindow::loadEditorIcon("Assets/video_clip.png"));

		PropBool* playOnStart = new PropBool(this, "Play on start", comp->getPlayOnStart());
		playOnStart->setOnChangeCallback([=](Property* prop, bool val) { onChangePlayOnStart(prop, val); });

		PropBool* loop = new PropBool(this, "Loop", comp->getLoop());
		loop->setOnChangeCallback([=](Property* prop, bool val) { onChangeLoop(prop, val); });

		addProperty(fileName);
		addProperty(playOnStart);
		addProperty(loop);
	}

	void VideoPlayerEditor::onChangePlayOnStart(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change video player play on start");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				VideoPlayer* comp = (VideoPlayer*)d.first;
				comp->setPlayOnStart(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				VideoPlayer* comp = (VideoPlayer*)d.first;
				comp->setPlayOnStart(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			VideoPlayer* comp = (VideoPlayer*)(*it);

			undoData->boolData[0][comp] = comp->getPlayOnStart();
			undoData->boolData[1][comp] = val;

			comp->setPlayOnStart(val);
		}
	}

	void VideoPlayerEditor::onChangeLoop(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change video player loop");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				VideoPlayer* comp = (VideoPlayer*)d.first;
				comp->setLoop(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				VideoPlayer* comp = (VideoPlayer*)d.first;
				comp->setLoop(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			VideoPlayer* comp = (VideoPlayer*)(*it);

			undoData->boolData[0][comp] = comp->getLoop();
			undoData->boolData[1][comp] = val;

			comp->setLoop(val);
		}
	}

	void VideoPlayerEditor::onDropVideoClip(TreeNode* prop, TreeNode* from)
	{
		std::string fullPath = CP_SYS(from->getPath());
		((PropButton*)prop)->setValue(IO::GetFileNameWithExt(fullPath));

		//Undo
		UndoData* undoData = Undo::addUndo("Change video player file");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				VideoClip* _audioClip = nullptr;

				if (!d.second.empty())
					_audioClip = VideoClip::load(Engine::getSingleton()->getAssetsPath(), d.second);

				VideoPlayer* comp = (VideoPlayer*)d.first;
				comp->setVideoClip(_audioClip);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[1])
			{
				VideoClip* _audioClip = nullptr;

				if (!d.second.empty())
					_audioClip = VideoClip::load(Engine::getSingleton()->getAssetsPath(), d.second);

				VideoPlayer* comp = (VideoPlayer*)d.first;
				comp->setVideoClip(_audioClip);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		VideoClip* audioClip = VideoClip::load(Engine::getSingleton()->getAssetsPath(), fullPath);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			VideoPlayer* comp = (VideoPlayer*)(*it);

			std::string clipPath = "";
			if (comp->getVideoClip() != nullptr)
				clipPath = comp->getVideoClip()->getName();

			undoData->stringData[0][comp] = clipPath;
			undoData->stringData[1][comp] = fullPath;

			comp->setVideoClip(audioClip);
		}
	}

	void VideoPlayerEditor::onClickVideoClip(Property* prop)
	{
		VideoPlayer* comp = nullptr;
		comp = (VideoPlayer*)components[0];

		VideoClip* audioClip = comp->getVideoClip();
		if (audioClip != nullptr && audioClip->isLoaded())
			MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(audioClip->getName());
	}
}