#include "Terrain.h"

#include "../glm/gtc/type_ptr.hpp"

#include <boost/iostreams/stream.hpp>

#include "Light.h"
#include "Transform.h"
#include "../Core/Engine.h"
#include "../Core/Debug.h"
#include "../Core/GameObject.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/VertexLayouts.h"
#include "../Renderer/Frustum.h"
#include "../Assets/Material.h"
#include "../Assets/Shader.h"
#include "../Assets/Texture.h"
#include "../Assets/Mesh.h"
#include "../Assets/Prefab.h"
#include "../Assets/Model3DLoader.h"
#include "../Assets/Cubemap.h"
#include "../Math/Mathf.h"
#include "../Math/Raycast.h"
#include "../Math/Plane.h"
#include "../Classes/IO.h"
#include "../Classes/GUIDGenerator.h"
#include "../Serialization/Scene/SScene.h"
#include "Camera.h"

#include "../Classes/ZipHelper.h"

namespace GX
{
    std::string Terrain::COMPONENT_TYPE = "Terrain";

    bgfx::UniformHandle Terrain::textureDiffuseHandles[MAX_TERRAIN_TEXTURES];
    bgfx::UniformHandle Terrain::textureNormalHandles[MAX_TERRAIN_TEXTURES];
    bgfx::UniformHandle Terrain::textureSplatHandles[MAX_TERRAIN_TEXTURES];
    bgfx::UniformHandle Terrain::textureSizesHandle = { bgfx::kInvalidHandle };
    bgfx::UniformHandle Terrain::textureCountHandle = { bgfx::kInvalidHandle };
    bgfx::UniformHandle Terrain::albedoTextureHandle = { bgfx::kInvalidHandle };

    //----------------------TREES DATA----------------------//

    TerrainTreeData::~TerrainTreeData()
    {
        destroy();
        meshList.clear();
        destroyImpostorTexture();
    }

    void TerrainTreeData::updateBounds()
    {
        bounds = AxisAlignedBox::BOX_NULL;
        for (auto it = meshList.begin(); it != meshList.end(); ++it)
        {
            ModelMeshData& data = *it;
            AxisAlignedBox aab1 = data.mesh->getBounds();
            aab1.transform(data.transform);
            bounds.merge(aab1);
        }
    }

    void TerrainTreeData::setName(std::string value)
    {
        name = value;

        meshList.clear();
        if (IO::GetFileExtension(value) != "prefab")
        {
            auto _meshes = Model3DLoader::load3DModelMeshes(Engine::getSingleton()->getAssetsPath(), name);
            for (auto it = _meshes.begin(); it != _meshes.end(); ++it)
            {
                ModelMeshData data;
                data.mesh = it->mesh;
                data.materials = it->materials;
                data.transform = it->transform;

                meshList.push_back(data);
            }
            _meshes.clear();
        }
        else
        {
            auto _meshes = Model3DLoader::load3DModelMeshesFromPrefab(Engine::getSingleton()->getAssetsPath(), name);
            for (auto it = _meshes.begin(); it != _meshes.end(); ++it)
            {
                ModelMeshData data;
                data.mesh = it->mesh;
                data.materials = it->materials;
                data.transform = it->transform;

                meshList.push_back(data);
            }
            _meshes.clear();
        }

        for (auto it = meshes.begin(); it != meshes.end(); ++it)
            (*it)->updateTransform();

        updateBounds();
        updateImpostorTexture();
    }

    void TerrainTreeData::setMinScale(float value)
    {
        minScale = value;

        for (auto m = meshes.begin(); m != meshes.end(); ++m)
        {
            (*m)->scale = Mathf::RandomFloat(minScale, maxScale);
            (*m)->updateTransform();
        }
    }

    void TerrainTreeData::setMaxScale(float value)
    {
        maxScale = value;

        for (auto m = meshes.begin(); m != meshes.end(); ++m)
        {
            (*m)->scale = Mathf::RandomFloat(minScale, maxScale);
            (*m)->updateTransform();
        }
    }

    void TerrainTreeData::destroyImpostorTexture()
    {
        if (bgfx::isValid(impostorTexture))
            bgfx::destroy(impostorTexture);

        impostorTexture = { bgfx::kInvalidHandle };
    }

    bool TerrainTreeData::addTree(glm::vec2 pos, float density)
    {
        glm::vec2 _wpos = glm::vec2(-pos.x + terrain->getSize() / 2.0f, pos.y - terrain->getSize() / 2.0f);

        float offset = 1.0f * terrain->getScale() + 2.0f;

        if (_wpos.x < (-terrain->getWorldSize() + offset) / 2.0f) return false;
        if (_wpos.y < -terrain->getWorldSize() / 2.0f) return false;
        if (_wpos.x > terrain->getWorldSize() / 2.0f) return false;
        if (_wpos.y > (terrain->getWorldSize() - offset) / 2.0f) return false;

        bool placeFree = true;
        for (auto it = terrain->treeList.begin(); it != terrain->treeList.end(); ++it)
        {
            auto bb = std::find_if((*it)->meshes.begin(), (*it)->meshes.end(), [=](TreeMesh* g) -> bool
                {
                    return std::abs(g->position.x - _wpos.x) < 1.0f / density && std::abs(g->position.z - _wpos.y) < 1.0f / density;
                }
            );

            if (bb != (*it)->meshes.end())
            {
                placeFree = false;
                break;
            }
        }

        if (placeFree)
        {
            Transform* terrainTrans = terrain->getGameObject()->getTransform();
            float y = terrain->getHeightAtWorldPos(glm::vec3(_wpos.x, 0, _wpos.y) + terrainTrans->getPosition());

            TreeMesh* mesh = new TreeMesh(this);
            mesh->position = glm::vec3(_wpos.x, y, _wpos.y);
            mesh->rotation = Mathf::RandomFloat(0, 180.0f);
            mesh->scale = Mathf::RandomFloat(minScale, maxScale);

            meshes.push_back(mesh);

            mesh->updateTransform();
        }

        return placeFree;
    }

    void TerrainTreeData::addTree(glm::vec3 position, float rotation, float scale)
    {
        TreeMesh* mesh = new TreeMesh(this);
        mesh->position = position;
        mesh->rotation = rotation;
        mesh->scale = scale;

        meshes.push_back(mesh);

        mesh->updateTransform();
    }

    bool TerrainTreeData::removeTree(glm::vec2 pos, float density)
    {
        if (meshes.size() == 0)
            return false;

        glm::vec2 _wpos = glm::vec2(-pos.x + terrain->getSize() / 2.0f, pos.y - terrain->getSize() / 2.0f);

        auto bb = std::find_if(meshes.begin(), meshes.end(), [=](TreeMesh* g) -> bool
            {
                return std::abs(g->position.x - _wpos.x) < 1.0f * density && std::abs(g->position.z - _wpos.y) < 1.0f * density;
            }
        );

        if (bb != meshes.end())
        {
            delete* bb;
            meshes.erase(bb);

            return true;
        }

        return false;
    }

    void TerrainTreeData::destroy()
    {
        //Destroy created meshes
        for (auto it = meshes.begin(); it != meshes.end(); ++it)
            delete* it;

        meshes.clear();
    }

    void TerrainTreeData::updateImpostorTexture()
    {
        int textureSize = 128;

        destroyImpostorTexture();

        if (meshList.size() == 0)
            return;

        bgfx::FrameBufferHandle frameBufferHandle = { bgfx::kInvalidHandle };
        bgfx::TextureHandle depthTextureHandle = { bgfx::kInvalidHandle };

        uint64_t flags = BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_ANISOTROPIC
            | BGFX_SAMPLER_MAG_ANISOTROPIC
            /*| BGFX_SAMPLER_MIP_POINT*/
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP;

        impostorTexture = bgfx::createTexture2D(uint16_t(textureSize), uint16_t(textureSize), false, 1, bgfx::TextureFormat::BGRA8, flags);
        depthTextureHandle = bgfx::createTexture2D(uint16_t(textureSize), uint16_t(textureSize), false, 1, bgfx::TextureFormat::D24S8, flags);

        bgfx::Attachment at[2];
        at[0].init(impostorTexture);
        at[1].init(depthTextureHandle);
        frameBufferHandle = bgfx::createFrameBuffer(2, at, false);

        int view = Renderer::getSingleton()->getNumViewsUsed() + 1;

        AxisAlignedBox aab = getBounds();

        glm::vec3 aabSize = aab.getSize();
        float maxSide = std::max(aabSize.x, aabSize.y);

        glm::mat4x4 viewTrans = glm::orthoRH(maxSide / 2.0f, -maxSide / 2.0f, 0.0f, maxSide, -100.0f, 100.0f);

        bgfx::setViewClear(view, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0);
        bgfx::setViewRect(view, 0, 0, textureSize, textureSize);
        bgfx::setViewFrameBuffer(view, frameBufferHandle);
        bgfx::setViewTransform(view, nullptr, glm::value_ptr(viewTrans));

        for (auto it = meshList.begin(); it != meshList.end(); ++it)
        {
            ModelMeshData& data = *it;
            
            for (int i = 0; i < data.mesh->getSubMeshCount(); ++i)
            {
                SubMesh* subMesh = data.mesh->getSubMesh(i);
                Material* material = nullptr;
                if (i < data.materials.size())
                    material = data.materials[i];

                // Set model matrix for rendering.
                bgfx::setTransform(glm::value_ptr(data.transform));

                // Set vertex and index buffer.
                bgfx::setVertexBuffer(0, subMesh->getVertexBufferHandle());
                bgfx::setIndexBuffer(subMesh->getIndexBufferHandle());

                // Set render states.
                bgfx::setState(0
                    | BGFX_STATE_WRITE_RGB
                    | BGFX_STATE_WRITE_A
                    | BGFX_STATE_WRITE_Z
                    | BGFX_STATE_DEPTH_TEST_LEQUAL
                    /*| BGFX_STATE_CULL_CW*/);

                bgfx::setTexture(0, Terrain::albedoTextureHandle, Texture::getNullTexture()->getHandle());
                bgfx::setUniform(Renderer::getSingleton()->getAmbientColorUniform(), glm::value_ptr(glm::vec4(1, 1, 1, 1)), 1);
                
                if (material != nullptr && material->isLoaded())
                {
                    std::vector<Uniform>& uniforms = material->getUniforms();
                    for (auto u = uniforms.begin(); u != uniforms.end(); ++u)
                    {
                        if (u->getType() == UniformType::Sampler2D)
                        {
                            auto smp = u->getValue<Sampler2DDef>();
                            Texture* tex = smp.second;
                            if (tex != nullptr)
                            {
                                bgfx::setTexture(0, Terrain::albedoTextureHandle, tex->getHandle());
                                break;
                            }
                        }
                    }
                    for (auto u = uniforms.begin(); u != uniforms.end(); ++u)
                    {
                        if (u->getType() == UniformType::Color)
                        {
                            bgfx::setUniform(Renderer::getSingleton()->getColorUniform(), u->getValue<Color>().ptr(), 1);
                            break;
                        }
                    }
                }

                // Render
                bgfx::ProgramHandle ph = Renderer::getSingleton()->getTerrainTreeBillboardProgram();

                bgfx::submit(view, ph);
            }
        }

        Renderer::getSingleton()->frame();
        bgfx::destroy(frameBufferHandle);
        bgfx::destroy(depthTextureHandle);
        Renderer::getSingleton()->frame();
    }

    glm::mat4x4 TerrainTreeData::TreeMesh::getTransform()
    {
        return cachedTransform;
    }

