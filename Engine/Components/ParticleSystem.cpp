#include "ParticleSystem.h"

#include "../glm/gtc/type_ptr.hpp"

#include <boost/algorithm/string.hpp>

#include "../Bullet/include/btBulletCollisionCommon.h"
#include "../Bullet/include/btBulletDynamicsCommon.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Core/Time.h"
#include "../Core/PhysicsManager.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/VertexLayouts.h"
#include "../Math/Mathf.h"
#include "../Core/GameObject.h"
#include "../Assets/Material.h"
#include "../Assets/Shader.h"
#include "../Assets/Texture.h"

#include "Camera.h"
#include "Transform.h"
#include "Light.h"

#include "../Classes/StringConverter.h"

namespace GX
{
    std::string ParticleSystem::COMPONENT_TYPE = "ParticleSystem";

    void Particle::reset()
    {
        active = false;
        position = glm::vec3(0.0f);
        direction = glm::vec3(0.0f);
        rotation = glm::identity<glm::highp_quat>();
        size = 1.0f;
        startSize = 1.0f;
        color = Color::White;
        speed = 0.0f;
        lifeTime = 0.0f;
        startLifeTime = 0.0f;
        currentFrame = 0.0f;
        psPosition = glm::vec3(0.0f);
        psRotation = glm::identity<glm::highp_quat>();
        paramsBool.clear();
        paramsVec3.clear();

        if (collider != nullptr)
            delete collider;

        if (motionState != nullptr)
            delete motionState;

        if (rigidbody != nullptr)
        {
            PhysicsManager::getSingleton()->getWorld()->removeRigidBody(rigidbody);
            delete rigidbody;
        }

        parent = nullptr;

        collider = nullptr;
        rigidbody = nullptr;
        motionState = nullptr;
    }

    float Particle::getPlaybackTime()
    {
        return startLifeTime - lifeTime;
    }

    //***PARTICLE MODIFIERS***//

    std::string ParticleGravityModifier::MODIFIER_TYPE = "Gravity modifier";
    std::string ParticleColorModifier::MODIFIER_TYPE = "Color modifier";
    std::string ParticleSizeModifier::MODIFIER_TYPE = "Size modifier";
    std::string ParticleDirectionModifier::MODIFIER_TYPE = "Direction modifier";
    std::string ParticleRotationModifier::MODIFIER_TYPE = "Rotation modifier";
    std::string ParticleSpeedModifier::MODIFIER_TYPE = "Speed modifier";

    //#define PARTICLE_POOL_SIZE 256

    //Gravity modifier
    void ParticleGravityModifier::onCreateParticle(Particle& particle)
    {
        particle.paramsVec3[this] = glm::vec3(0.0f);

        if (particle.parent->getEnablePhysics())
        {
            if (particle.rigidbody != nullptr)
                particle.rigidbody->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
        }
    }

    void ParticleGravityModifier::update(Particle& particle, float deltaTime)
    {
        if (particle.parent->getEnablePhysics() &&
            Engine::getSingleton()->getIsRuntimeMode())
            return;

        glm::vec3 _gravityTo = gravity;
        if (parent->getSimulationSpace() == ParticleEmitter::SimulationSpace::Local)
            _gravityTo = glm::inverse(particle.psRotation) * gravity;

        glm::vec3& _gravity = particle.paramsVec3[this];
        _gravity = Mathf::lerp(_gravity, _gravityTo, deltaTime * damping);

        particle.position += _gravity * deltaTime;
    }

    //Color modifier
    void ParticleColorModifier::onCreateParticle(Particle& particle)
    {
        if (colors.size() > 0)
        {
            if (startTime == 0.0f)
            {
                if (colors[0].first >= 0 && colors[0].first <= 0.03f)
                    particle.color = colors[0].second;
            }
        }
    }

    void ParticleColorModifier::update(Particle& particle, float deltaTime)
    {
        Color currentColor = particle.color;
        float minTime = 0.0f;
        float maxTime = 1.0f;

        float p = 1.0f - (1.0f / particle.startLifeTime * particle.lifeTime);

        for (size_t i = 0; i < colors.size(); ++i)
        {
            if (i > 0)
            {
                if (p >= colors[i - 1].first)
                {
                    if (p < colors[i].first)
                    {
                        if (colors[i].first >= colors[i - 1].first)
                        {
                            currentColor = colors[i].second;
                            minTime = colors[i - 1].first;
                            maxTime = colors[i].first;

                            break;
                        }
                    }
                    else if (i == colors.size() - 1)
                    {
                        currentColor = colors[i].second;
                        minTime = colors[i].first;
                        maxTime = colors[i].first;
                    }
                }
            }
            else
            {
                if (p >= 0 && p <= colors[i].first + 0.03f)
                {
                    currentColor = colors[i].second;
                    minTime = 0.0f;
                    maxTime = colors[i].first;
                    
                    break;
                }
                else if (i == colors.size() - 1)
                {
                    currentColor = colors[i].second;
                    minTime = colors[i].first;
                    maxTime = colors[i].first;
                }
            }
        }

        if (maxTime < minTime + 0.07f)
            maxTime = minTime;

        if (minTime == maxTime)
        {
            particle.color = currentColor;
        }
        else
        {
            float t = (p - minTime) / (maxTime - minTime);
            float dt = deltaTime * 100.0f;
            particle.color = Color::lerp(particle.color, currentColor, dt * (t / (particle.startLifeTime * (maxTime - minTime)) / 10.0f));
        }
    }

    //Size modifier
    void ParticleSizeModifier::onCreateParticle(Particle& particle)
    {
        if (sizes.size() > 0)
        {
            if (startTime == 0.0f)
            {
                if (sizes[0].first >= 0 && sizes[0].first <= 0.03f)
                {
                    particle.size = std::max(sizes[0].second, 0.0f);
                }
            }
        }
    }

