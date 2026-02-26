#include "AnimationClip.h"

#include <iostream>
#include <cassert>
#include <fstream>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "../Core/Engine.h"
#include "../Classes/IO.h"
#include "../Classes/Hash.h"
#include "../Serialization/Assets/SAnimationClip.h"
#include "../Core/Debug.h"
#include "../Core/APIManager.h"
#include "../Math/Mathf.h"

#include "../Components/Transform.h"
#include "../Classes/ZipHelper.h"

namespace GX
{
    std::string AnimationClip::ASSET_TYPE = "AnimationClip";

    AnimationClipNode::~AnimationClipNode()
    {
        parent = nullptr;

        positionKeys.clear();
        scalingKeys.clear();
        rotationKeys.clear();
    }

    void AnimationClipNode::setName(std::string value)
    {
        name = value;
        nameHash = Hash::getHash(name);
    }

    AnimationClip::AnimationClip() : Asset(APIManager::getSingleton()->animationclip_class)
    {
    }

    AnimationClip::~AnimationClip()
    {
    }

    void AnimationClip::load()
    {
        if (!isLoaded())
        {
            Asset::load();
        }
    }

    void AnimationClip::unload()
    {
        if (isLoaded())
        {
            Asset::unload();

            clear();
        }
    }

    void AnimationClip::unloadAll()
    {
        std::vector<AnimationClip*> deleteAssets;

        for (auto it = loadedInstances.begin(); it != loadedInstances.end(); ++it)
        {
            if (it->second->getAssetType() == AnimationClip::ASSET_TYPE)
            {
                if (!it->second->getPersistent())
                    deleteAssets.push_back((AnimationClip*)it->second);
            }
        }

        for (auto it = deleteAssets.begin(); it != deleteAssets.end(); ++it)
            delete (*it);

        deleteAssets.clear();
    }

    void AnimationClip::reload()
    {
        if (getOrigin().empty())
            return;

        if (isLoaded())
            unload();

        load(location, name);
    }

    AnimationClip* AnimationClip::create(std::string location, std::string name)
    {
        Asset* cachedAsset = getLoadedInstance(location, name);
        assert(cachedAsset == nullptr && "Asset with these location and name already exists");

        AnimationClip* animClip = new AnimationClip();
        animClip->setLocation(location);
        animClip->setName(name);

        return animClip;
    }

    void AnimationClip::save()
    {
        save(getOrigin());
    }

    void AnimationClip::save(std::string path)
    {
        SAnimationClip sAnimClip;
        sAnimClip.duration = duration;
        sAnimClip.framesPerSecond = framesPerSecond;

        for (auto it = animationClipNodes.begin(); it != animationClipNodes.end(); ++it)
        {
            AnimationClipNode* animClipNode = *it;
            SAnimationClipNode sAnimClipNode;

            sAnimClipNode.name = animClipNode->getName();

            std::vector<TimeVector3>& positions = animClipNode->getPositionKeys();
            std::vector<TimeVector3>& scalings = animClipNode->getScalingKeys();
            std::vector<TimeQuaternion>& rotations = animClipNode->getRotationKeys();

            for (auto kt = positions.begin(); kt != positions.end(); ++kt)
                sAnimClipNode.positionKeys.push_back(STimeVector3(kt->time, kt->value));

            for (auto kt = scalings.begin(); kt != scalings.end(); ++kt)
                sAnimClipNode.scalingKeys.push_back(STimeVector3(kt->time, kt->value));

            for (auto kt = rotations.begin(); kt != rotations.end(); ++kt)
                sAnimClipNode.rotationKeys.push_back(STimeQuaternion(kt->time, kt->value));

            sAnimClip.animationNodes.push_back(sAnimClipNode);
        }

        std::string _path = boost::replace_all_copy(path, "|", "_");
        
        if (_path.find(".animation") == std::string::npos)
            _path += ".animation";

        std::ofstream ofs(_path, std::ios::binary);
        BinarySerializer s;
        s.serialize(&ofs, &sAnimClip, AnimationClip::ASSET_TYPE);
        ofs.close();

        sAnimClip.animationNodes.clear();
    }

    void AnimationClip::addAnimationClipNode(AnimationClipNode* value)
    {
        value->parent = this;
        animationClipNodes.push_back(value);
    }

    void AnimationClip::deleteAnimationClipNode(AnimationClipNode* value)
    {
        auto it = std::find(animationClipNodes.begin(), animationClipNodes.end(), value);
        if (it != animationClipNodes.end())
        {
            animationClipNodes.erase(it);
            delete value;
            value = nullptr;
        }
    }

