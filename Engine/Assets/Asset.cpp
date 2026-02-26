#include "Asset.h"

#include <cassert>
#include <vector>

#include "../Core/APIManager.h"

namespace GX
{
    std::map<std::string, Asset*> Asset::loadedInstances;

    Asset::Asset(MonoClass* monoClass)
    {
        if (monoClass != nullptr)
        {
            managedClass = monoClass;
            createManagedObject();
        }
    }

    Asset::~Asset()
    {
        destroyManagedObject();

        persistent = false;
        unload();
        setLoadedInstance(location, alias.empty() ? name : alias, nullptr);
    }

    void Asset::createManagedObject()
    {
        if (!APIManager::getSingleton()->isInitialized())
            return;

        if (managedClass == nullptr)
            return;

        managedObject = mono_object_new(APIManager::getSingleton()->getDomain(), managedClass);
    
        //mono_runtime_object_init(managedObject);
        //createManagedObject(managedObject);
        managedGCHandle = mono_gchandle_new(managedObject, true); // Prevent this object to be destroyed by garbage collector

        Asset* _this = this;
        mono_field_set_value(managedObject, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_this));
    }

    void Asset::destroyManagedObject()
    {
        if (!APIManager::getSingleton()->isInitialized())
            return;

        if (managedObject == nullptr)
            return;

        mono_field_set_value(managedObject, APIManager::getSingleton()->asset_ptr_field, nullptr);

        if (managedGCHandle > 0)
        {
            mono_gchandle_free(managedGCHandle);
            managedGCHandle = 0;
        }

        managedObject = nullptr;
    }

    void Asset::setManagedObject(MonoObject* value)
    {
        destroyManagedObject();

        managedObject = value;
        managedGCHandle = mono_gchandle_new(managedObject, true); // Prevent this object to be destroyed by garbage collector

        Asset* _this = this;
        mono_field_set_value(managedObject, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_this));
    }

    void Asset::load()
    {
        if (!isLoaded())
        {
            assert(!name.empty() && "Name is not specified");
            assert(!location.empty() && "Location is not specified");

            loaded = true;
            setLoadedInstance(location, alias.empty() ? name : alias, this);
        }
    }

    void Asset::unload()
    {
        if (persistent)
            return;

        if (isLoaded())
        {
            loaded = false;
        }
    }

    void Asset::reload()
    {
        
    }

    Asset* Asset::getLoadedInstance(std::string location, std::string name)
    {
        if (loadedInstances.find(location + name) != loadedInstances.end())
            return loadedInstances[location + name];
        else return nullptr;
    }

    void Asset::setLoadedInstance(std::string location, std::string name, Asset* instance)
    {
        if (instance == nullptr)
        {
            //assert(loadedInstances.find(location + name) != loadedInstances.end() && "No asset loaded by this path");
            if (loadedInstances.find(location + name) != loadedInstances.end())
                loadedInstances.erase(location + name);
        }
        else
        {
            auto it = loadedInstances.find(location + name);
            if (it != loadedInstances.end())
            {
                if (it->second != instance)
                    assert(!"Asset is already loaded by this path");
            }
            loadedInstances[location + name] = instance;
        }
    }

    void Asset::unloadAll()
    {
        std::vector<Asset*> deleteAssets;

        for (auto it = loadedInstances.begin(); it != loadedInstances.end(); ++it)
            deleteAssets.push_back(it->second);

        for (auto it = deleteAssets.begin(); it != deleteAssets.end(); ++it)
            delete* it;

        deleteAssets.clear();
        loadedInstances.clear();
    }
}