    void ParticleSizeModifier::update(Particle& particle, float deltaTime)
    {
        float currentSize = 0.0f;
        float minTime = 0.0f;
        float maxTime = 1.0f;

        float p = 1.0f - (1.0f / particle.startLifeTime * particle.lifeTime);

        for (size_t i = 0; i < sizes.size(); ++i)
        {
            if (i > 0)
            {
                if (p >= sizes[i - 1].first)
                {
                    if (p < sizes[i].first)
                    {
                        if (sizes[i].first >= sizes[i - 1].first)
                        {
                            currentSize = sizes[i].second;
                            minTime = sizes[i - 1].first;
                            maxTime = sizes[i].first;

                            break;
                        }
                    }
                    else if (i == sizes.size() - 1)
                    {
                        currentSize = sizes[i].second;
                        minTime = sizes[i].first;
                        maxTime = sizes[i].first;
                    }
                }
            }
            else
            {
                if (p >= 0 && p <= sizes[i].first + 0.03f)
                {
                    currentSize = sizes[i].second;
                    minTime = 0.0f;
                    maxTime = sizes[i].first;

                    break;
                }
                else if (i == sizes.size() - 1)
                {
                    currentSize = sizes[i].second;
                    minTime = sizes[i].first;
                    maxTime = sizes[i].first;
                }
            }
        }

        if (maxTime < minTime + 0.07f)
            maxTime = minTime;

        if (minTime == maxTime)
        {
            float v = particle.startSize + currentSize;
            if (v < 0.0f)
                v = 0.0f;

            particle.size = v;
        }
        else
        {
            float t = (p - minTime) / (maxTime - minTime);
            float v = particle.startSize + currentSize;
            if (v < 0.0f)
                v = 0.0f;

            float dt = deltaTime * 100.0f;
            particle.size = Mathf::lerp(particle.size, v, dt * (t / (particle.startLifeTime * (maxTime - minTime)) / 10.0f));
        }
    }

    //Direction modifier
    void ParticleDirectionModifier::onCreateParticle(Particle& particle)
    {
        particle.paramsBool[this] = false;
    }

    void ParticleDirectionModifier::update(Particle& particle, float deltaTime)
    {
        if (directionType == DirectionType::Constant)
        {
            particle.direction = constantDirection;
            if (parent->getSimulationSpace() == ParticleEmitter::SimulationSpace::World)
                particle.direction = particle.psRotation * constantDirection;

            if (particle.parent->getEnablePhysics() &&
                particle.parent->getSimulationSpace() == ParticleEmitter::SimulationSpace::World &&
                particle.rigidbody != nullptr &&
                Engine::getSingleton()->getIsRuntimeMode())
            {
                glm::vec3 dir = glm::normalize(particle.direction) * particle.speed * 10.0f;
                if (glm::isnan(dir).x) dir.x = 0.0f;
                if (glm::isnan(dir).y) dir.y = 0.0f;
                if (glm::isnan(dir).z) dir.z = 0.0f;
                particle.rigidbody->applyCentralForce(btVector3(dir.x, dir.y, dir.z));
            }
        }
        if (directionType == DirectionType::OnceAtStart)
        {
            if (!particle.paramsBool[this])
            {
                particle.direction = constantDirection;
                if (parent->getSimulationSpace() == ParticleEmitter::SimulationSpace::World)
                    particle.direction = particle.psRotation * constantDirection;

                if (particle.parent->getEnablePhysics() &&
                    particle.parent->getSimulationSpace() == ParticleEmitter::SimulationSpace::World &&
                    particle.rigidbody != nullptr &&
                    Engine::getSingleton()->getIsRuntimeMode())
                {
                    glm::vec3 dir = glm::normalize(particle.direction) * particle.speed * 10.0f;
                    if (glm::isnan(dir).x) dir.x = 0.0f;
                    if (glm::isnan(dir).y) dir.y = 0.0f;
                    if (glm::isnan(dir).z) dir.z = 0.0f;
                    particle.rigidbody->applyCentralForce(btVector3(dir.x, dir.y, dir.z));
                }

                particle.paramsBool[this] = true;
            }
        }
        if (directionType == DirectionType::RandomAtStart)
        {
            if (!particle.paramsBool[this])
            {
                float rx = Mathf::RandomFloat(randomDirectionMin.x, randomDirectionMax.x);
                float ry = Mathf::RandomFloat(randomDirectionMin.y, randomDirectionMax.y);
                float rz = Mathf::RandomFloat(randomDirectionMin.z, randomDirectionMax.z);

                particle.direction = glm::vec3(rx, ry, rz);
                if (parent->getSimulationSpace() == ParticleEmitter::SimulationSpace::World)
                    particle.direction = particle.psRotation * glm::vec3(rx, ry, rz);

                if (particle.parent->getEnablePhysics() &&
                    particle.parent->getSimulationSpace() == ParticleEmitter::SimulationSpace::World &&
                    particle.rigidbody != nullptr &&
                    Engine::getSingleton()->getIsRuntimeMode())
                {
                    glm::vec3 dir = glm::normalize(particle.direction) * particle.speed * 10.0f;
                    if (glm::isnan(dir).x) dir.x = 0.0f;
                    if (glm::isnan(dir).y) dir.y = 0.0f;
                    if (glm::isnan(dir).z) dir.z = 0.0f;
                    particle.rigidbody->applyCentralForce(btVector3(dir.x, dir.y, dir.z));
                }

                particle.paramsBool[this] = true;
            }
        }
        if (directionType == DirectionType::RandomTimed)
        {
            timer += deltaTime;
            if (timer >= changeInterval)
            {
                timer = 0.0f;

                float rx = Mathf::RandomFloat(randomDirectionMin.x, randomDirectionMax.x);
                float ry = Mathf::RandomFloat(randomDirectionMin.y, randomDirectionMax.y);
                float rz = Mathf::RandomFloat(randomDirectionMin.z, randomDirectionMax.z);

                particle.direction = glm::vec3(rx, ry, rz);
                if (parent->getSimulationSpace() == ParticleEmitter::SimulationSpace::World)
                    particle.direction = particle.psRotation * glm::vec3(rx, ry, rz);

                if (particle.parent->getEnablePhysics() &&
                    particle.parent->getSimulationSpace() == ParticleEmitter::SimulationSpace::World &&
                    particle.rigidbody != nullptr &&
                    Engine::getSingleton()->getIsRuntimeMode())
                {
                    glm::vec3 dir = glm::normalize(particle.direction) * particle.speed * 10.0f;
                    if (glm::isnan(dir).x) dir.x = 0.0f;
                    if (glm::isnan(dir).y) dir.y = 0.0f;
                    if (glm::isnan(dir).z) dir.z = 0.0f;
                    particle.rigidbody->applyCentralForce(btVector3(dir.x, dir.y, dir.z));
                }
            }
        }
    }