    AnimationClip* AnimationClip::load(std::string location, std::string name, bool warn)
    {
        std::string fullPath = location + name;

        Asset* cachedAsset = getLoadedInstance(location, name);

        AnimationClip* animClip = nullptr;

        if (cachedAsset != nullptr)
        {
            animClip = (AnimationClip*)cachedAsset;

            if (animClip->isLoaded())
                return animClip;
        }

        if (IO::isDir(location))
        {
            if (!IO::FileExists(fullPath))
            {
                if (warn)
                    Debug::logWarning("[" + name + "] Error loading animation clip: file does not exists");
                return animClip;
            }
        }
        else
        {
            zip_t* arch = Engine::getSingleton()->getZipArchive(location);
            if (!ZipHelper::isFileInZip(arch, name))
            {
                if (warn)
                    Debug::logWarning("[" + fullPath + "] Error loading animation clip: file does not exists");
                return animClip;
            }
        }

        if (animClip == nullptr)
        {
            animClip = new AnimationClip();
            animClip->setLocation(location);
            animClip->setName(name);
        }

        SAnimationClip sAnimClip;

        if (IO::isDir(location))
        {
            try
            {
                std::ifstream ofs(fullPath, std::ios::binary);
                BinarySerializer s;
                s.deserialize(&ofs, &sAnimClip, AnimationClip::ASSET_TYPE);
                ofs.close();
            }
            catch (std::exception e)
            {
                Debug::log("[" + name + "] Error loading animation clip: " + e.what());
            }
        }
        else
        {
            zip_t* arch = Engine::getSingleton()->getZipArchive(location);

            int sz = 0;
            char* buffer = ZipHelper::readFileFromZip(arch, name, sz);

            try
            {
                boost::iostreams::stream<boost::iostreams::array_source> is(buffer, sz);
                BinarySerializer s;
                s.deserialize(&is, &sAnimClip, AnimationClip::ASSET_TYPE);
                is.close();
            }
            catch (std::exception e)
            {
                Debug::log("[" + name + "] Error loading animation clip: " + e.what());
            }

            delete[] buffer;
        }

        animClip->setDuration(sAnimClip.duration);
        animClip->setFramesPerSecond(sAnimClip.framesPerSecond);

        for (auto it = sAnimClip.animationNodes.begin(); it != sAnimClip.animationNodes.end(); ++it)
        {
            SAnimationClipNode& sAnimClipNode = *it;
            AnimationClipNode* animClipNode = new AnimationClipNode();

            animClipNode->setName(sAnimClipNode.name);

            for (auto kt = sAnimClipNode.positionKeys.begin(); kt != sAnimClipNode.positionKeys.end(); ++kt)
                animClipNode->addPositionKey(kt->time, kt->value.getValue());

            for (auto kt = sAnimClipNode.scalingKeys.begin(); kt != sAnimClipNode.scalingKeys.end(); ++kt)
                animClipNode->addScalingKey(kt->time, kt->value.getValue());

            for (auto kt = sAnimClipNode.rotationKeys.begin(); kt != sAnimClipNode.rotationKeys.end(); ++kt)
                animClipNode->addRotationKey(kt->time, kt->value.getValue());

            animClip->addAnimationClipNode(animClipNode);
        }

        animClip->load();

        sAnimClip.animationNodes.clear();

        return animClip;
    }

    AnimationClip* AnimationClip::crop(int startFrame, int endFrame, std::string name)
    {
        AnimationClip* animClip = AnimationClip::create(getLocation(), name);
        animClip->setDuration(endFrame - startFrame);
        animClip->setFramesPerSecond(framesPerSecond);

        for (auto it = animationClipNodes.begin(); it != animationClipNodes.end(); ++it)
        {
            AnimationClipNode* clipNode = *it;
            AnimationClipNode* newClipNode = new AnimationClipNode();

            newClipNode->setName(clipNode->getName());

            int start = startFrame;
            int end = endFrame;

            if (start < 0)
                start = 0;

            if (end > clipNode->getPositionKeys().size())
                end = clipNode->getPositionKeys().size();

            if (start > end)
                start = end;

            for (int k = start; k < end; ++k)
            {
                TimeVector3& val = clipNode->getPositionKeys()[k];
                newClipNode->addPositionKey(val.time - start, val.value);
            }

            for (int k = start; k < end; ++k)
            {
                TimeVector3& val = clipNode->getScalingKeys()[k];
                newClipNode->addScalingKey(val.time - start, val.value);
            }

            for (int k = start; k < end; ++k)
            {
                TimeQuaternion& val = clipNode->getRotationKeys()[k];
                newClipNode->addRotationKey(val.time - start, val.value);
            }

            animClip->addAnimationClipNode(newClipNode);
        }

        return animClip;
    }

