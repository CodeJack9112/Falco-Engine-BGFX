#include "AnimationEditor.h"

#include "../Windows/MainWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/InspectorWindow.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/Animation.h"
#include "../Engine/Assets/AnimationClip.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Classes/IO.h"

#include "Property.h"
#include "PropButton.h"
#include "PropString.h"
#include "PropInt.h"
#include "PropBool.h"
#include "PropButton2.h"
#include "PropFloat.h"

#include "../Classes/Undo.h"

namespace GX
{
	AnimationEditor::AnimationEditor()
	{
		setEditorName("AnimationEditor");
	}

	AnimationEditor::~AnimationEditor()
	{
	}

	void AnimationEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Animation* animation = (Animation*)comps[0];

		Property* animationsProp = new Property(this, "Animation clips");
		animationsProp->setSupportedFormats({ "animation" });
		animationsProp->setHasButton(true);
		animationsProp->setButtonText("+");
		animationsProp->setOnButtonClickCallback([=](TreeNode* prop) { onAddAnimationClip(prop); });
		animationsProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropAnimationClip(prop, from); });
		
		for (int i = 0; i < animation->getNumAnimationClips(); ++i)
		{
			AnimationClipInfo * clip = animation->getAnimationClip(i);

			PropInt* clipStartFrame = new PropInt(this, "Start frame", clip->startFrame);;
			PropInt* clipEndFrame = new PropInt(this, "End frame", clip->endFrame);;

			Property* clipGroup = new Property(this, IO::GetFileName(clip->name));
			clipGroup->setOnClearCallback([=](Property* prop) { onRemoveAnimationClip(prop, i); });
			clipGroup->setPopupMenu({ "Remove" }, [=](TreeNode* node, int val)
				{
					if (val == 0)
					{
						onRemoveAnimationClip(node, i);
					}
				}
			);

			std::string cName = "None";
			if (clip->clip != nullptr)
				cName = IO::GetFileName(clip->clip->getName());

			PropButton* clipProp = new PropButton(this, "Clip", cName);
			clipProp->setSupportedFormats({ "animation" });
			clipProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropAnimationClipClip(prop, from, i, clipEndFrame); });
			clipProp->setOnClickCallback([=](Property * prop)
				{
					AnimationClipInfo* inf = animation->getAnimationClip(i);
					if (inf->clip != nullptr)
						MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(inf->clip->getName());
				}
			);
			clipProp->setImage(MainWindow::loadEditorIcon("Assets/animation_clip.png"));

			PropString* clipName = new PropString(this, "Name", clip->name);
			clipName->setOnChangeCallback([=](Property* prop, std::string val) { onChangeAnimationClipName(prop, i, val); });

			PropFloat* clipSpeed = new PropFloat(this, "Speed", clip->speed);
			clipSpeed->setOnChangeCallback([=](Property* prop, float val) { onChangeAnimationClipSpeed(prop, i, val); });

			clipStartFrame->setIsDraggable(false);
			clipStartFrame->setOnChangeCallback([=](Property* prop, int val) { onChangeAnimationClipStartFrame(prop, i, val); });

			clipEndFrame->setIsDraggable(false);
			clipEndFrame->setOnChangeCallback([=](Property* prop, int val) { onChangeAnimationClipEndFrame(prop, i, val); });

			PropBool* loop = new PropBool(this, "Loop", clip->loop);
			loop->setOnChangeCallback([=](Property* prop, bool val) { onChangeAnimationClipLoop(prop, i, val); });

			PropButton2* playStop = new PropButton2(this, "Preview", "Play", "Stop");
			playStop->setOnClickCallback([=](Property* prop, int btn)
				{
					for (auto it = components.begin(); it != components.end(); ++it)
					{
						Animation* anim = (Animation*)*it;
						if (btn == 0)
							anim->play(i);
						else
							anim->stop();
					}
				}
			);

			clipGroup->addChild(clipProp);
			clipGroup->addChild(clipName);
			clipGroup->addChild(clipSpeed);
			clipGroup->addChild(clipStartFrame);
			clipGroup->addChild(clipEndFrame);
			clipGroup->addChild(loop);
			clipGroup->addChild(playStop);
			animationsProp->addChild(clipGroup);
		}

		addProperty(animationsProp);
	}

	void AnimationEditor::onAddAnimationClip(TreeNode* prop)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Add animation clip");
		undoData->objectData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Animation* anim = (Animation*)d;
				anim->removeAnimationClip(anim->getAnimationClips().size() - 1);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Animation* anim = (Animation*)d;
				
				AnimationClipInfo info;
				info.clip = nullptr;
				info.name = "New animation";
				info.startFrame = 0;
				info.endFrame = 100;
				anim->addAnimationClip(info);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Animation* anim = (Animation*)*it;

			undoData->objectData[0].push_back(anim);

			AnimationClipInfo info;
			info.clip = nullptr;
			info.name = "New animation";
			info.startFrame = 0;
			info.endFrame = 100;
			anim->addAnimationClip(info);
		}

		updateEditor();
	}

	void AnimationEditor::onDropAnimationClip(TreeNode* prop, TreeNode* from)
	{
		std::string clipPath = from->getPath();
		AnimationClip* clip = AnimationClip::load(Engine::getSingleton()->getAssetsPath(), clipPath);

		//Undo
		UndoData* undoData = Undo::addUndo("Add animation clip");
		undoData->stringData.resize(1);
		undoData->objectData.resize(1);

		undoData->stringData[0][nullptr] = clipPath;

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Animation* anim = (Animation*)d;
				anim->removeAnimationClip(anim->getAnimationClips().size() - 1);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			std::string _clipPath = data->stringData[0][nullptr];
			AnimationClip* _clip = AnimationClip::load(Engine::getSingleton()->getAssetsPath(), _clipPath);

			for (auto& d : data->objectData[0])
			{
				Animation* anim = (Animation*)d;
				AnimationClipInfo info;
				info.clip = _clip;
				info.name = IO::GetFileName(_clip->getName());
				info.startFrame = 0;
				info.endFrame = _clip->getDuration();
				anim->addAnimationClip(info);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Animation* anim = (Animation*)*it;

			undoData->objectData[0].push_back(anim);

			AnimationClipInfo info;
			info.clip = clip;
			info.name = IO::GetFileName(clip->getName());
			info.startFrame = 0;
			info.endFrame = clip->getDuration();
			anim->addAnimationClip(info);
		}

		updateEditor();
	}

	void AnimationEditor::onRemoveAnimationClip(TreeNode* prop, int index)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Remove animation clip");
		undoData->intData.resize(3);
		undoData->floatData.resize(1);
		undoData->boolData.resize(1);
		undoData->stringData.resize(2);
		undoData->objectData.resize(1);

		undoData->intData[0][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->objectData[0])
			{
				std::string _clipPath = data->stringData[0][d];
				AnimationClip* _clip = AnimationClip::load(Engine::getSingleton()->getAssetsPath(), _clipPath);

				Animation* anim = (Animation*)d;
				AnimationClipInfo info;
				info.clip = _clip;
				info.name = data->stringData[1][d];
				info.speed = data->floatData[0][d];
				info.startFrame = data->intData[1][d];
				info.endFrame = data->intData[2][d];
				info.loop = data->boolData[0][d];
				auto& anims = anim->getAnimationClips();
				anims.insert(anims.begin() + idx, info);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->objectData[0])
			{
				Animation* anim = (Animation*)d;
				anim->removeAnimationClip(idx);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Animation* anim = (Animation*)*it;

			AnimationClipInfo* inf = anim->getAnimationClip(index);
			std::string clipName = "";
			if (inf->clip != nullptr)
				clipName = inf->clip->getName();
			undoData->stringData[0][anim] = clipName;
			undoData->stringData[1][anim] = inf->name;
			undoData->floatData[0][anim] = inf->speed;
			undoData->intData[1][anim] = inf->startFrame;
			undoData->intData[2][anim] = inf->endFrame;
			undoData->boolData[0][anim] = inf->loop;
			undoData->objectData[0].push_back(anim);

			anim->removeAnimationClip(index);
		}

		updateEditor();
	}

	void AnimationEditor::onDropAnimationClipClip(TreeNode* prop, TreeNode* from, int index, Property* endFrameProp)
	{
		((PropButton*)prop)->setValue(IO::GetFileName(from->alias));
		std::string clipPath = from->getPath();

		AnimationClip* aclip = AnimationClip::load(Engine::getSingleton()->getAssetsPath(), clipPath);

		//Undo
		UndoData* undoData = Undo::addUndo("Change animation clip file");
		undoData->stringData.resize(2);
		undoData->intData.resize(5);
		undoData->intData[0][nullptr] = index;

		for (auto& it : components)
		{
			Animation* anim = (Animation*)it;
			AnimationClipInfo* inf = anim->getAnimationClip(index);
			std::string prevClipName = "";
			if (inf->clip != nullptr)
				prevClipName = inf->clip->getName();

			undoData->stringData[0][it] = prevClipName;
			undoData->stringData[1][it] = clipPath;
			undoData->intData[1][it] = inf->startFrame;
			undoData->intData[2][it] = inf->endFrame;
		}

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->stringData[0])
			{
				AnimationClip* _aclip = nullptr;
				if (!d.second.empty())
					_aclip = AnimationClip::load(Engine::getSingleton()->getAssetsPath(), d.second);
				AnimationClipInfo* inf = ((Animation*)d.first)->getAnimationClip(idx);
				inf->clip = _aclip;
				inf->startFrame = data->intData[1][d.first];
				inf->endFrame = data->intData[2][d.first];
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->stringData[1])
			{
				AnimationClip* _aclip = nullptr;
				if (!d.second.empty())
					_aclip = AnimationClip::load(Engine::getSingleton()->getAssetsPath(), d.second);
				AnimationClipInfo* inf = ((Animation*)d.first)->getAnimationClip(idx);
				inf->clip = _aclip;
				inf->startFrame = data->intData[3][d.first];
				inf->endFrame = data->intData[4][d.first];
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		Animation* anim0 = (Animation*)components[0];
		AnimationClipInfo* inf0 = anim0->getAnimationClip(index);
		if (inf0->endFrame > aclip->getDuration())
			((PropInt*)endFrameProp)->setValue(aclip->getDuration());

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Animation* anim = (Animation*)*it;
			AnimationClipInfo* inf = anim->getAnimationClip(index);
			inf->clip = aclip;
			if (inf->endFrame > inf->clip->getDuration())
				inf->endFrame = inf->clip->getDuration();
			if (inf->startFrame > inf->endFrame)
				inf->startFrame = inf->endFrame;

			undoData->intData[3][anim] = inf->startFrame;
			undoData->intData[4][anim] = inf->endFrame;
		}
	}

	void AnimationEditor::onChangeAnimationClipName(TreeNode* prop, int index, std::string val)
	{
		prop->parent->alias = val;

		//Undo
		UndoData* undoData = Undo::addUndo("Change animation clip name");
		undoData->stringData.resize(2);
		undoData->intData.resize(1);
		undoData->intData[0][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->stringData[0])
			{
				Animation* anim = (Animation*)d.first;
				AnimationClipInfo* inf = anim->getAnimationClip(idx);
				inf->name = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->stringData[1])
			{
				Animation* anim = (Animation*)d.first;
				AnimationClipInfo* inf = anim->getAnimationClip(idx);
				inf->name = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Animation* anim = (Animation*)*it;
			AnimationClipInfo* inf = anim->getAnimationClip(index);

			undoData->stringData[0][anim] = inf->name;
			undoData->stringData[1][anim] = val;

			inf->name = val;
		}
	}

	void AnimationEditor::onChangeAnimationClipStartFrame(TreeNode* prop, int index, int val)
	{
		Animation* anim0 = (Animation*)components[0];
		AnimationClipInfo* inf0 = anim0->getAnimationClip(index);

		if (val < 0)
			((PropInt*)prop)->setValue(0);

		if (val > inf0->clip->getDuration())
			((PropInt*)prop)->setValue(inf0->clip->getDuration());

		//Undo
		UndoData* undoData = Undo::addUndo("Change animation clip start frame");
		undoData->intData.resize(3);
		undoData->intData[0][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[1])
			{
				Animation* anim = (Animation*)d.first;
				AnimationClipInfo* inf = anim->getAnimationClip(idx);
				inf->startFrame = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[2])
			{
				Animation* anim = (Animation*)d.first;
				AnimationClipInfo* inf = anim->getAnimationClip(idx);
				inf->startFrame = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Animation* anim = (Animation*)*it;
			AnimationClipInfo* inf = anim->getAnimationClip(index);

			int _val = val;

			if (inf->clip != nullptr)
			{
				if (_val < 0)
					_val = 0;

				if (_val > inf->clip->getDuration())
					_val = inf->clip->getDuration();

				if (_val > inf->endFrame)
					inf->endFrame = _val;
			}

			undoData->intData[1][anim] = inf->startFrame;
			undoData->intData[2][anim] = _val;

			inf->startFrame = _val;
		}
	}

	void AnimationEditor::onChangeAnimationClipEndFrame(TreeNode* prop, int index, int val)
	{
		Animation* anim0 = (Animation*)components[0];
		AnimationClipInfo* inf0 = anim0->getAnimationClip(index);

		if (val < 0)
			((PropInt*)prop)->setValue(0);

		if (val > inf0->clip->getDuration())
			((PropInt*)prop)->setValue(inf0->clip->getDuration());

		//Undo
		UndoData* undoData = Undo::addUndo("Change animation clip end frame");
		undoData->intData.resize(3);
		undoData->intData[0][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[1])
			{
				Animation* anim = (Animation*)d.first;
				AnimationClipInfo* inf = anim->getAnimationClip(idx);
				inf->endFrame = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[2])
			{
				Animation* anim = (Animation*)d.first;
				AnimationClipInfo* inf = anim->getAnimationClip(idx);
				inf->endFrame = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Animation* anim = (Animation*)*it;
			AnimationClipInfo* inf = anim->getAnimationClip(index);

			int _val = val;
			
			if (inf->clip != nullptr)
			{
				if (_val < 0)
					_val = 0;

				if (_val > inf->clip->getDuration())
					_val = inf->clip->getDuration();

				if (_val < inf->startFrame)
					inf->startFrame = _val;
			}

			undoData->intData[1][anim] = inf->endFrame;
			undoData->intData[2][anim] = _val;

			inf->endFrame = _val;
		}
	}

	void AnimationEditor::onChangeAnimationClipSpeed(TreeNode* prop, int index, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change animation clip speed");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);
		undoData->intData[0][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				Animation* anim = (Animation*)d.first;
				AnimationClipInfo* inf = anim->getAnimationClip(idx);
				inf->speed = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				Animation* anim = (Animation*)d.first;
				AnimationClipInfo* inf = anim->getAnimationClip(idx);
				inf->speed = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Animation* anim = (Animation*)*it;
			AnimationClipInfo* inf = anim->getAnimationClip(index);

			undoData->floatData[0][anim] = inf->speed;
			undoData->floatData[1][anim] = val;

			inf->speed = val;
		}
	}

	void AnimationEditor::onChangeAnimationClipLoop(TreeNode* prop, int index, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change animation clip loop");
		undoData->boolData.resize(2);
		undoData->intData.resize(1);
		undoData->intData[0][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->boolData[0])
			{
				Animation* anim = (Animation*)d.first;
				AnimationClipInfo* inf = anim->getAnimationClip(idx);
				inf->loop = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->boolData[1])
			{
				Animation* anim = (Animation*)d.first;
				AnimationClipInfo* inf = anim->getAnimationClip(idx);
				inf->loop = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Animation* anim = (Animation*)*it;
			AnimationClipInfo* inf = anim->getAnimationClip(index);

			undoData->boolData[0][anim] = inf->loop;
			undoData->boolData[1][anim] = val;

			inf->loop = val;
		}
	}
}