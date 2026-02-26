#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/glm.hpp"
#include "../Engine/Renderer/Color.h"

namespace GX
{
	class Property;
	class ParticleEmitter;

	class ParticleSystemEditor : public ComponentEditor
	{
	public:
		ParticleSystemEditor();
		~ParticleSystemEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		std::string serializeEmitter(ParticleEmitter* emitter);
		void deserializeEmitter(ParticleEmitter* emitter, std::string value);

		void onChangeCastShadows(bool value);
		void onChangePlayAtStart(bool value);

		void onAddEmitter();
		void onRemoveEmitter(int index);
		void onCloneEmitter(int index);
		void onChangeEmitterSimulationSpace(int emitterIdx, int value);
		void onChangeEmitterRenderType(int emitterIdx, int value);
		void onChangeEmitterOrigin(int emitterIdx, int value);
		void onChangeEmitterShape(int emitterIdx, int value);
		void onChangeEmitterRadius(int emitterIdx, float value);
		void onChangeEmitterSize(int emitterIdx, glm::vec3 value);
		void onChangeTimeScale(int emitterIdx, float value);
		void onChangeEmissionRate(int emitterIdx, float value);
		void onChangeEmissionCount(int emitterIdx, int value);
		void onChangeDuration(int emitterIdx, float value);
		void onChangeMaxParticles(int emitterIdx, int value);
		void onChangeStartSizeMin(int emitterIdx, float value);
		void onChangeStartSizeMax(int emitterIdx, float value);
		void onChangeStartSpeed(int emitterIdx, float value);
		void onChangeStartDirection(int emitterIdx, glm::vec3 value);
		void onChangeLifeTimeMin(int emitterIdx, float value);
		void onChangeLifeTimeMax(int emitterIdx, float value);
		void onChangeEnablePhysics(int emitterIdx, bool value);
		void onChangeFriction(int emitterIdx, float value);
		void onChangeBounciness(int emitterIdx, float value);
		void onChangeAnimated(int emitterIdx, bool value);
		void onChangeAnimationGridSize(int emitterIdx, glm::vec2 value);
		void onChangeAnimationTimeMode(int emitterIdx, int value);
		void onChangeAnimationFps(int emitterIdx, int value);
		void onChangeLoop(int emitterIdx, bool value);

		void onAddModifier(int emitterIdx, int index);
		void onRemoveModifier(int emitterIdx, int index);
		void onChangeModifierStartTime(int emitterIdx, int modifierIdx, float value);

		void onChangeGravityModifierDamping(int emitterIdx, int modifierIdx, float value);
		void onChangeGravityModifierGravity(int emitterIdx, int modifierIdx, glm::vec3 value);

		void onAddColorModifierColor(int emitterIdx, int modifierIdx);
		void onRemoveColorModifierColor(int emitterIdx, int modifierIdx, int index);
		void onChangeColorModifierTime(int emitterIdx, int modifierIdx, int index, int value);
		void onChangeColorModifierColor(int emitterIdx, int modifierIdx, int index, Color value);

		void onAddSizeModifierSize(int emitterIdx, int modifierIdx);
		void onRemoveSizeModifierSize(int emitterIdx, int modifierIdx, int index);
		void onChangeSizeModifierTime(int emitterIdx, int modifierIdx, int index, int value);
		void onChangeSizeModifierSize(int emitterIdx, int modifierIdx, int index, float value);

		void onChangeDirectionModifierType(int emitterIdx, int modifierIdx, int value);
		void onChangeDirectionModifierChangeInterval(int emitterIdx, int modifierIdx, float value);
		void onChangeDirectionModifierConstantDir(int emitterIdx, int modifierIdx, glm::vec3 value);
		void onChangeDirectionModifierRandomDirMin(int emitterIdx, int modifierIdx, glm::vec3 value);
		void onChangeDirectionModifierRandomDirMax(int emitterIdx, int modifierIdx, glm::vec3 value);

		void onChangeRotationModifierType(int emitterIdx, int modifierIdx, int value);
		void onChangeRotationModifierChangeInterval(int emitterIdx, int modifierIdx, float value);
		void onChangeRotationModifierOffset(int emitterIdx, int modifierIdx, glm::vec3 value);
		void onChangeRotationModifierConstantRot(int emitterIdx, int modifierIdx, glm::vec3 value);
		void onChangeRotationModifierRandomRotMin(int emitterIdx, int modifierIdx, glm::vec3 value);
		void onChangeRotationModifierRandomRotMax(int emitterIdx, int modifierIdx, glm::vec3 value);

		void onChangeSpeedModifierType(int emitterIdx, int modifierIdx, int value);
		void onChangeSpeedModifierChangeInterval(int emitterIdx, int modifierIdx, float value);
		void onChangeSpeedModifierConstantSpeed(int emitterIdx, int modifierIdx, float value);
		void onChangeSpeedModifierRandomSpeedMin(int emitterIdx, int modifierIdx, float value);
		void onChangeSpeedModifierRandomSpeedMax(int emitterIdx, int modifierIdx, float value);
	};
}