    void AnimationClip::clear()
    {
        for (auto it = animationClipNodes.begin(); it != animationClipNodes.end(); ++it)
            delete* it;

        animationClipNodes.clear();

        duration = 0.0f;
        framesPerSecond = 25.0f;
    }

    glm::mat4x4 AnimationClipNode::getTransformAtTime(float time, Transform* transform)
    {
        if (parent->getDuration() <= 0)
            return transform->getLocalTransformMatrix();

        glm::mat4x4 matrix = glm::identity<glm::mat4x4>();
        glm::mat4x4 rotMat = glm::identity<glm::mat4x4>();
        glm::mat4x4 invRotMat = glm::identity<glm::mat4x4>();

        //Rotation
        glm::quat r1 = transform->getLocalRotation();
        glm::quat r2 = transform->getLocalRotation();

        float t1 = 0.0f;
        float t2 = parent->getDuration();

        if (rotationKeys.size() > 0)
        {
            for (int i = rotationKeys.size() - 1; i >= 0; --i)
            {
                if (time >= rotationKeys[i].time)
                {
                    if (rotationKeys[i].time >= t1)
                    {
                        t1 = rotationKeys[i].time;
                        r1 = rotationKeys[i].value;
                    }
                }
            }

            for (int i = 0; i < rotationKeys.size(); ++i)
            {
                if (rotationKeys[i].time > time)
                {
                    if (rotationKeys[i].time <= t2)
                    {
                        t2 = rotationKeys[i].time;
                        r2 = rotationKeys[i].value;
                    }
                }
            }
        }

        glm::quat r;
        float t = t2 - t1;
        if (t > 0)
        {
            float tt = time - t1;
            t = 1.0f - ((t - tt) / t);
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            r = glm::slerp(r1, r2, t);
        }
        else
        {
            r = r1;
        }

        rotMat = glm::mat4_cast(r);
        invRotMat = glm::inverse(rotMat);

        //Position
        glm::vec3 p1 = transform->getLocalPosition();
        glm::vec3 p2 = transform->getLocalPosition();

        t1 = 0.0f;
        t2 = parent->getDuration();

        if (positionKeys.size() > 0)
        {
            for (int i = positionKeys.size() - 1; i >= 0; --i)
            {
                if (time >= positionKeys[i].time)
                {
                    if (positionKeys[i].time >= t1)
                    {
                        t1 = positionKeys[i].time;
                        p1 = positionKeys[i].value;
                    }
                }
            }

            for (int i = 0; i < positionKeys.size(); ++i)
            {
                if (positionKeys[i].time > time)
                {
                    if (positionKeys[i].time <= t2)
                    {
                        t2 = positionKeys[i].time;
                        p2 = positionKeys[i].value;
                    }
                }
            }
        }

        glm::vec3 p;
        t = t2 - t1;
        if (t > 0)
        {
            float tt = time - t1;
            t = 1.0f - ((t - tt) / t);
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            p = Mathf::lerp(p1, p2, t);
        }
        else
            p = p1;

        matrix = glm::translate(matrix, glm::vec3(invRotMat * glm::vec4(p, 1.0f)));
        matrix = rotMat * matrix;

        //Scale
        glm::vec3 s1 = transform->getLocalScale();
        glm::vec3 s2 = transform->getLocalScale();

        t1 = 0.0f;
        t2 = parent->getDuration();

        if (scalingKeys.size() > 0)
        {
            for (int i = scalingKeys.size() - 1; i >= 0; --i)
            {
                if (time >= scalingKeys[i].time)
                {
                    if (scalingKeys[i].time >= t1)
                    {
                        t1 = scalingKeys[i].time;
                        s1 = scalingKeys[i].value;
                    }
                }
            }

            for (int i = 0; i < scalingKeys.size(); ++i)
            {
                if (scalingKeys[i].time > time)
                {
                    if (scalingKeys[i].time <= t2)
                    {
                        t2 = scalingKeys[i].time;
                        s2 = scalingKeys[i].value;
                    }
                }
            }
        }

        glm::vec3 s;
        t = t2 - t1;
        if (t > 0)
        {
            float tt = time - t1;
            t = 1.0f - ((t - tt) / t);
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            s = Mathf::lerp(s1, s2, t);
        }
        else
            s = s1;

        matrix = glm::scale(matrix, s);

        return matrix;
    }
}