#include "Mesh.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <boost/iostreams/stream.hpp>

#include "../Core/Engine.h"
#include "../Classes/IO.h"
#include "../Classes/Hash.h"
#include "../Renderer/Color.h"
#include "../Renderer/VertexLayouts.h"
#include "../Core/Debug.h"
#include "../Core/APIManager.h"

#include "../Serialization/Assets/SMesh.h"
#include "../Serialization/Data/SColor.h"

#include "../Classes/ZipHelper.h"

namespace GX
{
    std::string Mesh::ASSET_TYPE = "Mesh";

    SubMesh::SubMesh() : Object(APIManager::getSingleton()->submesh_class)
    {
    }

    SubMesh::~SubMesh()
    {
        unload();
    }

    bgfx::IndexBufferHandle& SubMesh::getLodIndexBufferHandle(int idx)
    {
        if (m_ibhLod.size() != lodIndexBuffer.size())
        {
            for (int i = 0; i < m_ibhLod.size(); ++i)
            {
                if (bgfx::isValid(m_ibhLod[i]))
                    bgfx::destroy(m_ibhLod[i]);
            }

            m_ibhLod.resize(lodIndexBuffer.size());

            for (int i = 0; i < m_ibhLod.size(); ++i)
                m_ibhLod[i] = { bgfx::kInvalidHandle };
        }

        if (!bgfx::isValid(m_ibhLod[idx]) && lodIndexBuffer[idx].size() > 0)
        {
            m_ibhLod[idx] = bgfx::createIndexBuffer(
                bgfx::makeRef(lodIndexBuffer[idx].data(), lodIndexBuffer[idx].size() * sizeof(uint32_t)), BGFX_BUFFER_INDEX32
            );
        }

        return m_ibhLod[idx];
    }

    void SubMesh::commit()
    {
        if (bgfx::isValid(m_vbh))
            bgfx::destroy(m_vbh);
        if (bgfx::isValid(m_ibh))
            bgfx::destroy(m_ibh);

        m_vbh = bgfx::createVertexBuffer(
            bgfx::makeRef(vertexBuffer.data(), vertexBuffer.size() * sizeof(VertexBuffer)),
            VertexLayouts::subMeshVertexLayout
        );

        m_ibh = bgfx::createIndexBuffer(
            bgfx::makeRef(indexBuffer.data(), indexBuffer.size() * sizeof(uint32_t)), BGFX_BUFFER_INDEX32
        );
    }

    void SubMesh::load()
    {
        if (!isLoaded)
        {
            if (!bgfx::isValid(m_vbh) && vertexBuffer.size() > 0)
            {
                m_vbh = bgfx::createVertexBuffer(
                    bgfx::makeRef(vertexBuffer.data(), vertexBuffer.size() * sizeof(VertexBuffer)),
                    VertexLayouts::subMeshVertexLayout
                );
            }

            if (!bgfx::isValid(m_ibh) && indexBuffer.size() > 0)
            {
                m_ibh = bgfx::createIndexBuffer(
                    bgfx::makeRef(indexBuffer.data(), indexBuffer.size() * sizeof(uint32_t)), BGFX_BUFFER_INDEX32
                );
            }

            isLoaded = true;
        }
    }

    void SubMesh::unload()
    {
        if (isLoaded)
        {
            if (bgfx::isValid(m_vbh))
                bgfx::destroy(m_vbh);
            
            if (bgfx::isValid(m_ibh))
                bgfx::destroy(m_ibh);
            
            for (int i = 0; i < m_ibhLod.size(); ++i)
            {
                if (bgfx::isValid(m_ibhLod[i]))
                    bgfx::destroy(m_ibhLod[i]);
            }

            m_vbh = { bgfx::kInvalidHandle };
            m_ibh = { bgfx::kInvalidHandle };

            m_ibhLod.clear();

            vertexBuffer.clear();
            indexBuffer.clear();
            lodIndexBuffer.clear();
            
            for (auto it = bones.begin(); it != bones.end(); ++it)
                delete* it;

            bones.clear();

            isLoaded = false;
        }
    }

    BoneInfo* SubMesh::getBone(std::string name)
    {
        auto it = std::find_if(bones.begin(), bones.end(), [=](BoneInfo* bone) -> bool
            {
                return bone->getName() == name;
            }
        );

        if (it != bones.end())
            return *it;

        return nullptr;
    }

    BoneInfo* SubMesh::getBoneByHash(size_t nameHash)
    {
        auto it = std::find_if(bones.begin(), bones.end(), [=](BoneInfo* bone) -> bool
            {
                return bone->getNameHash() == nameHash;
            }
        );

        if (it != bones.end())
            return *it;

        return nullptr;
    }

