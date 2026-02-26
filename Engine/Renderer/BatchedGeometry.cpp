#include "BatchedGeometry.h"

#include "../glm/gtc/type_ptr.hpp"

#include "../Assets/Material.h"
#include "../Assets/Shader.h"
#include "../Assets/Mesh.h"
#include "../Assets/Texture.h"
#include "../Assets/Scene.h"

#include "../Components/MeshRenderer.h"
#include "../Components/Transform.h"
#include "../Components/Camera.h"
#include "../Components/Light.h"

#include "../Core/Engine.h"
#include "../Core/GameObject.h"

#include "../Renderer/Renderer.h"
#include "../Renderer/VertexLayouts.h"

#include "../Classes/IO.h"
#include "../Classes/md5.h"
#include "../Classes/xatlas.h"

#include "../Serialization/Scene/SBatchedGeometry.h"

namespace GX
{
    BatchedGeometry BatchedGeometry::singleton;

    //Batch

    BatchedGeometry::Batch::Batch()
    {

    }

    BatchedGeometry::Batch::~Batch()
    {
        unload();
    }

    void BatchedGeometry::Batch::load()
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

        //vertexBuffer.clear();
        //indexBuffer.clear();

        attach();
    }

    void BatchedGeometry::Batch::unload()
    {
        if (bgfx::isValid(m_vbh))
            bgfx::destroy(m_vbh);

        if (bgfx::isValid(m_ibh))
            bgfx::destroy(m_ibh);

        m_vbh = { bgfx::kInvalidHandle };
        m_ibh = { bgfx::kInvalidHandle };

        vertexBuffer.clear();
        indexBuffer.clear();

        cachedAAB = AxisAlignedBox::BOX_NULL;

        transparent = false;
        layer = 0;

        if (lightmap != nullptr)
            lightmap->unload();

        lightmap = nullptr;

        detach();
    }

    AxisAlignedBox BatchedGeometry::Batch::getBounds(bool world)
    {
        return cachedAAB;
    }

    bool BatchedGeometry::Batch::checkCullingMask(LayerMask& mask)
    {
        return mask.getLayer(layer);
    }

    void BatchedGeometry::Batch::onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback)
    {
        Shader* shader = Renderer::getDefaultShader();

        if (material != nullptr && material->isLoaded() && material->getShader() != nullptr)
            shader = material->getShader();

        if (program.idx == bgfx::kInvalidHandle)
        {
            if (shader == nullptr || !shader->isLoaded())
                return;

            if (renderMode != static_cast<int>(shader->getRenderMode()))
                return;
        }

        //

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

                glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
                glm::mat4x4 invMtx = glm::inverse(mtx);

                bgfx::setTransform(glm::value_ptr(mtx));

                // Set vertex and index buffer.
                bgfx::setVertexBuffer(0, m_vbh);
                bgfx::setIndexBuffer(m_ibh);

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
                    bool textureWasSet = false;

                    if (pv != nullptr)
                    {
                        auto& uniforms = material->getUniforms();

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
                    }

                    if (!textureWasSet)
                    {
                        Texture* nullTexture = Texture::getNullTexture();
                        bgfx::setTexture(0, tex, nullTexture->getHandle());
                    }
                }

                glm::mat3x3 nrmMtx = mtx;

                //Bind system uniforms
                bgfx::setUniform(Renderer::getNormalMatrixUniform(), glm::value_ptr(nrmMtx), 1);
                bgfx::setUniform(Renderer::getGpuSkinningUniform(), glm::value_ptr(glm::vec4(0.0f)), 1);
                bgfx::setUniform(Renderer::getInvModelUniform(), glm::value_ptr(invMtx), 1);

                bool useLightmaps = lightmap != nullptr && !parent->_needRebuild;
                bgfx::setUniform(Renderer::getHasLightmapUniform(), glm::value_ptr(glm::vec4(useLightmaps ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f)), 1);

                if (useLightmaps)
                    bgfx::setTexture(8, Renderer::getLightmapUniform(), lightmap->getHandle());

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

    void BatchedGeometry::Batch::reloadLightmap()
    {
        if (lightmap != nullptr)
            lightmap->unload();

        lightmap = nullptr;

        if (!lightingStatic)
            return;

        std::string loadedScene = Scene::getLoadedScene();
        std::string dirPath = IO::GetFilePath(loadedScene) + IO::GetFileName(loadedScene) + "/Lightmaps/";
        std::string fileName = dirPath + guid + ".jpg";

        std::string location = Engine::getSingleton()->getAssetsPath();

        Texture* _lightmap = Texture::load(location, fileName, true, Texture::CompressionMethod::Default, false, false);
        if (_lightmap != nullptr && _lightmap->isLoaded())
            lightmap = _lightmap;
    }

    //Batched geometry

    BatchedGeometry::BatchedGeometry()
    {

    }

    BatchedGeometry::~BatchedGeometry()
    {

    }

    void BatchedGeometry::rebuild(bool buildUvs)
    {
        //Clear
        clear();

        //Build
        auto objects = Engine::getSingleton()->getGameObjects();

        int maxBatchSize = 64;
        int numBatches = 0;

        for (auto& obj : objects)
        {
            if (!obj->getActive())
                continue;

            if (obj->getBatchingStatic())
            {
                MeshRenderer* rend = (MeshRenderer*)obj->getComponent(MeshRenderer::COMPONENT_TYPE);

                if (rend != nullptr)
                {
                    if (!rend->getEnabled())
                        continue;

                    Transform* trans = obj->getTransform();

                    glm::mat4x4 mtx = trans->getTransformMatrix();
                    glm::quat nrmMtx = trans->getRotation();

                    bool transparent = rend->isTransparent();

                    Mesh* mesh = rend->getMesh();

                    if (mesh != nullptr)
                    {
                        for (int i = 0; i < mesh->getSubMeshCount(); ++i)
                        {
                            SubMesh* subMesh = mesh->getSubMesh(i);
                            Material* mat = rend->getSharedMaterial(i);

                            if (mat == nullptr || !mat->isLoaded())
                                continue;

                            auto& vb = subMesh->getVertexBuffer();
                            auto& ib = subMesh->getIndexBuffer();

                            Batch* batch = nullptr;
                            auto it = std::find_if(batches.begin(), batches.end(), [=](Batch* b) -> bool
                                {
                                    bool match = b->getMaterial() == mat &&
                                        b->getLayer() == obj->getLayer() &&
                                        b->getCastShadows() == rend->getCastShadows() &&
                                        b->getLightingStatic() == obj->getLightingStatic() &&
                                        b->getUseLightmapUVs() == subMesh->getUseLightmapUVs() &&
                                        b->getLightmapSize() == rend->getLightmapSize() &&
                                        b->getNumMeshes() < maxBatchSize;

                                    return match;
                                }
                            );

                            if (it != batches.end())
                            {
                                batch = *it;
                            }
                            else
                            {
                                batch = new Batch();
                                batch->setMaterial(mat);
                                batch->setCastShadows(rend->getCastShadows());
                                batch->parent = this;
                                batch->layer = obj->getLayer();
                                batch->transparent = transparent;
                                batch->lightingStatic = obj->getLightingStatic();
                                batch->useLightmapUVs = subMesh->getUseLightmapUVs();
                                batch->lightmapSize = rend->getLightmapSize();
                                batch->index = numBatches;

                                batches.push_back(batch);

                                ++numBatches;
                            }

                            batch->guid = md5(batch->guid + obj->getGuid() + "_" + std::to_string(i));
                            batch->numMeshes += 1;

                            auto& batchVb = batch->getVertexBuffer();
                            auto& batchIb = batch->getIndexBuffer();

                            int _v = batchVb.size();
                            int _i = batchIb.size();

                            batchVb.resize(batchVb.size() + vb.size());
                            batchIb.resize(batchIb.size() + ib.size());

                            for (auto& id : ib)
                            {
                                batchIb[_i] = _v + id;

                                ++_i;
                            }

                            for (auto& v : vb)
                            {
                                glm::vec4 pos = mtx * glm::vec4(v.position, 1.0f);
                                glm::vec3 nrm = nrmMtx * v.normal;
                                glm::vec3 tan = nrmMtx * v.tangent;
                                glm::vec3 bitan = nrmMtx * v.bitangent;

                                batchVb[_v] = v;
                                batchVb[_v].position = pos;
                                batchVb[_v].normal = nrm;
                                batchVb[_v].tangent = tan;
                                batchVb[_v].bitangent = bitan;

                                ++_v;
                            }

                            batch->cachedAAB.merge(rend->getBounds());
                        }
                    }
                }
            }
        }

        for (auto* batch : batches)
        {
            if (buildUvs)
            {
                if (batch->lightingStatic && batch->useLightmapUVs)
                {
                    auto batchVb = batch->getVertexBuffer();
                    auto batchIb = batch->getIndexBuffer();

                    std::vector<glm::vec3> vertices;
                    std::vector<glm::vec3> normals;
                    std::vector<glm::vec2> uvs;

                    vertices.resize(batchIb.size());
                    normals.resize(batchIb.size());
                    uvs.resize(batchIb.size());

                    for (int k = 0; k < batchVb.size(); ++k)
                    {
                        vertices[k] = batchVb[k].position;
                        normals[k] = batchVb[k].normal;
                        uvs[k] = batchVb[k].texcoord0;
                    }

                    //Gen lightmap UVs
                    xatlas::Atlas* atlas = xatlas::Create();
                    xatlas::MeshDecl mesh;
                    mesh.indexCount = batchIb.size();
                    mesh.indexFormat = xatlas::IndexFormat::UInt32;
                    mesh.indexData = batchIb.data();

                    mesh.vertexCount = vertices.size();
                    mesh.vertexPositionData = vertices.data();
                    mesh.vertexPositionStride = sizeof(float) * 3;
                    mesh.vertexNormalData = normals.data();
                    mesh.vertexNormalStride = sizeof(float) * 3;
                    mesh.vertexUvData = uvs.data();
                    mesh.vertexUvStride = sizeof(float) * 2;

                    xatlas::ChartOptions chartOptions = xatlas::ChartOptions();
                    xatlas::PackOptions packOptions = xatlas::PackOptions();
                    packOptions.padding = 4.0f;

                    xatlas::AddMesh(atlas, mesh);
                    xatlas::Generate(atlas, chartOptions, packOptions);

                    std::vector<int> vertIndexes;
                    std::vector<glm::vec2> lightmapUVs;

                    if (atlas->meshCount > 0)
                    {
                        xatlas::Mesh output_mesh = atlas->meshes[0];

                        batchIb.resize(output_mesh.indexCount);
                        batchVb.resize(output_mesh.vertexCount);

                        lightmapUVs.resize(output_mesh.vertexCount);
                        vertIndexes.resize(output_mesh.vertexCount);

                        for (int j = 0; j < output_mesh.indexCount; ++j)
                        {
                            int idx = output_mesh.indexArray[j];
                            batchIb[j] = idx;
                        }

                        for (int j = 0; j < output_mesh.vertexCount; ++j)
                        {
                            xatlas::Vertex vertex = output_mesh.vertexArray[j];

                            float x = vertex.uv[0] / atlas->width;
                            float y = vertex.uv[1] / atlas->height;

                            vertIndexes[j] = vertex.xref;
                            lightmapUVs[j] = glm::vec2(x, y);
                        }

                        for (int jj = 0; jj < vertIndexes.size(); ++jj)
                        {
                            uint32_t j = vertIndexes[jj];

                            batchVb[jj] = batch->getVertexBuffer()[j];
                            batchVb[jj].texcoord1 = lightmapUVs[jj];
                        }

                        batch->vertexBuffer = batchVb;
                        batch->indexBuffer = batchIb;
                    }

                    xatlas::Destroy(atlas);

                    batchVb.clear();
                    batchIb.clear();

                    lightmapUVs.clear();
                    vertIndexes.clear();

                    vertices.clear();
                    normals.clear();
                    uvs.clear();
                }

                batch->reloadLightmap();
            }

            batch->load();
        }

        if (batches.size() > 0)
            _needRebuild = !buildUvs;
        else
            _needRebuild = false;
    }

    void BatchedGeometry::reloadLightmaps()
    {
        for (auto* batch : batches)
            batch->reloadLightmap();
    }

    void BatchedGeometry::clear()
    {
        for (auto* batch : batches)
        {
            delete batch;
            Renderer::getSingleton()->frame();
        }

        batches.clear();
    }

    bool BatchedGeometry::loadFromFile(std::string location, std::string name)
    {
        clear();

        SBatchedGeometry data;
        bool loaded = data.load(location, name);

        if (!loaded)
            return false;

        for (auto& _batch : data.batches)
        {
            Material* mat = Material::load(Engine::getSingleton()->getAssetsPath(), _batch.material);

            Batch* batch = new Batch();

            batch->parent = this;

            batch->material = mat;
            batch->guid = _batch.guid;

            batch->transparent = _batch.transparent;
            batch->lightingStatic = _batch.lightingStatic;
            batch->useLightmapUVs = _batch.useLightmapUVs;
            batch->setCastShadows(_batch.castShadows);
            batch->lightmapSize = _batch.lightmapSize;

            batch->layer = _batch.layer;
            batch->index = _batch.index;

            batch->vertexBuffer.resize(_batch.vertexBuffer.size());

            for (int i = 0; i < _batch.vertexBuffer.size(); ++i)
            {
                SBatchVertexBuffer& v = _batch.vertexBuffer[i];

                VertexBuffer vb;
                vb.bitangent = v.bitangent.getValue();
                vb.color = Color::packABGR(v.color.getValue());
                vb.normal = v.normal.getValue();
                vb.position = v.position.getValue();
                vb.tangent = v.tangent.getValue();
                vb.texcoord0 = v.texcoord0.getValue();
                vb.texcoord1 = v.texcoord1.getValue();

                batch->vertexBuffer[i] = vb;

                batch->cachedAAB.merge(vb.position);
            }

            batch->indexBuffer = _batch.indexBuffer;

            batches.push_back(batch);
        }

        for (auto* batch : batches)
        {
            batch->load();
            batch->reloadLightmap();
        }

        data.clear();

        _needRebuild = false;

        return true;
    }

    void BatchedGeometry::saveToFile(std::string location, std::string name)
    {
        if (batches.size() == 0)
            return;

        SBatchedGeometry data;

        for (auto* batch : batches)
        {
            std::string mat = "";
            if (batch->material != nullptr)
                mat = batch->material->getName();

            SBatch _batch;

            _batch.material = mat;
            _batch.guid = batch->guid;

            _batch.transparent = batch->transparent;
            _batch.lightingStatic = batch->lightingStatic;
            _batch.useLightmapUVs = batch->useLightmapUVs;
            _batch.castShadows = batch->getCastShadows();
            _batch.lightmapSize = batch->lightmapSize;

            _batch.layer = batch->layer;
            _batch.index = batch->index;

            _batch.vertexBuffer.resize(batch->vertexBuffer.size());

            for (int i = 0; i < batch->vertexBuffer.size(); ++i)
            {
                VertexBuffer& vb = batch->vertexBuffer[i];
                SBatchVertexBuffer v;

                v.bitangent = vb.bitangent;
                v.color = Color::unpackABGR(vb.color);
                v.normal = vb.normal;
                v.position = vb.position;
                v.tangent = vb.tangent;
                v.texcoord0 = vb.texcoord0;
                v.texcoord1 = vb.texcoord1;

                _batch.vertexBuffer[i] = v;
            }

            _batch.indexBuffer = batch->indexBuffer;

            data.batches.push_back(_batch);
        }

        data.save(location, name);

        data.clear();
    }
}