#include "Engine.h"

#define FREEIMAGE_LIB
#include "../FreeImage/include/FreeImage.h"
#undef FREEIMAGE_LIB

#include "Serialization/Settings/PlayerPrefs.h"

#include "GameObject.h"
#include "Components/Transform.h"
#include "Components/Animation.h"
#include "Components/Camera.h"

#include "Renderer/Renderer.h"
#include "Classes/VectorUtils.h"
#include "Classes/IO.h"
#include "Classes/StringConverter.h"
#include "Assets/Asset.h"
#include "Core/Debug.h"

#include "../LibZip/include/zip.h"

namespace GX
{
    std::vector<std::string> Engine::audioFileFormats = { "ogg", "wav", "mp3" };
    std::vector<std::string> Engine::videoFileFormats = { "mp4", "avi", "wmv", "3gp", "mkv" };
    std::vector<std::string> Engine::imageFileFormats = { "jpg", "png", "jpeg", "bmp", "psd", "tga", "gif", "pic", "ppm", "pgm", "hdr", "dds", "tif" };
    std::vector<std::string> Engine::model3dFileFormats = { "3d", "3ds", "3mf", "ac", "ac3d", "acc", "amj", "ase", "ask", "b3d", "blend", "bvh", "cms", "cob", "dae", "dxf", "enff", "fbx", "gltf",
                                                            "hmb", "ifc", "irr", "lwo", "lws", "lxo", "m3d", "md2", "md3", "md5", "mdc", "mdl", "mesh", "mot", "ms3d", "ndo", "nff", "obj", "off",
                                                            "ogex", "ply", "pmx", "prj", "q3o", "q3s", "raw", "scn", "sib", "smd", "stp", "stl", "ter", "uc", "vta", "x", "x3d", "xgl", "zgl" };

    Engine Engine::singleton;
    ProjectSettings Engine::settings;

    Engine::Engine()
    {
        
    }

    Engine::~Engine()
    {
        clear();

        for (auto& it : zipArchives)
            zip_close_z(it.second);

        zipArchives.clear();
    }

    void Engine::openArchive(std::string path)
    {
        std::string _path = path;

        if (_path[_path.size() - 1] == '/')
            _path.resize(_path.size() - 1);

        if (IO::FileExists(_path) && zipArchives[_path] == nullptr)
        {
            struct zip* za = nullptr;
            int err = 0;
            char buf[100];
            if ((za = zip_open_z(CP_UNI(_path).c_str(), 0, &err)) == NULL)
            {
                zip_error_to_str(buf, sizeof(buf), err, errno);
                Debug::logError(std::string(buf));
            }
            else
                zipArchives[path] = za;
        }
    }

    void Engine::setAssetsPath(std::string path)
    {
        assetsPath = path;
        if (!IO::DirExists(assetsPath))
        {
            if (!assetsPath.empty())
            {
                if (assetsPath[assetsPath.size() - 1] == '/')
                    assetsPath.resize(assetsPath.size() - 1);

                assetsPath += ".resources/";

                openArchive(assetsPath);
            }
        }
    }

    void Engine::setBuiltinResourcesPath(std::string path)
    {
        builtinResourcesPath = path;
        if (!IO::DirExists(builtinResourcesPath))
        {
            if (!builtinResourcesPath.empty())
            {
                if (builtinResourcesPath[builtinResourcesPath.size() - 1] == '/')
                    builtinResourcesPath.resize(builtinResourcesPath.size() - 1);

                builtinResourcesPath += ".resources/";

                openArchive(builtinResourcesPath);
            }
        }
    }

    void Engine::setLibraryPath(std::string path)
    {
        libraryPath = path;

        if (!libraryPath.empty())
        {
            if (!IO::DirExists(libraryPath))
            {
                if (libraryPath[libraryPath.size() - 1] == '/')
                    libraryPath.resize(libraryPath.size() - 1);

                libraryPath += ".resources/";

                openArchive(libraryPath);
            }

            if (IO::DirExists(libraryPath))
                PlayerPrefs::Singleton.load(libraryPath + "PlayerPrefs.bin");
            else
                PlayerPrefs::Singleton.load(rootPath + "PlayerPrefs.bin");
        }
    }

    void Engine::loadPlugins()
    {
        FreeImage_Initialise();
    }

    void Engine::unloadPlugins()
    {
        FreeImage_DeInitialise();
    }

