#pragma once

#include "Component.h"
#include "Renderable.h"

#include <bgfx/bgfx.h>

#include "../Math/AxisAlignedBox.h"
#include "../Renderer/Color.h"

#include "../glm/glm.hpp"

class btSphereShape;
class btRigidBody;
class btDefaultMotionState;

namespace GX
{
	class ParticleSystem;
	class ParticleEmitter;
	class Material;
	class ParticleModifier;

	enum class ParticleEmitterShape
	{
		Point,
		Sphere,
		Box,
		Circle
	};

	struct Particle
	{
	public:
		bool active = false;
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 direction = glm::vec3(0.0f);
		glm::highp_quat rotation = glm::identity<glm::highp_quat>();
		float size = 1.0f;
		float startSize = 1.0f;
		Color color = Color::White;
		float speed = 0.0f;
		float lifeTime = 0.0f;
		float startLifeTime = 0.0f;
		float currentFrame = 0;
		glm::vec3 psPosition = glm::vec3(0.0f);
		glm::highp_quat psRotation = glm::identity<glm::highp_quat>();
		std::map<ParticleModifier*, bool> paramsBool;
		std::map<ParticleModifier*, glm::vec3> paramsVec3;

		btSphereShape* collider = nullptr;
		btRigidBody* rigidbody = nullptr;
		btDefaultMotionState* motionState = nullptr;

		ParticleEmitter* parent = nullptr;

		void reset();
		float getPlaybackTime();
	};

	class ParticleModifier
	{
	protected:
		float startTime = 0.0f;
		ParticleEmitter* parent = nullptr;

	public:
		ParticleModifier(ParticleEmitter* _parent) { parent = _parent; }
		virtual ~ParticleModifier() {}

		virtual void onCreateParticle(Particle& particle) {}
		virtual void update(Particle& particle, float deltaTime) {}
		virtual std::string getType() { return "ParticleModifier"; }
		
		float getStartTime() { return startTime; }
		void setStartTime(float value) { startTime = value; }
	};

	class ParticleGravityModifier : public ParticleModifier
	{
	private:
		glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);
		float damping = 0.4f;

	public:
		static std::string MODIFIER_TYPE;

		ParticleGravityModifier(ParticleEmitter* _parent) : ParticleModifier(_parent) {}
		virtual ~ParticleGravityModifier() {}

		virtual std::string getType() { return MODIFIER_TYPE; }

		glm::vec3 getGravity() { return gravity; }
		void setGravity(glm::vec3 value) { gravity = value; }

		float getDamping() { return damping; }
		void setDamping(float value) { damping = value; }

