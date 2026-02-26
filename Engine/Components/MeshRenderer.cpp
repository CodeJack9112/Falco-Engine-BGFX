#include "MeshRenderer.h"

#include "glm/vec4.hpp"
#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/matrix_operation.hpp"

#include "Camera.h"
#include "Transform.h"
#include "Light.h"
#include "../Classes/brtshaderc.h"
#include "../Core/Engine.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Frustum.h"
#include "../Core/GameObject.h"
#include "../Assets/Texture.h"
#include "../Assets/Material.h"
#include "../Assets/Mesh.h"
#include "../Assets/Shader.h"
#include "../Assets/Scene.h"
#include "../Core/APIManager.h"
#include "../Classes/GUIDGenerator.h"
#include "../Classes/IO.h"
#include "../Classes/Hash.h"

#include "../OcclusionCulling/CullingThreadpool.h"

namespace GX
{
    std::string MeshRenderer::COMPONENT_TYPE = "MeshRenderer";

    uint64_t u_lightColorNameHash = Hash::getHash("u_lightColor");

    MeshRenderer::MeshRenderer() : Component(APIManager::getSingleton()->meshrenderer_class), Renderable()
    {
        
    }

    MeshRenderer::~MeshRenderer()
    {
        setMesh(nullptr);
    }

    std::string MeshRenderer::getComponentType()
    {
        return COMPONENT_TYPE;
    }

    void MeshRenderer::onSceneLoaded()
    {
        if (!rootObjectGuid.empty())
            rootObject = Engine::getSingleton()->getGameObject(rootObjectGuid);

        resetBoneLinks();

        materials.resize(sharedMaterials.size());
        for (int i = 0; i < sharedMaterials.size(); ++i)
            materials[i] = nullptr;

        reloadLightmaps();
    }

    Component* MeshRenderer::onClone()
    {
        MeshRenderer* newComponent = new MeshRenderer();
        newComponent->enabled = enabled;
        newComponent->mesh = mesh;
        newComponent->sharedMaterials = sharedMaterials;
        newComponent->boneLinks = boneLinks;
        newComponent->rootObjectGuid = rootObjectGuid;
        newComponent->rootObject = rootObject;
        newComponent->castShadows = castShadows;
        newComponent->is_skinned = is_skinned;

        return newComponent;
    }

    void MeshRenderer::onRebindObject(std::string oldObj, std::string newObj)
    {
        if (rootObjectGuid == oldObj)
        {
            setRootObjectGuid(newObj);
            setRootObject(Engine::getSingleton()->getGameObject(newObj));
        }
    }

    void MeshRenderer::onRefresh()
    {
        resetBoneLinks();
    }

    void MeshRenderer::onAttach()
    {
        Component::onAttach();

        transform = getGameObject()->getTransform();

        attach();
    }

    void MeshRenderer::onDetach()
    {
        Component::onDetach();

        detach();
    }

    AxisAlignedBox MeshRenderer::getBounds(bool world)
    {
        if (mesh != nullptr)
        {
            if (world)
            {
                glm::bvec4 _nan = glm::isnan(transform->rotation);
                if (_nan.x || _nan.y || _nan.z || _nan.w)
                    transform->setRotation(glm::identity<glm::highp_quat>());

                glm::mat4x4 mtx = transform->getTransformMatrix();

                _nan = glm::isnan(mtx[3]);
                if (_nan.x || _nan.y || _nan.z || _nan.w)
                    mtx[3] = glm::vec4(0.0f);

                if (mtx != prevTransform)
                {
                    cachedAAB = mesh->getBounds();    
                    cachedAAB.transform(mtx);

                    prevTransform = mtx;

                    return cachedAAB;
                }
                else
                    return cachedAAB;
            }
            else
            {
                AxisAlignedBox aab = mesh->getBounds();
                return aab;
            }
        }
        else
        {
            AxisAlignedBox aab = AxisAlignedBox::BOX_NULL;
            if (getGameObject() != nullptr)
            {
                if (world)
                {
                    Transform* t = getGameObject()->getTransform();
                    glm::vec3 _min = t->getPosition() - glm::vec3(0.5f, 0.5f, 0.5f);
                    glm::vec3 _max = t->getPosition() + glm::vec3(0.5f, 0.5f, 0.5f);
                    aab = AxisAlignedBox(_min, _max);
                }
                else
                {
                    aab = AxisAlignedBox(glm::vec3(-0.5f), glm::vec3(0.5f));
                }
            }
            return aab;
        }
    }