    //Rotation modifier
    void ParticleRotationModifier::onCreateParticle(Particle& particle)
    {
        particle.paramsBool[this] = false;
        particle.paramsVec3[this] = glm::vec3(FLT_MAX);
    }

    void ParticleRotationModifier::update(Particle& particle, float deltaTime)
    {
        if (rotationType == RotationType::Constant)
        {
            particle.rotation = Mathf::toQuaternion(constantRotation);
        }
        if (rotationType == RotationType::OnceAtStart)
        {
            if (!particle.paramsBool[this])
            {
                particle.rotation = Mathf::toQuaternion(constantRotation);
                particle.paramsBool[this] = true;
            }
        }
        if (rotationType == RotationType::RandomAtStart)
        {
            if (!particle.paramsBool[this])
            {
                float rx = Mathf::RandomFloat(randomRotationMin.x, randomRotationMax.x);
                float ry = Mathf::RandomFloat(randomRotationMin.y, randomRotationMax.y);
                float rz = Mathf::RandomFloat(randomRotationMin.z, randomRotationMax.z);

                particle.rotation = Mathf::toQuaternion(glm::vec3(rx, ry, rz));
                particle.paramsBool[this] = true;
            }
        }
        if (rotationType == RotationType::RandomTimed)
        {
            timer += deltaTime;
            if (timer >= changeInterval)
            {
                timer = 0.0f;

                float rx = Mathf::RandomFloat(randomRotationMin.x, randomRotationMax.x);
                float ry = Mathf::RandomFloat(randomRotationMin.y, randomRotationMax.y);
                float rz = Mathf::RandomFloat(randomRotationMin.z, randomRotationMax.z);

                particle.rotation = Mathf::toQuaternion(glm::vec3(rx, ry, rz));
            }
        }
        if (rotationType == RotationType::FromDirection)
        {
            if (Renderer::getSingleton()->getCameras().size() > 0)
            {
                glm::vec3 pwPos = particle.position;
                glm::vec3 prevPos = particle.paramsVec3[this];
                if (prevPos != glm::vec3(FLT_MAX) && prevPos != pwPos)
                {
                    Camera* cam = Renderer::getSingleton()->getCameras()[0];
                    glm::vec3 dir = glm::normalize(pwPos - prevPos);
                    glm::vec3 camDir = glm::normalize(pwPos - particle.psPosition);
                    if (parent->getSimulationSpace() == ParticleEmitter::SimulationSpace::Local)
                        camDir = glm::normalize(pwPos);

                    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

                    if (abs(glm::dot(dir, camDir)) < 1.0f)
                        up = glm::normalize(glm::cross(dir, camDir));

                    glm::highp_quat angle = glm::quatLookAtRH(dir, up) * Mathf::toQuaternion(offset);
                    particle.rotation = angle;
                }

                particle.paramsVec3[this] = pwPos;
            }
        }

        if (glm::isnan(particle.rotation).x || glm::isnan(particle.rotation).y || glm::isnan(particle.rotation).z || glm::isnan(particle.rotation).w)
        {
            particle.rotation = glm::identity<glm::highp_quat>();
        }
    }

    //Speed modifier
    void ParticleSpeedModifier::onCreateParticle(Particle& particle)
    {
        particle.paramsBool[this] = false;
    }

    void ParticleSpeedModifier::update(Particle& particle, float deltaTime)
    {
        if (particle.parent->getEnablePhysics())
            return;

        if (speedType == SpeedType::Constant)
        {
            particle.speed = constantSpeed;
        }
        if (speedType == SpeedType::OnceAtStart)
        {
            if (!particle.paramsBool[this])
            {
                particle.speed = constantSpeed;
                particle.paramsBool[this] = true;
            }
        }
        if (speedType == SpeedType::RandomAtStart)
        {
            if (!particle.paramsBool[this])
            {
                float val = Mathf::RandomFloat(randomSpeedMin, randomSpeedMax);

                particle.speed = val;
                particle.paramsBool[this] = true;
            }
        }
        if (speedType == SpeedType::RandomTimed)
        {
            timer += deltaTime;
            if (timer >= changeInterval)
            {
                timer = 0.0f;

                float val = Mathf::RandomFloat(randomSpeedMin, randomSpeedMax);

                particle.speed = val;
            }
        }
    }

    //***PARTICLE EMITTER***//

    ParticleEmitter::ParticleEmitter(ParticleSystem* ps) : Renderable()
    {
        setRenderQueue(2);

        parent = ps;
    }

    ParticleEmitter::~ParticleEmitter()
    {
        for (auto it = modifiers.begin(); it != modifiers.end(); ++it)
            delete* it;

        modifiers.clear();
        destroyParticles();
        destroy();
    }

    void ParticleEmitter::create()
    {
        int particlesCount = particles.size();

        vertexCount = 4 * particlesCount;
        indexCount = 6 * particlesCount;
        vertices = new VertexBuffer[vertexCount];
        indices = new uint32_t[indexCount];

        vbh = bgfx::createDynamicVertexBuffer(vertexCount, VertexLayouts::particleVertexLayout);
        ibh = bgfx::createDynamicIndexBuffer(indexCount, BGFX_BUFFER_INDEX32);
    }