		virtual void onCreateParticle(Particle& particle);
		virtual void update(Particle& particle, float deltaTime);
	};

	class ParticleColorModifier : public ParticleModifier
	{
	private:
		std::vector<std::pair<float, Color>> colors;

	public:
		static std::string MODIFIER_TYPE;

		ParticleColorModifier(ParticleEmitter* _parent) : ParticleModifier(_parent) {}
		virtual ~ParticleColorModifier() {}

		virtual std::string getType() { return MODIFIER_TYPE; }

		std::vector<std::pair<float, Color>>& getColors() { return colors; }

		virtual void onCreateParticle(Particle& particle);
		virtual void update(Particle& particle, float deltaTime);
	};

	class ParticleSizeModifier : public ParticleModifier
	{
	private:
		std::vector<std::pair<float, float>> sizes;

	public:
		static std::string MODIFIER_TYPE;

		ParticleSizeModifier(ParticleEmitter* _parent) : ParticleModifier(_parent) {}
		virtual ~ParticleSizeModifier() {}

		virtual std::string getType() { return MODIFIER_TYPE; }

		std::vector<std::pair<float, float>>& getSizes() { return sizes; }

		virtual void onCreateParticle(Particle& particle);
		virtual void update(Particle& particle, float deltaTime);
	};

	class ParticleDirectionModifier : public ParticleModifier
	{
	public:
		enum class DirectionType
		{
			Constant,
			OnceAtStart,
			RandomAtStart,
			RandomTimed
		};

		float timer = 0.0f;

	private:
		glm::vec3 randomDirectionMin = glm::vec3(-1.0f);
		glm::vec3 randomDirectionMax = glm::vec3(1.0f);
		glm::vec3 constantDirection = glm::vec3(1.0f, 0.0f, 0.0f);
		float changeInterval = 0.1f;
		DirectionType directionType = DirectionType::Constant;

	public:
		static std::string MODIFIER_TYPE;

		ParticleDirectionModifier(ParticleEmitter* _parent) : ParticleModifier(_parent) {}
		virtual ~ParticleDirectionModifier() {}

		virtual std::string getType() { return MODIFIER_TYPE; }

		DirectionType getDirectionType() { return directionType; }
		void setDirectionType(DirectionType value) { directionType = value; }

		float getChangeInterval() { return changeInterval; }
		void setChangeInterval(float value) { changeInterval = value; }

		glm::vec3 getRandomDirectionMin() { return randomDirectionMin; }
		void setRandomDirectionMin(glm::vec3 value) { randomDirectionMin = value; }

		glm::vec3 getRandomDirectionMax() { return randomDirectionMax; }
		void setRandomDirectionMax(glm::vec3 value) { randomDirectionMax = value; }

		glm::vec3 getConstantDirection() { return constantDirection; }
		void setConstantDirection(glm::vec3 value) { constantDirection = value; }

		virtual void onCreateParticle(Particle& particle);
		virtual void update(Particle& particle, float deltaTime);
	};

	class ParticleRotationModifier : public ParticleModifier
	{
	public:
		enum class RotationType
		{
			Constant,
			OnceAtStart,
			RandomAtStart,
			RandomTimed,
			FromDirection
		};

		float timer = 0.0f;

	private:
		glm::vec3 randomRotationMin = glm::vec3(-180.0f);
		glm::vec3 randomRotationMax = glm::vec3(180.0f);
		glm::vec3 constantRotation = glm::vec3(0.0f);
		float changeInterval = 0.1f;
		glm::vec3 offset = glm::vec3(0.0f);
		RotationType rotationType = RotationType::Constant;

	public:
		static std::string MODIFIER_TYPE;

		ParticleRotationModifier(ParticleEmitter* _parent) : ParticleModifier(_parent) {}
		virtual ~ParticleRotationModifier() {}

		virtual std::string getType() { return MODIFIER_TYPE; }

		RotationType getRotationType() { return rotationType; }
		void setRotationType(RotationType value) { rotationType = value; }

		float getChangeInterval() { return changeInterval; }
		void setChangeInterval(float value) { changeInterval = value; }

		glm::vec3 getRandomRotationMin() { return randomRotationMin; }
		void setRandomRotationMin(glm::vec3 value) { randomRotationMin = value; }

		glm::vec3 getRandomRotationMax() { return randomRotationMax; }
		void setRandomRotationMax(glm::vec3 value) { randomRotationMax = value; }

		glm::vec3 getConstantRotation() { return constantRotation; }
		void setConstantRotation(glm::vec3 value) { constantRotation = value; }

		glm::vec3 getOffset() { return offset; }
		void setOffset(glm::vec3 value) { offset = value; }

		virtual void onCreateParticle(Particle& particle);
		virtual void update(Particle& particle, float deltaTime);
	};

	class ParticleSpeedModifier : public ParticleModifier
	{
	public:
		enum class SpeedType
		{
			Constant,
			OnceAtStart,
			RandomAtStart,
			RandomTimed
		};

		float timer = 0.0f;

	private:
		float randomSpeedMin = 0.1f;
		float randomSpeedMax = 0.5f;
		float constantSpeed = 0.1f;
		float changeInterval = 0.1f;
		SpeedType speedType = SpeedType::Constant;

	public:
		static std::string MODIFIER_TYPE;

		ParticleSpeedModifier(ParticleEmitter* _parent) : ParticleModifier(_parent) {}
		virtual ~ParticleSpeedModifier() {}

		virtual std::string getType() { return MODIFIER_TYPE; }

		SpeedType getSpeedType() { return speedType; }
		void setSpeedType(SpeedType value) { speedType = value; }

		float getChangeInterval() { return changeInterval; }
		void setChangeInterval(float value) { changeInterval = value; }

		float getRandomSpeedMin() { return randomSpeedMin; }
		void setRandomSpeedMin(float value) { randomSpeedMin = value; }

		float getRandomSpeedMax() { return randomSpeedMax; }
		void setRandomSpeedMax(float value) { randomSpeedMax = value; }

		float getConstantSpeed() { return constantSpeed; }
		void setConstantSpeed(float value) { constantSpeed = value; }

		virtual void onCreateParticle(Particle& particle);
		virtual void update(Particle& particle, float deltaTime);
	};

	class ParticleEmitter: public Renderable
	{
		friend class ParticleSystem;
		friend class ParticleColorModifier;

	public:
		enum class SimulationSpace
		{
			Local,
			World
		};

		enum class RenderType
		{
			Billboard,
			D3
		};

		enum class Origin
		{
			Center,
			Bottom
		};

		enum class AnimationTimeMode
		{
			Lifetime,
			Speed,
			FPS
		};

	private:
		struct VertexBuffer
		{
			glm::vec3 position = glm::vec3(0, 0, 0);
			glm::vec2 texcoord0 = glm::vec2(0, 0);
			uint32_t color = 0xffffffff;
		};

		VertexBuffer* vertices = nullptr;
		uint32_t* indices = nullptr;
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		bgfx::DynamicVertexBufferHandle vbh = { bgfx::kInvalidHandle };
		bgfx::DynamicIndexBufferHandle ibh = { bgfx::kInvalidHandle };

		AxisAlignedBox bounds = AxisAlignedBox::BOX_INFINITE;

		Material* material = nullptr;
		ParticleSystem* parent = nullptr;

		std::vector<Particle> particles;
		std::vector<ParticleModifier*> modifiers;

		//Runtime vars
		float playbackTime = 0.0f;
		float emissionTime = 0.0f;
		bool isPlaying = false;

		//Parameters
		SimulationSpace simulationSpace = SimulationSpace::Local;
		RenderType renderType = RenderType::Billboard;
		Origin origin = Origin::Center;
		ParticleEmitterShape shape = ParticleEmitterShape::Point;
		float radius = 1.0f;
		glm::vec3 size = glm::vec3(1.0f);
		float timeScale = 1.0f;
		float emissionRate = 0.02f;
		uint32_t emissionCount = 2;
		float duration = 2.0f;
		uint32_t maxParticles = 100;
		float startSizeMin = 0.5f;
		float startSizeMax = 1.0f;
		float lifeTimeMin = 0.7f;
		float lifeTimeMax = 1.0f;
		float startSpeed = 2.0f;
		glm::vec3 startDirection = glm::vec3(0.0f, -1.0f, 0.0f);
		bool enablePhysics = false;
		float friction = 0.85f;
		float bounciness = 0.25f;
		bool animated = false;
		glm::vec2 animationGridSize = glm::vec2(1, 1);
		AnimationTimeMode animationTimeMode = AnimationTimeMode::Lifetime;
		int animationFps = 28;
		bool loop = true;

		void create();
		void destroy();
		void recreate();

		void destroyParticles();

	public:
		ParticleEmitter(ParticleSystem* ps);
		virtual ~ParticleEmitter();

		virtual AxisAlignedBox getBounds(bool world = true);
		virtual bool isTransparent();
		virtual bool getCastShadows();
		virtual void onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback);
		virtual bool checkCullingMask(LayerMask& mask);

		void update(float deltaTime);
		void play();
		void stop();

		void cloneProperties(ParticleEmitter* to);
		void cloneModifier(int index, ParticleEmitter* to);

		ParticleSystem* getParent() { return parent; }

		bool getIsPlaying() { return isPlaying; }
		float getPlaybackTime() { return playbackTime; }

		template<typename T>
		T addModifier() {}

		std::vector<ParticleModifier*>& getModifiers() { return modifiers; }
		ParticleModifier* getModifier(int index);
		void removeModifier(ParticleModifier* modifier);
		void removeModifier(int index);

		//Parameters
		Material* getMaterial() { return material; }
		void setMaterial(Material* value) { material = value; }

		SimulationSpace getSimulationSpace() { return simulationSpace; }
		void setSimulationSpace(SimulationSpace value);

		RenderType getRenderType() { return renderType; }
		void setRenderType(RenderType value) { renderType = value; }

		Origin getOrigin() { return origin; }
		void setOrigin(Origin value) { origin = value; }

		ParticleEmitterShape getShape() { return shape; }
		void setShape(ParticleEmitterShape value) { shape = value; }

		float getRadius() { return radius; }
		void setRadius(float value) { radius = value; }

		glm::vec3 getSize() { return size; }
		void setSize(glm::vec3 value) { size = value; }

		float getTimeScale() { return timeScale; }
		void setTimeScale(float value) { timeScale = value; }

		float getEmissionRate() { return emissionRate; }
		void setEmissionRate(float value) { emissionRate = value; }

		uint32_t getEmissionCount() { return emissionCount; }
		void setEmissionCount(uint32_t value) { emissionCount = value; }

		float getDuration() { return duration; }
		void setDuration(float value) { duration = value; }

		uint32_t getMaxParticles() { return maxParticles; }
		void setMaxParticles(uint32_t value) { maxParticles = value; }

		float getStartSizeMin() { return startSizeMin; }
		void setStartSizeMin(float value) { startSizeMin = value; }

		float getStartSizeMax() { return startSizeMax; }
		void setStartSizeMax(float value) { startSizeMax = value; }

		float getLifeTimeMin() { return lifeTimeMin; }
		void setLifeTimeMin(float value) { lifeTimeMin = value; }

		float getLifeTimeMax() { return lifeTimeMax; }
		void setLifeTimeMax(float value) { lifeTimeMax = value; }

		float getStartSpeed() { return startSpeed; }
		void setStartSpeed(float value) { startSpeed = value; }

		glm::vec3 getStartDirection() { return startDirection; }
		void setStartDirection(glm::vec3 value) { startDirection = value; }

		bool getEnablePhysics() { return enablePhysics; }
		void setEnablePhysics(bool value) { enablePhysics = value; }

		float getFriction() { return friction; }
		void setFriction(float value) { friction = value; }
		
		float getBounciness() { return bounciness; }
		void setBounciness(float value) { bounciness = value; }

		bool getAnimated() { return animated; }
		void setAnimated(bool value) { animated = value; }

		glm::vec2 getAnimationGridSize() { return animationGridSize; }
		void setAnimationGridSize(glm::vec2 value) { animationGridSize = value; }

		AnimationTimeMode getAnimationTimeMode() { return animationTimeMode; }
		void setAnimationTimeMode(AnimationTimeMode value) { animationTimeMode = value; }

		int getAnimationFps() { return animationFps; }
		void setAnimationFps(int value) { animationFps = value; }

		bool getLoop() { return loop; }
		void setLoop(bool value) { loop = value; }
	};

	template <> ParticleGravityModifier* ParticleEmitter::addModifier<ParticleGravityModifier*>();
	template <> ParticleColorModifier* ParticleEmitter::addModifier<ParticleColorModifier*>();
	template <> ParticleSizeModifier* ParticleEmitter::addModifier<ParticleSizeModifier*>();
	template <> ParticleDirectionModifier* ParticleEmitter::addModifier<ParticleDirectionModifier*>();
	template <> ParticleRotationModifier* ParticleEmitter::addModifier<ParticleRotationModifier*>();
	template <> ParticleSpeedModifier* ParticleEmitter::addModifier<ParticleSpeedModifier*>();
		
	class ParticleSystem : public Component
	{
	private:
		bool castShadows = false;
		bool playAtStart = true;
		std::vector<ParticleEmitter*> emitters;

	public:
		ParticleSystem();
		virtual ~ParticleSystem();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType() { return COMPONENT_TYPE; }
		virtual void onUpdate(float deltaTime);
		virtual void onSceneLoaded();
		virtual Component* onClone();
		virtual bool isEqualsTo(Component* other);
		virtual void onAttach();
		virtual void onDetach();

		bool getCastShadows() { return castShadows; }
		void setCastShadows(bool value) { castShadows = value; }

		bool getPlayAtStart() { return playAtStart; }
		void setPlayAtStart(bool value) { playAtStart = value; }

		std::vector<ParticleEmitter*>& getEmitters() { return emitters; }
		ParticleEmitter* getEmitter(int index);
		ParticleEmitter* addEmitter();
		void removeEmitter(ParticleEmitter* emitter);
		void removeEmitter(int index);

		bool isPlaying();
		void play();
		void stop();
	};
}