    bool Engine::getIsEditorMode()
    {
        bool isEditor = false;

        auto& cameras = Renderer::getSingleton()->getCameras();
        for (auto cam : cameras)
        {
            if (cam->getIsEditorCamera())
            {
                isEditor = true;
                break;
            }
        }

        return isEditor;
    }

    void Engine::clear()
    {
        while (rootTransforms.size() > 0)
        {
            GameObject* obj = rootTransforms[0]->getGameObject();
            destroyGameObject(obj);
        }

        rootTransforms.clear();

        Renderer::getSingleton()->clearTransientRenderables();
    }

    zip_t* Engine::getZipArchive(std::string path)
    {
        zip_t* arch = zipArchives[path];
        if (arch == nullptr)
            Debug::logError("No archives opened by path \"" + path + "\"");

        assert(arch != nullptr && "No archives opened by specified path");

        return arch;
    }

    std::vector<GameObject*>& Engine::getGameObjects()
    {
        if (needUpdateGameObjectCache)
        {
            gameObjectCache.clear();

            std::vector<Transform*>& rootTransforms = Engine::getSingleton()->getRootTransforms();
            std::vector<Transform*> nstack;
            for (auto it = rootTransforms.begin(); it != rootTransforms.end(); ++it)
                nstack.push_back(*it);

            while (nstack.size() > 0)
            {
                Transform* child = *nstack.begin();
                nstack.erase(nstack.begin());

                //
                GameObject* obj = child->gameObject;
                gameObjectCache.push_back(obj);
                //

                int j = 0;
                for (auto it = child->children.begin(); it != child->children.end(); ++it, ++j)
                {
                    Transform* ch = *it;
                    nstack.insert(nstack.begin() + j, ch);
                }
            }

            needUpdateGameObjectCache = false;
        }

        return gameObjectCache;
    }

    std::vector<Transform*>& Engine::getRootTransforms()
    {
        return rootTransforms;
    }

    int Engine::getRootObjectIndex(GameObject* object)
    {
        std::vector<Transform*>& transforms = getRootTransforms();

        auto it = std::find(transforms.begin(), transforms.end(), object->getTransform());
        if (it != transforms.end())
            return std::distance(transforms.begin(), it);

        return -1;
    }

    void Engine::setRootObjectIndex(GameObject* object, int index)
    {
        if (index < rootTransforms.size() && index >= 0)
        {
            auto it = std::find(rootTransforms.begin(), rootTransforms.end(), object->getTransform());

            if (it != rootTransforms.end())
            {
                int oldIdx = std::distance(rootTransforms.begin(), it);
                VectorUtils::move(rootTransforms, oldIdx, index);
            }
        }

        markGameObjectsOutdated();
    }

    void Engine::stopAllAnimations()
    {
        std::vector<Transform*> nstack;
        for (auto it = rootTransforms.begin(); it != rootTransforms.end(); ++it)
            nstack.push_back(*it);

        while (nstack.size() > 0)
        {
            Transform* child = *nstack.begin();
            nstack.erase(nstack.begin());

            //
            GameObject* obj = child->gameObject;
            Animation* anim = (Animation*)obj->getComponent(Animation::COMPONENT_TYPE);
            if (anim != nullptr)
                anim->stop();
            //

            int j = 0;
            for (auto it = child->children.begin(); it != child->children.end(); ++it, ++j)
            {
                Transform* ch = *it;
                nstack.insert(nstack.begin() + j, ch);
            }
        }
    }

    void Engine::stopObjectAnimations(GameObject* object)
    {
        std::vector<GameObject*> nstack;
        nstack.push_back(object);

        while (nstack.size() > 0)
        {
            GameObject* child = *nstack.begin();
            nstack.erase(nstack.begin());

            Animation* anim = (Animation*)child->getComponent(Animation::COMPONENT_TYPE);
            if (anim != nullptr)
                anim->stop();

            std::vector<Transform*>& children = child->getTransform()->getChildren();

            int j = 0;
            for (auto it = children.begin(); it != children.end(); ++it, ++j)
            {
                Transform* ch = *it;
                nstack.insert(nstack.begin() + j, ch->getGameObject());
            }
        }
    }

    GameObject* Engine::createGameObject()
    {
        GameObject* gameObject = new GameObject();
        rootTransforms.push_back(gameObject->getTransform());

        markGameObjectsOutdated();

        return gameObject;
    }

    GameObject* Engine::createGameObject(std::string guid)
    {
        GameObject* gameObject = new GameObject(guid);
        rootTransforms.push_back(gameObject->getTransform());

        markGameObjectsOutdated();

        return gameObject;
    }