    void BoneInfo::setName(std::string value)
    {
        name = value;
        nameHash = Hash::getHash(name);
    }

    Mesh::Mesh() : Asset(APIManager::getSingleton()->mesh_class)
    {
    }

    Mesh::~Mesh()
    {
        unload();
    }

    void Mesh::load()
    {
        if (!isLoaded())
        {
            Asset::load();

            for (auto it = subMeshes.begin(); it != subMeshes.end(); ++it)
            {
                SubMesh* subMesh = *it;
                subMesh->load();
            }

            recalculateBounds();
        }
    }

    void Mesh::unload()
    {
        if (isLoaded())
        {
            Asset::unload();

            if (persistent)
                return;

            for (auto it = subMeshes.begin(); it != subMeshes.end(); ++it)
                delete (*it);

            subMeshes.clear();
        }
    }

    void Mesh::unloadAll()
    {
        std::vector<Mesh*> deleteAssets;

        for (auto it = loadedInstances.begin(); it != loadedInstances.end(); ++it)
        {
            if (it->second->getAssetType() == Mesh::ASSET_TYPE)
            {
                if (!it->second->getPersistent())
                    deleteAssets.push_back((Mesh*)it->second);
            }
        }

        for (auto it = deleteAssets.begin(); it != deleteAssets.end(); ++it)
            delete (*it);

        deleteAssets.clear();
    }

    void Mesh::reload()
    {
        if (getOrigin().empty())
            return;

        if (isLoaded())
            unload();

        load(location, name);
    }

    Mesh* Mesh::create(std::string location, std::string name)
    {
        Asset* cachedAsset = getLoadedInstance(location, name);
        assert(cachedAsset == nullptr && "Asset with these location and name already exists");

        Mesh* mesh = new Mesh();
        mesh->setLocation(location);
        mesh->setName(name);

        return mesh;
    }

    void Mesh::save()
    {
        save(getOrigin());
    }

    void Mesh::save(std::string path)
    {
        SMesh mesh;
        mesh.alias = alias;
        mesh.sourceFile = sourceFile;

        for (auto it = subMeshes.begin(); it != subMeshes.end(); ++it)
        {
            SubMesh * subMesh = *it;
            SSubMesh sSubMesh;

            sSubMesh.materialName = subMesh->materialName;
            sSubMesh.useLightmapUVs = subMesh->useLightmapUVs;
            
            for (auto v = subMesh->vertexBuffer.begin(); v != subMesh->vertexBuffer.end(); ++v)
            {
                SVertexBuffer svBuf;
                svBuf.position = v->position;
                svBuf.normal = v->normal;
                svBuf.tangent = v->tangent;
                svBuf.bitangent = v->bitangent;
                svBuf.texcoord0 = v->texcoord0;
                svBuf.texcoord1 = v->texcoord1;
                svBuf.blendWeights = v->blendWeights;
                svBuf.blendIndices = v->blendIndices;
                svBuf.color = SColor(Color::unpackABGR(v->color));
                
                sSubMesh.vertexBuffer.push_back(svBuf);
            }

            sSubMesh.indexBuffer = subMesh->indexBuffer;

            for (auto& lod : subMesh->lodIndexBuffer)
            {
                SLodInfo lodInfo;
                lodInfo.indexBuffer = lod;

                sSubMesh.lods.push_back(lodInfo);
            }

            for (auto it = subMesh->bones.begin(); it != subMesh->bones.end(); ++it)
            {
                BoneInfo* bone = *it;
                SBoneInfo sBone;

                sBone.name = bone->getName();
                sBone.offsetMatrix = SMatrix4(bone->getOffsetMatrix());

                sSubMesh.bones.push_back(sBone);
            }

            mesh.subMeshes.push_back(sSubMesh);
        }

        std::ofstream ofs(path, std::ios::binary);
        BinarySerializer s;
        s.serialize(&ofs, &mesh, Mesh::ASSET_TYPE);
        ofs.close();

        mesh.subMeshes.clear();
    }

    void Mesh::commit()
    {
        for (auto& it : subMeshes)
            it->commit();

        recalculateBounds();
    }

    void Mesh::addSubMesh(SubMesh* subMesh)
    {
        subMeshes.push_back(subMesh);

        subMesh->parent = this;
    }

    void Mesh::removeSubMesh(int index)
    {
        if (index < subMeshes.size())
        {
            SubMesh* subMesh = subMeshes[index];
            subMeshes.erase(subMeshes.begin() + index);

            subMesh->parent = nullptr;
        }
    }

    void Mesh::removeSubMesh(SubMesh* subMesh)
    {
        auto it = std::find(subMeshes.begin(), subMeshes.end(), subMesh);

        if (it != subMeshes.end())
        {
            subMeshes.erase(it);

            subMesh->parent = nullptr;
        }
    }