    void ParticleEmitter::destroy()
    {
        if (bgfx::isValid(vbh))
            bgfx::destroy(vbh);

        if (bgfx::isValid(ibh))
            bgfx::destroy(ibh);

        vbh = { bgfx::kInvalidHandle };
        ibh = { bgfx::kInvalidHandle };

        if (vertices != nullptr)
            delete[] vertices;

        if (indices != nullptr)
            delete[] indices;

        vertices = nullptr;
        indices = nullptr;

        vertexCount = 0;
        indexCount = 0;
    }

    void ParticleEmitter::recreate()
    {
        destroy();
        create();
    }

    void ParticleEmitter::destroyParticles()
    {
        for (auto it = particles.begin(); it != particles.end(); ++it)
            it->reset();

        particles.clear();
    }

    void ParticleEmitter::update(float deltaTime)
    {
        if (!isPlaying)
            return;

        if (deltaTime <= 0)
            return;

        //1. Update particles
        //2. Update modifiers
        //3. Update buffers

        if (emissionTime < emissionRate)
        {
            emissionTime += deltaTime;
        }
        else
        {
            emissionTime = 0.0f;

            if (emissionCount > 0)
            {
                for (int i = 0; i < emissionCount; ++i)
                {
                    if (particles.size() < maxParticles)
                    {
                        Transform* psTransform = parent->getGameObject()->getTransform();

                        Particle p;

                        p.reset();
                        p.active = true;
                        p.lifeTime = Mathf::RandomFloat(lifeTimeMin, lifeTimeMax);
                        p.startLifeTime = p.lifeTime;
                        p.size = Mathf::RandomFloat(startSizeMin, startSizeMax);
                        p.speed = startSpeed;
                        p.direction = startDirection;
                        p.parent = this;

                        if (simulationSpace == SimulationSpace::World)
                        {
                            p.position = psTransform->getPosition();
                            p.direction = psTransform->getRotation() * startDirection;
                            glm::vec3 scl = psTransform->getScale();
                            float sz = std::max(scl.x, scl.y);
                            sz = std::max(sz, scl.z);
                            p.size = sz * Mathf::RandomFloat(startSizeMin, startSizeMax);
                            p.speed = sz * startSpeed;
                        }

                        p.startSize = p.size;

                        glm::vec3 shapePosition = glm::vec3(0.0f);

                        if (shape == ParticleEmitterShape::Box)
                        {
                            shapePosition.x = Mathf::RandomFloat(-size.x, size.x);
                            shapePosition.y = Mathf::RandomFloat(-size.y, size.y);
                            shapePosition.z = Mathf::RandomFloat(-size.z, size.z);
                        }

                        if (shape == ParticleEmitterShape::Sphere
                            || shape == ParticleEmitterShape::Circle)
                        {
                            if (radius != 0.0f)
                            {
                                shapePosition.x = Mathf::RandomFloat(-radius / 2.0f, radius / 2.0f);
                                shapePosition.y = Mathf::RandomFloat(-radius / 2.0f, radius / 2.0f);
                                shapePosition.z = Mathf::RandomFloat(-radius / 2.0f, radius / 2.0f);

                                shapePosition *= glm::vec3(1.0f / std::sqrt(shapePosition.x * shapePosition.x + shapePosition.y * shapePosition.y + shapePosition.z * shapePosition.z));
                                shapePosition *= glm::vec3(radius);
                            }
                        }

                        if (shape == ParticleEmitterShape::Circle)
                        {
                            shapePosition.y = 0.0f;
                        }

                        if (simulationSpace == SimulationSpace::World)
                        {
                            shapePosition = psTransform->getRotation() * shapePosition;
                        }

                        p.position += shapePosition;
                        p.psPosition = psTransform->getPosition();
                        p.psRotation = psTransform->getRotation();

                        //Physics
                        if (enablePhysics &&
                            simulationSpace == SimulationSpace::World)
                        {
                            p.direction = glm::vec3(0.0f);

                            if (Engine::getSingleton()->getIsRuntimeMode())
                            {
                                btTransform startTransform;
                                btVector3 localInertia(0, 0, 0);
                                startTransform.setOrigin(btVector3(p.position.x, p.position.y, p.position.z));

                                p.collider = new btSphereShape(std::max(p.size, 0.1f));
                                p.motionState = new btDefaultMotionState(startTransform);

                                btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0f, p.motionState, p.collider);
                                p.rigidbody = new btRigidBody(rbInfo);

                                p.rigidbody->setActivationState(DISABLE_DEACTIVATION);
                                p.rigidbody->setMassProps(0.1f, localInertia);
                                p.rigidbody->setDamping(0.0f, 0.0f);
                                p.rigidbody->updateInertiaTensor();
                                p.rigidbody->clearForces();

                                for (auto& it : particles)
                                {
                                    if (it.rigidbody != nullptr)
                                    {
                                        it.rigidbody->setIgnoreCollisionCheck(p.rigidbody, true);
                                        p.rigidbody->setIgnoreCollisionCheck(it.rigidbody, true);
                                    }
                                }

                                PhysicsManager::getSingleton()->getWorld()->addRigidBody(p.rigidbody);

                                p.rigidbody->setGravity(btVector3(0.0f, 0.0f, 0.0f));
                                p.rigidbody->setFriction(friction);
                                p.rigidbody->setRestitution(bounciness);
                            }
                        }

                        particles.push_back(p);

                        for (auto jt = modifiers.begin(); jt != modifiers.end(); ++jt)
                        {
                            ParticleModifier* modifier = *jt;
                            modifier->onCreateParticle(p);
                        }
                    }
                }
            }
        }

