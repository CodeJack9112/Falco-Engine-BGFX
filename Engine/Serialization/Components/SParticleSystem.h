#pragma once

#include <string>

#include "SComponent.h"
#include "../Data/SVector.h"
#include "../Data/SColor.h"

namespace GX
{
	class SParticleModifier : public Archive
	{
	public:
		SParticleModifier() {}
		~SParticleModifier() {}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(startTime);
		}

	public:
		float startTime = 0.0f;
	};

	class SParticleGravityModifier : public SParticleModifier
	{
	public:
		SParticleGravityModifier() {}
		~SParticleGravityModifier() {}

		virtual void serialize(Serializer* s)
		{
			SParticleModifier::serialize(s);
			data(gravity);
			data(damping);
		}

	public:
		SVector3 gravity = SVector3(0.0f, -1.0f, 0.0f);
		float damping = 0.07f;
	};

	class SParticleColorModifier : public SParticleModifier
	{
	public:
		SParticleColorModifier() {}
		~SParticleColorModifier() {}

		virtual void serialize(Serializer* s)
		{
			SParticleModifier::serialize(s);
			dataVector(times);
			data(colors);
		}

	public:
		std::vector<float> times;
		std::vector<SColor> colors;
	};

	class SParticleSizeModifier : public SParticleModifier
	{
	public:
		SParticleSizeModifier() {}
		~SParticleSizeModifier() {}

		virtual void serialize(Serializer* s)
		{
			SParticleModifier::serialize(s);
			dataVector(times);
			dataVector(sizes);
		}

	public:
		std::vector<float> times;
		std::vector<float> sizes;
	};

	class SParticleDirectionModifier : public SParticleModifier
	{
	public:
		SParticleDirectionModifier() {}
		~SParticleDirectionModifier() {}

		virtual void serialize(Serializer* s)
		{
			SParticleModifier::serialize(s);
			data(randomDirectionMin);
			data(randomDirectionMax);
			data(constantDirection);
			data(changeInterval);
			data(directionType);
		}

	public:
		SVector3 randomDirectionMin = SVector3(-1.0f, -1.0f, -1.0f);
		SVector3 randomDirectionMax = SVector3(1.0f, 1.0f, 1.0f);
		SVector3 constantDirection = SVector3(1.0f, 0.0f, 0.0f);
		float changeInterval = 0.1f;
		int directionType = 0;
	};

	class SParticleRotationModifier : public SParticleModifier
	{
	public:
		SParticleRotationModifier() {}
		~SParticleRotationModifier() {}

		virtual void serialize(Serializer* s)
		{
			SParticleModifier::serialize(s);
			data(randomRotationMin);
			data(randomRotationMax);
			data(constantRotation);
			data(offset);
			data(changeInterval);
			data(rotationType);
		}

	public:
		SVector3 randomRotationMin = SVector3(-180.0f, -180.0f, -180.0f);
		SVector3 randomRotationMax = SVector3(180.0f, 180.0f, 180.0f);
		SVector3 constantRotation = SVector3(0.0f, 0.0f, 0.0f);
		SVector3 offset = SVector3(0.0f, 0.0f, 0.0f);
		float changeInterval = 0.1f;
		int rotationType = 0;
	};

	class SParticleSpeedModifier : public SParticleModifier
	{
	public:
		SParticleSpeedModifier() {}
		~SParticleSpeedModifier() {}

		virtual void serialize(Serializer* s)
		{
			SParticleModifier::serialize(s);
			data(constantSpeed);
			data(randomSpeedMin);
			data(randomSpeedMax);
			data(changeInterval);
			data(speedType);
		}

	public:
		float randomSpeedMin = 0.1f;
		float randomSpeedMax = 0.5f;
		float constantSpeed = 0.1f;
		float changeInterval = 0.1f;
		int speedType = 0;
	};

	class SParticleEmitter : public Archive
	{
	public:
		SParticleEmitter() {}
		~SParticleEmitter() {}

		virtual int getVersion() { return 3; }

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(material);
			data(simulationSpace);
			data(renderType);
			data(origin);
			data(shape);
			data(radius);
			data(size);
			data(timeScale);
			data(emissionRate);
			data(emissionCount);
			data(duration);
			data(maxParticles);
			data(startSizeMin);
			data(startSizeMax);
			data(lifeTimeMin);
			data(lifeTimeMax);
			data(startSpeed);
			data(startDirection);
			data(loop);

			if (version > 0)
			{
				data(enablePhysics);
				data(bounciness);
			}

			if (version > 1)
			{
				data(friction);
			}

			data(gravityModifiers);
			data(colorModifiers);
			data(sizeModifiers);
			data(directionModifiers);
			data(rotationModifiers);
			data(speedModifiers);

			if (version > 2)
			{
				data(animated);
				data(animationGridSize);
				data(animationTimeMode);
				data(animationFps);
			}
		}

	public:
		std::string material = "";
		int simulationSpace = 0;
		int renderType = 0;
		int origin = 0;
		int shape = 0;
		float radius = 1.0f;
		SVector3 size = SVector3(1.0f, 1.0f, 1.0f);
		float timeScale = 1.0f;
		float emissionRate = 0.02f;
		unsigned int emissionCount = 2;
		float duration = 2.0f;
		unsigned int maxParticles = 100;
		float startSizeMin = 0.5f;
		float startSizeMax = 1.0f;
		float lifeTimeMin = 0.7f;
		float lifeTimeMax = 1.0f;
		float startSpeed = 2.0f;
		SVector3 startDirection = SVector3(0.0f, -1.0f, 0.0f);
		bool enablePhysics = false;
		float friction = 0.85f;
		float bounciness = 0.25f;
		bool animated = false;
		SVector2 animationGridSize = SVector2(1, 1);
		int animationTimeMode = 0;
		int animationFps = 28;
		bool loop = true;

		std::vector<SParticleGravityModifier> gravityModifiers;
		std::vector<SParticleColorModifier> colorModifiers;
		std::vector<SParticleSizeModifier> sizeModifiers;
		std::vector<SParticleDirectionModifier> directionModifiers;
		std::vector<SParticleRotationModifier> rotationModifiers;
		std::vector<SParticleSpeedModifier> speedModifiers;
	};

	class SParticleSystem : public SComponent
	{
	public:
		SParticleSystem() {}
		~SParticleSystem() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(castShadows);
			data(playAtStart);
			data(emitters);
		}

	public:
		bool castShadows = false;
		bool playAtStart = true;
		std::vector<SParticleEmitter> emitters;
	};
}