    Mesh* Mesh::load(std::string location, std::string name)
    {
        std::string fullPath = location + name;

        Asset* cachedAsset = getLoadedInstance(location, name);

        if (cachedAsset != nullptr && cachedAsset->isLoaded())
        {
            return (Mesh*)cachedAsset;
        }
        else
        {
            if (IO::isDir(location))
            {
                if (!IO::FileExists(fullPath))
                {
                    Debug::log("[" + fullPath + "] Error loading mesh: file does not exists", Debug::DbgColorRed);
                    return nullptr;
                }
            }
            else
            {
                zip_t* arch = Engine::getSingleton()->getZipArchive(location);
                if (!ZipHelper::isFileInZip(arch, name))
                {
                    Debug::log("[" + fullPath + "] Error loading mesh: file does not exists", Debug::DbgColorRed);
                    return nullptr;
                }
            }

            Mesh* mesh = nullptr;
            if (cachedAsset == nullptr)
            {
                mesh = new Mesh();
                mesh->setLocation(location);
                mesh->setName(name);
            }
            else
                mesh = (Mesh*)cachedAsset;

            SMesh sMesh;

            if (IO::isDir(location))
            {
                try
                {
                    std::ifstream ofs(fullPath, std::ios::binary);
                    BinarySerializer s;
                    s.deserialize(&ofs, &sMesh, Mesh::ASSET_TYPE);
                    ofs.close();
                }
                catch (std::exception e)
                {
                    Debug::log("[" + name + "] Error loading model cache: " + e.what(), Debug::DbgColorRed);
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
                    s.deserialize(&is, &sMesh, Mesh::ASSET_TYPE);
                    is.close();
                }
                catch (std::exception e)
                {
                    Debug::log("[" + name + "] Error loading model cache: " + e.what(), Debug::DbgColorRed);
                }

                delete[] buffer;
            }

            mesh->setAlias(sMesh.alias);
            mesh->setSourceFile(sMesh.sourceFile);

            for (auto it = sMesh.subMeshes.begin(); it != sMesh.subMeshes.end(); ++it)
            {
                SSubMesh& sSubMesh = *it;
                SubMesh* subMesh = new SubMesh();

                subMesh->materialName = sSubMesh.materialName;
                subMesh->useLightmapUVs = sSubMesh.useLightmapUVs;

                subMesh->vertexBuffer.resize(sSubMesh.vertexBuffer.size());

                int i = 0;
                for (auto it = sSubMesh.vertexBuffer.begin(); it != sSubMesh.vertexBuffer.end(); ++it, ++i)
                {
                    VertexBuffer vbuf;
                    vbuf.position = it->position.getValue();
                    vbuf.normal = it->normal.getValue();
                    vbuf.tangent = it->tangent.getValue();
                    vbuf.bitangent = it->bitangent.getValue();
                    vbuf.texcoord0 = it->texcoord0.getValue();
                    vbuf.texcoord1 = it->texcoord1.getValue();
                    vbuf.blendWeights = it->blendWeights.getValue();
                    vbuf.blendIndices = it->blendIndices.getValue();
                    vbuf.color = Color::packABGR(it->color.getValue());

                    subMesh->vertexBuffer[i] = vbuf;
                }

                subMesh->indexBuffer = sSubMesh.indexBuffer;

                for (auto& lodInfo : sSubMesh.lods)
                    subMesh->lodIndexBuffer.push_back(lodInfo.indexBuffer);

                for (auto it = sSubMesh.bones.begin(); it != sSubMesh.bones.end(); ++it)
                {
                    SBoneInfo& sBone = *it;

                    BoneInfo* bone = new BoneInfo();
                    bone->setName(sBone.name);
                    bone->setOffsetMatrix(sBone.offsetMatrix.getValue());

                    subMesh->addBone(bone);
                }

                mesh->subMeshes.push_back(subMesh);
            }

            mesh->load();

            sMesh.subMeshes.clear();

            return mesh;
        }
    }

    void Mesh::recalculateBounds()
    {
        for (auto it = subMeshes.begin(); it != subMeshes.end(); ++it)
        {
            SubMesh* subMesh = *it;

            for (auto it2 = subMesh->vertexBuffer.begin(); it2 != subMesh->vertexBuffer.end(); ++it2)
            {
                VertexBuffer & buf = *it2;
                boundingBox.merge(buf.position);
            }
        }

        if (boundingBox == AxisAlignedBox::BOX_INFINITE || boundingBox == AxisAlignedBox::BOX_NULL)
            boundingBox.setExtents(glm::vec3(-1.0f), glm::vec3(1.0f));
    }
}