    void TerrainTreeData::TreeMesh::updateTransform()
    {
        //Update transform
        Transform* terrTrans = parent->terrain->getGameObject()->getTransform();
        glm::mat4x4 trans = terrTrans->getTransformMatrix();

        glm::mat4x4 rotMat = glm::mat4_cast(Mathf::toQuaternion(glm::vec3(0, rotation, 0)));
        glm::vec3 _pos = glm::inverse(rotMat) * (glm::vec4(position, 1.0f));

        glm::mat4x4 transform = glm::translate(glm::identity<glm::mat4x4>(), _pos);
        transform = rotMat * transform;
        transform = glm::scale(transform, glm::vec3(scale));

        cachedTransform = terrTrans->getTransformMatrix() * transform;

        //Update AAB
        cachedAAB = parent->getBounds();
        cachedAAB.transform(cachedTransform);
    }

    TerrainTreeData::TreeMesh::TreeMesh(TerrainTreeData* _parent) : Renderable()
    {
        parent = _parent;
    }

    TerrainTreeData::TreeMesh::~TreeMesh()
    {
        
    }

    AxisAlignedBox TerrainTreeData::TreeMesh::getBounds(bool world)
    {
        if (world)
        {
            return cachedAAB;
        }
        else
        {
            return parent->getBounds();
        }
    }

    bool TerrainTreeData::TreeMesh::getCastShadows()
    {
        return parent->terrain->getTreesCastShadows();
    }

    bool TerrainTreeData::TreeMesh::checkCullingMask(LayerMask& mask)
    {
        return parent->terrain->checkCullingMask(mask);
    }

    //----------------------DETAIL MESHES DATA----------------------//

    TerrainDetailMeshData::Batch::Batch(TerrainDetailMeshData* _parent) : Renderable()
    {
        parent = _parent;
    }

    TerrainDetailMeshData::Batch::~Batch()
    {
        meshes.clear();
        destroy();
    }

    void TerrainDetailMeshData::Batch::releaseMemory(void* _ptr, void* _userData)
    {
        delete[] _ptr;
    }

    void TerrainDetailMeshData::Batch::init()
    {
        subBatches.resize(parent->meshList.size());

        for (int k = 0; k < parent->meshList.size(); ++k)
        {
            ModelMeshData& meshData = parent->meshList[k];
            subBatches[k].resize(meshData.mesh->getSubMeshCount());

            prevTransform = glm::mat4x4(FLT_MAX);
            bounds = AxisAlignedBox::BOX_NULL;

            for (int i = 0; i < meshData.mesh->getSubMeshCount(); ++i)
            {
                SubMesh* subMesh = meshData.mesh->getSubMesh(i);
                SubBatch& subBatch = subBatches[k][i];

                subBatch.vertexCount = (uint32_t)subMesh->getVertexBuffer().size() * (uint32_t)meshes.size();
                subBatch.indexCount = (uint32_t)subMesh->getIndexBuffer().size() * (uint32_t)meshes.size();
                subBatch.vertices = new SubBatch::VertexBuffer[subBatch.vertexCount];
                subBatch.indices = new uint32_t[subBatch.indexCount];
            }

            for (int i = 0; i < meshData.mesh->getSubMeshCount(); ++i)
            {
                SubMesh* subMesh = meshData.mesh->getSubMesh(i);
                SubBatch& subBatch = subBatches[k][i];

                uint32_t vert = 0;
                uint32_t index = 0;
                uint32_t indexStride = 0;

                for (int j = 0; j < meshes.size(); ++j)
                {
                    DetailMesh& meshInf = meshes[j];

                    glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
                    glm::highp_quat rmtx = Mathf::toQuaternion(glm::vec3(0, meshInf.rotation, 0));
                    mtx = glm::translate(mtx, glm::inverse(rmtx) * meshInf.position);
                    mtx = glm::scale(mtx, glm::vec3(meshInf.scale));
                    mtx = glm::mat4_cast(rmtx) * mtx;

                    std::vector<VertexBuffer>& vbuf = subMesh->getVertexBuffer();
                    for (auto v = vbuf.begin(); v != vbuf.end(); ++v)
                    {
                        subBatch.vertices[vert].position = glm::vec3(mtx * glm::vec4(v->position, 1.0));
                        subBatch.vertices[vert].normal = glm::vec3(rmtx * glm::vec4(v->normal, 1.0));
                        subBatch.vertices[vert].tangent = glm::vec3(rmtx * glm::vec4(v->tangent, 1.0));
                        subBatch.vertices[vert].bitangent = glm::vec3(rmtx * glm::vec4(v->bitangent, 1.0));
                        subBatch.vertices[vert].texcoord0 = v->texcoord0;
                        subBatch.vertices[vert].texcoord1 = v->texcoord1;

                        bounds.merge(subBatch.vertices[vert].position);

                        ++vert;
                    }

                    std::vector<uint32_t>& ibuf = subMesh->getIndexBuffer();
                    for (int id = 0; id < ibuf.size(); ++id)
                    {
                        subBatch.indices[index] = ibuf[id] + indexStride;
                        ++index;
                    }

                    indexStride += vbuf.size();
                }

                if (!bgfx::isValid(subBatch.vbh))
                {
                    const bgfx::Memory* mem = bgfx::makeRef(subBatch.vertices, sizeof(SubBatch::VertexBuffer) * subBatch.vertexCount, releaseMemory);
                    subBatch.vbh = bgfx::createVertexBuffer(mem, VertexLayouts::terrainVertexLayout);
                }

                if (!bgfx::isValid(subBatch.ibh))
                {
                    const bgfx::Memory* mem = bgfx::makeRef(subBatch.indices, sizeof(uint32_t) * subBatch.indexCount, releaseMemory);
                    subBatch.ibh = bgfx::createIndexBuffer(mem, BGFX_BUFFER_INDEX32);
                }
            }
        }
    }

    void TerrainDetailMeshData::Batch::destroy()
    {
        for (auto it = subBatches.begin(); it != subBatches.end(); ++it)
        {
            for (auto kt = it->begin(); kt != it->end(); ++kt)
            {
                if (bgfx::isValid(kt->vbh))
                    bgfx::destroy(kt->vbh);
                if (bgfx::isValid(kt->ibh))
                    bgfx::destroy(kt->ibh);
            }

            it->clear();
        }

        subBatches.clear();
    }

    void TerrainDetailMeshData::Batch::update()
    {
        destroy();
        init();
    }

    AxisAlignedBox TerrainDetailMeshData::Batch::getBounds(bool world)
    {
        if (world)
        {
            Transform* terrTrans = parent->terrain->getGameObject()->getTransform();
            glm::mat4x4 mtx = terrTrans->getTransformMatrix();

            if (mtx != prevTransform)
            {
                cachedAAB = bounds;
                cachedAAB.transform(mtx);

                prevTransform = mtx;

                return cachedAAB;
            }
            else
                return cachedAAB;
        }
        else
        {
            return bounds;
        }
    }

    bool TerrainDetailMeshData::Batch::getCastShadows()
    {
        return parent->terrain->getDetailMeshesCastShadows();
    }

    bool TerrainDetailMeshData::Batch::checkCullingMask(LayerMask& mask)
    {
        return parent->terrain->checkCullingMask(mask);
    }

    TerrainDetailMeshData::~TerrainDetailMeshData()
    {
        destroy();
        meshList.clear();
        destroyImpostorTexture();
    }

    void TerrainDetailMeshData::updateBounds()
    {
        bounds = AxisAlignedBox::BOX_NULL;
        for (auto it = meshList.begin(); it != meshList.end(); ++it)
        {
            ModelMeshData& data = *it;
            AxisAlignedBox aab1 = data.mesh->getBounds();
            aab1.transform(data.transform);
            bounds.merge(aab1);
        }
    }

    void TerrainDetailMeshData::setName(std::string value)
    {
        name = value;

        meshList.clear();
        if (IO::GetFileExtension(value) != "prefab")
        {
            auto _meshes = Model3DLoader::load3DModelMeshes(Engine::getSingleton()->getAssetsPath(), name);
            for (auto it = _meshes.begin(); it != _meshes.end(); ++it)
            {
                ModelMeshData data;
                data.mesh = it->mesh;
                data.materials = it->materials;
                data.transform = it->transform;

                meshList.push_back(data);
            }
            _meshes.clear();
        }
        else
        {
            auto _meshes = Model3DLoader::load3DModelMeshesFromPrefab(Engine::getSingleton()->getAssetsPath(), name);
            for (auto it = _meshes.begin(); it != _meshes.end(); ++it)
            {
                ModelMeshData data;
                data.mesh = it->mesh;
                data.materials = it->materials;
                data.transform = it->transform;

                meshList.push_back(data);
            }
            _meshes.clear();
        }

        updateBounds();
        updateImpostorTexture();

        for (auto it = batches.begin(); it != batches.end(); ++it)
            (*it)->update();
    }

    void TerrainDetailMeshData::setMinScale(float value)
    {
        minScale = value;

        for (auto m = batches.begin(); m != batches.end(); ++m)
        {
            Batch* batch = *m;
            for (auto m = batch->meshes.begin(); m != batch->meshes.end(); ++m)
            {
                DetailMesh& mesh = *m;
                mesh.scale = Mathf::RandomFloat(minScale, maxScale);
            }

            (*m)->update();
        }
    }

    void TerrainDetailMeshData::setMaxScale(float value)
    {
        maxScale = value;

        for (auto m = batches.begin(); m != batches.end(); ++m)
        {
            Batch* batch = *m;
            for (auto m = batch->meshes.begin(); m != batch->meshes.end(); ++m)
            {
                DetailMesh& mesh = *m;
                mesh.scale = Mathf::RandomFloat(minScale, maxScale);
            }

            (*m)->update();
        }
    }

    void TerrainDetailMeshData::destroyImpostorTexture()
    {
        if (bgfx::isValid(impostorTexture))
            bgfx::destroy(impostorTexture);

        impostorTexture = { bgfx::kInvalidHandle };
    }

    bool TerrainDetailMeshData::addMesh(std::vector<glm::vec2> positions, float density)
    {
        std::vector<Batch*> updBatches;

        Transform* terrainTrans = terrain->getGameObject()->getTransform();

        for (auto pt = positions.begin(); pt != positions.end(); ++pt)
        {
            glm::vec2 pos = *pt;
            glm::vec2 _wpos = glm::vec2(-pos.x + terrain->getSize() / 2.0f, pos.y - terrain->getSize() / 2.0f);
            float y = terrain->getHeightAtWorldPos(glm::vec3(_wpos.x, 0, _wpos.y) + terrainTrans->getPosition());

            float offset = 1.0f * terrain->getScale() + 2.0f;

            if (_wpos.x < (-terrain->getWorldSize() + offset) / 2.0f) continue;
            if (_wpos.y < -terrain->getWorldSize() / 2.0f) continue;
            if (_wpos.x > terrain->getWorldSize() / 2.0f) continue;
            if (_wpos.y > (terrain->getWorldSize() - offset) / 2.0f) continue;

            glm::vec3 position = glm::vec3(_wpos.x, y, _wpos.y);
            float rotation = Mathf::RandomFloat(0, 180.0f);
            float scale = Mathf::RandomFloat(minScale, maxScale);

            Batch* batch = nullptr;
            if (batches.size() > 0)
                batch = batches[batches.size() - 1];
            if (batch == nullptr || batch->meshes.size() >= batchSize)
            {
                batch = new Batch(this);
                batches.push_back(batch);
            }

            bool found = false;
            for (auto it = batches.begin(); it != batches.end(); ++it)
            {
                Batch* _batch = *it;

                auto bb = std::find_if(_batch->meshes.begin(), _batch->meshes.end(), [=](DetailMesh& g) -> bool
                    {
                        return std::abs(g.position.x - _wpos.x) < 1.0f / density && std::abs(g.position.z - _wpos.y) < 1.0f / density;
                    }
                );

                if (bb != _batch->meshes.end())
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                DetailMesh mesh;
                mesh.position = position;
                mesh.rotation = rotation;
                mesh.scale = scale;

                batch->meshes.push_back(mesh);

                auto sb = std::find(updBatches.begin(), updBatches.end(), batch);
                if (sb == updBatches.end())
                    updBatches.push_back(batch);
            }
        }

        for (auto it = updBatches.begin(); it != updBatches.end(); ++it)
            (*it)->update();

        bool ret = updBatches.size() > 0;
        updBatches.clear();

        return ret;
    }