    void Engine::destroyGameObject(GameObject* gameObject)
    {
        auto it = std::find(rootTransforms.begin(), rootTransforms.end(), gameObject->getTransform());

        if (it != rootTransforms.end())
            rootTransforms.erase(it);

        delete gameObject;
        gameObject = nullptr;

        markGameObjectsOutdated();
    }

    void Engine::addGameObject(GameObject* gameObject)
    {
        auto it = std::find(rootTransforms.begin(), rootTransforms.end(), gameObject->getTransform());
        if (it == rootTransforms.end())
            rootTransforms.push_back(gameObject->getTransform());

        markGameObjectsOutdated();
    }

    void Engine::removeGameObject(GameObject* gameObject)
    {
        auto it = std::find(rootTransforms.begin(), rootTransforms.end(), gameObject->getTransform());

        if (it != rootTransforms.end())
            rootTransforms.erase(it);

        markGameObjectsOutdated();
    }

    GameObject* Engine::getGameObject(std::string guid)
    {
        GameObject* result = nullptr;

        auto& gameObjects = getGameObjects();

        for (auto& obj : gameObjects)
        {
            if (obj->getGuid() == guid)
            {
                result = obj;
                break;
            }
        }

        return result;
    }

    GameObject* Engine::getGameObject(size_t guidHash)
    {
        GameObject* result = nullptr;

        auto& gameObjects = getGameObjects();

        for (auto& obj : gameObjects)
        {
            if (obj->getGuidHash() == guidHash)
            {
                result = obj;
                break;
            }
        }

        return result;
    }

    GameObject* Engine::findGameObject(std::string name)
    {
        GameObject* result = nullptr;

        auto& gameObjects = getGameObjects();

        for (auto& obj : gameObjects)
        {
            if (obj->getName() == name)
            {
                result = obj;
                break;
            }
        }

        return result;
    }

    GameObject* Engine::findGameObject(size_t nameHash)
    {
        GameObject* result = nullptr;

        auto& gameObjects = getGameObjects();

        for (auto& obj : gameObjects)
        {
            if (obj->getNameHash() == nameHash)
            {
                result = obj;
                break;
            }
        }

        return result;
    }

    GameObject* Engine::findGameObject(std::string name, GameObject* root)
    {
        GameObject * result = nullptr;

        std::vector<GameObject*> nstack;
        nstack.push_back(root);

        while (nstack.size() > 0)
        {
            GameObject* child = *nstack.begin();
            nstack.erase(nstack.begin());

            if (child->getName() == name)
            {
                result = child;
                nstack.clear();
                break;
            }

            std::vector<Transform*>& children = child->getTransform()->getChildren();

            int j = 0;
            for (auto it = children.begin(); it != children.end(); ++it, ++j)
            {
                Transform* ch = *it;
                nstack.insert(nstack.begin() + j, ch->getGameObject());
            }
        }

        return result;
    }

    GameObject* Engine::findGameObject(size_t nameHash, GameObject* root)
    {
        GameObject* result = nullptr;

        std::vector<GameObject*> nstack;
        nstack.push_back(root);

        while (nstack.size() > 0)
        {
            GameObject* child = *nstack.begin();
            nstack.erase(nstack.begin());

            if (child->getNameHash() == nameHash)
            {
                result = child;
                nstack.clear();
                break;
            }

            std::vector<Transform*>& children = child->getTransform()->getChildren();

            int j = 0;
            for (auto it = children.begin(); it != children.end(); ++it, ++j)
            {
                Transform* ch = *it;
                nstack.insert(nstack.begin() + j, ch->getGameObject());
            }
        }

        return result;
    }

    int Engine::getGameObjectIndex(GameObject* obj)
    {
        int result = -1;
        int idx = 0;

        std::vector<Transform*> nstack;
        for (auto it = rootTransforms.begin(); it != rootTransforms.end(); ++it)
            nstack.push_back(*it);

        while (nstack.size() > 0)
        {
            Transform* child = *nstack.begin();
            nstack.erase(nstack.begin());

            //
            GameObject* _obj = child->gameObject;
            if (_obj == obj)
            {
                result = idx;
                break;
            }
            //

            ++idx;

            int j = 0;
            for (auto it = child->children.begin(); it != child->children.end(); ++it, ++j)
            {
                Transform* ch = *it;
                nstack.insert(nstack.begin() + j, ch);
            }
        }

        return result;
    }
}