    bool MeshRenderer::checkCullingMask(LayerMask& mask)
    {
        if (gameObject == nullptr)
            return false;

        return mask.getLayer(gameObject->getLayer());
    }

    void MeshRenderer::addSharedMaterial(Material* mat)
    {
        sharedMaterials.push_back(mat);
    }

    void MeshRenderer::removeSharedMaterial(Material* mat)
    {
        auto it = std::find(sharedMaterials.begin(), sharedMaterials.end(), mat);
        if (it != sharedMaterials.end())
            sharedMaterials.erase(it);
    }

    void MeshRenderer::removeSharedMaterial(int index)
    {
        if (index < sharedMaterials.size())
        {
            auto it = sharedMaterials.begin() + index;
            sharedMaterials.erase(it);
        }
    }

    Material* MeshRenderer::getSharedMaterial(int index)
    {
        if (index < sharedMaterials.size())
            return sharedMaterials[index];

        return nullptr;
    }

    void MeshRenderer::setSharedMaterial(int index, Material* material)
    {
        if (index < sharedMaterials.size())
            sharedMaterials[index] = material;

        if (index < materials.size())
        {
            if (materials[index] != nullptr)
                delete materials[index];

            materials[index] = nullptr;
        }
    }

    Material* MeshRenderer::getMaterial(int index)
    {
        if (index < sharedMaterials.size() && index < materials.size())
        {
            if (materials[index] == nullptr)
            {
                if (sharedMaterials[index] != nullptr)
                    materials[index] = sharedMaterials[index]->clone(sharedMaterials[index]->getName() + "_" + GUIDGenerator::genGuid());
            }

            return materials[index];
        }

        return nullptr;
    }

    void MeshRenderer::applyMaterials()
    {
        materials.clear();
        sharedMaterials.clear();

        if (mesh == nullptr)
            return;

        for (int i = 0; i < mesh->getSubMeshCount(); ++i)
        {
            std::string matName = mesh->getSubMesh(i)->getMaterialName();
            
            if (!matName.empty() && mesh->getUserData() != Engine::getSingleton()->getBuiltinResourcesPath()) //Prevent saving built-in meshes materials to assets folder
            {
                Material* material = Material::load(Engine::getSingleton()->getAssetsPath(), matName);
                if (material == nullptr)
                    material = Material::load(Engine::getSingleton()->getBuiltinResourcesPath(), matName);

                if (material == nullptr)
                {
                    material = Material::create(Engine::getSingleton()->getAssetsPath(), matName);
                    material->load();
                    material->save();
                }

                sharedMaterials.push_back(material);
            }
            else
                sharedMaterials.push_back(nullptr);
        }

        materials.resize(sharedMaterials.size());
        for (int i = 0; i < materials.size(); ++i)
            materials[i] = nullptr;
    }