        for (auto it = particles.begin(); it != particles.end(); ++it)
        {
            Particle& particle = *it;

            if (!particle.active)
                continue;

            glm::vec3 dir = glm::normalize(particle.direction);
            if (glm::isnan(dir).x) dir.x = 0.0f;
            if (glm::isnan(dir).y) dir.y = 0.0f;
            if (glm::isnan(dir).z) dir.z = 0.0f;

            if (enablePhysics &&
                simulationSpace == SimulationSpace::World &&
                particle.rigidbody != nullptr &&
                Engine::getSingleton()->getIsRuntimeMode())
            {
                btTransform trans;
                particle.motionState->getWorldTransform(trans);
                btVector3 pos = trans.getOrigin();
                particle.position = glm::vec3(pos.x(), pos.y(), pos.z());
            }
            else
            {
                particle.position += (dir * particle.speed) * deltaTime;
            }

            if (animated && animationGridSize.x > 0 && animationGridSize.y > 0)
            {
                float maxFrames = animationGridSize.x * animationGridSize.y;

                if (animationTimeMode == AnimationTimeMode::Speed)
                    particle.currentFrame += Time::getDeltaTime() * Time::getTimeScale() * particle.speed * (float)animationFps;
                else if (animationTimeMode == AnimationTimeMode::Lifetime)
                    particle.currentFrame = maxFrames - (maxFrames / particle.startLifeTime * particle.lifeTime);
                else
                    particle.currentFrame += (Time::getDeltaTime() * (float)animationFps) * Time::getTimeScale();

                if (particle.currentFrame > maxFrames)
                    particle.currentFrame = 0;
            }

            particle.lifeTime -= deltaTime;

            if (particle.lifeTime < 0.0f)
            {
                particle.reset();
            }
            else
            {
                for (auto jt = modifiers.begin(); jt != modifiers.end(); ++jt)
                {
                    ParticleModifier* modifier = *jt;

                    float ptime = particle.getPlaybackTime();
                    if (ptime >= modifier->getStartTime())
                    {
                        modifier->update(particle, deltaTime);
                    }
                }
            }
        }

        auto it = std::find_if(particles.begin(), particles.end(), [=](Particle& p) -> bool { return !p.active; });
        while (it != particles.end())
        {
            particles.erase(it);
            it = std::find_if(particles.begin(), particles.end(), [=](Particle& p) -> bool { return !p.active; });
        }

        if (particles.size() == 0)
            destroy();

        if (playbackTime < duration)
        {
            playbackTime += deltaTime;
        }
        else
        {
            if (!loop)
                stop();
            else
            {
                playbackTime = 0.0f;
                bounds = AxisAlignedBox::BOX_INFINITE;
            }
        }

