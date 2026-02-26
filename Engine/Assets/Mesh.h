#pragma once

#include <vector>
#include <string>
#include <bgfx/bgfx.h>

#include "Asset.h"

#include "../glm/glm.hpp"
#include "../Core/Object.h"
#include "../Math/AxisAlignedBox.h"
#include "../Renderer/VertexBuffer.h"

namespace GX
{
    class Mesh;

    class BoneInfo
    {
    private:
        std::string name = "";
        size_t nameHash = 0;
        glm::mat4x4 offsetMatrix = glm::identity<glm::mat4x4>();

    public:
        BoneInfo() = default;
        ~BoneInfo() = default;

        std::string getName() { return name; }
        size_t getNameHash() { return nameHash; }
        void setName(std::string value);

        glm::mat4x4 getOffsetMatrix() { return offsetMatrix; }
        void setOffsetMatrix(glm::mat4x4 value) { offsetMatrix = value; }
    };

    class SubMesh : public Object
    {
        friend class Mesh;

    private:
        std::vector<VertexBuffer> vertexBuffer;
        std::vector<uint32_t> indexBuffer;
        std::vector<std::vector<uint32_t>> lodIndexBuffer;

        bgfx::VertexBufferHandle m_vbh = { bgfx::kInvalidHandle };
        bgfx::IndexBufferHandle m_ibh = { bgfx::kInvalidHandle };

        std::vector<bgfx::IndexBufferHandle> m_ibhLod;

        Mesh* parent = nullptr;

        std::vector<BoneInfo*> bones;

        bool isLoaded = false;
        std::string materialName = "";

        bool useLightmapUVs = false;

    public:
        SubMesh();
        virtual ~SubMesh();

        bgfx::VertexBufferHandle& getVertexBufferHandle() { return m_vbh; }
        bgfx::IndexBufferHandle& getIndexBufferHandle() { return m_ibh; }
        bgfx::IndexBufferHandle& getLodIndexBufferHandle(int idx);

        std::vector<VertexBuffer>& getVertexBuffer() { return vertexBuffer; }
        std::vector<uint32_t>& getIndexBuffer() { return indexBuffer; }
        std::vector<uint32_t>& getLodIndexBuffer(int idx) { return lodIndexBuffer[idx]; }

        int getLodLevelsCount() { return lodIndexBuffer.size(); }
        void setLodLevelsCount(int value) { lodIndexBuffer.resize(value); }

        void load();
        void unload();
        void commit();

        Mesh* getParent() { return parent; }

        std::string getMaterialName() { return materialName; }
        void setMaterialName(std::string value) { materialName = value; }

        void addBone(BoneInfo* bone) { bones.push_back(bone); }
        void removeBone(int index) { bones.erase(bones.begin() + index); }
        int getBoneCount() { return (int)bones.size(); }
        BoneInfo* getBone(int index) { return bones.at(index); }
        BoneInfo* getBone(std::string name);
        BoneInfo* getBoneByHash(size_t nameHash);

        bool getUseLightmapUVs() { return useLightmapUVs; }
        void setUseLightmapUVs(bool value) { useLightmapUVs = value; }
    };

    class Mesh : public Asset
    {
    private:
        std::vector<SubMesh*> subMeshes;    
        std::string alias = "";
        std::string sourceFile = "";
        std::string userData = "";

        AxisAlignedBox boundingBox = AxisAlignedBox::BOX_NULL;

    public:
        Mesh();
        virtual ~Mesh();

        static std::string ASSET_TYPE;

        virtual void load();
        virtual void unload();
        static void unloadAll();
        virtual void reload();
        virtual std::string getAssetType() { return ASSET_TYPE; }
        static Mesh* create(std::string location, std::string name);
        static Mesh* load(std::string location, std::string name);
        void save();
        void save(std::string path);
        void commit();

        void addSubMesh(SubMesh* subMesh);
        void removeSubMesh(int index);
        void removeSubMesh(SubMesh* subMesh);
        int getSubMeshCount() { return (int)subMeshes.size(); }
        SubMesh* getSubMesh(int index) { return subMeshes.at(index); }

        void recalculateBounds();
        AxisAlignedBox getBounds() { return boundingBox; }

        std::string getAlias() { return alias; }
        void setAlias(std::string value) { alias = value; }

        std::string getUserData() { return userData; }
        void setUserData(std::string value) { userData = value; }

        std::string getSourceFile() { return sourceFile; }
        void setSourceFile(std::string value) { sourceFile = value; }
    };
}