    void MeshRenderer::setMesh(Mesh* meshPtr, bool setMaterials)
    {
        if (mesh != nullptr)
        {
            for (auto& it : lodVertexBuffer)
                it.clear();

            for (auto& it : lodVertexBufferMtxCache)
                it.clear();

            lodVertexBuffer.clear();
            lodVertexBufferMtxCache.clear();
        }

        mesh = meshPtr;
        
        if (setMaterials)
            applyMaterials();

        resetBoneLinks();
        reloadLightmaps();

        for (auto& it : occluded)
            it.second.clear();

        occluded.clear();

        if (mesh != nullptr)
        {
            lodVertexBuffer.resize(mesh->getSubMeshCount());
            lodVertexBufferMtxCache.resize(mesh->getSubMeshCount());

            for (int i = 0; i < mesh->getSubMeshCount(); ++i)
            {
                auto& vbuf = mesh->getSubMesh(i)->getVertexBuffer();

                lodVertexBuffer[i].resize(vbuf.size(), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                lodVertexBufferMtxCache[i].resize(vbuf.size(), false);
            }
        }
    }

    void MeshRenderer::resetBoneLinks()
    {
        for (auto it = boneLinks.begin(); it != boneLinks.end(); ++it)
        {
            for (auto jt = it->begin(); jt != it->end(); ++jt)
            {
                if (jt->target != nullptr)
                    jt->target->setOnDestroyCallback(nullptr);
            }
        }

        boneLinks.clear();
        is_skinned = false;

        if (mesh != nullptr && rootObject != nullptr)
        {
            boneLinks.resize(mesh->getSubMeshCount());

            for (int j = 0; j < mesh->getSubMeshCount(); ++j)
            {
                SubMesh* subMesh = mesh->getSubMesh(j);

                for (int i = 0; i < subMesh->getBoneCount(); ++i)
                {
                    BoneInfo* bone = subMesh->getBone(i);
                    GameObject* obj = Engine::getSingleton()->findGameObject(bone->getNameHash(), rootObject);
                    if (obj != nullptr)
                    {
                        BoneLink link;
                        link.boneIndex = i;
                        link.boneName = bone->getName();
                        link.boneNameHash = bone->getNameHash();
                        link.target = obj;
                        link.target->setOnDestroyCallback([=]()
                            {
                                auto srch = std::find_if(boneLinks[j].begin(), boneLinks[j].end(), [=](BoneLink& lnk) -> bool { return lnk.target == obj; });
                                if (srch != boneLinks[j].end())
                                    boneLinks[j].erase(srch);
                            }
                        );
                        boneLinks[j].push_back(link);
                        is_skinned = true;
                    }
                }
            }
        }
    }

    void MeshRenderer::setRootObject(GameObject* value)
    {
        rootObject = value;

        if (rootObject != nullptr)
            rootObjectGuid = rootObject->getGuid();
        else
            rootObjectGuid = "";

        resetBoneLinks();
    }

    bool MeshRenderer::isTransparent()
    {
        bool transp = false;

        if (mesh == nullptr)
            return transp;

        for (int i = 0; i < mesh->getSubMeshCount(); ++i)
        {
            if (transp)
                break;

            SubMesh* subMesh = mesh->getSubMesh(i);
            Material* material = nullptr;
            Shader* shader = nullptr;

            if (Engine::getSingleton()->getIsRuntimeMode())
            {
                if (i < materials.size())
                    material = materials[i];
            }
            else
            {
                if (i < sharedMaterials.size())
                    material = sharedMaterials[i];
            }

            if (material != nullptr && material->isLoaded())
                shader = material->getShader();

            if (shader != nullptr)
            {
                for (int j = 0; j < shader->getPassCount(); ++j)
                {
                    Pass * pass = shader->getPass(j);
                    ProgramVariant* pv = nullptr;
                    if (material != nullptr && material->isLoaded() && pass != nullptr)
                        pv = pass->getProgramVariant(material->getDefinesStringHash());
                    if (pv != nullptr && pv->blendMode == BlendMode::Alpha)
                    {
                        transp = true;
                        break;
                    }
                }
            }
        }

        return transp;
    }

    bool MeshRenderer::isStatic()
    {
        if (gameObject == nullptr)
            return false;

        return gameObject->getLightingStatic();
    }

    bool MeshRenderer::getSkipRendering()
    {
        if (gameObject == nullptr)
            return false;

        return gameObject->getBatchingStatic();
    }

    bool MeshRenderer::isSkinned()
    {
        return is_skinned;
    }

    void MeshRenderer::reloadLightmaps()
    {
        if (gameObject == nullptr)
            return;

        for (auto& lightmap : lightmaps)
            delete lightmap;

        lightmaps.clear();

        if (!gameObject->getLightingStatic())
            return;

        if (mesh != nullptr)
        {
            for (int i = 0; i < mesh->getSubMeshCount(); ++i)
            {
                std::string loadedScene = Scene::getLoadedScene();
                std::string dirPath = IO::GetFilePath(loadedScene) + IO::GetFileName(loadedScene) + "/Lightmaps/";
                std::string fileName = dirPath + gameObject->getGuid() + "_" + std::to_string(i) + ".jpg";

                std::string location = Engine::getSingleton()->getAssetsPath();

                Texture* lightmap = Texture::load(location, fileName, true, Texture::CompressionMethod::Default, false, false);
                if (lightmap != nullptr && lightmap->isLoaded())
                    lightmaps.push_back(lightmap);
            }
        }
    }

    void MeshRenderer::calcBoneData(int subMeshIndex, SubMesh* subMesh)
    {
        if (rootObject == nullptr)
            return;

        int i = 0;
        for (auto it = boneLinks[subMeshIndex].begin(); it != boneLinks[subMeshIndex].end(); ++it, ++i)
        {
            BoneLink& link = *it;
            if (link.target == nullptr) continue;

            glm::mat4x4 invGlobalMtx = transform->getTransformMatrixInverse();
            glm::mat4x4 invBindMtx = subMesh->getBone(link.boneIndex)->getOffsetMatrix();
            glm::mat4x4 boneMtx = link.target->getTransform()->getTransformMatrix();

            boneMatrices[i] = invGlobalMtx * boneMtx * invBindMtx;
        }
    }

    void MeshRenderer::onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback)
    {
        if (gameObject == nullptr)
            return;

        if (!gameObject->getActive())
            return;

        if (!getEnabled())
            return;

        if (mesh == nullptr || !mesh->isLoaded())
            return;

        if (gameObject->getBatchingStatic())
            return;

        glm::mat4x4 trans = transform->getTransformMatrix();
        glm::mat4x4 invTrans = transform->getTransformMatrixInverse();
        glm::mat3x3 normalMatrix = trans;

        if (!gameObject->getOcclusionStatic())
        {
            if (occluded.size() > 0)
            {
                for (auto& it : occluded)
                    it.second.clear();

                occluded.clear();
            }
        }

        float lodDist = 0.0f;
        float aabbRadius = 1.0f;
        
        if (camera != nullptr)
        {
            AxisAlignedBox aabb = getBounds();

            aabbRadius = aabb.getRadius();
            glm::vec3 centerPosition = aabb.getCenter();

            Transform* camTransform = camera->getTransform();
            glm::vec3 relDir = glm::normalize(camTransform->getPosition() - centerPosition);
            glm::vec3 lodCenter = centerPosition + (relDir * aabbRadius);

            lodDist = glm::distance(lodCenter, camTransform->getPosition());
        }

        for (int i = 0; i < mesh->getSubMeshCount(); ++i)
        {
            SubMesh* subMesh = mesh->getSubMesh(i);
            Material* material = Renderer::getDefaultMaterial();
            Shader* shader = Renderer::getDefaultShader();

            //LOD
            int currentLod = 0;

            if (camera != nullptr)
            {
                int maxLod = subMesh->getLodLevelsCount();
                if (maxLod > 0)
                {
                    float sectionSize = lodMaxDistance / (float)maxLod;
                    currentLod = sectionSize > 0 ? floorf(lodDist / (sectionSize * aabbRadius)) : 0;
                    int _maxLod = currentLod;
                    currentLod = std::min(currentLod, maxLod);

                    if (cullOverMaxDistance)
                    {
                        if (_maxLod > currentLod)
                            continue;
                    }
                }
            }

            //Get material
            if (i < sharedMaterials.size())
            {
                if (sharedMaterials[i] != nullptr)
                    material = sharedMaterials[i];

                if (i < materials.size())
                {
                    if (materials[i] != nullptr)
                        material = materials[i];
                }
            }

            if (material != nullptr && material->isLoaded() && material->getShader() != nullptr)
                shader = material->getShader();

            if (program.idx == bgfx::kInvalidHandle)
            {
                if (shader == nullptr || !shader->isLoaded())
                    continue;

                if (renderMode != static_cast<int>(shader->getRenderMode()))
                    continue;
            }
            //

            /////-------Occlusion Culling
            if (gameObject->getOcclusionStatic())
            {
                if (program.idx == bgfx::kInvalidHandle)
                {
                    if (camera != nullptr)
                    {
                        auto& otx = occluded[camera];

                        if (otx.size() != mesh->getSubMeshCount())
                            otx.resize(mesh->getSubMeshCount());

                        if (camera->getOcclusionCulling())
                        {
                            auto* moc = Renderer::getSingleton()->getOcclusionCullingProcessor();

                            auto& vbuf = subMesh->getVertexBuffer();
                            auto& ibuf = subMesh->getIndexBuffer();

                            if (lodVertexBuffer[i].size() == vbuf.size())
                            {
                                int maxLod = subMesh->getLodLevelsCount() - 1;

                                MaskedOcclusionCulling::CullingResult result = MaskedOcclusionCulling::CullingResult::VISIBLE;

                                if (maxLod >= 0)
                                {
                                    auto& lodIbuf = subMesh->getLodIndexBuffer(maxLod);
                                    result = moc->TestTriangles(&lodVertexBuffer[i][0].x, lodIbuf.data(), (int)lodIbuf.size() / 3);
                                }
                                else
                                {
                                    result = moc->TestTriangles(&lodVertexBuffer[i][0].x, ibuf.data(), (int)ibuf.size() / 3);
                                }

                                otx[i] = false;

                                if (result == MaskedOcclusionCulling::CullingResult::OCCLUDED)
                                {
                                    otx[i] = true;
                                    continue;
                                }
                            }
                        }
                        else
                        {
                            otx[i] = false;
                        }
                    }
                }
                else
                {
                    if (camera == nullptr)
                    {
                        bool skip = true;

                        for (auto& it : occluded)
                        {
                            if (it.second.size() != mesh->getSubMeshCount())
                                continue;

                            if (!it.second[i])
                            {
                                skip = false;
                                break;
                            }
                        }

                        if (skip)
                            continue;
                    }
                    else
                    {
                        if (camera->getOcclusionCulling())
                        {
                            auto& otx = occluded[camera];

                            if (otx.size() == mesh->getSubMeshCount())
                            {
                                if (otx[i])
                                    continue;
                            }
                        }
                    }
                }
            }
            /////----------------

            if (is_skinned)
                calcBoneData(i, subMesh);

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

                size_t iterationCount = 1;

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

                    if (currentLod > 0 && subMesh->getLodLevelsCount() > 0)
                    {
                        auto& lodIndexBufId = subMesh->getLodIndexBufferHandle(currentLod - 1);

                        if (subMesh->getLodIndexBuffer(currentLod - 1).size() > 0)
                            bgfx::setIndexBuffer(lodIndexBufId);
                    }
                    else
                    {
                        if (subMesh->getIndexBuffer().size() > 0)
                            bgfx::setIndexBuffer(subMesh->getIndexBufferHandle());
                    }

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
                        if (pv != nullptr)
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
                    }

                    //Bind system uniforms
                    bgfx::setUniform(Renderer::uNormalMatrix, glm::value_ptr(normalMatrix), 1);

                    if (is_skinned)
                        bgfx::setUniform(Renderer::uBoneMtx, &boneMatrices, 128);

                    bgfx::setUniform(Renderer::uGpuSkinning, glm::value_ptr(glm::vec4(is_skinned ? 1.0 : 0.0, 0.0, 0.0, 0.0)), 1);
                    bgfx::setUniform(Renderer::uInvModel, glm::value_ptr(invTrans), 1);

                    bool useLightmaps = i < lightmaps.size() && lightmaps[i] != nullptr;
                    bgfx::setUniform(Renderer::uHasLightmap, glm::value_ptr(glm::vec4(useLightmaps ? 1.0 : 0.0, 0.0, 0.0, 0.0)), 1);

                    if (useLightmaps)
                        bgfx::setTexture(8, Renderer::uLightmap, lightmaps[i]->getHandle());

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