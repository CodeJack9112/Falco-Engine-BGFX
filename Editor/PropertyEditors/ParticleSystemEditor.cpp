#include "ParticleSystemEditor.h"

#include <imgui.h>

#include "../Engine/Core/Engine.h"
#include "../Engine/Components/ParticleSystem.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Classes/IO.h"

#include "../Engine/Serialization/Components/SParticleSystem.h"

#include <sstream>

#include "PropComboBox.h"
#include "PropButton.h"
#include "PropFloat.h"
#include "PropBool.h"
#include "PropInt.h"
#include "PropVector3.h"
#include "PropVector2.h"
#include "PropColorPicker.h"
#include "PropInfo.h"

#include "../Classes/Undo.h"
#include "../Windows/MainWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	ParticleSystemEditor::ParticleSystemEditor()
	{
		setEditorName("ParticleSystemEditor");
	}

	ParticleSystemEditor::~ParticleSystemEditor()
	{
	}

	void ParticleSystemEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Texture* warnIcon = MainWindow::loadEditorIcon("other/warning.png");

		ParticleSystem* ps = (ParticleSystem*)comps[0];

		PropBool* castShadows = new PropBool(this, "Cast shadows", ps->getCastShadows());
		castShadows->setOnChangeCallback([=](Property* prop, bool val) { onChangeCastShadows(val); });

		addProperty(castShadows);

		PropBool* playAtStart = new PropBool(this, "Play at start", ps->getPlayAtStart());
		playAtStart->setOnChangeCallback([=](Property* prop, bool val) { onChangePlayAtStart(val); });

		addProperty(playAtStart);

		PropButton* playPs = new PropButton(this, "Play", "Play");
		playPs->setOnClickCallback([=](Property* prop) { ps->play(); });

		PropButton* stopPs = new PropButton(this, "Stop", "Stop");
		stopPs->setOnClickCallback([=](Property* prop) { ps->stop(); });

		addProperty(playPs);
		addProperty(stopPs);

		Property* emittersList = new Property(this, "Emitters");
		emittersList->setHasButton(true);
		emittersList->setButtonText("+");
		emittersList->setOnButtonClickCallback([=](TreeNode* prop) { onAddEmitter(); });

		addProperty(emittersList);

		std::vector<ParticleEmitter*>& emitters = ps->getEmitters();
		int i = 0;
		for (auto it = emitters.begin(); it != emitters.end(); ++it, ++i)
		{
			ParticleEmitter* emitter = *it;
			Property* emitterProp = new Property(this, std::to_string(i));
			emitterProp->setPopupMenu({ "Clone", "Remove" }, [=](TreeNode* node, int val)
				{
					if (val == 0)
						onCloneEmitter(i);

					if (val == 1)
						onRemoveEmitter(i);
				}
			);

			emittersList->addChild(emitterProp);

			Material* material = emitter->getMaterial();
			std::string matName = "None";
			if (material != nullptr)
				matName = IO::GetFileNameWithExt(material->getName());

			PropButton* materialProp = new PropButton(this, "Material", matName);
			materialProp->setSupportedFormats({ "material" });
			materialProp->setOnDropCallback([=, &material](TreeNode* prop, TreeNode* from)
				{
					std::string path = from->getPath();
					((PropButton*)prop)->setValue(IO::GetFileNameWithExt(path));
					Material* _mat = Material::load(Engine::getSingleton()->getAssetsPath(), path);
					material = _mat;

					for (auto jt = components.begin(); jt != components.end(); ++jt)
					{
						ParticleSystem* ps = (ParticleSystem*)*jt;
						ParticleEmitter* emitter = ps->getEmitter(i);
						emitter->setMaterial(_mat);
					}
				}
			);
			materialProp->setOnClickCallback([=](Property* prop)
				{
					if (material != nullptr)
					{
						std::string path = material->getName();;
						MainWindow::getAssetsWindow()->focusOnFile(path);
					}
				}
			);
			materialProp->setImage(MainWindow::loadEditorIcon("Assets/material.png"));

			emitterProp->addChild(materialProp);

			PropComboBox* simulationSpace = new PropComboBox(this, "Simulation space", { "Local", "World" });
			simulationSpace->setCurrentItem(static_cast<int>(emitter->getSimulationSpace()));
			simulationSpace->setOnChangeCallback([=](Property* prop, int val) { onChangeEmitterSimulationSpace(i, val); });
			emitterProp->addChild(simulationSpace);

			PropComboBox* renderType = new PropComboBox(this, "Render type", { "Billboard", "3D" });
			renderType->setCurrentItem(static_cast<int>(emitter->getRenderType()));
			renderType->setOnChangeCallback([=](Property* prop, int val) { onChangeEmitterRenderType(i, val); });
			emitterProp->addChild(renderType);

			PropComboBox* origin = new PropComboBox(this, "Origin", { "Center", "Bottom" });
			origin->setCurrentItem(static_cast<int>(emitter->getOrigin()));
			origin->setOnChangeCallback([=](Property* prop, int val) { onChangeEmitterOrigin(i, val); });
			emitterProp->addChild(origin);

			PropComboBox* shape = new PropComboBox(this, "Shape", { "Point", "Sphere", "Box", "Circle" });
			shape->setCurrentItem(static_cast<int>(emitter->getShape()));
			shape->setOnChangeCallback([=](Property* prop, int val) { onChangeEmitterShape(i, val); });
			emitterProp->addChild(shape);

			PropInt* emissionCount = new PropInt(this, "Emission count", emitter->getEmissionCount());
			emissionCount->setOnChangeCallback([=](Property* prop, int val) { onChangeEmissionCount(i, val); });
			emitterProp->addChild(emissionCount);

			PropFloat* emissionRate = new PropFloat(this, "Emission rate", emitter->getEmissionRate());
			emissionRate->setOnChangeCallback([=](Property* prop, float val) { onChangeEmissionRate(i, val); });
			emitterProp->addChild(emissionRate);

			PropInt* maxParticles = new PropInt(this, "Max particles", emitter->getMaxParticles());
			maxParticles->setOnChangeCallback([=](Property* prop, int val) { onChangeMaxParticles(i, val); });
			emitterProp->addChild(maxParticles);

			PropFloat* timeScale = new PropFloat(this, "Time scale", emitter->getTimeScale());
			timeScale->setOnChangeCallback([=](Property* prop, float val) { onChangeTimeScale(i, val); });
			emitterProp->addChild(timeScale);

			PropFloat* duration = new PropFloat(this, "Duration", emitter->getDuration());
			duration->setOnChangeCallback([=](Property* prop, float val) { onChangeDuration(i, val); });
			emitterProp->addChild(duration);

			PropFloat* startSizeMin = new PropFloat(this, "Start size min", emitter->getStartSizeMin());
			startSizeMin->setOnChangeCallback([=](Property* prop, float val) { onChangeStartSizeMin(i, val); });
			emitterProp->addChild(startSizeMin);

			PropFloat* startSizeMax = new PropFloat(this, "Start size max", emitter->getStartSizeMax());
			startSizeMax->setOnChangeCallback([=](Property* prop, float val) { onChangeStartSizeMax(i, val); });
			emitterProp->addChild(startSizeMax);

			PropFloat* lifeTimeMin = new PropFloat(this, "Life time min", emitter->getLifeTimeMin());
			lifeTimeMin->setOnChangeCallback([=](Property* prop, float val) { onChangeLifeTimeMin(i, val); });
			emitterProp->addChild(lifeTimeMin);

			PropFloat* lifeTimeMax = new PropFloat(this, "Life time max", emitter->getLifeTimeMax());
			lifeTimeMax->setOnChangeCallback([=](Property* prop, float val) { onChangeLifeTimeMax(i, val); });
			emitterProp->addChild(lifeTimeMax);

			PropFloat* startSpeed = new PropFloat(this, "Start speed", emitter->getStartSpeed());
			startSpeed->setOnChangeCallback([=](Property* prop, float val) { onChangeStartSpeed(i, val); });
			emitterProp->addChild(startSpeed);

			if (!emitter->getEnablePhysics())
			{
				PropVector3* startDirection = new PropVector3(this, "Start direction", emitter->getStartDirection());
				startDirection->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeStartDirection(i, val); });
				emitterProp->addChild(startDirection);
			}

			PropBool* enablePhysics = new PropBool(this, "Enable physics", emitter->getEnablePhysics());
			enablePhysics->setOnChangeCallback([=](Property* prop, bool val) { onChangeEnablePhysics(i, val); });
			emitterProp->addChild(enablePhysics);

			if (emitter->getEnablePhysics())
			{
				PropInfo* alert = new PropInfo(this, "", "Physics simulation will be enabled only in runtime");
				alert->setIcon(warnIcon);
				emitterProp->addChild(alert);
				
				PropFloat* friction = new PropFloat(this, "Friction", emitter->getFriction());
				friction->setOnChangeCallback([=](Property* prop, float val) { onChangeFriction(i, val); });
				emitterProp->addChild(friction);

				PropFloat* bounciness = new PropFloat(this, "Bounciness", emitter->getBounciness());
				bounciness->setOnChangeCallback([=](Property* prop, float val) { onChangeBounciness(i, val); });
				emitterProp->addChild(bounciness);
			}

			PropBool* animated = new PropBool(this, "Animated", emitter->getAnimated());
			animated->setOnChangeCallback([=](Property* prop, bool val) { onChangeAnimated(i, val); });
			emitterProp->addChild(animated);

			if (emitter->getAnimated())
			{
				PropVector2* animationGridSize = new PropVector2(this, "Grid size", emitter->getAnimationGridSize());
				animationGridSize->setValueType(PropVector2::ValueType::Integer);
				animationGridSize->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeAnimationGridSize(i, val); });
				emitterProp->addChild(animationGridSize);

				PropComboBox* animationTimeMode = new PropComboBox(this, "Time mode", { "Lifetime", "Speed", "FPS" });
				animationTimeMode->setCurrentItem(static_cast<int>(emitter->getAnimationTimeMode()));
				animationTimeMode->setOnChangeCallback([=](Property* prop, int val) { onChangeAnimationTimeMode(i, val); });
				emitterProp->addChild(animationTimeMode);

				if (emitter->getAnimationTimeMode() == ParticleEmitter::AnimationTimeMode::FPS ||
					emitter->getAnimationTimeMode() == ParticleEmitter::AnimationTimeMode::Speed)
				{
					PropInt* animationFps = new PropInt(this, "FPS", emitter->getAnimationFps());
					animationFps->setOnChangeCallback([=](Property* prop, int val) { onChangeAnimationFps(i, val); });
					emitterProp->addChild(animationFps);
				}
			}

			PropBool* loop = new PropBool(this, "Loop", emitter->getLoop());
			loop->setOnChangeCallback([=](Property* prop, bool val) { onChangeLoop(i, val); });
			emitterProp->addChild(loop);

			if (emitter->getSimulationSpace() == ParticleEmitter::SimulationSpace::Local)
			{
				if (emitter->getEnablePhysics())
				{
					PropInfo* alert = new PropInfo(this, "", "Physics simulation is not available in local space");
					alert->setIcon(warnIcon);
					emitterProp->addChild(alert);
				}
			}

			if (emitter->getShape() == ParticleEmitterShape::Sphere
				|| emitter->getShape() == ParticleEmitterShape::Circle)
			{
				PropFloat* radius = new PropFloat(this, "Radius", emitter->getRadius());
				radius->setOnChangeCallback([=](Property* prop, float val) { onChangeEmitterRadius(i, val); });
				emitterProp->addChild(radius);
			}

			if (emitter->getShape() == ParticleEmitterShape::Box)
			{
				PropVector3* size = new PropVector3(this, "Size", emitter->getSize());
				size->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeEmitterSize(i, val); });
				emitterProp->addChild(size);
			}

			Property* modifiersList = new Property(this, "Modifiers");
			modifiersList->setHasButton(true);
			modifiersList->setButtonText("+");
			modifiersList->setUseButtonForPopup(true);
			modifiersList->setPopupMenu(
				{
				"Gravity modifier",
				"Color modifier",
				"Size modifier",
				"Direction modifier",
				"Rotation modifier",
				"Speed modifier",
				},
				[=](TreeNode* node, int val)
				{
					onAddModifier(i, val);
				}
			);

			emitterProp->addChild(modifiersList);

			std::vector<ParticleModifier*>& modifiers = emitter->getModifiers();
			int modifierIdx = 0;
			for (auto jt = modifiers.begin(); jt != modifiers.end(); ++jt, ++modifierIdx)
			{
				ParticleModifier* modifier = *jt;

				Property* modifierProp = new Property(this, modifier->getType());
				modifierProp->setPopupMenu({ "Remove" }, [=](TreeNode* node, int val)
					{
						if (val == 0)
							onRemoveModifier(i, modifierIdx);
					}
				);

				PropFloat* startTime = new PropFloat(this, "Start time", modifier->getStartTime());
				startTime->setOnChangeCallback([=](Property* prop, float val) { onChangeModifierStartTime(i, modifierIdx, val); });

				modifierProp->addChild(startTime);

				//Gravity modifier
				if (modifier->getType() == ParticleGravityModifier::MODIFIER_TYPE)
				{
					ParticleGravityModifier* mod = (ParticleGravityModifier*)modifier;
					PropFloat* damping = new PropFloat(this, "Damping", mod->getDamping());
					damping->setOnChangeCallback([=](Property* prop, float val) { onChangeGravityModifierDamping(i, modifierIdx, val); });

					PropVector3* gravity = new PropVector3(this, "Gravity", mod->getGravity());
					gravity->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeGravityModifierGravity(i, modifierIdx, val); });

					modifierProp->addChild(damping);
					modifierProp->addChild(gravity);
				}

				//Color modifier
				if (modifier->getType() == ParticleColorModifier::MODIFIER_TYPE)
				{
					ParticleColorModifier* mod = (ParticleColorModifier*)modifier;
					Property* colorStates = new Property(this, "Color states");
					colorStates->setHasButton(true);
					colorStates->setButtonText("+");
					colorStates->setOnButtonClickCallback([=](TreeNode* prop) { onAddColorModifierColor(i, modifierIdx); });

					std::vector<std::pair<float, Color>>& colors = mod->getColors();
					int c = 0;
					for (auto ct = colors.begin(); ct != colors.end(); ++ct, ++c)
					{
						Property* colorProp = new Property(this, std::to_string(c));
						colorProp->setPopupMenu({ "Remove" }, [=](TreeNode* prop, int val) { onRemoveColorModifierColor(i, modifierIdx, c); });

						PropInt* time = new PropInt(this, "Time (%%)", ct->first * 100);
						time->setMinValue(0);
						time->setMaxValue(100);
						time->setOnChangeCallback([=](Property* prop, int val) { onChangeColorModifierTime(i, modifierIdx, c, val); });

						PropColorPicker* color = new PropColorPicker(this, "Color", ct->second);
						color->setOnChangeCallback([=](Property* prop, Color val) { onChangeColorModifierColor(i, modifierIdx, c, val); });

						colorProp->addChild(time);
						colorProp->addChild(color);
						colorStates->addChild(colorProp);
					}

					modifierProp->addChild(colorStates);
				}

				//Size modifier
				if (modifier->getType() == ParticleSizeModifier::MODIFIER_TYPE)
				{
					ParticleSizeModifier* mod = (ParticleSizeModifier*)modifier;
					Property* sizeStates = new Property(this, "Size states");
					sizeStates->setHasButton(true);
					sizeStates->setButtonText("+");
					sizeStates->setOnButtonClickCallback([=](TreeNode* prop) { onAddSizeModifierSize(i, modifierIdx); });

					std::vector<std::pair<float, float>>& sizes = mod->getSizes();
					int c = 0;
					for (auto ct = sizes.begin(); ct != sizes.end(); ++ct, ++c)
					{
						Property* sizeProp = new Property(this, std::to_string(c));
						sizeProp->setPopupMenu({ "Remove" }, [=](TreeNode* prop, int val) { onRemoveSizeModifierSize(i, modifierIdx, c); });

						PropInt* time = new PropInt(this, "Time (%%)", ct->first * 100);
						time->setMinValue(0);
						time->setMaxValue(100);
						time->setOnChangeCallback([=](Property* prop, int val) { onChangeSizeModifierTime(i, modifierIdx, c, val); });

						PropFloat* size = new PropFloat(this, "Size", ct->second);
						size->setOnChangeCallback([=](Property* prop, float val) { onChangeSizeModifierSize(i, modifierIdx, c, val); });

						sizeProp->addChild(time);
						sizeProp->addChild(size);
						sizeStates->addChild(sizeProp);
					}

					modifierProp->addChild(sizeStates);
				}

				//Direction modifier
				if (modifier->getType() == ParticleDirectionModifier::MODIFIER_TYPE)
				{
					ParticleDirectionModifier* mod = (ParticleDirectionModifier*)modifier;
					PropComboBox* directionType = new PropComboBox(this, "Direction type", { "Constant", "OnceAtStart", "RandomAtStart", "RandomTimed" });
					directionType->setCurrentItem(static_cast<int>(mod->getDirectionType()));
					directionType->setOnChangeCallback([=](Property* prop, int val) { onChangeDirectionModifierType(i, modifierIdx, val); });

					modifierProp->addChild(directionType);

					if (mod->getDirectionType() == ParticleDirectionModifier::DirectionType::Constant
						|| mod->getDirectionType() == ParticleDirectionModifier::DirectionType::OnceAtStart)
					{
						PropVector3* constantDir = new PropVector3(this, "Direction", mod->getConstantDirection());
						constantDir->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeDirectionModifierConstantDir(i, modifierIdx, val); });

						modifierProp->addChild(constantDir);
					}

					if (mod->getDirectionType() == ParticleDirectionModifier::DirectionType::RandomAtStart
						|| mod->getDirectionType() == ParticleDirectionModifier::DirectionType::RandomTimed)
					{
						PropVector3* randomDirMin = new PropVector3(this, "Direction min", mod->getRandomDirectionMin());
						randomDirMin->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeDirectionModifierRandomDirMin(i, modifierIdx, val); });

						modifierProp->addChild(randomDirMin);

						PropVector3* randomDirMax = new PropVector3(this, "Direction max", mod->getRandomDirectionMax());
						randomDirMax->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeDirectionModifierRandomDirMax(i, modifierIdx, val); });

						modifierProp->addChild(randomDirMax);
					}

					if (mod->getDirectionType() == ParticleDirectionModifier::DirectionType::RandomTimed)
					{
						PropFloat* changeInterval = new PropFloat(this, "Change interval", mod->getChangeInterval());
						changeInterval->setOnChangeCallback([=](Property* prop, float val) { onChangeDirectionModifierChangeInterval(i, modifierIdx, val); });

						modifierProp->addChild(changeInterval);
					}
				}

				//Rotation modifier
				if (modifier->getType() == ParticleRotationModifier::MODIFIER_TYPE)
				{
					ParticleRotationModifier* mod = (ParticleRotationModifier*)modifier;
					PropComboBox* rotationType = new PropComboBox(this, "Rotation type", { "Constant", "OnceAtStart", "RandomAtStart", "RandomTimed", "FromDirection" });
					rotationType->setCurrentItem(static_cast<int>(mod->getRotationType()));
					rotationType->setOnChangeCallback([=](Property* prop, int val) { onChangeRotationModifierType(i, modifierIdx, val); });

					modifierProp->addChild(rotationType);

					if (mod->getRotationType() == ParticleRotationModifier::RotationType::FromDirection)
					{
						PropVector3* offsetRot = new PropVector3(this, "Offset rotation", mod->getOffset());
						offsetRot->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeRotationModifierOffset(i, modifierIdx, val); });

						modifierProp->addChild(offsetRot);
					}
					else
					{
						if (mod->getRotationType() == ParticleRotationModifier::RotationType::Constant
							|| mod->getRotationType() == ParticleRotationModifier::RotationType::OnceAtStart)
						{
							PropVector3* constantRot = new PropVector3(this, "Rotation", mod->getConstantRotation());
							constantRot->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeRotationModifierConstantRot(i, modifierIdx, val); });

							modifierProp->addChild(constantRot);
						}

						if (mod->getRotationType() == ParticleRotationModifier::RotationType::RandomAtStart
							|| mod->getRotationType() == ParticleRotationModifier::RotationType::RandomTimed)
						{
							PropVector3* randomRotMin = new PropVector3(this, "Rotation min", mod->getRandomRotationMin());
							randomRotMin->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeRotationModifierRandomRotMin(i, modifierIdx, val); });

							modifierProp->addChild(randomRotMin);

							PropVector3* randomRotMax = new PropVector3(this, "Rotation max", mod->getRandomRotationMax());
							randomRotMax->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeRotationModifierRandomRotMax(i, modifierIdx, val); });

							modifierProp->addChild(randomRotMax);
						}

						if (mod->getRotationType() == ParticleRotationModifier::RotationType::RandomTimed)
						{
							PropFloat* changeInterval = new PropFloat(this, "Change interval", mod->getChangeInterval());
							changeInterval->setOnChangeCallback([=](Property* prop, float val) { onChangeRotationModifierChangeInterval(i, modifierIdx, val); });

							modifierProp->addChild(changeInterval);
						}
					}
				}

				//Speed modifier
				if (modifier->getType() == ParticleSpeedModifier::MODIFIER_TYPE)
				{
					ParticleSpeedModifier* mod = (ParticleSpeedModifier*)modifier;

					PropComboBox* speedType = new PropComboBox(this, "Speed type", { "Constant", "OnceAtStart", "RandomAtStart", "RandomTimed" });
					speedType->setCurrentItem(static_cast<int>(mod->getSpeedType()));
					speedType->setOnChangeCallback([=](Property* prop, int val) { onChangeSpeedModifierType(i, modifierIdx, val); });

					modifierProp->addChild(speedType);

					if (mod->getSpeedType() == ParticleSpeedModifier::SpeedType::Constant
						|| mod->getSpeedType() == ParticleSpeedModifier::SpeedType::OnceAtStart)
					{
						PropFloat* constantSpeed = new PropFloat(this, "Speed", mod->getConstantSpeed());
						constantSpeed->setOnChangeCallback([=](Property* prop, float val) { onChangeSpeedModifierConstantSpeed(i, modifierIdx, val); });

						modifierProp->addChild(constantSpeed);
					}

					if (mod->getSpeedType() == ParticleSpeedModifier::SpeedType::RandomAtStart
						|| mod->getSpeedType() == ParticleSpeedModifier::SpeedType::RandomTimed)
					{
						PropFloat* randomSpeedMin = new PropFloat(this, "Speed min", mod->getRandomSpeedMin());
						randomSpeedMin->setOnChangeCallback([=](Property* prop, float val) { onChangeSpeedModifierRandomSpeedMin(i, modifierIdx, val); });

						modifierProp->addChild(randomSpeedMin);

						PropFloat* randomSpeedMax = new PropFloat(this, "Speed max", mod->getRandomSpeedMax());
						randomSpeedMax->setOnChangeCallback([=](Property* prop, float val) { onChangeSpeedModifierRandomSpeedMax(i, modifierIdx, val); });

						modifierProp->addChild(randomSpeedMax);
					}

					if (mod->getSpeedType() == ParticleSpeedModifier::SpeedType::RandomTimed)
					{
						PropFloat* changeInterval = new PropFloat(this, "Change interval", mod->getChangeInterval());
						changeInterval->setOnChangeCallback([=](Property* prop, float val) { onChangeSpeedModifierChangeInterval(i, modifierIdx, val); });

						modifierProp->addChild(changeInterval);
					}
				}

				modifiersList->addChild(modifierProp);
			}
		}
	}

	std::string ParticleSystemEditor::serializeEmitter(ParticleEmitter* emitter)
	{
		SParticleEmitter sEmitter;
		Material* mat = emitter->getMaterial();
		if (mat != nullptr && mat->isLoaded())
			sEmitter.material = mat->getName();
		sEmitter.simulationSpace = static_cast<int>(emitter->getSimulationSpace());
		sEmitter.renderType = static_cast<int>(emitter->getRenderType());
		sEmitter.origin = static_cast<int>(emitter->getOrigin());
		sEmitter.shape = static_cast<int>(emitter->getShape());
		sEmitter.radius = emitter->getRadius();
		sEmitter.size = emitter->getSize();
		sEmitter.timeScale = emitter->getTimeScale();
		sEmitter.duration = emitter->getDuration();
		sEmitter.maxParticles = emitter->getMaxParticles();
		sEmitter.emissionCount = emitter->getEmissionCount();
		sEmitter.emissionRate = emitter->getEmissionRate();
		sEmitter.lifeTimeMin = emitter->getLifeTimeMin();
		sEmitter.lifeTimeMax = emitter->getLifeTimeMax();
		sEmitter.startSizeMin = emitter->getStartSizeMin();
		sEmitter.startSizeMax = emitter->getStartSizeMax();
		sEmitter.startDirection = emitter->getStartDirection();
		sEmitter.startSpeed = emitter->getStartSpeed();
		sEmitter.animated = emitter->getAnimated();
		sEmitter.animationGridSize = emitter->getAnimationGridSize();
		sEmitter.animationTimeMode = static_cast<int>(emitter->getAnimationTimeMode());
		sEmitter.animationFps = emitter->getAnimationFps();
		sEmitter.loop = emitter->getLoop();

		std::vector<ParticleModifier*>& modifiers = emitter->getModifiers();
		for (auto mod = modifiers.begin(); mod != modifiers.end(); ++mod)
		{
			ParticleModifier* modifier = *mod;
			if (modifier->getType() == ParticleGravityModifier::MODIFIER_TYPE)
			{
				SParticleGravityModifier sModifier;
				ParticleGravityModifier* modifier1 = (ParticleGravityModifier*)modifier;
				sModifier.startTime = modifier->getStartTime();
				sModifier.gravity = modifier1->getGravity();
				sModifier.damping = modifier1->getDamping();

				sEmitter.gravityModifiers.push_back(sModifier);
			}

			if (modifier->getType() == ParticleColorModifier::MODIFIER_TYPE)
			{
				SParticleColorModifier sModifier;
				ParticleColorModifier* modifier1 = (ParticleColorModifier*)modifier;
				sModifier.startTime = modifier->getStartTime();
				auto& colors = modifier1->getColors();
				for (auto c = colors.begin(); c != colors.end(); ++c)
				{
					sModifier.times.push_back(c->first);
					sModifier.colors.push_back(c->second);
				}

				sEmitter.colorModifiers.push_back(sModifier);
			}

			if (modifier->getType() == ParticleSizeModifier::MODIFIER_TYPE)
			{
				SParticleSizeModifier sModifier;
				ParticleSizeModifier* modifier1 = (ParticleSizeModifier*)modifier;
				sModifier.startTime = modifier->getStartTime();
				auto& sizes = modifier1->getSizes();
				for (auto c = sizes.begin(); c != sizes.end(); ++c)
				{
					sModifier.times.push_back(c->first);
					sModifier.sizes.push_back(c->second);
				}

				sEmitter.sizeModifiers.push_back(sModifier);
			}

			if (modifier->getType() == ParticleDirectionModifier::MODIFIER_TYPE)
			{
				SParticleDirectionModifier sModifier;
				ParticleDirectionModifier* modifier1 = (ParticleDirectionModifier*)modifier;
				sModifier.startTime = modifier->getStartTime();
				sModifier.directionType = static_cast<int>(modifier1->getDirectionType());
				sModifier.changeInterval = modifier1->getChangeInterval();
				sModifier.constantDirection = modifier1->getConstantDirection();
				sModifier.randomDirectionMin = modifier1->getRandomDirectionMin();
				sModifier.randomDirectionMax = modifier1->getRandomDirectionMax();

				sEmitter.directionModifiers.push_back(sModifier);
			}

			if (modifier->getType() == ParticleRotationModifier::MODIFIER_TYPE)
			{
				SParticleRotationModifier sModifier;
				ParticleRotationModifier* modifier1 = (ParticleRotationModifier*)modifier;
				sModifier.startTime = modifier->getStartTime();
				sModifier.rotationType = static_cast<int>(modifier1->getRotationType());
				sModifier.changeInterval = modifier1->getChangeInterval();
				sModifier.constantRotation = modifier1->getConstantRotation();
				sModifier.randomRotationMin = modifier1->getRandomRotationMin();
				sModifier.randomRotationMax = modifier1->getRandomRotationMax();
				sModifier.offset = modifier1->getOffset();

				sEmitter.rotationModifiers.push_back(sModifier);
			}

			if (modifier->getType() == ParticleSpeedModifier::MODIFIER_TYPE)
			{
				SParticleSpeedModifier sModifier;
				ParticleSpeedModifier* modifier1 = (ParticleSpeedModifier*)modifier;
				sModifier.startTime = modifier->getStartTime();
				sModifier.speedType = static_cast<int>(modifier1->getSpeedType());
				sModifier.changeInterval = modifier1->getChangeInterval();
				sModifier.constantSpeed = modifier1->getConstantSpeed();
				sModifier.randomSpeedMin = modifier1->getRandomSpeedMin();
				sModifier.randomSpeedMax = modifier1->getRandomSpeedMax();

				sEmitter.speedModifiers.push_back(sModifier);
			}
		}

		std::stringstream ofs;
		BinarySerializer s;
		s.serialize(&ofs, &sEmitter, "");
		
		return ofs.str();
	}

	void ParticleSystemEditor::deserializeEmitter(ParticleEmitter* emitter, std::string value)
	{
		SParticleEmitter sEmitter;

		std::stringstream ifs(value);
		BinarySerializer s;
		s.deserialize(&ifs, &sEmitter, "");
		
		Material* mat = Material::load(Engine::getSingleton()->getAssetsPath(), sEmitter.material);
		emitter->setMaterial(mat);
		emitter->setSimulationSpace(static_cast<ParticleEmitter::SimulationSpace>(sEmitter.simulationSpace));
		emitter->setRenderType(static_cast<ParticleEmitter::RenderType>(sEmitter.renderType));
		emitter->setOrigin(static_cast<ParticleEmitter::Origin>(sEmitter.origin));
		emitter->setShape(static_cast<ParticleEmitterShape>(sEmitter.shape));
		emitter->setRadius(sEmitter.radius);
		emitter->setSize(sEmitter.size.getValue());
		emitter->setTimeScale(sEmitter.timeScale);
		emitter->setDuration(sEmitter.duration);
		emitter->setMaxParticles(sEmitter.maxParticles);
		emitter->setEmissionCount(sEmitter.emissionCount);
		emitter->setEmissionRate(sEmitter.emissionRate);
		emitter->setLifeTimeMin(sEmitter.lifeTimeMin);
		emitter->setLifeTimeMax(sEmitter.lifeTimeMax);
		emitter->setStartSizeMin(sEmitter.startSizeMin);
		emitter->setStartSizeMax(sEmitter.startSizeMax);
		emitter->setStartDirection(sEmitter.startDirection.getValue());
		emitter->setStartSpeed(sEmitter.startSpeed);
		emitter->setAnimated(sEmitter.animated);
		emitter->setAnimationGridSize(sEmitter.animationGridSize.getValue());
		emitter->setAnimationTimeMode(static_cast<ParticleEmitter::AnimationTimeMode>(sEmitter.animationTimeMode));
		emitter->setAnimationFps(sEmitter.animationFps);
		emitter->setLoop(sEmitter.loop);

		while (emitter->getModifiers().size() > 0)
			emitter->removeModifier(0);

		for (auto mod = sEmitter.gravityModifiers.begin(); mod != sEmitter.gravityModifiers.end(); ++mod)
		{
			SParticleGravityModifier& sModifier = *mod;
			ParticleGravityModifier* modifier = emitter->addModifier<ParticleGravityModifier*>();
			modifier->setStartTime(sModifier.startTime);
			modifier->setGravity(sModifier.gravity.getValue());
			modifier->setDamping(sModifier.damping);
		}

		for (auto mod = sEmitter.colorModifiers.begin(); mod != sEmitter.colorModifiers.end(); ++mod)
		{
			SParticleColorModifier& sModifier = *mod;
			ParticleColorModifier* modifier = emitter->addModifier<ParticleColorModifier*>();
			modifier->setStartTime(sModifier.startTime);
			for (int c = 0; c != sModifier.times.size(); ++c)
			{
				modifier->getColors().push_back(std::make_pair(sModifier.times[c], sModifier.colors[c].getValue()));
			}
		}

		for (auto mod = sEmitter.sizeModifiers.begin(); mod != sEmitter.sizeModifiers.end(); ++mod)
		{
			SParticleSizeModifier& sModifier = *mod;
			ParticleSizeModifier* modifier = emitter->addModifier<ParticleSizeModifier*>();
			modifier->setStartTime(sModifier.startTime);
			for (int c = 0; c != sModifier.times.size(); ++c)
			{
				modifier->getSizes().push_back(std::make_pair(sModifier.times[c], sModifier.sizes[c]));
			}
		}

		for (auto mod = sEmitter.directionModifiers.begin(); mod != sEmitter.directionModifiers.end(); ++mod)
		{
			SParticleDirectionModifier& sModifier = *mod;
			ParticleDirectionModifier* modifier = emitter->addModifier<ParticleDirectionModifier*>();
			modifier->setStartTime(sModifier.startTime);
			modifier->setDirectionType(static_cast<ParticleDirectionModifier::DirectionType>(sModifier.directionType));
			modifier->setChangeInterval(sModifier.changeInterval);
			modifier->setConstantDirection(sModifier.constantDirection.getValue());
			modifier->setRandomDirectionMin(sModifier.randomDirectionMin.getValue());
			modifier->setRandomDirectionMax(sModifier.randomDirectionMax.getValue());
		}

		for (auto mod = sEmitter.rotationModifiers.begin(); mod != sEmitter.rotationModifiers.end(); ++mod)
		{
			SParticleRotationModifier& sModifier = *mod;
			ParticleRotationModifier* modifier = emitter->addModifier<ParticleRotationModifier*>();
			modifier->setStartTime(sModifier.startTime);
			modifier->setRotationType(static_cast<ParticleRotationModifier::RotationType>(sModifier.rotationType));
			modifier->setChangeInterval(sModifier.changeInterval);
			modifier->setConstantRotation(sModifier.constantRotation.getValue());
			modifier->setRandomRotationMin(sModifier.randomRotationMin.getValue());
			modifier->setRandomRotationMax(sModifier.randomRotationMax.getValue());
			modifier->setOffset(sModifier.offset.getValue());
		}

		for (auto mod = sEmitter.speedModifiers.begin(); mod != sEmitter.speedModifiers.end(); ++mod)
		{
			SParticleSpeedModifier& sModifier = *mod;
			ParticleSpeedModifier* modifier = emitter->addModifier<ParticleSpeedModifier*>();
			modifier->setStartTime(sModifier.startTime);
			modifier->setSpeedType(static_cast<ParticleSpeedModifier::SpeedType>(sModifier.speedType));
			modifier->setChangeInterval(sModifier.changeInterval);
			modifier->setConstantSpeed(sModifier.constantSpeed);
			modifier->setRandomSpeedMin(sModifier.randomSpeedMin);
			modifier->setRandomSpeedMax(sModifier.randomSpeedMax);
		}
	}

	void ParticleSystemEditor::onChangeCastShadows(bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change particle system cast shadows");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				comp->setCastShadows(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				comp->setCastShadows(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;

			undoData->boolData[0][ps] = ps->getCastShadows();
			undoData->boolData[1][ps] = value;

			ps->setCastShadows(value);
		}
	}

	void ParticleSystemEditor::onChangePlayAtStart(bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change particle system play at start");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				comp->setPlayAtStart(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				comp->setPlayAtStart(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;

			undoData->boolData[0][ps] = ps->getPlayAtStart();
			undoData->boolData[1][ps] = value;

			ps->setPlayAtStart(value);
		}
	}

	void ParticleSystemEditor::onAddEmitter()
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Particle system add emitter");
		undoData->objectData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				comp->removeEmitter(comp->getEmitters().size() - 1);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				comp->addEmitter();

				if (comp->isPlaying())
				{
					comp->stop();
					comp->play();
				}
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ps->addEmitter();

			if (ps->isPlaying())
			{
				ps->stop();
				ps->play();
			}

			undoData->objectData[0].push_back(ps);
		}

		updateEditor();
	}

	void ParticleSystemEditor::onRemoveEmitter(int index)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Particle system remove emitter");
		undoData->objectData.resize(1);
		undoData->stringData.resize(1);
		undoData->intData.resize(1);
		undoData->boolData.resize(1);

		undoData->intData[0][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				auto& emitters = comp->getEmitters();

				ParticleEmitter* em = new ParticleEmitter(comp);
				deserializeEmitter(em, data->stringData[0][comp]);

				emitters.insert(emitters.begin() + idx, em);

				if (data->boolData[0][comp])
				{
					comp->stop();
					comp->play();
				}
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				comp->removeEmitter(idx);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* em = ps->getEmitter(index);

			undoData->objectData[0].push_back(ps);
			undoData->stringData[0][ps] = serializeEmitter(em);
			undoData->boolData[0][ps] = ps->isPlaying();

			ps->removeEmitter(index);
		}

		updateEditor();
	}

	void ParticleSystemEditor::onCloneEmitter(int index)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Particle system clone emitter");
		undoData->objectData.resize(1);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				auto& emitters = comp->getEmitters();
				comp->removeEmitter(emitters.size() - 1);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				
				ParticleEmitter* emitter = comp->getEmitter(idx);
				ParticleEmitter* newEmitter = comp->addEmitter();
				emitter->cloneProperties(newEmitter);
				for (int i = 0; i < emitter->getModifiers().size(); ++i)
					emitter->cloneModifier(i, newEmitter);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;

			undoData->objectData[0].push_back(ps);

			ParticleEmitter* emitter = ps->getEmitter(index);
			ParticleEmitter* newEmitter = ps->addEmitter();
			emitter->cloneProperties(newEmitter);
			for (int i = 0; i < emitter->getModifiers().size(); ++i)
				emitter->cloneModifier(i, newEmitter);
		}

		updateEditor();
	}

	void ParticleSystemEditor::onChangeEmitterSimulationSpace(int emitterIdx, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter simulation space");
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setSimulationSpace(static_cast<ParticleEmitter::SimulationSpace>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[2])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setSimulationSpace(static_cast<ParticleEmitter::SimulationSpace>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->intData[1][ps] = static_cast<int>(emitter->getSimulationSpace());
			undoData->intData[2][ps] = value;

			emitter->setSimulationSpace(static_cast<ParticleEmitter::SimulationSpace>(value));
		}

		updateEditor();
	}

	void ParticleSystemEditor::onChangeEmitterRenderType(int emitterIdx, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter render type");
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setRenderType(static_cast<ParticleEmitter::RenderType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[2])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setRenderType(static_cast<ParticleEmitter::RenderType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->intData[1][ps] = static_cast<int>(emitter->getRenderType());
			undoData->intData[2][ps] = value;

			emitter->setRenderType(static_cast<ParticleEmitter::RenderType>(value));
		}

		updateEditor();
	}

	void ParticleSystemEditor::onChangeEmitterOrigin(int emitterIdx, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter origin");
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setOrigin(static_cast<ParticleEmitter::Origin>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[2])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setOrigin(static_cast<ParticleEmitter::Origin>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->intData[1][ps] = static_cast<int>(emitter->getOrigin());
			undoData->intData[2][ps] = value;

			emitter->setOrigin(static_cast<ParticleEmitter::Origin>(value));
		}
	}

	void ParticleSystemEditor::onChangeEmitterShape(int emitterIdx, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter shape");
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setShape(static_cast<ParticleEmitterShape>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[2])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setShape(static_cast<ParticleEmitterShape>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->intData[1][ps] = static_cast<int>(emitter->getShape());
			undoData->intData[2][ps] = value;

			emitter->setShape(static_cast<ParticleEmitterShape>(value));
		}

		updateEditor();
	}

	void ParticleSystemEditor::onChangeEmitterRadius(int emitterIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter radius");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setRadius(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->floatData[0][ps] = emitter->getRadius();
			undoData->floatData[1][ps] = value;

			emitter->setRadius(value);
		}
	}

	void ParticleSystemEditor::onChangeEmitterSize(int emitterIdx, glm::vec3 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter size");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->vec3Data[0][ps] = emitter->getSize();
			undoData->vec3Data[1][ps] = value;

			emitter->setSize(value);
		}
	}

	void ParticleSystemEditor::onChangeTimeScale(int emitterIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter time scale");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setTimeScale(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setTimeScale(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->floatData[0][ps] = emitter->getTimeScale();
			undoData->floatData[1][ps] = value;

			emitter->setTimeScale(value);
		}
	}

	void ParticleSystemEditor::onChangeEmissionRate(int emitterIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter emission rate");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setEmissionRate(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setEmissionRate(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->floatData[0][ps] = emitter->getEmissionRate();
			undoData->floatData[1][ps] = value;

			emitter->setEmissionRate(value);
		}
	}

	void ParticleSystemEditor::onChangeEmissionCount(int emitterIdx, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter emission count");
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setEmissionCount(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[2])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setEmissionCount(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->intData[1][ps] = emitter->getEmissionCount();
			undoData->intData[2][ps] = value;

			emitter->setEmissionCount(value >= 0 ? value : 0);
		}
	}

	void ParticleSystemEditor::onChangeDuration(int emitterIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter duration");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setDuration(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setDuration(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->floatData[0][ps] = emitter->getDuration();
			undoData->floatData[1][ps] = value;

			emitter->setDuration(value);
		}
	}

	void ParticleSystemEditor::onChangeMaxParticles(int emitterIdx, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter max particles");
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setMaxParticles(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[2])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setMaxParticles(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->intData[1][ps] = emitter->getMaxParticles();
			undoData->intData[2][ps] = value >= 0 ? value : 0;

			emitter->setMaxParticles(value >= 0 ? value : 0);
		}
	}

	void ParticleSystemEditor::onChangeStartSizeMin(int emitterIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter start size min");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setStartSizeMin(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setStartSizeMin(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->floatData[0][ps] = emitter->getStartSizeMin();
			undoData->floatData[1][ps] = value;

			emitter->setStartSizeMin(value);
		}
	}

	void ParticleSystemEditor::onChangeStartSizeMax(int emitterIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter start size max");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setStartSizeMax(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setStartSizeMax(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->floatData[0][ps] = emitter->getStartSizeMax();
			undoData->floatData[1][ps] = value;

			emitter->setStartSizeMax(value);
		}
	}

	void ParticleSystemEditor::onChangeStartSpeed(int emitterIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter start speed");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setStartSpeed(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setStartSpeed(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->floatData[0][ps] = emitter->getStartSpeed();
			undoData->floatData[1][ps] = value;

			emitter->setStartSpeed(value);
		}
	}

	void ParticleSystemEditor::onChangeStartDirection(int emitterIdx, glm::vec3 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter start direction");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setStartDirection(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setStartDirection(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->vec3Data[0][ps] = emitter->getStartDirection();
			undoData->vec3Data[1][ps] = value;

			emitter->setStartDirection(value);
		}
	}

	void ParticleSystemEditor::onChangeLifeTimeMin(int emitterIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter life time min");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setLifeTimeMin(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setLifeTimeMin(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->floatData[0][ps] = emitter->getLifeTimeMin();
			undoData->floatData[1][ps] = value;

			emitter->setLifeTimeMin(value);
		}
	}

	void ParticleSystemEditor::onChangeLifeTimeMax(int emitterIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter life time max");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setLifeTimeMax(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setLifeTimeMax(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->floatData[0][ps] = emitter->getLifeTimeMax();
			undoData->floatData[1][ps] = value;

			emitter->setLifeTimeMax(value);
		}
	}

	void ParticleSystemEditor::onChangeEnablePhysics(int emitterIdx, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter enable physics");
		undoData->boolData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->boolData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setEnablePhysics(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->boolData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setEnablePhysics(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->boolData[0][ps] = emitter->getEnablePhysics();
			undoData->boolData[1][ps] = value;

			emitter->setEnablePhysics(value);
		}

		MainWindow::getInspectorWindow()->updateCurrentEditor();
	}

	void ParticleSystemEditor::onChangeFriction(int emitterIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter friction");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setFriction(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setFriction(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->floatData[0][ps] = emitter->getFriction();
			undoData->floatData[1][ps] = value;

			emitter->setFriction(value);
		}
	}

	void ParticleSystemEditor::onChangeBounciness(int emitterIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter bounciness");
		undoData->floatData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setBounciness(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setBounciness(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->floatData[0][ps] = emitter->getBounciness();
			undoData->floatData[1][ps] = value;

			emitter->setBounciness(value);
		}
	}

	void ParticleSystemEditor::onChangeAnimated(int emitterIdx, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter is animated");
		undoData->boolData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->boolData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setAnimated(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->boolData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setAnimated(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->boolData[0][ps] = emitter->getAnimated();
			undoData->boolData[1][ps] = value;

			emitter->setAnimated(value);
		}

		MainWindow::getInspectorWindow()->updateCurrentEditor();
	}

	void ParticleSystemEditor::onChangeAnimationGridSize(int emitterIdx, glm::vec2 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter animation grid size");
		undoData->vec2Data.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec2Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setAnimationGridSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->vec2Data[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setAnimationGridSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->vec2Data[0][ps] = emitter->getAnimationGridSize();
			undoData->vec2Data[1][ps] = value;

			emitter->setAnimationGridSize(value);
		}
	}

	void ParticleSystemEditor::onChangeAnimationTimeMode(int emitterIdx, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter animation time mode");
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setAnimationTimeMode(static_cast<ParticleEmitter::AnimationTimeMode>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[2])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setAnimationTimeMode(static_cast<ParticleEmitter::AnimationTimeMode>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->intData[1][ps] = static_cast<int>(emitter->getAnimationTimeMode());
			undoData->intData[2][ps] = value;

			emitter->setAnimationTimeMode(static_cast<ParticleEmitter::AnimationTimeMode>(value));
		}

		MainWindow::getInspectorWindow()->updateCurrentEditor();
	}

	void ParticleSystemEditor::onChangeAnimationFps(int emitterIdx, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter animation fps");
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setAnimationFps(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->intData[2])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setAnimationFps(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->intData[1][ps] = emitter->getAnimationFps();
			undoData->intData[2][ps] = value;

			emitter->setAnimationFps(value);
		}
	}

	void ParticleSystemEditor::onChangeLoop(int emitterIdx, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change emitter is looped");
		undoData->boolData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = emitterIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->boolData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setLoop(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->boolData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->setLoop(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->boolData[0][ps] = emitter->getLoop();
			undoData->boolData[1][ps] = value;

			emitter->setLoop(value);
		}
	}

	void ParticleSystemEditor::onAddModifier(int emitterIdx, int index)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Add emitter modifier");
		undoData->objectData.resize(1);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				emitter->removeModifier(emitter->getModifiers().size() - 1);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				ParticleEmitter* emitter = comp->getEmitter(idx);
				
				if (midx == 0) emitter->addModifier<ParticleGravityModifier*>();
				if (midx == 1) emitter->addModifier<ParticleColorModifier*>();
				if (midx == 2) emitter->addModifier<ParticleSizeModifier*>();
				if (midx == 3) emitter->addModifier<ParticleDirectionModifier*>();
				if (midx == 4) emitter->addModifier<ParticleRotationModifier*>();
				if (midx == 5) emitter->addModifier<ParticleSpeedModifier*>();
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			if (index == 0) emitter->addModifier<ParticleGravityModifier*>();
			if (index == 1) emitter->addModifier<ParticleColorModifier*>();
			if (index == 2) emitter->addModifier<ParticleSizeModifier*>();
			if (index == 3) emitter->addModifier<ParticleDirectionModifier*>();
			if (index == 4) emitter->addModifier<ParticleRotationModifier*>();
			if (index == 5) emitter->addModifier<ParticleSpeedModifier*>();

			undoData->objectData[0].push_back(ps);
		}

		updateEditor();
	}

	void ParticleSystemEditor::onRemoveModifier(int emitterIdx, int index)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Remove emitter modifier");
		undoData->objectData.resize(1);
		undoData->stringData.resize(1);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];

				deserializeEmitter(em, data->stringData[0][comp]);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];

				em->removeModifier(midx);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);

			undoData->objectData[0].push_back(ps);
			undoData->stringData[0][ps] = serializeEmitter(emitter);

			emitter->removeModifier(index);
		}

		updateEditor();
	}

	void ParticleSystemEditor::onChangeModifierStartTime(int emitterIdx, int modifierIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change modifier start time");
		undoData->floatData.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleModifier* modifier = em->getModifier(midx);

				modifier->setStartTime(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleModifier* modifier = em->getModifier(midx);

				modifier->setStartTime(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleModifier* modifier = emitter->getModifier(modifierIdx);

			undoData->floatData[0][ps] = modifier->getStartTime();
			undoData->floatData[1][ps] = value;

			modifier->setStartTime(value);
		}
	}

	void ParticleSystemEditor::onChangeGravityModifierDamping(int emitterIdx, int modifierIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change gravity modifier damping");
		undoData->floatData.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleGravityModifier* modifier = (ParticleGravityModifier*)em->getModifier(midx);

				modifier->setDamping(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleGravityModifier* modifier = (ParticleGravityModifier*)em->getModifier(midx);

				modifier->setDamping(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleGravityModifier* modifier = (ParticleGravityModifier*)emitter->getModifier(modifierIdx);

			undoData->floatData[0][ps] = modifier->getDamping();
			undoData->floatData[1][ps] = value;

			modifier->setDamping(value);
		}
	}

	void ParticleSystemEditor::onChangeGravityModifierGravity(int emitterIdx, int modifierIdx, glm::vec3 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change gravity modifier gravity");
		undoData->vec3Data.resize(1);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleGravityModifier* modifier = (ParticleGravityModifier*)em->getModifier(midx);

				modifier->setGravity(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleGravityModifier* modifier = (ParticleGravityModifier*)em->getModifier(midx);

				modifier->setGravity(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleGravityModifier* modifier = (ParticleGravityModifier*)emitter->getModifier(modifierIdx);

			modifier->setGravity(value);
		}
	}

	void ParticleSystemEditor::onAddColorModifierColor(int emitterIdx, int modifierIdx)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Add color modifier state");
		undoData->objectData.resize(1);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleColorModifier* modifier = (ParticleColorModifier*)em->getModifier(midx);

				auto& colors = modifier->getColors();
				colors.erase(colors.begin() + colors.size() - 1);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleColorModifier* modifier = (ParticleColorModifier*)em->getModifier(midx);

				modifier->getColors().push_back(std::make_pair(0, Color::White));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleColorModifier* modifier = (ParticleColorModifier*)emitter->getModifier(modifierIdx);

			modifier->getColors().push_back(std::make_pair(0, Color::White));

			undoData->objectData[0].push_back(ps);
		}

		updateEditor();
	}

	void ParticleSystemEditor::onRemoveColorModifierColor(int emitterIdx, int modifierIdx, int index)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Remove color modifier state");
		undoData->floatData.resize(1);
		undoData->vec4Data.resize(1);
		undoData->objectData.resize(1);
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;
		undoData->intData[2][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];
			int cidx = data->intData[2][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleColorModifier* modifier = (ParticleColorModifier*)em->getModifier(midx);

				auto& colors = modifier->getColors();
				Color col = Color(data->vec4Data[0][comp].x, data->vec4Data[0][comp].y, data->vec4Data[0][comp].z, data->vec4Data[0][comp].w);
				colors.insert(colors.begin() + cidx, { data->floatData[0][comp], col });
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];
			int cidx = data->intData[2][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleColorModifier* modifier = (ParticleColorModifier*)em->getModifier(midx);

				auto& colors = modifier->getColors();
				colors.erase(colors.begin() + cidx);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleColorModifier* modifier = (ParticleColorModifier*)emitter->getModifier(modifierIdx);

			std::vector<std::pair<float, Color>>& colors = modifier->getColors();

			undoData->floatData[0][ps] = colors[index].first;
			undoData->vec4Data[0][ps] = glm::vec4(colors[index].second[0], colors[index].second[1], colors[index].second[2], colors[index].second[3]);
			undoData->objectData[0].push_back(ps);

			colors.erase(colors.begin() + index);
		}

		updateEditor();
	}

	void ParticleSystemEditor::onChangeColorModifierTime(int emitterIdx, int modifierIdx, int index, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change color modifier time");
		undoData->floatData.resize(2);
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;
		undoData->intData[2][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];
			int cidx = data->intData[2][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleColorModifier* modifier = (ParticleColorModifier*)em->getModifier(midx);

				auto& colors = modifier->getColors();
				colors[index].first = (float)d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];
			int cidx = data->intData[2][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleColorModifier* modifier = (ParticleColorModifier*)em->getModifier(midx);

				auto& colors = modifier->getColors();
				colors[index].first = (float)d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleColorModifier* modifier = (ParticleColorModifier*)emitter->getModifier(modifierIdx);

			std::vector<std::pair<float, Color>>& colors = modifier->getColors();

			undoData->floatData[0][ps] = colors[index].first;
			undoData->floatData[1][ps] = (float)value / 100.0f;

			colors[index].first = (float)value / 100.0f;
		}
	}

	void ParticleSystemEditor::onChangeColorModifierColor(int emitterIdx, int modifierIdx, int index, Color value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change color modifier color");
		undoData->vec4Data.resize(2);
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;
		undoData->intData[2][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];
			int cidx = data->intData[2][nullptr];

			for (auto& d : data->vec4Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleColorModifier* modifier = (ParticleColorModifier*)em->getModifier(midx);

				auto& colors = modifier->getColors();
				colors[index].second = Color(d.second.x, d.second.y, d.second.z, d.second.w);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];
			int cidx = data->intData[2][nullptr];

			for (auto& d : data->vec4Data[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleColorModifier* modifier = (ParticleColorModifier*)em->getModifier(midx);

				auto& colors = modifier->getColors();
				colors[index].second = Color(d.second.x, d.second.y, d.second.z, d.second.w);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleColorModifier* modifier = (ParticleColorModifier*)emitter->getModifier(modifierIdx);

			std::vector<std::pair<float, Color>>& colors = modifier->getColors();

			undoData->vec4Data[0][ps] = glm::vec4(colors[index].second[0], colors[index].second[1], colors[index].second[2], colors[index].second[3]);
			undoData->vec4Data[1][ps] = glm::vec4(value[0], value[1], value[2], value[3]);

			colors[index].second = value;
		}
	}

	void ParticleSystemEditor::onAddSizeModifierSize(int emitterIdx, int modifierIdx)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Add size modifier state");
		undoData->objectData.resize(1);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSizeModifier* modifier = (ParticleSizeModifier*)em->getModifier(midx);

				auto& sizes = modifier->getSizes();
				sizes.erase(sizes.begin() + sizes.size() - 1);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSizeModifier* modifier = (ParticleSizeModifier*)em->getModifier(midx);

				modifier->getSizes().push_back(std::make_pair(0, 0));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleSizeModifier* modifier = (ParticleSizeModifier*)emitter->getModifier(modifierIdx);

			modifier->getSizes().push_back(std::make_pair(0, 0));
		}

		updateEditor();
	}

	void ParticleSystemEditor::onRemoveSizeModifierSize(int emitterIdx, int modifierIdx, int index)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Remove size modifier state");
		undoData->floatData.resize(2);
		undoData->objectData.resize(1);
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;
		undoData->intData[2][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];
			int cidx = data->intData[2][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSizeModifier* modifier = (ParticleSizeModifier*)em->getModifier(midx);

				auto& sizes = modifier->getSizes();
				sizes.insert(sizes.begin() + cidx, { data->floatData[0][comp], data->floatData[1][comp] });
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];
			int cidx = data->intData[2][nullptr];

			for (auto& d : data->objectData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSizeModifier* modifier = (ParticleSizeModifier*)em->getModifier(midx);

				auto& sizes = modifier->getSizes();
				sizes.erase(sizes.begin() + cidx);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleSizeModifier* modifier = (ParticleSizeModifier*)emitter->getModifier(modifierIdx);

			std::vector<std::pair<float, float>>& sizes = modifier->getSizes();

			undoData->floatData[0][ps] = sizes[index].first;
			undoData->floatData[1][ps] = sizes[index].second;
			undoData->objectData[0].push_back(ps);

			sizes.erase(sizes.begin() + index);
		}

		updateEditor();
	}

	void ParticleSystemEditor::onChangeSizeModifierTime(int emitterIdx, int modifierIdx, int index, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change size modifier time");
		undoData->floatData.resize(2);
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;
		undoData->intData[2][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];
			int cidx = data->intData[2][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSizeModifier* modifier = (ParticleSizeModifier*)em->getModifier(midx);

				auto& sizes = modifier->getSizes();
				sizes[index].first = (float)d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];
			int cidx = data->intData[2][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSizeModifier* modifier = (ParticleSizeModifier*)em->getModifier(midx);

				auto& sizes = modifier->getSizes();
				sizes[index].first = (float)d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleSizeModifier* modifier = (ParticleSizeModifier*)emitter->getModifier(modifierIdx);

			std::vector<std::pair<float, float>>& sizes = modifier->getSizes();

			undoData->floatData[0][ps] = sizes[index].first;
			undoData->floatData[1][ps] = (float)value / 100.0f;

			sizes[index].first = (float)value / 100.0f;
		}
	}

	void ParticleSystemEditor::onChangeSizeModifierSize(int emitterIdx, int modifierIdx, int index, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change size modifier size");
		undoData->floatData.resize(2);
		undoData->intData.resize(3);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;
		undoData->intData[2][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];
			int cidx = data->intData[2][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSizeModifier* modifier = (ParticleSizeModifier*)em->getModifier(midx);

				auto& sizes = modifier->getSizes();
				sizes[index].second = (float)d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];
			int cidx = data->intData[2][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSizeModifier* modifier = (ParticleSizeModifier*)em->getModifier(midx);

				auto& sizes = modifier->getSizes();
				sizes[index].second = (float)d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleSizeModifier* modifier = (ParticleSizeModifier*)emitter->getModifier(modifierIdx);

			std::vector<std::pair<float, float>>& sizes = modifier->getSizes();

			undoData->floatData[0][ps] = sizes[index].first;
			undoData->floatData[1][ps] = value;

			sizes[index].second = value;
		}
	}

	void ParticleSystemEditor::onChangeDirectionModifierType(int emitterIdx, int modifierIdx, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change direction modifier type");
		undoData->intData.resize(4);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->intData[2])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)em->getModifier(midx);

				modifier->setDirectionType(static_cast<ParticleDirectionModifier::DirectionType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->intData[3])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)em->getModifier(midx);

				modifier->setDirectionType(static_cast<ParticleDirectionModifier::DirectionType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)emitter->getModifier(modifierIdx);

			undoData->intData[2][ps] = static_cast<int>(modifier->getDirectionType());
			undoData->intData[3][ps] = value;

			modifier->setDirectionType(static_cast<ParticleDirectionModifier::DirectionType>(value));
		}

		updateEditor();
	}

	void ParticleSystemEditor::onChangeDirectionModifierChangeInterval(int emitterIdx, int modifierIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change direction modifier change interval");
		undoData->floatData.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)em->getModifier(midx);

				modifier->setChangeInterval(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)em->getModifier(midx);

				modifier->setChangeInterval(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)emitter->getModifier(modifierIdx);

			undoData->floatData[0][ps] = modifier->getChangeInterval();
			undoData->floatData[1][ps] = value;

			modifier->setChangeInterval(value);
		}
	}

	void ParticleSystemEditor::onChangeDirectionModifierConstantDir(int emitterIdx, int modifierIdx, glm::vec3 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change direction modifier constant direction");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)em->getModifier(midx);

				modifier->setConstantDirection(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)em->getModifier(midx);

				modifier->setConstantDirection(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)emitter->getModifier(modifierIdx);

			undoData->vec3Data[0][ps] = modifier->getConstantDirection();
			undoData->vec3Data[1][ps] = value;

			modifier->setConstantDirection(value);
		}
	}

	void ParticleSystemEditor::onChangeDirectionModifierRandomDirMin(int emitterIdx, int modifierIdx, glm::vec3 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change random direction min");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)em->getModifier(midx);

				modifier->setRandomDirectionMin(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)em->getModifier(midx);

				modifier->setRandomDirectionMin(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)emitter->getModifier(modifierIdx);

			undoData->vec3Data[0][ps] = modifier->getRandomDirectionMin();
			undoData->vec3Data[1][ps] = value;

			modifier->setRandomDirectionMin(value);
		}
	}

	void ParticleSystemEditor::onChangeDirectionModifierRandomDirMax(int emitterIdx, int modifierIdx, glm::vec3 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change random direction max");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)em->getModifier(midx);

				modifier->setRandomDirectionMax(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)em->getModifier(midx);

				modifier->setRandomDirectionMax(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleDirectionModifier* modifier = (ParticleDirectionModifier*)emitter->getModifier(modifierIdx);

			undoData->vec3Data[0][ps] = modifier->getRandomDirectionMax();
			undoData->vec3Data[1][ps] = value;

			modifier->setRandomDirectionMax(value);
		}
	}

	void ParticleSystemEditor::onChangeRotationModifierType(int emitterIdx, int modifierIdx, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rotation modifier type");
		undoData->intData.resize(4);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->intData[2])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleRotationModifier* modifier = (ParticleRotationModifier*)em->getModifier(midx);

				modifier->setRotationType(static_cast<ParticleRotationModifier::RotationType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->intData[3])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleRotationModifier* modifier = (ParticleRotationModifier*)em->getModifier(midx);

				modifier->setRotationType(static_cast<ParticleRotationModifier::RotationType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleRotationModifier* modifier = (ParticleRotationModifier*)emitter->getModifier(modifierIdx);

			undoData->intData[2][ps] = static_cast<int>(modifier->getRotationType());
			undoData->intData[3][ps] = value;

			modifier->setRotationType(static_cast<ParticleRotationModifier::RotationType>(value));
		}

		updateEditor();
	}

	void ParticleSystemEditor::onChangeRotationModifierChangeInterval(int emitterIdx, int modifierIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rotation modifier change interval");
		undoData->floatData.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleRotationModifier* modifier = (ParticleRotationModifier*)em->getModifier(midx);

				modifier->setChangeInterval(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleRotationModifier* modifier = (ParticleRotationModifier*)em->getModifier(midx);

				modifier->setChangeInterval(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleRotationModifier* modifier = (ParticleRotationModifier*)emitter->getModifier(modifierIdx);

			undoData->floatData[0][ps] = modifier->getChangeInterval();
			undoData->floatData[1][ps] = value;

			modifier->setChangeInterval(value);
		}
	}

	void ParticleSystemEditor::onChangeRotationModifierOffset(int emitterIdx, int modifierIdx, glm::vec3 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rotation modifier offset");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleRotationModifier* modifier = (ParticleRotationModifier*)em->getModifier(midx);

				modifier->setOffset(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleRotationModifier* modifier = (ParticleRotationModifier*)em->getModifier(midx);

				modifier->setOffset(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleRotationModifier* modifier = (ParticleRotationModifier*)emitter->getModifier(modifierIdx);

			undoData->vec3Data[0][ps] = modifier->getOffset();
			undoData->vec3Data[1][ps] = value;

			modifier->setOffset(value);
		}
	}

	void ParticleSystemEditor::onChangeRotationModifierConstantRot(int emitterIdx, int modifierIdx, glm::vec3 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rotation modifier constant rotation");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleRotationModifier* modifier = (ParticleRotationModifier*)em->getModifier(midx);

				modifier->setConstantRotation(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleRotationModifier* modifier = (ParticleRotationModifier*)em->getModifier(midx);

				modifier->setConstantRotation(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleRotationModifier* modifier = (ParticleRotationModifier*)emitter->getModifier(modifierIdx);

			undoData->vec3Data[0][ps] = modifier->getConstantRotation();
			undoData->vec3Data[1][ps] = value;

			modifier->setConstantRotation(value);
		}
	}

	void ParticleSystemEditor::onChangeRotationModifierRandomRotMin(int emitterIdx, int modifierIdx, glm::vec3 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rotation modifier random rotation min");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleRotationModifier* modifier = (ParticleRotationModifier*)em->getModifier(midx);

				modifier->setRandomRotationMin(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleRotationModifier* modifier = (ParticleRotationModifier*)em->getModifier(midx);

				modifier->setRandomRotationMin(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleRotationModifier* modifier = (ParticleRotationModifier*)emitter->getModifier(modifierIdx);

			undoData->vec3Data[0][ps] = modifier->getRandomRotationMin();
			undoData->vec3Data[1][ps] = value;

			modifier->setRandomRotationMin(value);
		}
	}

	void ParticleSystemEditor::onChangeRotationModifierRandomRotMax(int emitterIdx, int modifierIdx, glm::vec3 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change rotation modifier random rotation max");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleRotationModifier* modifier = (ParticleRotationModifier*)em->getModifier(midx);

				modifier->setRandomRotationMax(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleRotationModifier* modifier = (ParticleRotationModifier*)em->getModifier(midx);

				modifier->setRandomRotationMax(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleRotationModifier* modifier = (ParticleRotationModifier*)emitter->getModifier(modifierIdx);

			undoData->vec3Data[0][ps] = modifier->getRandomRotationMax();
			undoData->vec3Data[1][ps] = value;

			modifier->setRandomRotationMax(value);
		}
	}

	void ParticleSystemEditor::onChangeSpeedModifierType(int emitterIdx, int modifierIdx, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change speed modifier type");
		undoData->intData.resize(4);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->intData[2])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)em->getModifier(midx);

				modifier->setSpeedType(static_cast<ParticleSpeedModifier::SpeedType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->intData[3])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)em->getModifier(midx);

				modifier->setSpeedType(static_cast<ParticleSpeedModifier::SpeedType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)emitter->getModifier(modifierIdx);

			undoData->intData[2][ps] = static_cast<int>(modifier->getSpeedType());
			undoData->intData[3][ps] = value;

			modifier->setSpeedType(static_cast<ParticleSpeedModifier::SpeedType>(value));
		}

		updateEditor();
	}

	void ParticleSystemEditor::onChangeSpeedModifierChangeInterval(int emitterIdx, int modifierIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change speed modifier change interval");
		undoData->floatData.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)em->getModifier(midx);

				modifier->setChangeInterval(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)em->getModifier(midx);

				modifier->setChangeInterval(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)emitter->getModifier(modifierIdx);

			undoData->floatData[0][ps] = modifier->getChangeInterval();
			undoData->floatData[1][ps] = value;

			modifier->setChangeInterval(value);
		}
	}

	void ParticleSystemEditor::onChangeSpeedModifierConstantSpeed(int emitterIdx, int modifierIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change speed modifier constant speed");
		undoData->floatData.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)em->getModifier(midx);

				modifier->setConstantSpeed(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)em->getModifier(midx);

				modifier->setConstantSpeed(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)emitter->getModifier(modifierIdx);

			undoData->floatData[0][ps] = modifier->getConstantSpeed();
			undoData->floatData[1][ps] = value;

			modifier->setConstantSpeed(value);
		}
	}

	void ParticleSystemEditor::onChangeSpeedModifierRandomSpeedMin(int emitterIdx, int modifierIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change speed modifier random speed min");
		undoData->floatData.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)em->getModifier(midx);

				modifier->setRandomSpeedMin(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)em->getModifier(midx);

				modifier->setRandomSpeedMin(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)emitter->getModifier(modifierIdx);

			undoData->floatData[0][ps] = modifier->getRandomSpeedMin();
			undoData->floatData[1][ps] = value;

			modifier->setRandomSpeedMin(value);
		}
	}

	void ParticleSystemEditor::onChangeSpeedModifierRandomSpeedMax(int emitterIdx, int modifierIdx, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change speed modifier random speed max");
		undoData->floatData.resize(2);
		undoData->intData.resize(2);

		undoData->intData[0][nullptr] = emitterIdx;
		undoData->intData[1][nullptr] = modifierIdx;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[0])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)em->getModifier(midx);

				modifier->setRandomSpeedMax(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];
			int midx = data->intData[1][nullptr];

			for (auto& d : data->floatData[1])
			{
				ParticleSystem* comp = (ParticleSystem*)d.first;
				auto& emitters = comp->getEmitters();
				ParticleEmitter* em = emitters[idx];
				ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)em->getModifier(midx);

				modifier->setRandomSpeedMax(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			ParticleSystem* ps = (ParticleSystem*)*jt;
			ParticleEmitter* emitter = ps->getEmitter(emitterIdx);
			ParticleSpeedModifier* modifier = (ParticleSpeedModifier*)emitter->getModifier(modifierIdx);

			undoData->floatData[0][ps] = modifier->getRandomSpeedMax();
			undoData->floatData[1][ps] = value;

			modifier->setRandomSpeedMax(value);
		}
	}
}