        //Update bounds
        if (bounds == AxisAlignedBox::BOX_NULL)
            bounds = AxisAlignedBox::BOX_INFINITE;
    }

    void ParticleEmitter::play()
    {
        if (isPlaying)
            stop();

        if (duration > 0)
        {
            playbackTime = 0.0f;
            emissionTime = 0.0f;
            isPlaying = true;
        }
    }

    void ParticleEmitter::stop()
    {
        isPlaying = false;
        playbackTime = 0.0f;
        emissionTime = 0.0f;
        destroyParticles();
        destroy();
        bounds = AxisAlignedBox::BOX_INFINITE;
    }

    void ParticleEmitter::cloneProperties(ParticleEmitter* to)
    {
        to->setDuration(getDuration());
        to->setEmissionCount(getEmissionCount());
        to->setEmissionRate(getEmissionRate());
        to->setLifeTimeMin(getLifeTimeMin());
        to->setLifeTimeMax(getLifeTimeMax());
        to->setLoop(getLoop());
        to->setMaterial(getMaterial());
        to->setMaxParticles(getMaxParticles());
        to->setOrigin(getOrigin());
        to->setRenderType(getRenderType());
        to->setShape(getShape());
        to->setSimulationSpace(getSimulationSpace());
        to->setStartDirection(getStartDirection());
        to->setStartSizeMin(getStartSizeMin());
        to->setStartSizeMax(getStartSizeMax());
        to->setStartSpeed(getStartSpeed());
        to->setTimeScale(getTimeScale());
        to->setRadius(getRadius());
        to->setSize(getSize());
        to->setAnimated(getAnimated());
        to->setAnimationGridSize(getAnimationGridSize());
        to->setAnimationTimeMode(getAnimationTimeMode());
        to->setAnimationFps(getAnimationFps());
    }

    void ParticleEmitter::cloneModifier(int index, ParticleEmitter* to)
    {
        ParticleModifier* modifier = getModifier(index);

        if (modifier->getType() == ParticleGravityModifier::MODIFIER_TYPE)
        {
            ParticleGravityModifier* modifier1 = (ParticleGravityModifier*)modifier;
            ParticleGravityModifier* newModifier = to->addModifier<ParticleGravityModifier*>();
            newModifier->setStartTime(modifier1->getStartTime());
            newModifier->setDamping(modifier1->getDamping());
            newModifier->setGravity(modifier1->getGravity());
        }

        if (modifier->getType() == ParticleColorModifier::MODIFIER_TYPE)
        {
            ParticleColorModifier* modifier1 = (ParticleColorModifier*)modifier;
            ParticleColorModifier* newModifier = to->addModifier<ParticleColorModifier*>();
            newModifier->setStartTime(modifier1->getStartTime());
            std::vector<std::pair<float, Color>>& colors = modifier1->getColors();
            for (auto c = colors.begin(); c != colors.end(); ++c)
                newModifier->getColors().push_back(*c);
        }

        if (modifier->getType() == ParticleSizeModifier::MODIFIER_TYPE)
        {
            ParticleSizeModifier* modifier1 = (ParticleSizeModifier*)modifier;
            ParticleSizeModifier* newModifier = to->addModifier<ParticleSizeModifier*>();
            newModifier->setStartTime(modifier1->getStartTime());
            std::vector<std::pair<float, float>>& sizes = modifier1->getSizes();
            for (auto c = sizes.begin(); c != sizes.end(); ++c)
                newModifier->getSizes().push_back(*c);
        }

        if (modifier->getType() == ParticleDirectionModifier::MODIFIER_TYPE)
        {
            ParticleDirectionModifier* modifier1 = (ParticleDirectionModifier*)modifier;
            ParticleDirectionModifier* newModifier = to->addModifier<ParticleDirectionModifier*>();
            newModifier->setStartTime(modifier1->getStartTime());
            newModifier->setChangeInterval(modifier1->getChangeInterval());
            newModifier->setConstantDirection(modifier1->getConstantDirection());
            newModifier->setDirectionType(modifier1->getDirectionType());
            newModifier->setRandomDirectionMin(modifier1->getRandomDirectionMin());
            newModifier->setRandomDirectionMax(modifier1->getRandomDirectionMax());
        }

        if (modifier->getType() == ParticleRotationModifier::MODIFIER_TYPE)
        {
            ParticleRotationModifier* modifier1 = (ParticleRotationModifier*)modifier;
            ParticleRotationModifier* newModifier = to->addModifier<ParticleRotationModifier*>();
            newModifier->setStartTime(modifier1->getStartTime());
            newModifier->setChangeInterval(modifier1->getChangeInterval());
            newModifier->setConstantRotation(modifier1->getConstantRotation());
            newModifier->setRotationType(modifier1->getRotationType());
            newModifier->setRandomRotationMin(modifier1->getRandomRotationMin());
            newModifier->setRandomRotationMax(modifier1->getRandomRotationMax());
            newModifier->setOffset(modifier1->getOffset());
        }

        if (modifier->getType() == ParticleSpeedModifier::MODIFIER_TYPE)
        {
            ParticleSpeedModifier* modifier1 = (ParticleSpeedModifier*)modifier;
            ParticleSpeedModifier* newModifier = to->addModifier<ParticleSpeedModifier*>();
            newModifier->setStartTime(modifier1->getStartTime());
            newModifier->setConstantSpeed(modifier1->getConstantSpeed());
        }
    }

    void ParticleEmitter::setSimulationSpace(SimulationSpace value)
    {
        simulationSpace = value;
        if (isPlaying)
            play();
    }

    template<>
    ParticleGravityModifier* ParticleEmitter::addModifier()
    {
        ParticleGravityModifier* modifier = new ParticleGravityModifier(this);
        modifiers.push_back(modifier);

        return modifier;
    }

    template<>
    ParticleColorModifier* ParticleEmitter::addModifier()
    {
        ParticleColorModifier* modifier = new ParticleColorModifier(this);
        modifiers.push_back(modifier);

        return modifier;
    }

    template<>
    ParticleSizeModifier* ParticleEmitter::addModifier()
    {
        ParticleSizeModifier* modifier = new ParticleSizeModifier(this);
        modifiers.push_back(modifier);

        return modifier;
    }

    template<>
    ParticleDirectionModifier* ParticleEmitter::addModifier()
    {
        ParticleDirectionModifier* modifier = new ParticleDirectionModifier(this);
        modifiers.push_back(modifier);

        return modifier;
    }

    template<>
    ParticleRotationModifier* ParticleEmitter::addModifier()
    {
        ParticleRotationModifier* modifier = new ParticleRotationModifier(this);
        modifiers.push_back(modifier);

        return modifier;
    }

    template<>
    ParticleSpeedModifier* ParticleEmitter::addModifier()
    {
        ParticleSpeedModifier* modifier = new ParticleSpeedModifier(this);
        modifiers.push_back(modifier);

        return modifier;
    }

    ParticleModifier* ParticleEmitter::getModifier(int index)
    {
        assert(index < modifiers.size() && "Out of bounds");
        return modifiers[index];
    }

    void ParticleEmitter::removeModifier(ParticleModifier* modifier)
    {
        auto it = std::find(modifiers.begin(), modifiers.end(), modifier);
        if (it != modifiers.end())
        {
            ParticleModifier* mod = *it;
            delete mod;
            modifiers.erase(it);
        }
    }

    void ParticleEmitter::removeModifier(int index)
    {
        if (index < modifiers.size())
        {
            auto it = modifiers.begin() + index;
            ParticleModifier* mod = *it;
            delete mod;
            modifiers.erase(it);
        }
    }

    AxisAlignedBox ParticleEmitter::getBounds(bool world)
    {
        if (world)
        {
            if (simulationSpace == SimulationSpace::Local)
            {
                Transform* transform = parent->getGameObject()->getTransform();
                glm::mat4x4 mtx = transform->getTransformMatrix();

                AxisAlignedBox cachedAAB = bounds;
                cachedAAB.transform(mtx);

                return cachedAAB;
            }
            else
                return bounds;
        }
        else
        {
            return bounds;
        }
    }

    bool ParticleEmitter::isTransparent()
    {
        return false;
    }

    bool ParticleEmitter::getCastShadows()
    {
        return parent->getCastShadows();
    }

    void ParticleEmitter::onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback)
    {
        if (!isPlaying)
            return;

        if (parent == nullptr)
            return;

        if (parent->getGameObject() == nullptr)
            return;

        if (!parent->getGameObject()->getActive())
            return;

        if (!parent->getEnabled())
            return;

        if (particles.size() == 0)
            return;

        Transform* psTransform = parent->getGameObject()->getTransform();
        glm::mat4x4 trans = glm::identity<glm::mat4x4>();
        if (simulationSpace == SimulationSpace::Local)
            trans = psTransform->getTransformMatrix();

        glm::mat3x3 normalMatrix = glm::identity<glm::mat3x3>();

        if (program.idx == bgfx::kInvalidHandle)
            normalMatrix = trans;

        ///------------------------------------------------------------------
        //Update buffers

        glm::highp_quat rot = glm::identity<glm::highp_quat>();
        if (camera != nullptr)
        {
            Transform* ct = camera->getTransform();
            if (renderType == RenderType::Billboard)
            {
                rot = glm::quatLookAt(-ct->getForward(), ct->getUp());

                if (simulationSpace == SimulationSpace::Local)
                    rot = glm::inverse(psTransform->getRotation()) * rot;
            }
        }

        int particlesCount = particles.size();

        uint32_t _vertexCount = 4 * particlesCount;
        uint32_t _indexCount = 6 * particlesCount;

        if (_vertexCount > 0)
        {
            if (_vertexCount != vertexCount)
                recreate();

            bounds = AxisAlignedBox::BOX_NULL;

            int curParticle = 0;
            for (uint32_t i = 0; i < vertexCount; i += 4, ++curParticle)
            {
                Particle& particle = particles[curParticle];
                
                float x = particle.position.x;
                float y = particle.position.y;
                float z = particle.position.z;
                float w = particle.size;
                float h = particle.size;

                if (w < 0.0f) w = 0.0f;
                if (h < 0.0f) h = 0.0f;

                glm::highp_quat angle = particle.rotation;
                glm::vec3 center = glm::vec3(x, y, z);
                
                glm::vec3 positions[4];
                float hb = h / 2.0f;
                float ht = h / 2.0f;
                    
                positions[0] = glm::vec3(x, y - hb, z) - glm::vec3((float)w / 2, 0, 0);
                positions[1] = glm::vec3(x, y + ht, z) - glm::vec3((float)w / 2, 0, 0);
                positions[2] = glm::vec3(x, y + ht, z) + glm::vec3((float)w / 2, 0, 0);
                positions[3] = glm::vec3(x, y - hb, z) + glm::vec3((float)w / 2, 0, 0);

                float x1 = 1;
                float y1 = 1;
                float x0 = 0;
                float y0 = 0;

                if (animated && animationGridSize.x > 0 && animationGridSize.y > 0)
                {
                    int y = std::ceilf(particle.currentFrame / animationGridSize.x);
                    int x = (int)(particle.currentFrame + 1.0f) - (animationGridSize.x * y);

                    float w = 1.0f / animationGridSize.x;
                    float h = 1.0f / animationGridSize.y;

                    x0 = (w * (float)x);
                    y0 = 1.0f - (h * (float)y);
                    x1 = x0 - w;
                    y1 = y0 + h;
                }

                for (int j = 0; j < 4; ++j)
                {
                    glm::vec2 uv = glm::vec2(0, 0);
                    if (j == 1) uv = glm::vec2(0, 1);
                    if (j == 2) uv = glm::vec2(1, 1);
                    if (j == 3) uv = glm::vec2(1, 0);

                    if (animated && animationGridSize.x > 0 && animationGridSize.y > 0)
                    {
                        uv = glm::vec2(x0, y0);
                        if (j == 1) uv = glm::vec2(x0, y1);
                        if (j == 2) uv = glm::vec2(x1, y1);
                        if (j == 3) uv = glm::vec2(x1, y0);
                    }

                    VertexBuffer* vert = &vertices[i + j];

                    vert->position = ((rot * angle) * (positions[j] - center)) + center;
                    if (origin == Origin::Bottom)
                    {
                        glm::vec3 dir = particle.direction;
                        dir = glm::normalize(dir);
                        if (glm::isnan(dir).x) dir.x = 0.0f;
                        if (glm::isnan(dir).y) dir.y = 0.0f;
                        if (glm::isnan(dir).z) dir.z = 0.0f;
                        vert->position += dir * glm::vec3(h / 2.0f);
                    }

                    if (!glm::isnan(vert->position).x && !glm::isnan(vert->position).y && !glm::isnan(vert->position).z)
                    {
                        bounds.merge(vert->position);
                    }

                    vert->texcoord0 = uv;
                    vert->color = Color::packABGR(particle.color);
                }
            }

            int _indexCount = 0;
            for (uint32_t i = 0; i < vertexCount; i += 4)
            {
                indices[_indexCount + 0] = i;
                indices[_indexCount + 1] = i + 1;
                indices[_indexCount + 2] = i + 2;
                indices[_indexCount + 3] = i;
                indices[_indexCount + 4] = i + 2;
                indices[_indexCount + 5] = i + 3;
                _indexCount += 6;
            }

            const bgfx::Memory* mem = bgfx::makeRef(vertices, sizeof(VertexBuffer) * vertexCount);
            bgfx::update(vbh, 0, mem);

            mem = bgfx::makeRef(indices, sizeof(uint32_t) * indexCount);
            bgfx::update(ibh, 0, mem);
        }
        else
        {
            destroyParticles();
            destroy();
        }
        ///------------------------------------------------------------------

        if (!bgfx::isValid(vbh) || !bgfx::isValid(ibh))
            return;

        Shader* shader = nullptr;

        if (material != nullptr && material->isLoaded())
            shader = material->getShader();

        if (program.idx == bgfx::kInvalidHandle)
        {
            if (shader == nullptr || !shader->isLoaded())
                return;

            if (renderMode != static_cast<int>(shader->getRenderMode()))
                return;
        }

        int passCount = 1;

        if (program.idx == bgfx::kInvalidHandle)
        {
            if (shader != nullptr && shader->isLoaded())
                passCount = shader->getPassCount();
        }

        for (int j = 0; j < passCount; ++j)
        {
            Pass* pass = nullptr;

            if (shader != nullptr && shader->isLoaded())
                pass = shader->getPass(j);

            ProgramVariant* pv = nullptr;
            if (material != nullptr && material->isLoaded() && pass != nullptr)
                pv = pass->getProgramVariant(material->getDefinesStringHash());

            int iterationCount = 1;

            std::vector<Light*>& lights = Renderer::getSingleton()->getLights();

            if (program.idx == bgfx::kInvalidHandle)
            {
                if (pv != nullptr && pv->iterationMode == IterationMode::PerLight)
                {
                    iterationCount = lights.size();
                }
            }

            for (int iter = 0; iter < iterationCount; ++iter)
            {
                uint64_t passState = state;

                if (pv != nullptr)
                {
                    if (program.idx == bgfx::kInvalidHandle)
                    {
                        if (pv->iterationMode == IterationMode::PerLight)
                        {
                            Light* light = lights[iter];
                            if (!light->submitUniforms())
                                continue;
                        }
                        else
                        {
                            if (lights.size() > 0)
                            {
                                Light* light = Renderer::getSingleton()->getFirstLight();
                                if (light != nullptr)
                                    light->submitUniforms();
                            }
                        }
                    }
                }

                if (pv != nullptr)
                    passState = pv->getRenderState(state);

                // Set model matrix for rendering.
                bgfx::setTransform(glm::value_ptr(trans));

                // Set vertex and index buffer.
                bgfx::setVertexBuffer(0, vbh);
                bgfx::setIndexBuffer(ibh);

                // Set render states.
                bgfx::setState(passState);

                // Pass uniforms
                if (program.idx == bgfx::kInvalidHandle)
                {
                    if (pv != nullptr)
                        material->submitUniforms(pv, camera);
                }
                else
                {
                    bgfx::UniformHandle tex = Renderer::getSingleton()->getAlbedoMapUniform();
                    auto& uniforms = material->getUniforms();
                    bool textureWasSet = false;
                    for (auto& u : uniforms)
                    {
                        if (u.getType() == UniformType::Sampler2D)
                        {
                            const UniformVariant* var = pv->getUniform(u.getNameHash());
                            if (var != nullptr)
                            {
                                Texture* tx = u.getValue<Sampler2DDef>().second;
                                if (tx != nullptr)
                                {
                                    bgfx::setTexture(0, tex, tx->getHandle());
                                    textureWasSet = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (!textureWasSet)
                    {
                        Texture* nullTexture = Texture::getNullTexture();
                        bgfx::setTexture(0, tex, nullTexture->getHandle());
                    }
                }

                //Bind system uniforms
                bgfx::setUniform(Renderer::getNormalMatrixUniform(), glm::value_ptr(normalMatrix), 1);
                bgfx::setUniform(Renderer::getGpuSkinningUniform(), glm::value_ptr(glm::vec4(0.0, 0.0, 0.0, 0.0)), 1);
                bgfx::setUniform(Renderer::getHasLightmapUniform(), glm::value_ptr(glm::vec4(0.0, 0.0, 0.0, 0.0)), 1);

                if (preRenderCallback != nullptr)
                    preRenderCallback();

                // Render
                bgfx::ProgramHandle ph = { bgfx::kInvalidHandle };

                if (program.idx != bgfx::kInvalidHandle)
                    ph = program;
                else if (pv != nullptr)
                    ph = pv->programHandle;

                bgfx::submit(view, ph);
            }
        }
    }

    bool ParticleEmitter::checkCullingMask(LayerMask& mask)
    {
        GameObject* gameObject = getParent()->getGameObject();

        if (gameObject == nullptr)
            return false;

        return mask.getLayer(gameObject->getLayer());
    }

    //***PARTICLE SYSTEM***//

    ParticleSystem::ParticleSystem() : Component(APIManager::getSingleton()->particlesystem_class)
    {
    }

    ParticleSystem::~ParticleSystem()
    {
        for (auto it = emitters.begin(); it != emitters.end(); ++it)
            delete* it;

        emitters.clear();
    }

    ParticleEmitter* ParticleSystem::getEmitter(int index)
    {
        assert(index < emitters.size() && "Out of bounds");
        return emitters[index];
    }

    ParticleEmitter* ParticleSystem::addEmitter()
    {
        ParticleEmitter* emitter = new ParticleEmitter(this);
        emitters.push_back(emitter);

        return emitter;
    }

    void ParticleSystem::removeEmitter(ParticleEmitter* emitter)
    {
        auto it = std::find(emitters.begin(), emitters.end(), emitter);
        if (it != emitters.end())
        {
            ParticleEmitter* em = *it;
            delete em;
            emitters.erase(it);
        }
    }

    void ParticleSystem::removeEmitter(int index)
    {
        if (index < emitters.size())
        {
            auto it = emitters.begin() + index;
            ParticleEmitter* em = *it;
            delete em;
            emitters.erase(it);
        }
    }

    bool ParticleSystem::isPlaying()
    {
        bool value = false;

        for (auto it = emitters.begin(); it != emitters.end(); ++it)
        {
            ParticleEmitter* emitter = *it;

            if (emitter->getIsPlaying())
            {
                value = true;
                break;
            }
        }

        return value;
    }

    void ParticleSystem::play()
    {
        for (auto it = emitters.begin(); it != emitters.end(); ++it)
        {
            ParticleEmitter* emitter = *it;
            emitter->play();
        }
    }

    void ParticleSystem::stop()
    {
        for (auto it = emitters.begin(); it != emitters.end(); ++it)
        {
            ParticleEmitter* emitter = *it;
            emitter->stop();
        }
    }

    void ParticleSystem::onSceneLoaded()
    {
        if (playAtStart)
            play();
    }

    void ParticleSystem::onUpdate(float deltaTime)
    {
        float timeScale = Time::getTimeScale();

        for (auto it = emitters.begin(); it != emitters.end(); ++it)
        {
            ParticleEmitter* emitter = *it;
            emitter->update((deltaTime * emitter->getTimeScale()) * timeScale);
        }
    }

    Component* ParticleSystem::onClone()
    {
        ParticleSystem* newComponent = new ParticleSystem();
        newComponent->enabled = enabled;
        newComponent->castShadows = castShadows;
        newComponent->playAtStart = playAtStart;

        for (auto it = emitters.begin(); it != emitters.end(); ++it)
        {
            ParticleEmitter* emitter = *it;
            ParticleEmitter* newEmitter = newComponent->addEmitter();

            emitter->cloneProperties(newEmitter);

            std::vector<ParticleModifier*>& modifiers = emitter->getModifiers();
            for (int i = 0; i < modifiers.size(); ++i)
            {
                emitter->cloneModifier(i, newEmitter);
            }
        }

        if (isPlaying())
            newComponent->play();

        return newComponent;
    }

    bool ParticleSystem::isEqualsTo(Component* other)
    {
        ParticleSystem* comp2 = (ParticleSystem*)other;

        bool eq1 = getEmitters().size() == comp2->getEmitters().size();

        return eq1;
    }

    void ParticleSystem::onAttach()
    {
        Component::onAttach();

        for (auto& em : emitters)
            em->attach();
    }

    void ParticleSystem::onDetach()
    {
        Component::onDetach();

        for (auto& em : emitters)
            em->detach();
    }
}