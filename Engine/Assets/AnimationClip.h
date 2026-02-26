#pragma once

#include <vector>
#include <string>
#include <bgfx/bgfx.h>
#include "../glm/vec3.hpp"
#include "../glm/vec2.hpp"
#include "../glm/gtc/quaternion.hpp"

#include "Asset.h"

namespace GX
{
    class AnimationClip;
    class Transform;

    struct TimeVector3
    {
    public:
        TimeVector3(double _time, glm::vec3 _value)
        {
            time = _time;
            value = _value;
        }

        double time = 0;
        glm::vec3 value = glm::vec3(0, 0, 0);
    };

    struct TimeQuaternion
    {
    public:
        TimeQuaternion(double _time, glm::highp_quat _value)
        {
            time = _time;
            value = _value;
        }

        double time = 0;
        glm::highp_quat value = glm::identity<glm::highp_quat>();
    };

    class AnimationClipNode
    {
        friend class AnimationClip;

    private:
        std::string name = "";
        size_t nameHash = 0;

        std::vector<TimeVector3> positionKeys;
        std::vector<TimeVector3> scalingKeys;
        std::vector<TimeQuaternion> rotationKeys;
        
        AnimationClip* parent = nullptr;

    public:
        AnimationClipNode() = default;
        ~AnimationClipNode();

        std::vector<TimeVector3>& getPositionKeys() { return positionKeys; }
        std::vector<TimeVector3>& getScalingKeys() { return scalingKeys; }
        std::vector<TimeQuaternion>& getRotationKeys() { return rotationKeys; }

        void addPositionKey(float time, glm::vec3 value) { positionKeys.push_back(TimeVector3(time, value)); }
        void addScalingKey(float time, glm::vec3 value) { scalingKeys.push_back(TimeVector3(time, value)); }
        void addRotationKey(float time, glm::highp_quat value) { rotationKeys.push_back(TimeQuaternion(time, value)); }

        glm::mat4x4 getTransformAtTime(float time, Transform* transform);

        std::string getName() { return name; }
        size_t getNameHash() { return nameHash; }
        void setName(std::string value);
    };

    class AnimationClip : public Asset
    {
    private:
        std::vector<AnimationClipNode*> animationClipNodes;
        float duration = 0.0f;
        float framesPerSecond = 25.0f;

    public:
        AnimationClip();
        virtual ~AnimationClip();

        static std::string ASSET_TYPE;

        virtual void load();
        virtual void unload();
        static void unloadAll();
        virtual void reload();
        virtual std::string getAssetType() { return ASSET_TYPE; }
        static AnimationClip* create(std::string location, std::string name);
        static AnimationClip* load(std::string location, std::string name, bool warn = true);
        void save();
        void save(std::string path);

        std::vector<AnimationClipNode*>& getAnimationClipNodes() { return animationClipNodes; }
        void addAnimationClipNode(AnimationClipNode* value);
        void deleteAnimationClipNode(AnimationClipNode* value);

        float getDuration() { return duration; }
        void setDuration(float value) { duration = value; }

        float getFramesPerSecond() { return framesPerSecond; }
        void setFramesPerSecond(float value) { framesPerSecond = value; }

        AnimationClip* crop(int startFrame, int endFrame, std::string name);
        void clear();
    };
}