    void TerrainDetailMeshData::addMesh(glm::vec3 position, float rotation, float scale)
    {
        Transform* terrainTrans = terrain->getGameObject()->getTransform();

        Batch* batch = nullptr;
        if (batches.size() > 0)
            batch = batches[batches.size() - 1];
        if (batch == nullptr || batch->meshes.size() >= batchSize)
        {
            batch = new Batch(this);
            batches.push_back(batch);
        }

        DetailMesh mesh;
        mesh.position = position;
        mesh.rotation = rotation;
        mesh.scale = scale;

        batch->meshes.push_back(mesh);

        if (batch->meshes.size() >= batchSize)
            batch->update();
    }

    bool TerrainDetailMeshData::removeMesh(std::vector<glm::vec2> positions, float density)
    {
        std::vector<Batch*> updBatches;

        glm::vec2 _min = glm::vec2(FLT_MAX);
        glm::vec2 _max = glm::vec2(FLT_MIN);
        for (auto pt = positions.begin(); pt != positions.end(); ++pt)
        {
            _min = glm::min(_min, *pt);
            _max = glm::max(_max, *pt);
        }
        float _minx = std::min(_min.x, _min.y);
        float _maxx = std::min(_max.x, _max.y);
        float _size = _maxx - _minx;

        for (auto pt = positions.begin(); pt != positions.end(); ++pt)
        {
            glm::vec2 pos = *pt;
            glm::vec2 _wpos = glm::vec2(-pos.x + terrain->getSize() / 2.0f, pos.y - terrain->getSize() / 2.0f);

            std::vector<Batch*> del;

            for (auto it = batches.begin(); it != batches.end(); ++it)
            {
                Batch* batch = *it;

                auto bb = std::find_if(batch->meshes.begin(), batch->meshes.end(), [=](DetailMesh& g) -> bool
                    {
                        float _mx = 1.0f * density;
                        _mx = glm::clamp(_mx, 0.0f, _size);
                        return std::abs(g.position.x - _wpos.x) < _mx && std::abs(g.position.z - _wpos.y) < _mx;
                    }
                );

                if (bb != batch->meshes.end())
                {
                    batch->meshes.erase(bb);
                    if (batch->meshes.size() > 0)
                    {
                        auto sb = std::find(updBatches.begin(), updBatches.end(), batch);
                        if (sb == updBatches.end())
                            updBatches.push_back(batch);
                    }
                    else
                    {
                        auto sb = std::find(del.begin(), del.end(), batch);
                        if (sb == del.end())
                            del.push_back(batch);
                    }
                }
            }

            for (auto it = del.begin(); it != del.end(); ++it)
            {
                Batch* b = *it;
                auto tt = std::find(batches.begin(), batches.end(), b);
                if (tt != batches.end())
                    batches.erase(tt);
                auto dd = std::find(updBatches.begin(), updBatches.end(), b);
                if (dd != updBatches.end())
                    updBatches.erase(dd);
                delete b;
            }
            del.clear();
        }

        for (auto it = updBatches.begin(); it != updBatches.end(); ++it)
            (*it)->update();

        bool ret = updBatches.size() > 0;
        updBatches.clear();

        return ret;
    }

    void TerrainDetailMeshData::destroy()
    {
        //Destroy created meshes
        for (auto it = batches.begin(); it != batches.end(); ++it)
            delete* it;

        batches.clear();
    }

    void TerrainDetailMeshData::updateImpostorTexture()
    {
        int textureSize = 256;

        destroyImpostorTexture();

        if (meshList.size() == 0)
            return;

        bgfx::FrameBufferHandle frameBufferHandle = { bgfx::kInvalidHandle };
        bgfx::TextureHandle depthTextureHandle = { bgfx::kInvalidHandle };

        uint64_t flags = BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_ANISOTROPIC
            | BGFX_SAMPLER_MAG_ANISOTROPIC
            /*| BGFX_SAMPLER_MIP_POINT*/
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP;

        impostorTexture = bgfx::createTexture2D(uint16_t(textureSize), uint16_t(textureSize), false, 1, bgfx::TextureFormat::BGRA8, flags);
        depthTextureHandle = bgfx::createTexture2D(uint16_t(textureSize), uint16_t(textureSize), false, 1, bgfx::TextureFormat::D24S8, flags);

        bgfx::Attachment at[2];
        at[0].init(impostorTexture);
        at[1].init(depthTextureHandle);
        frameBufferHandle = bgfx::createFrameBuffer(2, at, false);

        int view = Renderer::getSingleton()->getNumViewsUsed() + 1;

        AxisAlignedBox aab = getBounds();

        glm::vec3 aabSize = aab.getSize();
        float maxSide = std::max(aabSize.x, aabSize.y);

        glm::mat4x4 viewTrans = glm::orthoRH(maxSide / 2.0f, -maxSide / 2.0f, -maxSide / 2.0f, maxSide / 2.0f, -100.0f, 100.0f);

        bgfx::setViewClear(view, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0);
        bgfx::setViewRect(view, 0, 0, textureSize, textureSize);
        bgfx::setViewFrameBuffer(view, frameBufferHandle);
        bgfx::setViewTransform(view, nullptr, glm::value_ptr(viewTrans));

        glm::mat4x4 rotMat = glm::mat4_cast(Mathf::toQuaternion(glm::vec3(90.0f, 0, 90.0f)));

        for (auto it = meshList.begin(); it != meshList.end(); ++it)
        {
            ModelMeshData& data = *it;

            for (int i = 0; i < data.mesh->getSubMeshCount(); ++i)
            {
                SubMesh* subMesh = data.mesh->getSubMesh(i);
                Material* material = nullptr;
                if (i < data.materials.size())
                    material = data.materials[i];

                // Set model matrix for rendering.
                bgfx::setTransform(glm::value_ptr(rotMat * data.transform));

                // Set vertex and index buffer.
                bgfx::setVertexBuffer(0, subMesh->getVertexBufferHandle());
                bgfx::setIndexBuffer(subMesh->getIndexBufferHandle());

                // Set render states.
                bgfx::setState(0
                    | BGFX_STATE_WRITE_RGB
                    | BGFX_STATE_WRITE_A
                    | BGFX_STATE_WRITE_Z
                    | BGFX_STATE_DEPTH_TEST_LEQUAL
                    /*| BGFX_STATE_CULL_CW*/);

                bgfx::setTexture(0, Terrain::albedoTextureHandle, Texture::getNullTexture()->getHandle());

                if (material != nullptr && material->isLoaded())
                {
                    std::vector<Uniform>& uniforms = material->getUniforms();
                    for (auto u = uniforms.begin(); u != uniforms.end(); ++u)
                    {
                        if (u->getType() == UniformType::Sampler2D)
                        {
                            Texture* tex = u->getValue<Sampler2DDef>().second;
                            if (tex != nullptr)
                            {
                                bgfx::setTexture(0, Terrain::albedoTextureHandle, tex->getHandle());
                                break;
                            }
                        }
                    }
                }

                // Render
                bgfx::ProgramHandle ph = Renderer::getSingleton()->getSimpleTextureProgram();

                bgfx::submit(view, ph);
            }
        }

        Renderer::getSingleton()->frame();
        bgfx::destroy(frameBufferHandle);
        bgfx::destroy(depthTextureHandle);
        Renderer::getSingleton()->frame();
    }

    //----------------------GRASS DATA----------------------//

    TerrainGrassData::Batch::Batch(TerrainGrassData* grassData) : Renderable()
    {
        parent = grassData;
    }

    TerrainGrassData::Batch::~Batch()
    {
        meshes.clear();
        destroy();
    }

    void TerrainGrassData::Batch::releaseMemory(void* _ptr, void* _userData)
    {
        delete[] _ptr;
    }

    AxisAlignedBox TerrainGrassData::Batch::getBounds(bool world)
    {
        if (world)
        {
            Transform* terrTrans = parent->terrain->getGameObject()->getTransform();
            glm::mat4x4 mtx = terrTrans->getTransformMatrix();

            if (mtx != prevTransform)
            {
                cachedAAB = bounds;
                cachedAAB.transform(mtx);

                prevTransform = mtx;

                return cachedAAB;
            }
            else
                return cachedAAB;
        }
        else
        {
            return bounds;
        }
    }

    void TerrainGrassData::Batch::init()
    {
        assert(vertices == nullptr && "Vertices must be null");
        assert(indices == nullptr && "Indices must be null");

        vertexCount = 4 * meshes.size();
        indexCount = 6 * meshes.size();
        vertices = new VertexBuffer[vertexCount];
        indices = new uint32_t[indexCount];

        glm::vec2 minSize = parent->minSize;
        glm::vec2 maxSize = parent->maxSize;

        Transform* terrainTrans = parent->terrain->getGameObject()->getTransform();

        prevTransform = glm::mat4x4(FLT_MAX);
        bounds = AxisAlignedBox::BOX_NULL;

        int curMesh = 0;
        for (uint32_t i = 0; i < vertexCount; i += 4, ++curMesh)
        {
            float x = meshes[curMesh].position.x;
            float y = meshes[curMesh].position.y;
            float z = meshes[curMesh].position.z;
            float w = meshes[curMesh].width;
            float h = meshes[curMesh].height;
            float angle = meshes[curMesh].rotation;

            glm::vec3 positions[4];
            positions[0] = glm::vec4(x, y, z, 1.0f) - (Mathf::toQuaternion(glm::vec3(0, angle, 0)) * glm::vec4((float)w / 2, 0, 0, 1.0f));
            positions[1] = glm::vec4(x, y + h, z, 1.0f) - (Mathf::toQuaternion(glm::vec3(0, angle, 0)) * glm::vec4((float)w / 2, 0, 0, 1.0f));
            positions[2] = glm::vec4(x, y + h, z, 1.0f) + (Mathf::toQuaternion(glm::vec3(0, angle, 0)) * glm::vec4((float)w / 2, 0, 0, 1.0f));
            positions[3] = glm::vec4(x, y, z, 1.0f) + (Mathf::toQuaternion(glm::vec3(0, angle, 0)) * glm::vec4((float)w / 2, 0, 0, 1.0f));

            for (int j = 0; j < 4; ++j)
            {
                glm::vec2 uv = glm::vec2(0, 0);
                if (j == 1) uv = glm::vec2(0, 1);
                if (j == 2) uv = glm::vec2(1, 1);
                if (j == 3) uv = glm::vec2(1, 0);

                VertexBuffer* vert = &vertices[i + j];
                vert->position = positions[j];
                vert->texcoord0 = uv;
                bounds.merge(positions[j]);
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

        //Calculate normals + tangents + bitangents
        for (uint32_t i = 0; i < _indexCount; i += 3)
        {
            //Normals
            uint32_t id0 = indices[i + 0];
            uint32_t id1 = indices[i + 1];
            uint32_t id2 = indices[i + 2];

            glm::vec3 v1 = vertices[id0].position;
            glm::vec3 v2 = vertices[id1].position;
            glm::vec3 v3 = vertices[id2].position;

            glm::vec3 normalA = glm::normalize(glm::cross(v2 - v1, v3 - v1));

            vertices[id0].normal = normalA;
            vertices[id1].normal = normalA;
            vertices[id2].normal = normalA;

            //Tangent space
            glm::vec3 deltaPos = glm::vec3(0);
            if (v1 == v2)
                deltaPos = v3 - v1;
            else
                deltaPos = v2 - v1;

            glm::vec2 uv0 = vertices[id0].texcoord0;
            glm::vec2 uv1 = vertices[id1].texcoord0;
            glm::vec2 uv2 = vertices[id2].texcoord0;

            glm::vec2 deltaUV1 = uv1 - uv0;
            glm::vec2 deltaUV2 = uv2 - uv0;

            glm::vec3 tan = glm::vec3(0); // tangent
            glm::vec3 bin = glm::vec3(0); // binormal

            // avoid divion with 0
            if (deltaUV1.s != 0)
                tan = deltaPos / deltaUV1.s;
            else
                tan = deltaPos / 1.0f;

            tan = glm::normalize(tan - glm::dot(normalA, tan) * normalA);
            bin = glm::normalize(glm::cross(tan, normalA));

            vertices[id0].tangent = tan;
            vertices[id0].bitangent = bin;
            vertices[id1].tangent = tan;
            vertices[id1].bitangent = bin;
            vertices[id2].tangent = tan;
            vertices[id2].bitangent = bin;
        }

        //Create buffers
        if (!bgfx::isValid(vbh))
        {
            const bgfx::Memory* mem = bgfx::makeRef(vertices, sizeof(VertexBuffer) * vertexCount, releaseMemory);
            vbh = bgfx::createVertexBuffer(mem, VertexLayouts::terrainVertexLayout);
        }

        if (!bgfx::isValid(ibh))
        {
            const bgfx::Memory* mem = bgfx::makeRef(indices, sizeof(uint32_t) * indexCount, releaseMemory);
            ibh = bgfx::createIndexBuffer(mem, BGFX_BUFFER_INDEX32);
        }
    }

    void TerrainGrassData::Batch::destroy()
    {
        //Destroy buffers
        if (bgfx::isValid(vbh))
            bgfx::destroy(vbh);
        if (bgfx::isValid(ibh))
            bgfx::destroy(ibh);

        vbh = { bgfx::kInvalidHandle };
        ibh = { bgfx::kInvalidHandle };

        /*if (vertices != nullptr)
            delete[] vertices;
        if (indices != nullptr)
            delete[] indices;*/

        vertices = nullptr;
        indices = nullptr;
        vertexCount = 0;
        indexCount = 0;
    }

    void TerrainGrassData::Batch::update()
    {
        destroy();
        init();
    }

    bool TerrainGrassData::Batch::getCastShadows()
    {
        if (parent == nullptr || parent->terrain == nullptr)
            return false;

        return parent->terrain->getGrassCastShadows();
    }

    bool TerrainGrassData::Batch::checkCullingMask(LayerMask& mask)
    {
        return parent->terrain->checkCullingMask(mask);
    }

    bool TerrainGrassData::addGrass(std::vector<glm::vec2> positions, float density)
    {
        if (material == nullptr)
            return false;

        std::vector<Batch*> updBatches;
        Transform* terrainTrans = terrain->getGameObject()->getTransform();

        for (auto pt = positions.begin(); pt != positions.end(); ++pt)
        {
            glm::vec2 pos = *pt;
            glm::vec2 _wpos = glm::vec2(-pos.x + terrain->getSize() / 2.0f, pos.y - terrain->getSize() / 2.0f);
            float y = terrain->getHeightAtWorldPos(glm::vec3(_wpos.x, 0, _wpos.y) + terrainTrans->getPosition());

            float offset = 1.0f * terrain->getScale() + 2.0f;

            if (_wpos.x < (-terrain->getWorldSize() + offset) / 2.0f) continue;
            if (_wpos.y < -terrain->getWorldSize() / 2.0f) continue;
            if (_wpos.x > terrain->getWorldSize() / 2.0f) continue;
            if (_wpos.y > (terrain->getWorldSize() - offset) / 2.0f) continue;
            
            Batch* batch = nullptr;
            if (batches.size() > 0)
                batch = batches[batches.size() - 1];
            if (batch == nullptr || batch->meshes.size() >= batchSize)
            {
                batch = new Batch(this);
                batches.push_back(batch);
            }

            bool found = false;
            for (auto it = batches.begin(); it != batches.end(); ++it)
            {
                Batch* _batch = *it;

                auto bb = std::find_if(_batch->meshes.begin(), _batch->meshes.end(), [=](GrassMesh& g) -> bool
                    {
                        return std::abs(g.position.x - _wpos.x) < 1.0f / density && std::abs(g.position.z - _wpos.y) < 1.0f / density;
                    }
                );

                if (bb != _batch->meshes.end())
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                GrassMesh mesh;
                mesh.position = glm::vec3(_wpos.x, y, _wpos.y);
                mesh.rotation = Mathf::RandomFloat(0, 180.0f);
                mesh.width = Mathf::RandomFloat(minSize.x, maxSize.x);
                mesh.height = Mathf::RandomFloat(minSize.y, maxSize.y);

                batch->meshes.push_back(mesh);

                auto sb = std::find(updBatches.begin(), updBatches.end(), batch);
                if (sb == updBatches.end())
                    updBatches.push_back(batch);
            }
        }

        for (auto it = updBatches.begin(); it != updBatches.end(); ++it)
            (*it)->update();

        bool ret = updBatches.size() > 0;
        updBatches.clear();

        return ret;
    }

    void TerrainGrassData::addGrass(glm::vec3 position, float rotation, float width, float height)
    {
        Transform* terrainTrans = terrain->getGameObject()->getTransform();

        Batch* batch = nullptr;
        if (batches.size() > 0)
            batch = batches[batches.size() - 1];
        if (batch == nullptr || batch->meshes.size() >= batchSize)
        {
            batch = new Batch(this);
            batches.push_back(batch);
        }

        GrassMesh mesh;
        mesh.position = position;
        mesh.rotation = rotation;
        mesh.width = width;
        mesh.height = height;

        batch->meshes.push_back(mesh);

        if (batch->meshes.size() >= batchSize)
            batch->update();
    }

    bool TerrainGrassData::removeGrass(std::vector<glm::vec2> positions, float density)
    {
        std::vector<Batch*> updBatches;

        glm::vec2 _min = glm::vec2(FLT_MAX);
        glm::vec2 _max = glm::vec2(FLT_MIN);
        for (auto pt = positions.begin(); pt != positions.end(); ++pt)
        {
            _min = glm::min(_min, *pt);
            _max = glm::max(_max, *pt);
        }
        float _minx = std::min(_min.x, _min.y);
        float _maxx = std::min(_max.x, _max.y);
        float _size = _maxx - _minx;

        for (auto pt = positions.begin(); pt != positions.end(); ++pt)
        {
            glm::vec2 pos = *pt;
            glm::vec2 _wpos = glm::vec2(-pos.x + terrain->getSize() / 2.0f, pos.y - terrain->getSize() / 2.0f);

            std::vector<Batch*> del;

            for (auto it = batches.begin(); it != batches.end(); ++it)
            {
                Batch* batch = *it;

                auto bb = std::find_if(batch->meshes.begin(), batch->meshes.end(), [=](GrassMesh& g) -> bool
                    {
                        float _mx = 1.0f * density;
                        _mx = glm::clamp(_mx, 0.0f, _size);
                        return std::abs(g.position.x - _wpos.x) < _mx && std::abs(g.position.z - _wpos.y) < _mx;
                    }
                );

                if (bb != batch->meshes.end())
                {
                    batch->meshes.erase(bb);
                    if (batch->meshes.size() > 0)
                    {
                        auto sb = std::find(updBatches.begin(), updBatches.end(), batch);
                        if (sb == updBatches.end())
                            updBatches.push_back(batch);
                    }
                    else
                    {
                        auto sb = std::find(del.begin(), del.end(), batch);
                        if (sb == del.end())
                            del.push_back(batch);
                    }
                }
            }

            for (auto it = del.begin(); it != del.end(); ++it)
            {
                Batch* b = *it;
                auto tt = std::find(batches.begin(), batches.end(), b);
                if (tt != batches.end())
                    batches.erase(tt);
                auto dd = std::find(updBatches.begin(), updBatches.end(), b);
                if (dd != updBatches.end())
                    updBatches.erase(dd);
                delete b;
            }
            del.clear();
        }

        for (auto it = updBatches.begin(); it != updBatches.end(); ++it)
            (*it)->update();

        bool ret = updBatches.size() > 0;
        updBatches.clear();

        return ret;
    }

    TerrainGrassData::TerrainGrassData(Terrain* _terrain)
    {
        terrain = _terrain;
    }

    TerrainGrassData::~TerrainGrassData()
    {
        destroy();
    }

    void TerrainGrassData::destroy()
    {
        for (auto it = batches.begin(); it != batches.end(); ++it)
            delete* it;

        batches.clear();
    }

    void TerrainGrassData::setMaterial(Material* value)
    {
        material = value;
    }

    void TerrainGrassData::setMinSize(glm::vec2 value)
    {
        minSize = value;

        for (auto it = batches.begin(); it != batches.end(); ++it)
        {
            Batch* batch = *it;
            for (auto m = batch->meshes.begin(); m != batch->meshes.end(); ++m)
            {
                GrassMesh& mesh = *m;
                mesh.width = Mathf::RandomFloat(minSize.x, maxSize.x);
                mesh.height = Mathf::RandomFloat(minSize.y, maxSize.y);
            }

            batch->update();
        }
    }

    void TerrainGrassData::setMaxSize(glm::vec2 value)
    {
        maxSize = value;

        for (auto it = batches.begin(); it != batches.end(); ++it)
        {
            Batch* batch = *it;
            for (auto m = batch->meshes.begin(); m != batch->meshes.end(); ++m)
            {
                GrassMesh& mesh = *m;
                mesh.width = Mathf::RandomFloat(minSize.x, maxSize.x);
                mesh.height = Mathf::RandomFloat(minSize.y, maxSize.y);
            }

            batch->update();
        }
    }

    //----------------------TERRAIN----------------------//

    Terrain::Terrain() : Component(nullptr)
    {
        material = (Material*)Asset::getLoadedInstance("system/terrain/materials/", "StandardTerrain");
        if (material == nullptr)
        {
            Shader* shader = Shader::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Shaders/StandardDeferredTerrain.shader");
            //Shader* shader = Shader::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Shaders/StandardTerrain.shader");
            material = new Material();
            material->setLocation("system/terrain/materials/");
            material->setName("StandardTerrain");
            material->setShader(shader);
            material->load();
        }

        for (int i = 0; i < MAX_TERRAIN_TEXTURES; ++i)
        {
            textureDiffuseHandles[i] = bgfx::createUniform(("u_texture" + std::to_string(i)).c_str(), bgfx::UniformType::Sampler, 1);
            textureNormalHandles[i] = bgfx::createUniform(("u_texture" + std::to_string(i) + "Normal").c_str(), bgfx::UniformType::Sampler, 1);
        }

        int splatCount = 0;
        float t = MAX_TERRAIN_TEXTURES % 4;
        if (t > 0)
            splatCount = std::trunc(MAX_TERRAIN_TEXTURES / 4) + 1;
        else
            splatCount = MAX_TERRAIN_TEXTURES / 4;

        for (int i = 0; i < splatCount; ++i)
        {
            textureSplatHandles[i] = bgfx::createUniform(("u_texture" + std::to_string(i) + "Splat").c_str(), bgfx::UniformType::Sampler, 1);
        }

        textureCountHandle = bgfx::createUniform("u_textureCount", bgfx::UniformType::Vec4, 1);
        textureSizesHandle = bgfx::createUniform("u_textureSizes", bgfx::UniformType::Vec4, MAX_TERRAIN_TEXTURES);
        albedoTextureHandle = bgfx::createUniform("u_albedoMap", bgfx::UniformType::Sampler, 1);
    }

    Terrain::~Terrain()
    {
        destroy();
    }

    void Terrain::releaseData(void* _ptr, void* _userData)
    {
        
    }

    void Terrain::create()
    {
        uint32_t num = size * size;
        vertices = new VertexBuffer[num];
        indices = new uint32_t[num * (uint32_t)6];
        heightMap = new float[num];

        memset(heightMap, 0, num * sizeof(float));

        for (uint32_t i = 0; i < num; ++i)
            vertices[i].normal = glm::vec3(0.0f);

        int splatCount = 0;
        float t = MAX_TERRAIN_TEXTURES % 4;
        if (t > 0)
            splatCount = std::trunc(MAX_TERRAIN_TEXTURES / 4) + 1;
        else
            splatCount = MAX_TERRAIN_TEXTURES / 4;

        for (int i = 0; i < splatCount; ++i)
        {
            Texture* splatTexture = Texture::create("system/terrain/splat_textures/", GUIDGenerator::genGuid() + "_texture_" + std::to_string(i), size, size, 1, Texture::TextureType::Texture2D, bgfx::TextureFormat::RGBA8);

            int sz = size * size * 4;
            splatTexture->allocData(sz);
            unsigned char* data = splatTexture->getData();

            for (int i = 0; i < sz; ++i)
                data[i] = 0;

            splatTexture->updateTexture();
            splatTextures.push_back(splatTexture);
        }
    }

    void Terrain::destroy()
    {
        if (bgfx::isValid(ibh))
            bgfx::destroy(ibh);

        if (bgfx::isValid(vbh))
            bgfx::destroy(vbh);

        ibh = { bgfx::kInvalidHandle };
        vbh = { bgfx::kInvalidHandle };

        if (heightMap != nullptr)
            delete[] heightMap;
        if (vertices != nullptr)
            delete[] vertices;
        if (indices != nullptr)
            delete[] indices;

        vertices = nullptr;
        indices = nullptr;
        heightMap = nullptr;

        for (auto it = splatTextures.begin(); it != splatTextures.end(); ++it)
            delete* it;

        splatTextures.clear();

        for (auto it = grassList.begin(); it != grassList.end(); ++it)
            delete* it;

        for (auto it = treeList.begin(); it != treeList.end(); ++it)
            delete* it;

        for (auto it = detailMeshList.begin(); it != detailMeshList.end(); ++it)
            delete* it;

        textureList.clear();
        grassList.clear();
        detailMeshList.clear();
        treeList.clear();
    }

    void Terrain::recreate()
    {
        destroy();
        create();
    }

    void Terrain::setSize(int32_t value)
    {
        size = value;
        isDirty = true;

        if (size > worldSize)
            worldSize = size;

        delete[] heightMap;
        size_t num = size * size;
        heightMap = new float[num];
        memset(heightMap, 0, num * sizeof(float));

        for (int i = 0; i < splatTextures.size(); ++i)
        {
            delete splatTextures[i];

            Texture* splatTexture = Texture::create("system/terrain/splat_textures/", GUIDGenerator::genGuid() + "_texture_" + std::to_string(i), size, size, 1, Texture::TextureType::Texture2D, bgfx::TextureFormat::RGBA8);

            int sz = size * size * 4;
            splatTexture->allocData(sz);
            unsigned char* data = splatTexture->getData();

            for (int i = 0; i < sz; ++i)
                data[i] = 0;

            splatTexture->updateTexture();
            splatTextures[i] = splatTexture;
        }

        STerrainData data = serialize();
        recreate();
        deserialize(data, getFilePath());

        for (auto& tree : treeList)
        {
            for (auto& mesh : tree->meshes)
                delete mesh;

            tree->meshes.clear();
        }

        for (auto& grass : grassList)
        {
            for (auto& batch : grass->batches)
                delete batch;

            grass->batches.clear();
        }

        for (auto& detail : detailMeshList)
        {
            for (auto& batch : detail->batches)
                delete batch;

            detail->batches.clear();
        }
    }

    void Terrain::setWorldSize(int32_t value)
    {
        worldSize = value;
        isDirty = true;

        if (size > worldSize)
            size = worldSize;

        STerrainData data = serialize();
        recreate();
        deserialize(data, getFilePath());

        for (auto& tree : treeList)
        {
            for (auto& mesh : tree->meshes)
                delete mesh;

            tree->meshes.clear();
        }

        for (auto& grass : grassList)
        {
            for (auto& batch : grass->batches)
                delete batch;

            grass->batches.clear();
        }

        for (auto& detail : detailMeshList)
        {
            for (auto& batch : detail->batches)
                delete batch;

            detail->batches.clear();
        }
    }

    TerrainGrassData* Terrain::addGrassData()
    {
        TerrainGrassData* data = new TerrainGrassData(this);
        grassList.push_back(data);

        return data;
    }

    void Terrain::removeGrassData(int index)
    {
        auto it = grassList.begin() + index;
        if (it != grassList.end())
        {
            delete* it;
            grassList.erase(it);
        }
    }

    TerrainTreeData* Terrain::addTreeData()
    {
        TerrainTreeData* data = new TerrainTreeData(this);
        treeList.push_back(data);

        return data;
    }

    void Terrain::removeTreeData(int index)
    {
        auto it = treeList.begin() + index;
        if (it != treeList.end())
        {
            delete* it;
            treeList.erase(it);
        }
    }

    TerrainDetailMeshData* Terrain::addDetailMeshData()
    {
        TerrainDetailMeshData* data = new TerrainDetailMeshData(this);
        detailMeshList.push_back(data);

        return data;
    }

    void Terrain::removeDetailMeshData(int index)
    {
        auto it = detailMeshList.begin() + index;
        if (it != detailMeshList.end())
        {
            delete* it;
            detailMeshList.erase(it);
        }
    }

    AxisAlignedBox Terrain::getBounds(bool world)
    {
        if (world)
        {
            glm::mat4x4 mtx = transform->getTransformMatrix();

            if (mtx != prevTransform)
            {
                cachedAAB = bounds;
                cachedAAB.transform(mtx);

                prevTransform = mtx;

                return cachedAAB;
            }
            else
                return cachedAAB;
        }
        else
        {
            return bounds;
        }
    }

    float Terrain::getHeight(uint32_t x, uint32_t y)
    {
        x = (uint32_t)glm::clamp((float)x, 0.0f, (float)size - 1);
        y = (uint32_t)glm::clamp((float)y, 0.0f, (float)size - 1);

        uint32_t heightMapPos0 = (y * (int32_t)size) + x;
        heightMapPos0 = glm::clamp((float)heightMapPos0, 0.0f, ((float)size * (float)size) - 1.0f);
        return heightMap[heightMapPos0];
    }

    void Terrain::setHeight(uint32_t x, uint32_t y, float height)
    {
        x = (uint32_t)glm::clamp((float)x, 0.0f, (float)size - 1);
        y = (uint32_t)glm::clamp((float)y, 0.0f, (float)size - 1);

        uint32_t heightMapPos0 = (y * (int32_t)size) + x;
        heightMapPos0 = glm::clamp((float)heightMapPos0, 0.0f, ((float)size * (float)size) - 1.0f);
        heightMap[heightMapPos0] = height;
        isDirty = true;
    }

    float Terrain::getHeightAtWorldPos(glm::vec3 worldPos)
    {
        glm::vec3 tp = transform->getPosition();
        float scale = getScale();
        float offset = 1.0f * scale;

        glm::vec3 pos = (worldPos / scale) - (tp / scale) + (size / 2.0f);
        pos.x = size - pos.x;

        float factor = 1.0f;// (float)size - 1.0f;
        float invFactor = 1.0f / factor;

        int32_t startX = (pos.x * factor);
        int32_t startY = (pos.z * factor);
        int32_t endX = startX + 1;
        int32_t endY = startY + 1;

        float startXTS = startX * invFactor;
        float startYTS = startY * invFactor;
        float endXTS = endX * invFactor;
        float endYTS = endY * invFactor;

        float xParam = (pos.x - startXTS) / invFactor;
        float yParam = (pos.z - startYTS) / invFactor;

        glm::vec3 v0(startXTS, startYTS, getHeight(startX, startY));
        glm::vec3 v1(endXTS, startYTS, getHeight(endX, startY));
        glm::vec3 v2(endXTS, endYTS, getHeight(endX, endY));
        glm::vec3 v3(startXTS, endYTS, getHeight(startX, endY));
        
        Plane plane;
        if (startY % 2)
        {
            // odd row
            bool secondTri = ((1.0 - yParam) > xParam);
            if (secondTri)
                plane.redefine(v0, v1, v3);
            else
                plane.redefine(v1, v2, v3);
        }
        else
        {
            // even row
            bool secondTri = (yParam > xParam);
            if (secondTri)
                plane.redefine(v0, v2, v3);
            else
                plane.redefine(v0, v1, v2);
        }

        // Solve plane equation for z
        return (-plane.normal.x * pos.x - plane.normal.y * pos.z - plane.d) / plane.normal.z;
    }

    float Terrain::getHeightAtWorldPosUnclamped(glm::vec3 worldPos)
    {
        glm::vec3 tp = transform->getPosition();
        float scale = getScale();

        glm::vec3 pos = (worldPos / scale) - (tp / scale) + (size / 2.0f);
        pos.x = size - pos.x;

        return getHeight((uint32_t)pos.x, (uint32_t)pos.z);
    }

    void Terrain::setHeightAtWorldPos(glm::vec3 worldPos, float height)
    {
        glm::vec3 tp = transform->getPosition();
        float scale = getScale();

        glm::vec3 pos = (worldPos / scale) - (tp / scale) + (size / 2.0f);
        pos.x = size - pos.x;

        setHeight((uint32_t)pos.x, (uint32_t)pos.z, height);
    }

    void Terrain::updateTerrainMesh()
    {
        prevTransform = glm::mat4x4(FLT_MAX);
        bounds = AxisAlignedBox::BOX_NULL;

        float scale = worldSize / size;

        vertexCount = 0;
        for (uint32_t y = 0; y < size; y++)
        {
            for (uint32_t x = 0; x < size; x++)
            {
                int h = (y * size) + x;
                glm::vec3 position = Mathf::toQuaternion(180.0f, 0, 0) * glm::vec3(((float)x - size / 2) * scale, -heightMap[h], ((float)y - size / 2) * scale);
                glm::vec2 texcoord = glm::vec2((x + 0.5f) / size, (y + 0.5f) / size);

                VertexBuffer* vert = &vertices[vertexCount];
                vert->position = position;
                vert->texcoord0 = texcoord;

                vertexCount++;

                bounds.merge(position);
            }
        }

        indexCount = 0;
        for (uint32_t y = 0; y < (size - 1); y++)
        {
            uint32_t y_offset = (y * size);
            for (uint32_t x = 0; x < (size - 1); x++)
            {
                indices[indexCount + 0] = y_offset + x + 1;
                indices[indexCount + 1] = y_offset + x + size;
                indices[indexCount + 2] = y_offset + x;
                indices[indexCount + 3] = y_offset + x + size + 1;
                indices[indexCount + 4] = y_offset + x + size;
                indices[indexCount + 5] = y_offset + x + 1;

                indexCount += 6;
            }
        }

        //Calculate normals + tangents + bitangents
        for (uint32_t i = 0; i < indexCount; i += 3)
        {
            //Normals
            uint32_t id0 = indices[i + 0];
            uint32_t id1 = indices[i + 1];
            uint32_t id2 = indices[i + 2];

            glm::vec3 v1 = vertices[id0].position;
            glm::vec3 v2 = vertices[id1].position;
            glm::vec3 v3 = vertices[id2].position;

            glm::vec3 normalA = glm::normalize(glm::cross(v2 - v1, v3 - v1));

            vertices[id0].normal = normalA;
            vertices[id1].normal = normalA;
            vertices[id2].normal = normalA;

            //Tangent space
            glm::vec3 deltaPos = glm::vec3(0);
            if (v1 == v2)
                deltaPos = v3 - v1;
            else
                deltaPos = v2 - v1;

            glm::vec2 uv0 = vertices[id0].texcoord0;
            glm::vec2 uv1 = vertices[id1].texcoord0;
            glm::vec2 uv2 = vertices[id2].texcoord0;

            glm::vec2 deltaUV1 = uv1 - uv0;
            glm::vec2 deltaUV2 = uv2 - uv0;

            glm::vec3 tan = glm::vec3(0); // tangent
            glm::vec3 bin = glm::vec3(0); // binormal

            // avoid divion with 0
            if (deltaUV1.s != 0)
                tan = deltaPos / deltaUV1.s;
            else
                tan = deltaPos / 1.0f;

            tan = glm::normalize(tan - glm::dot(normalA, tan) * normalA);
            bin = glm::normalize(glm::cross(tan, normalA));

            vertices[id0].tangent = tan;
            vertices[id0].bitangent = bin;
            vertices[id1].tangent = tan;
            vertices[id1].bitangent = bin;
            vertices[id2].tangent = tan;
            vertices[id2].bitangent = bin;
        }
    }

    void Terrain::updateTerrain()
    {
        updateTerrainMesh();

        if (!bgfx::isValid(vbh))
            vbh = bgfx::createDynamicVertexBuffer(vertexCount, VertexLayouts::terrainVertexLayout);

        if (!bgfx::isValid(ibh))
            ibh = bgfx::createDynamicIndexBuffer(indexCount, BGFX_BUFFER_INDEX32);

        const bgfx::Memory* mem = bgfx::makeRef(&vertices[0], sizeof(VertexBuffer) * vertexCount, releaseData);
        bgfx::update(vbh, 0, mem);

        mem = bgfx::makeRef(&indices[0], sizeof(uint32_t) * indexCount, releaseData);
        bgfx::update(ibh, 0, mem);
    }

    void Terrain::updatePositions(/*glm::vec3 _min, glm::vec3 _max*/)
    {
        //glm::vec3 _wmin = glm::vec3(_min.x, _min.y, _min.z);// -transform->getPosition());
        //glm::vec3 _wmax = glm::vec3(_max.x, _max.y, _max.z);// -transform->getPosition());
        Transform* terrainTrans = getGameObject()->getTransform();

        for (auto g = grassList.begin(); g != grassList.end(); ++g)
        {
            TerrainGrassData* grass = *g;
            for (auto it = grass->batches.begin(); it != grass->batches.end(); ++it)
            {
                TerrainGrassData::Batch* batch = *it;
                for (auto m = batch->meshes.begin(); m != batch->meshes.end(); ++m)
                {
                    //AxisAlignedBox aab = AxisAlignedBox(_wmin, _wmax);
                    //if (aab.contains(glm::vec3(m->position.x, (_wmin.y + _wmax.y) / 2.0f, m->position.y)))
                    //{
                    //batch->update();
                    //break;
                    //}
                    float y = getHeightAtWorldPos(glm::vec3(m->position.x, 0, m->position.z) + terrainTrans->getPosition());
                    m->position.y = y;
                }

                batch->update();
            }
        }

        for (auto t = treeList.begin(); t != treeList.end(); ++t)
        {
            const std::vector<TerrainTreeData::TreeMesh*>& meshes = (*t)->getMeshes();
            for (auto m = meshes.begin(); m != meshes.end(); ++m)
            {
                float y = getHeightAtWorldPos(glm::vec3((*m)->position.x, 0, (*m)->position.z) + terrainTrans->getPosition());
                (*m)->position.y = y;
                (*m)->updateTransform(); //prevTransform = glm::mat4x4(FLT_MAX);
            }
        }

        for (auto t = detailMeshList.begin(); t != detailMeshList.end(); ++t)
        {
            const std::vector<TerrainDetailMeshData::Batch*>& batches = (*t)->getBatches();
            for (auto b = batches.begin(); b != batches.end(); ++b)
            {
                TerrainDetailMeshData::Batch* batch = *b;
                for (auto m = batch->meshes.begin(); m != batch->meshes.end(); ++m)
                {
                    float y = getHeightAtWorldPos(glm::vec3(m->position.x, 0, m->position.z) + terrainTrans->getPosition());
                    m->position.y = y;
                }
                batch->update();
            }
        }
    }

    void Terrain::onAttach()
    {
        Component::onAttach();

        transform = getGameObject()->getTransform();
        transform->setOnChangeCallback([=]() { updateTreesTransforms(); });

        if (heightMap == nullptr)
            create();

        attach();
        updateTreesTransforms();

        for (auto& t : treeList)
        {
            for (auto& m : t->getMeshes())
                m->attach();
        }

        for (auto& g : grassList)
        {
            for (auto& m : g->getBatches())
                m->attach();
        }

        for (auto& d : detailMeshList)
        {
            for (auto& m : d->getBatches())
                m->attach();
        }
    }

    void Terrain::onDetach()
    {
        Component::onDetach();

        transform->setOnChangeCallback(nullptr);

        detach();

        for (auto& t : treeList)
        {
            for (auto& m : t->getMeshes())
                m->detach();
        }

        for (auto& g : grassList)
        {
            for (auto& m : g->getBatches())
                m->detach();
        }

        for (auto& d : detailMeshList)
        {
            for (auto& m : d->getBatches())
                m->detach();
        }
    }

    void Terrain::updateTreesTransforms()
    {
        for (auto t = treeList.begin(); t != treeList.end(); ++t)
        {
            const std::vector<TerrainTreeData::TreeMesh*>& meshes = (*t)->getMeshes();
            for (auto m = meshes.begin(); m != meshes.end(); ++m)
                (*m)->updateTransform();
        }
    }

    Component* Terrain::onClone()
    {
        Terrain* newComponent = new Terrain();
        newComponent->material = material;
        newComponent->size = size;
        newComponent->worldSize = worldSize;
        newComponent->detailMeshesCastShadows = detailMeshesCastShadows;
        newComponent->detailMeshesDrawDistance = detailMeshesDrawDistance;
        newComponent->drawDetailMeshes = drawDetailMeshes;
        newComponent->drawGrass = drawGrass;
        newComponent->drawTrees = drawTrees;
        newComponent->grassCastShadows = grassCastShadows;
        newComponent->grassDrawDistance = grassDrawDistance;
        newComponent->treeDrawDistance = treeDrawDistance;
        newComponent->treeImpostorStartDistance = treeImpostorStartDistance;
        newComponent->treesCastShadows = treesCastShadows;
        newComponent->filePath = filePath;

        return newComponent;
    }

    void Terrain::onRefresh()
    {
        load(filePath);
    }

    bool Terrain::checkCullingMask(LayerMask& mask)
    {
        if (gameObject == nullptr)
            return false;

        return mask.getLayer(gameObject->getLayer());
    }

    STerrainGrassData Terrain::serializeGrass(TerrainGrassData* grass)
    {
        STerrainGrassData sGrass;
        if (grass->getMaterial() != nullptr)
            sGrass.name = grass->getMaterial()->getName();
        sGrass.minSize = grass->getMinSize();
        sGrass.maxSize = grass->getMaxSize();
        for (auto jt = grass->getBatches().begin(); jt != grass->getBatches().end(); ++jt)
        {
            for (auto kt = (*jt)->getMeshes().begin(); kt != (*jt)->getMeshes().end(); ++kt)
            {
                TerrainGrassData::GrassMesh& mesh = *kt;
                STerrainGrassMeshData sMesh;
                sMesh.position = mesh.position;
                sMesh.rotation = mesh.rotation;
                sMesh.width = mesh.width;
                sMesh.height = mesh.height;
                sGrass.meshes.push_back(sMesh);
            }
        }

        return sGrass;
    }

    TerrainGrassData* Terrain::deserializeGrass(STerrainGrassData sGrass)
    {
        TerrainGrassData* grass = new TerrainGrassData(this);

        if (!sGrass.name.empty())
        {
            Material* mat = Material::load(Engine::getSingleton()->getAssetsPath(), sGrass.name);
            if (mat != nullptr && mat->isLoaded())
                grass->setMaterial(mat);
        }
        grass->setMinSize(sGrass.minSize.getValue());
        grass->setMaxSize(sGrass.maxSize.getValue());
        for (auto jt = sGrass.meshes.begin(); jt != sGrass.meshes.end(); ++jt)
        {
            STerrainGrassMeshData& sMesh = *jt;
            grass->addGrass(sMesh.position.getValue(), sMesh.rotation, sMesh.width, sMesh.height);
        }
        for (auto b = grass->getBatches().begin(); b != grass->getBatches().end(); ++b)
            (*b)->update();

        return grass;
    }

    STerrainTreeData Terrain::serializeTree(TerrainTreeData* tree)
    {
        STerrainTreeData sTree;
        sTree.name = tree->getName();
        sTree.minScale = tree->getMinScale();
        sTree.maxScale = tree->getMaxScale();
        for (auto& ml : tree->meshList)
        {
            STerrainModelMeshData mmd;
            mmd.transform = ml.transform;
            if (ml.mesh != nullptr) mmd.mesh = ml.mesh->getName();
            for (auto& mtl : ml.materials)
                if (mtl != nullptr) mmd.materials.push_back(mtl->getName());
            sTree.meshModels.push_back(mmd);
        }
        for (auto jt = tree->getMeshes().begin(); jt != tree->getMeshes().end(); ++jt)
        {
            TerrainTreeData::TreeMesh* mesh = *jt;
            STerrainTreeMeshData sMesh;
            sMesh.position = mesh->position;
            sMesh.rotation = mesh->rotation;
            sMesh.scale = mesh->scale;
            sTree.meshes.push_back(sMesh);
        }

        return sTree;
    }

    TerrainTreeData* Terrain::deserializeTree(STerrainTreeData sTree)
    {
        std::string location = Engine::getSingleton()->getAssetsPath();

        TerrainTreeData* tree = new TerrainTreeData(this);
        if (!Engine::getSingleton()->getIsRuntimeMode())
        {
            tree->setName(sTree.name);
        }
        else
        {
            tree->name = sTree.name;
            for (auto& md : sTree.meshModels)
            {
                TerrainTreeData::ModelMeshData mdt;
                mdt.transform = md.transform.getValue();
                for (auto& mat : md.materials)
                {
                    Material* m = Material::load(Engine::getSingleton()->getAssetsPath(), mat);
                    if (m != nullptr) mdt.materials.push_back(m);
                }

                if (!IO::FileExists(Engine::getSingleton()->getLibraryPath() + md.mesh))
                {
                    if (IO::FileExists(location + tree->name))
                        Model3DLoader::cache3DModel(location, tree->name, nullptr);
                }

                Mesh* mesh = Mesh::load(Engine::getSingleton()->getLibraryPath(), md.mesh);
                if (mesh != nullptr)
                    mdt.mesh = mesh;
                tree->meshList.push_back(mdt);
            }

            tree->updateBounds();
            tree->updateImpostorTexture();
        }
        tree->setMinScale(sTree.minScale);
        tree->setMaxScale(sTree.maxScale);
        for (auto jt = sTree.meshes.begin(); jt != sTree.meshes.end(); ++jt)
        {
            STerrainTreeMeshData sMesh = *jt;
            tree->addTree(sMesh.position.getValue(), sMesh.rotation, sMesh.scale);
        }

        return tree;
    }

    STerrainDetailMeshData Terrain::serializeDetailMesh(TerrainDetailMeshData* detailMesh)
    {
        STerrainDetailMeshData sDetailMesh;
        sDetailMesh.name = detailMesh->getName();
        sDetailMesh.minScale = detailMesh->getMinScale();
        sDetailMesh.maxScale = detailMesh->getMaxScale();
        for (auto& ml : detailMesh->meshList)
        {
            STerrainModelMeshData mmd;
            mmd.transform = ml.transform;
            if (ml.mesh != nullptr) mmd.mesh = ml.mesh->getName();
            for (auto& mtl : ml.materials)
                if (mtl != nullptr) mmd.materials.push_back(mtl->getName());
            sDetailMesh.meshModels.push_back(mmd);
        }
        for (auto jt = detailMesh->getBatches().begin(); jt != detailMesh->getBatches().end(); ++jt)
        {
            for (auto kt = (*jt)->getMeshes().begin(); kt != (*jt)->getMeshes().end(); ++kt)
            {
                TerrainDetailMeshData::DetailMesh& mesh = *kt;
                STerrainDetailMeshInfoData sMesh;
                sMesh.position = mesh.position;
                sMesh.rotation = mesh.rotation;
                sMesh.scale = mesh.scale;
                sDetailMesh.meshes.push_back(sMesh);
            }
        }

        return sDetailMesh;
    }

    TerrainDetailMeshData* Terrain::deserializeDetailMesh(STerrainDetailMeshData sDetailMesh)
    {
        std::string location = Engine::getSingleton()->getAssetsPath();

        TerrainDetailMeshData* detailMesh = new TerrainDetailMeshData(this);

        if (!Engine::getSingleton()->getIsRuntimeMode())
        {
            detailMesh->setName(sDetailMesh.name);
        }
        else
        {
            detailMesh->name = sDetailMesh.name;
            for (auto& md : sDetailMesh.meshModels)
            {
                TerrainDetailMeshData::ModelMeshData mdt;
                mdt.transform = md.transform.getValue();
                for (auto& mat : md.materials)
                {
                    Material* m = Material::load(Engine::getSingleton()->getAssetsPath(), mat);
                    if (m != nullptr) mdt.materials.push_back(m);
                }

                if (!IO::FileExists(Engine::getSingleton()->getLibraryPath() + md.mesh))
                {
                    if (IO::FileExists(location + detailMesh->name))
                        Model3DLoader::cache3DModel(location, detailMesh->name, nullptr);
                }

                Mesh* mesh = Mesh::load(Engine::getSingleton()->getLibraryPath(), md.mesh);
                if (mesh != nullptr)
                    mdt.mesh = mesh;
                detailMesh->meshList.push_back(mdt);
            }
            detailMesh->updateBounds();
            detailMesh->updateImpostorTexture();
        }
        detailMesh->setMinScale(sDetailMesh.minScale);
        detailMesh->setMaxScale(sDetailMesh.maxScale);
        for (auto jt = sDetailMesh.meshes.begin(); jt != sDetailMesh.meshes.end(); ++jt)
        {
            STerrainDetailMeshInfoData& sMesh = *jt;
            detailMesh->addMesh(sMesh.position.getValue(), sMesh.rotation, sMesh.scale);
        }
        for (auto b = detailMesh->getBatches().begin(); b != detailMesh->getBatches().end(); ++b)
            (*b)->update();

        return detailMesh;
    }

    STerrainData Terrain::serialize()
    {
        STerrainData data;

        if (material->getName() != "StandardTerrain")
            data.material = material->getName();
        data.size = size;
        data.worldSize = worldSize;
        data.detailMeshesCastShadows = detailMeshesCastShadows;
        data.detailMeshesDrawDistance = detailMeshesDrawDistance;
        data.drawDetailMeshes = drawDetailMeshes;
        data.drawGrass = drawGrass;
        data.drawTrees = drawTrees;
        data.terrainCastShadows = castShadows;
        data.grassCastShadows = grassCastShadows;
        data.grassDrawDistance = grassDrawDistance;
        data.treeDrawDistance = treeDrawDistance;
        data.treeImpostorStartDistance = treeImpostorStartDistance;
        data.treesCastShadows = treesCastShadows;

        uint32_t num = size * size;
        data.heightMap.resize(num);
        memcpy(&data.heightMap[0], heightMap, num * sizeof(float));

        //Splat textures
        for (auto it = splatTextures.begin(); it != splatTextures.end(); ++it)
        {
            Texture* splatTexture = *it;
            if (splatTexture->getSize() == 0)
                continue;
            STerrainSplatTexture sSplatTexture;
            sSplatTexture.pixels.resize(splatTexture->getSize());
            memcpy(&sSplatTexture.pixels[0], splatTexture->getData(), splatTexture->getSize());
            data.splatTextures.push_back(sSplatTexture);
        }

        //Textures
        for (auto it = textureList.begin(); it != textureList.end(); ++it)
        {
            TerrainTextureData& texture = *it;
            STerrainTextureData sTexture;
            if (texture.diffuseTexture != nullptr)
                sTexture.diffuseTexture = texture.diffuseTexture->getName();
            if (texture.normalTexture != nullptr)
                sTexture.normalTexture = texture.normalTexture->getName();
            sTexture.worldSize = texture.worldSize;

            data.textureList.push_back(sTexture);
        }

        //Trees
        for (auto it = treeList.begin(); it != treeList.end(); ++it)
        {
            TerrainTreeData* tree = *it;
            STerrainTreeData sTree = serializeTree(tree);
            data.treeList.push_back(sTree);
        }

        //Detail meshes
        for (auto it = detailMeshList.begin(); it != detailMeshList.end(); ++it)
        {
            TerrainDetailMeshData* detailMesh = *it;
            STerrainDetailMeshData sDetailMesh = serializeDetailMesh(detailMesh);
            data.detailMeshList.push_back(sDetailMesh);
        }

        //Grass
        for (auto it = grassList.begin(); it != grassList.end(); ++it)
        {
            TerrainGrassData* grass = *it;
            STerrainGrassData sGrass = serializeGrass(grass);
            data.grassList.push_back(sGrass);
        }

        return data;
    }

    void Terrain::deserialize(STerrainData data, std::string path)
    {
        filePath = path;

        destroy();

        if (!data.material.empty())
        {
            Material* mat = Material::load(Engine::getSingleton()->getAssetsPath(), data.material);
            if (mat != nullptr && mat->isLoaded())
                setMaterial(mat);
        }

        size = data.size;
        worldSize = data.worldSize;
        detailMeshesCastShadows = data.detailMeshesCastShadows;
        detailMeshesDrawDistance = data.detailMeshesDrawDistance;
        drawDetailMeshes = data.drawDetailMeshes;
        drawGrass = data.drawGrass;
        drawTrees = data.drawTrees;
        castShadows = data.terrainCastShadows;
        grassCastShadows = data.grassCastShadows;
        grassDrawDistance = data.grassDrawDistance;
        treeDrawDistance = data.treeDrawDistance;
        treeImpostorStartDistance = data.treeImpostorStartDistance;
        treesCastShadows = data.treesCastShadows;

        create();

        uint32_t num = size * size;
        memcpy(heightMap, &data.heightMap[0], num * sizeof(float));

        //Splat textures
        int i = 0;
        for (auto it = data.splatTextures.begin(); it != data.splatTextures.end(); ++it, ++i)
        {
            STerrainSplatTexture& sSplatTexture = *it;
            if (sSplatTexture.pixels.size() == 0) continue;
            if (i < splatTextures.size())
            {
                memcpy(splatTextures[i]->getData(), &sSplatTexture.pixels[0], splatTextures[i]->getSize());
            }
            sSplatTexture.pixels.clear();
            splatTextures[i]->updateTexture();
        }

        //Textures
        for (auto it = data.textureList.begin(); it != data.textureList.end(); ++it)
        {
            STerrainTextureData& sTexture = *it;
            TerrainTextureData texture;
            if (!sTexture.diffuseTexture.empty())
                texture.diffuseTexture = Texture::load(Engine::getSingleton()->getAssetsPath(), sTexture.diffuseTexture, true, Texture::CompressionMethod::Default);
            if (!sTexture.diffuseTexture.empty())
                texture.normalTexture = Texture::load(Engine::getSingleton()->getAssetsPath(), sTexture.normalTexture, true, Texture::CompressionMethod::Default);
            texture.worldSize = sTexture.worldSize;

            textureList.push_back(texture);
        }

        //Trees
        for (auto it = data.treeList.begin(); it != data.treeList.end(); ++it)
        {
            STerrainTreeData& sTree = *it;
            TerrainTreeData* tree = deserializeTree(sTree);
            treeList.push_back(tree);
        }

        //Detail meshes
        for (auto it = data.detailMeshList.begin(); it != data.detailMeshList.end(); ++it)
        {
            STerrainDetailMeshData sDetailMesh = *it;
            TerrainDetailMeshData* detailMesh = deserializeDetailMesh(sDetailMesh);
            detailMeshList.push_back(detailMesh);
        }

        //Grass
        for (auto it = data.grassList.begin(); it != data.grassList.end(); ++it)
        {
            STerrainGrassData sGrass = *it;
            TerrainGrassData* grass = deserializeGrass(sGrass);
            grassList.push_back(grass);
        }

        prevTransform = glm::mat4x4(FLT_MAX);
        isDirty = true;

        //updateTreesTransforms();
    }

    void Terrain::save()
    {
        if (!filePath.empty())
            save(filePath);
    }

    void Terrain::save(std::string path)
    {
        filePath = path;

        STerrainData data = serialize();

        std::string fullPath = Engine::getSingleton()->getAssetsPath() + filePath;
        std::ofstream ofs(fullPath, std::ios::binary);
        BinarySerializer s;
        s.serialize(&ofs, &data, Terrain::COMPONENT_TYPE);
        ofs.close();

        /*data.heightMap.clear();
        for (auto it = data.splatTextures.begin(); it != data.splatTextures.end(); ++it)
            it->data.clear();*/
    }

    void Terrain::load(std::string path)
    {
        filePath = path;

        std::string location = Engine::getSingleton()->getAssetsPath();
        std::string fullPath = location + filePath;

        if (IO::isDir(location))
        {
            if (!IO::FileExists(fullPath))
            {
                Debug::log("[" + filePath + "] Error loading terrain: file does not exists", Debug::DbgColorRed);
                return;
            }
        }
        else
        {
            zip_t* arch = Engine::getSingleton()->getZipArchive(location);
            if (!ZipHelper::isFileInZip(arch, filePath))
            {
                Debug::log("[" + fullPath + "] Error loading terrain: file does not exists", Debug::DbgColorRed);
                return;
            }
        }

        STerrainData data;

        if (IO::isDir(location))
        {
            std::ifstream ofs(fullPath, std::ios::binary);
            try
            {
                BinarySerializer s;
                s.deserialize(&ofs, &data, Terrain::COMPONENT_TYPE);
            }
            catch (std::exception e)
            {
                Debug::log("[" + fullPath + "] Error loading terrain: " + e.what(), Debug::DbgColorRed);
            }
            ofs.close();
        }
        else
        {
            zip_t* arch = Engine::getSingleton()->getZipArchive(location);

            int sz = 0;
            char* buffer = ZipHelper::readFileFromZip(arch, filePath, sz);
            boost::iostreams::stream<boost::iostreams::array_source> is(buffer, sz);
            try
            {
                BinarySerializer s;
                s.deserialize(&is, &data, Terrain::COMPONENT_TYPE);
            }
            catch (std::exception e)
            {
                Debug::log("[" + fullPath + "] Error loading terrain: " + e.what(), Debug::DbgColorRed);
            }
            is.close();
            delete[] buffer;
        }

        deserialize(data, path);
    }

    //----------------------RENDER----------------------//

    bool Terrain::isStatic()
    {
        return gameObject->getLightingStatic();
    }

    void Terrain::onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback)
    {
        if (gameObject == nullptr)
            return;

        if (!gameObject->getActive())
            return;

        if (!getEnabled())
            return;

        if (isDirty)
        {
            updateTerrain();
            isDirty = false;
        }

        const bgfx::Caps* caps = bgfx::getCaps();

        glm::mat4x4 trans = transform->getTransformMatrix();
        glm::mat3x3 normalMatrix = glm::identity<glm::mat3x3>();

        if (program.idx == bgfx::kInvalidHandle)
        {
            normalMatrix = trans;
        }

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

                if (program.idx == bgfx::kInvalidHandle)
                {
                    if (pv != nullptr)
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
                    bgfx::UniformHandle tex = Renderer::getSingleton()->u_albedoMap;
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

                //Bind terrain textures
                int texStage = 0;
                glm::vec4 texSizes[MAX_TERRAIN_TEXTURES];
                for (auto tt = textureList.begin(); tt != textureList.end(); ++tt, ++texStage)
                {
                    if (texStage > caps->limits.maxTextureSamplers)
                        break;

                    TerrainTextureData& texData = *tt;

                    bgfx::TextureHandle diffuseHandle = Texture::getNullTexture()->getHandle();
                    if (texData.diffuseTexture != nullptr)
                        diffuseHandle = texData.diffuseTexture->getHandle();

                    bgfx::TextureHandle normalHandle = Texture::getNullTexture()->getHandle();
                    if (texData.normalTexture != nullptr)
                        normalHandle = texData.normalTexture->getHandle();

                    bgfx::setTexture(texStage, textureDiffuseHandles[texStage], diffuseHandle);
                    bgfx::setTexture(texStage + MAX_TERRAIN_TEXTURES, textureNormalHandles[texStage], normalHandle);

                    texSizes[texStage] = glm::vec4((texData.worldSize / size) * 2.0f * getScale());
                }

                texStage = 0;
                for (auto tt = splatTextures.begin(); tt != splatTextures.end(); ++tt, ++texStage)
                {
                    Texture* tex = *tt;
                    bgfx::setTexture(texStage + 10, textureSplatHandles[texStage], tex->getHandle());
                }

                bgfx::setUniform(textureSizesHandle, &texSizes, MAX_TERRAIN_TEXTURES);
                bgfx::setUniform(textureCountHandle, glm::value_ptr(glm::vec4(textureList.size(), 0, 0, 0)), 1);

                //Bind system uniforms
                bgfx::setUniform(Renderer::uNormalMatrix, glm::value_ptr(normalMatrix), 1);
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

    void TerrainGrassData::Batch::onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback)
    {
        if (parent == nullptr)
            return;

        if (parent->terrain->getGameObject() == nullptr)
            return;

        if (!parent->terrain->getGameObject()->getActive())
            return;

        if (!parent->terrain->getEnabled())
            return;

        if (!parent->terrain->getDrawGrass())
            return;

        if (camera != nullptr)
        {
            AxisAlignedBox bounds = getBounds();

            if (bounds.isNull())
                return;

            if (!bounds.isInfinite())
            {
                bool visible = Mathf::intersects(camera->getTransform()->getPosition(), parent->terrain->getGrassDrawDistance(), bounds);
                if (!visible)
                    return;
            }
        }

        glm::mat4x4 trans = parent->terrain->getGameObject()->getTransform()->getTransformMatrix();
        glm::mat3x3 normalMatrix = glm::identity<glm::mat3x3>();

        if (program.idx == bgfx::kInvalidHandle)
        {
            normalMatrix = trans;
        }

        Shader* shader = nullptr;

        if (parent->material != nullptr && parent->material->isLoaded())
            shader = parent->material->getShader();

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
            if (parent->material != nullptr && parent->material->isLoaded() && pass != nullptr)
                pv = pass->getProgramVariant(parent->material->getDefinesStringHash());

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

                if (program.idx == bgfx::kInvalidHandle)
                {
                    if (pv != nullptr)
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
                        parent->material->submitUniforms(pv, camera);
                }
                else
                {
                    bgfx::UniformHandle tex = Renderer::getSingleton()->getAlbedoMapUniform();
                    auto& uniforms = parent->material->getUniforms();
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

    void TerrainTreeData::TreeMesh::onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback)
    {
        if (parent == nullptr)
            return;

        Terrain* terrain = parent->terrain;

        if (terrain->getGameObject() == nullptr)
            return;

        if (!terrain->getGameObject()->getActive())
            return;

        if (!terrain->getEnabled())
            return;

        if (!terrain->getDrawTrees())
            return;

        glm::mat4x4 trans = getTransform();
        glm::mat3x3 normalMatrix = glm::identity<glm::mat3x3>();

        if (program.idx == bgfx::kInvalidHandle)
            normalMatrix = trans;

        if (camera != nullptr)
        {
            Transform* terrTrans = terrain->getGameObject()->getTransform();
            float dist = glm::distance(camera->getTransform()->getPosition(), glm::vec3(trans[3]));

            bool visible = dist <= terrain->getTreeDrawDistance();
            if (!visible)
                return;

            if (dist > terrain->treeImpostorStartDistance)
            {
                if (program.idx == bgfx::kInvalidHandle && static_cast<RenderMode>(renderMode) == RenderMode::Forward)
                {
                    if (6 == bgfx::getAvailTransientVertexBuffer(6, VertexLayouts::primitiveVertexLayout))
                    {
                        bgfx::TransientVertexBuffer vb;
                        bgfx::allocTransientVertexBuffer(&vb, 6, VertexLayouts::primitiveVertexLayout);
                        PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

                        AxisAlignedBox parentBounds = parent->getBounds();

                        float zz = 0;
                        float szX = parentBounds.getSize().y * 0.5f;
                        float szY = parentBounds.getSize().y;

                        vertex[0].pos = glm::vec3(-szX, 0, zz);
                        vertex[0].uv = glm::vec2(0, 0);
                        vertex[1].pos = glm::vec3(-szX, szY, zz);
                        vertex[1].uv = glm::vec2(0, 1);
                        vertex[2].pos = glm::vec3(szX, szY, zz);
                        vertex[2].uv = glm::vec2(1, 1);
                        //--
                        vertex[3].pos = glm::vec3(szX, szY, zz);
                        vertex[3].uv = glm::vec2(1, 1);
                        vertex[4].pos = glm::vec3(szX, 0, zz);
                        vertex[4].uv = glm::vec2(1, 0);
                        vertex[5].pos = glm::vec3(-szX, 0, zz);
                        vertex[5].uv = glm::vec2(0, 0);

                        glm::mat4x4 trans2 = glm::translate(glm::identity<glm::mat4x4>(), position);
                        trans2 = glm::scale(trans2, glm::vec3(scale));
                        trans2 = terrTrans->getTransformMatrix() * trans2;

                        Transform* ct = camera->getTransform();
                        glm::mat4x4 mtx = trans2;
                        glm::highp_quat rot = glm::quatLookAt(-ct->getForward(), ct->getUp());
                        mtx = mtx * glm::mat4_cast(glm::inverse(terrTrans->getRotation()) * rot);

                        glm::vec3 nrm = ct->getPosition() - position;

                        bool rendered = false;
                        //Lighting pass
                        std::vector<Light*>& lights = Renderer::getSingleton()->getLights();
                        for (int iter = 0; iter < lights.size(); ++iter)
                        {
                            Light* light = lights[iter];

                            if (light->getLightType() != LightType::Directional)
                                continue;

                            if (!light->submitUniforms())
                                continue;

                            bgfx::setVertexBuffer(0, &vb);
                            bgfx::setTransform(glm::value_ptr(mtx), 1);
                            //bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LEQUAL | BGFX_STATE_CULL_CW);
                            bgfx::setState((state &= ~(BGFX_STATE_WRITE_A)));

                            if (bgfx::isValid(parent->impostorTexture))
                                bgfx::setTexture(0, Terrain::albedoTextureHandle, parent->impostorTexture);

                            //Renderer::getSingleton()->setSystemUniforms(camera);
                            bgfx::setUniform(Renderer::getNormalUniform(), glm::value_ptr(glm::vec4(nrm.x, nrm.y, nrm.z, 1.0f)), 1);

                            if (preRenderCallback != nullptr)
                                preRenderCallback();

                            bgfx::submit(view, Renderer::getSingleton()->getTerrainTreeBillboardLightProgram());

                            rendered = true;
                            break;
                        }

                        if (!rendered)
                        {
                            //Ambient pass
                            bgfx::setVertexBuffer(0, &vb);
                            bgfx::setTransform(glm::value_ptr(mtx), 1);
                            //bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LEQUAL | BGFX_STATE_CULL_CW);
                            bgfx::setState((state &= ~(BGFX_STATE_WRITE_A)));

                            if (bgfx::isValid(parent->impostorTexture))
                                bgfx::setTexture(0, Terrain::albedoTextureHandle, parent->impostorTexture);

                            //Renderer::getSingleton()->setSystemUniforms(camera);
                            bgfx::setUniform(Renderer::getNormalUniform(), glm::value_ptr(glm::vec4(nrm.x, nrm.y, nrm.z, 1.0f)), 1);

                            if (preRenderCallback != nullptr)
                                preRenderCallback();

                            bgfx::submit(view, Renderer::getSingleton()->getTerrainTreeBillboardProgram());
                        }
                    }
                }

                return;
            }
        }

        for (auto it = parent->meshList.begin(); it != parent->meshList.end(); ++it)
        {
            ModelMeshData& data = *it;

            for (int i = 0; i < data.mesh->getSubMeshCount(); ++i)
            {
                SubMesh* subMesh = data.mesh->getSubMesh(i);
                Material* material = nullptr;
                if (i < data.materials.size())
                    material = data.materials[i];

                Shader* shader = nullptr;

                if (material != nullptr && material->isLoaded())
                    shader = material->getShader();

                if (program.idx == bgfx::kInvalidHandle)
                {
                    if (shader == nullptr || !shader->isLoaded())
                        continue;

                    if (renderMode != static_cast<int>(shader->getRenderMode()))
                        continue;
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

                        if (program.idx == bgfx::kInvalidHandle)
                        {
                            if (pv != nullptr)
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
                        bgfx::setVertexBuffer(0, subMesh->getVertexBufferHandle());
                        bgfx::setIndexBuffer(subMesh->getIndexBufferHandle());

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
        }
    }

    void TerrainDetailMeshData::Batch::onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback)
    {
        if (parent == nullptr)
            return;

        Terrain* terrain = parent->terrain;

        if (terrain->getGameObject() == nullptr)
            return;

        if (!terrain->getGameObject()->getActive())
            return;

        if (!terrain->getEnabled())
            return;

        if (!terrain->getDrawDetailMeshes())
            return;

        glm::mat4x4 trans = terrain->getGameObject()->getTransform()->getTransformMatrix();
        glm::mat3x3 normalMatrix = glm::identity<glm::mat3x3>();

        if (program.idx == bgfx::kInvalidHandle)
            normalMatrix = trans;

        if (camera != nullptr)
        {
            AxisAlignedBox bounds = getBounds();

            if (bounds.isNull())
                return;

            if (!bounds.isInfinite())
            {
                bool visible = Mathf::intersects(camera->getTransform()->getPosition(), terrain->getDetailMeshesDrawDistance(), bounds);
                if (!visible)
                    return;
            }
        }

        int mesh = 0;
        for (auto jt = subBatches.begin(); jt != subBatches.end(); ++jt, ++mesh)
        {
            int idx = 0;
            for (auto it = jt->begin(); it != jt->end(); ++it, ++idx)
            {
                SubBatch& subBatch = *it;
                Shader* shader = nullptr;

                Material* material = parent->meshList[mesh].materials[idx];
                if (material != nullptr && material->isLoaded())
                    shader = material->getShader();

                if (program.idx == bgfx::kInvalidHandle)
                {
                    if (shader == nullptr || !shader->isLoaded())
                        continue;

                    if (renderMode != static_cast<int>(shader->getRenderMode()))
                        continue;
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

                        if (program.idx == bgfx::kInvalidHandle)
                        {
                            if (pv != nullptr)
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
                        bgfx::setVertexBuffer(0, subBatch.vbh);
                        bgfx::setIndexBuffer(subBatch.ibh);

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
        }
    }
}