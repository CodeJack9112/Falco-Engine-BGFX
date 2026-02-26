#include "Water.h"

#include "../glm/gtc/type_ptr.hpp"

#include "Transform.h"
#include "Light.h"

#include "../Core/Engine.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/VertexLayouts.h"
#include "../Renderer/RenderTexture.h"
#include "../Renderer/Primitives.h"
#include "../Core/GameObject.h"
#include "../Components/Camera.h"

#include "../Assets/Material.h"
#include "../Assets/Shader.h"
#include "../Assets/Texture.h"

#include "../Math/Mathf.h"
#include "../Classes/GUIDGenerator.h"

namespace GX
{
    std::string Water::COMPONENT_TYPE = "Water";

    float clipPlaneOffset = 0.5f;

    Water::Water() : Component(nullptr), Renderable()
    {
        setRenderQueue(1);

        setCastShadows(false);

        Shader* shader = Shader::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Shaders/StandardWater.shader");
        
        material = Material::create(Engine::getSingleton()->getAssetsPath(), GUIDGenerator::genGuid());
        material->setShader(shader);
        material->load();

        Texture* foam = Texture::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Water/foam.png");
        Texture* foam_shore = Texture::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Water/foam_shore.png");
        Texture* water_height = Texture::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Water/water_height.png");
        Texture* water_normal = Texture::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Water/water_normal.png");
        Texture* caustics = Texture::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Water/caustics.png");
        
        material->setUniform("_NormalTexture", std::make_pair(1, water_normal), false);
        material->setUniform("_HeightTexture", std::make_pair(2, water_height), false);
        material->setUniform("_FoamTexture", std::make_pair(3, foam), false);
        material->setUniform("_ShoreTexture", std::make_pair(4, foam_shore), false);
        material->setUniform("_CausticsTex", std::make_pair(5, caustics), false);

        u_reflectionTexture = bgfx::createUniform("u_reflectionTexture", bgfx::UniformType::Sampler, 1);

        auto reg = material->getUniform<Sampler2DDef>("u_reflectionTexture");
        reflectionTexReg = reg.first;

        create();
    }

    Water::~Water()
    {
        destroyReflections();
        destroy();
    }

    void Water::destroy()
    {
        if (bgfx::isValid(ibh))
            bgfx::destroy(ibh);

        if (bgfx::isValid(vbh))
            bgfx::destroy(vbh);

        ibh = { bgfx::kInvalidHandle };
        vbh = { bgfx::kInvalidHandle };

        //video driver crashes here without this line
        Renderer::getSingleton()->frame();

        if (vertices != nullptr)
            delete[] vertices;
        if (indices != nullptr)
            delete[] indices;

        vertices = nullptr;
        indices = nullptr;
    }

    void Water::create()
    {
        uint32_t num = size * size;
        vertices = new VertexBuffer[num];
        indices = new uint32_t[num * (uint32_t)6];

        for (uint32_t i = 0; i < num; ++i)
            vertices[i].normal = glm::vec3(0.0f);

        prevTransform = glm::mat4x4(FLT_MAX);
        bounds = AxisAlignedBox::BOX_NULL;
        bounds.setExtents(glm::vec3(-0.5f), glm::vec3(0.5f));

        float worldSize = 25.0f;
        float scale = worldSize / size;

        vertexCount = 0;
        for (int y = 0; y < size; y++)
        {
            for (int x = 0; x < size; x++)
            {
                //int h = (y * size) + x;
                glm::vec3 position = Mathf::toQuaternion(180.0f, 0, 0) * glm::vec3(((float)x - size / 2) * scale, 0, ((float)y - size / 2) * scale);
                glm::vec2 texcoord = glm::vec2((x + 0.5f) / size, (y + 0.5f) / size);

                VertexBuffer* vert = &vertices[vertexCount];
                vert->position = position;
                vert->texcoord0 = texcoord;

                vertexCount++;

                bounds.merge(position);
            }
        }

        bounds.merge(glm::vec3(0, 0.1f, 0));

        indexCount = 0;
        for (int y = 0; y < (size - 1); y++)
        {
            uint32_t y_offset = (y * size);
            for (int x = 0; x < (size - 1); x++)
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

        const bgfx::Memory* mem = bgfx::makeRef(&vertices[0], sizeof(VertexBuffer) * vertexCount);
        vbh = bgfx::createVertexBuffer(mem, VertexLayouts::waterVertexLayout);

        mem = bgfx::makeRef(&indices[0], sizeof(uint32_t) * indexCount);
        ibh = bgfx::createIndexBuffer(mem, BGFX_BUFFER_INDEX32);
    }

    void Water::recreate()
    {
        destroy();
        create();
    }

    void Water::updateReflectionsCamera()
    {
        if (reflections)
            createReflections();
        else
            destroyReflections();
    }

    void Water::createReflections()
    {
        if (cameraTransform == nullptr)
        {
            int w = Renderer::getSingleton()->getWidth();
            int h = Renderer::getSingleton()->getHeight();

            if (reflectionsQuality == 0)
            {
                w *= 0.25f;
                h *= 0.25f;
            }
            else if (reflectionsQuality == 1)
            {
                w *= 0.5f;
                h *= 0.5f;
            }

            cameraTransform = new Transform();
            renderTarget = new RenderTexture(w, h, RenderTexture::TextureType::ColorWithDepth);
            
            bgfx::Attachment gbufferAt[5];

            const uint64_t tsFlags = 0
                | BGFX_SAMPLER_MIN_POINT
                | BGFX_SAMPLER_MAG_POINT
                | BGFX_SAMPLER_MIP_POINT
                | BGFX_SAMPLER_U_CLAMP
                | BGFX_SAMPLER_V_CLAMP
                ;

            //GBuffer
            gbufferTex[0] = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
            gbufferTex[1] = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
            gbufferTex[2] = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
            gbufferTex[3] = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
            gbufferTex[4] = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT | tsFlags);

            gbufferAt[0].init(gbufferTex[0]);
            gbufferAt[1].init(gbufferTex[1]);
            gbufferAt[2].init(gbufferTex[2]);
            gbufferAt[3].init(gbufferTex[3]);
            gbufferAt[4].init(gbufferTex[4]);

            gbuffer = bgfx::createFrameBuffer(BX_COUNTOF(gbufferAt), gbufferAt, true);

            //Light buffer
            lightBufferTex = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
            lightBuffer = bgfx::createFrameBuffer(1, &lightBufferTex, true);
        }
    }

    void Water::destroyReflections()
    {
        if (cameraTransform != nullptr)
        {
            delete cameraTransform;
            cameraTransform = nullptr;
        }

        if (renderTarget != nullptr)
        {
            delete renderTarget;
            renderTarget = nullptr;

            if (bgfx::isValid(gbuffer))
                bgfx::destroy(gbuffer);
            if (bgfx::isValid(lightBuffer))
                bgfx::destroy(lightBuffer);

            gbuffer = { bgfx::kInvalidHandle };
            lightBuffer = { bgfx::kInvalidHandle };

            gbufferTex[0] = { bgfx::kInvalidHandle };
            gbufferTex[1] = { bgfx::kInvalidHandle };
            gbufferTex[2] = { bgfx::kInvalidHandle };
            gbufferTex[3] = { bgfx::kInvalidHandle };
            gbufferTex[4] = { bgfx::kInvalidHandle };
        }
    }

    AxisAlignedBox Water::getBounds(bool world)
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

    void Water::onAttach()
    {
        Component::onAttach();

        transform = getGameObject()->getTransform();

        attach();
        if (reflections)
            createReflections();
    }

    void Water::onDetach()
    {
        Component::onDetach();

        detach();
        if (reflections)
            destroyReflections();
    }

    Component* Water::onClone()
    {
        Water* newComponent = new Water();
        newComponent->size = size;
        newComponent->reflections = reflections;
        newComponent->reflectSkybox = reflectSkybox;
        newComponent->reflectObjects = reflectObjects;
        newComponent->reflectionsDistance = reflectionsDistance;
        newComponent->reflectionsQuality = reflectionsQuality;
        newComponent->material = material->clone(GUIDGenerator::genGuid());

        return newComponent;
    }

    void Water::onScreenResized(int w, int h)
    {
        if (renderTarget != nullptr)
        {
            destroyReflections();
            createReflections();
        }
    }

    void Water::setSize(int32_t value)
    {
        if (value < 10 || value > 500)
            return;

        size = value;
        recreate();
    }

    void Water::setReflections(bool value)
    {
        reflections = value;
        material->setUniform("USE_REFLECTIONS", value, true);

        updateReflectionsCamera();
    }

    void Water::setReflectionsQuality(int value)
    {
        reflectionsQuality = value;
        if (reflections)
        {
            destroyReflections();
            createReflections();
        }
    }

    bool Water::checkCullingMask(LayerMask& mask)
    {
        if (gameObject == nullptr)
            return false;

        return mask.getLayer(gameObject->getLayer());
    }

    float Sgn(float a)
    {
        if (a > 0.0F)
        {
            return 1.0F;
        }
        if (a < 0.0F)
        {
            return -1.0F;
        }
        return 0.0F;
    }

    glm::vec3 multiplyPoint(const glm::mat4x4& mat, glm::vec3 point)
    {
        glm::vec3 result = glm::vec3();

        result.x = mat[0][0] * point.x + mat[1][0] * point.y + mat[2][0] * point.z + mat[3][0];
        result.y = mat[1][0] * point.x + mat[1][1] * point.y + mat[2][1] * point.z + mat[3][1];
        result.z = mat[0][2] * point.x + mat[1][2] * point.y + mat[2][2] * point.z + mat[3][2];
        float num = mat[0][3] * point.x + mat[1][3] * point.y + mat[2][2] * point.z + mat[3][3];
        num = 1.0f / num;
        result.x *= num;
        result.y *= num;
        result.z *= num;
        return result;
    }

    glm::vec3 multiplyVector(const glm::mat4x4& mat, glm::vec3 vector)
    {
        glm::vec3 result = glm::vec3();
        result.x = mat[0][0] * vector.x + mat[1][0] * vector.y + mat[2][0] * vector.z;
        result.y = mat[0][1] * vector.x + mat[1][1] * vector.y + mat[2][1] * vector.z;
        result.z = mat[0][2] * vector.x + mat[1][2] * vector.y + mat[2][2] * vector.z;
        return result;
    }

    glm::vec4 CameraSpacePlane(const glm::mat4x4& mtx, glm::vec3 pos, glm::vec3 normal, float sideSign)
    {
        glm::vec3 offsetPos = pos + normal * clipPlaneOffset;
        glm::vec3 cpos = multiplyPoint(mtx, offsetPos);
        glm::vec3 cnormal = glm::normalize(multiplyVector(mtx, normal)) * sideSign;

        return glm::vec4(cnormal.x, cnormal.y, cnormal.z, -glm::dot(cpos, cnormal));
    }

    glm::mat4x4 CalculateObliqueMatrix(glm::mat4x4& projection, glm::vec4& clipPlane)
    {
        glm::mat4x4 result = projection;

        glm::vec4 q = glm::inverse(result) * glm::vec4(
            Sgn(clipPlane.x),
            Sgn(clipPlane.y),
            1.0f,
            1.0f
        );

        glm::vec4 c = clipPlane * (2.0f / (glm::dot(clipPlane, q)));

        // third row = clip plane - fourth row
        result[0][2] = c.x - projection[0][3];
        result[1][2] = c.y - projection[1][3];
        result[2][2] = c.z - projection[2][3];
        result[3][2] = c.w - projection[3][3];

        return result;
    }

    glm::mat4x4 CalculateReflectionMatrix(const glm::vec4& plane)
    {
        glm::mat4x4 result = glm::mat4x4();// glm::identity<glm::mat4x4>();
        result[0] = glm::vec4(0.0f);
        result[1] = glm::vec4(0.0f);
        result[2] = glm::vec4(0.0f);
        result[3] = glm::vec4(0.0f);

        result[0][0] = (1.0F - 2.0F * plane[0] * plane[0]);
        result[1][0] = (-2.0F * plane[0] * plane[1]);
        result[2][0] = (-2.0F * plane[0] * plane[2]);
        result[3][0] = (-2.0F * plane[3] * plane[0]);

        result[0][1] = (-2.0F * plane[1] * plane[0]);
        result[1][1] = (1.0F - 2.0F * plane[1] * plane[1]);
        result[2][1] = (-2.0F * plane[1] * plane[2]);
        result[3][1] = (-2.0F * plane[3] * plane[1]);

        result[0][2] = (-2.0F * plane[2] * plane[0]);
        result[1][2] = (-2.0F * plane[2] * plane[1]);
        result[2][2] = (1.0F - 2.0F * plane[2] * plane[2]);
        result[3][2] = (-2.0F * plane[3] * plane[2]);

        result[0][3] = 0.0F;
        result[1][3] = 0.0F;
        result[2][3] = 0.0F;
        result[3][3] = 1.0F;

        return result;
    }

    void Water::onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback)
    {
        if (gameObject == nullptr)
            return;

        if (!gameObject->getActive())
            return;

        if (!getEnabled())
            return;

        //const bgfx::Caps* caps = bgfx::getCaps();

        glm::mat4x4 trans = transform->getTransformMatrix();
        glm::mat4x4 invTrans = transform->getTransformMatrixInverse();
        glm::mat3x3 normalMatrix = glm::identity<glm::mat3x3>();

        if (program.idx == bgfx::kInvalidHandle)
            normalMatrix = trans;

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

        ///Update reflection camera
        int viewId = Renderer::getSingleton()->getNumViewsUsed() + 1;

        int viewSkybox = viewId;
        int viewGBuffer = viewId + 1;
        int viewLight = viewId + 2;
        int viewCombine = viewId + 3;
        int viewForward = viewId + 4;

        bgfx::touch(viewSkybox);
        bgfx::touch(viewGBuffer);
        bgfx::touch(viewLight);
        bgfx::touch(viewCombine);
        bgfx::touch(viewForward);

        if (reflections && (reflectSkybox || reflectObjects) && program.idx == bgfx::kInvalidHandle)
        {
            if (cameraTransform != nullptr)
            {
                Transform* t = camera->getTransform();

                glm::vec3 euler = Mathf::toEuler(t->getRotation());
                //cameraTransform->setPosition(t->getPosition());
                //cameraTransform->setRotation(Mathf::toQuaternion(-euler.x, euler.y, euler.z));

                glm::vec3 pos = transform->getPosition();
                glm::vec3 normal = transform->getUp();
                float d = -glm::dot(normal, pos) - clipPlaneOffset;
                
                glm::vec4 reflectionPlane = glm::vec4(normal.x, normal.y, normal.z, d);

                glm::mat4x4 reflection = CalculateReflectionMatrix(reflectionPlane);
                glm::vec3 newpos = multiplyPoint(reflection, t->getPosition());

                glm::mat4x4 viewMatrix = camera->getViewMatrix() * reflection;
                glm::vec4 clipPlane = CameraSpacePlane(viewMatrix, pos, normal, 1.0f);
                //glm::mat4x4 projectionMatrix = CalculateObliqueMatrix(camera->getProjectionMatrix(), clipPlane);
                //glm::mat4x4 projectionMatrix = camera->getProjectionMatrix();
                glm::mat4x4 projectionMatrix = camera->makeProjectionMatrix(camera->getFOVy(), camera->getAspectRatio(), 0.1f, camera->getFar());
                //glm::mat4x4 projectionMatrix = _pmtx;// CalculateObliqueMatrix(_pmtx, clipPlane);

                /*cameraTransform->setPosition(newpos);
                euler = Mathf::toEuler(t->getRotation());
                glm::quat mRot = Mathf::toQuaternion(glm::vec3(-euler.x, euler.y, euler.z));
                cameraTransform->setRotation(mRot);*/

                glm::mat4x4 invViewProj = glm::inverse(projectionMatrix * viewMatrix);
                glm::mat4x4 geomProj = glm::orthoRH(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f);

                if (reflectSkybox || reflectObjects)
                {
                    //Skybox
                    bgfx::setViewClear(viewSkybox, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);
                    bgfx::setViewRect(viewSkybox, 0, 0, renderTarget->getWidth(), renderTarget->getHeight());
                    bgfx::setViewTransform(viewSkybox, glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix));
                    bgfx::setViewFrameBuffer(viewSkybox, renderTarget->getFrameBufferHandle());

                    //GBuffer
                    bgfx::setViewClear(viewGBuffer, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);
                    bgfx::setViewRect(viewGBuffer, 0, 0, renderTarget->getWidth(), renderTarget->getHeight());
                    bgfx::setViewTransform(viewGBuffer, glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix));
                    bgfx::setViewFrameBuffer(viewGBuffer, gbuffer);

                    //Light
                    bgfx::setViewClear(viewLight, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);
                    bgfx::setViewRect(viewLight, 0, 0, renderTarget->getWidth(), renderTarget->getHeight());
                    bgfx::setViewTransform(viewLight, NULL, glm::value_ptr(geomProj));
                    bgfx::setViewFrameBuffer(viewLight, lightBuffer);

                    //Combine deferred
                    bgfx::setViewClear(viewCombine, BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);
                    bgfx::setViewRect(viewCombine, 0, 0, renderTarget->getWidth(), renderTarget->getHeight());
                    bgfx::setViewTransform(viewCombine, NULL, glm::value_ptr(geomProj));
                    bgfx::setViewFrameBuffer(viewCombine, renderTarget->getFrameBufferHandle());

                    //Forward
                    bgfx::setViewClear(viewForward, 0, 0x00000000, 1.0f, 0);
                    bgfx::setViewRect(viewForward, 0, 0, renderTarget->getWidth(), renderTarget->getHeight());
                    bgfx::setViewTransform(viewForward, glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix));
                    bgfx::setViewFrameBuffer(viewForward, renderTarget->getFrameBufferHandle());
                }

                if (reflectSkybox)
                {
                    glm::mat4x4 skyMtx = glm::identity<glm::mat4x4>();
                    skyMtx = glm::translate(skyMtx, t->getPosition());
                    float dist = camera->getFar();
                    if (dist > reflectionsDistance) dist = dist - reflectionsDistance;
                    skyMtx = glm::scale(skyMtx, glm::vec3(dist));
                    Renderer::getSingleton()->renderSkybox(viewSkybox, camera, skyMtx);
                }

                //bgfx::setViewTransform(viewId, glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix));

                if (reflectObjects)
                {
                    auto& renderables = Renderer::getSingleton()->getRenderables();
                    for (auto it = renderables.begin(); it != renderables.end(); ++it)
                    {
                        Renderable* comp = *it;
                        if (comp->getRenderQueue() == 1)
                            continue;

                        if (comp == this)
                            continue;

                        AxisAlignedBox aab1 = getBounds();
                        AxisAlignedBox aab2 = comp->getBounds();

                        if (aab1.isFinite() && aab2.isFinite())
                        {
                            if (!Mathf::intersects(t->getPosition(), reflectionsDistance, aab2))
                                continue;

                            /*if (aab1.getCenter().y > aab2.getCenter().y)
                                continue;*/
                        }

                        if (!comp->checkCullingMask(camera->getCullingMask()))
                            continue;

                        if (!comp->isAlwaysVisible())
                        {
                            if (!camera->isVisible(comp->getBounds()))
                                continue;
                        }

                        comp->onRender(camera, viewGBuffer, 0
                            | BGFX_STATE_WRITE_RGB
                            | BGFX_STATE_WRITE_A
                            | BGFX_STATE_WRITE_Z
                            | BGFX_STATE_DEPTH_TEST_LEQUAL
                            | BGFX_STATE_CULL_CCW,
                            { bgfx::kInvalidHandle }, static_cast<int>(RenderMode::Deferred), [=]() {
                                Renderer::getSingleton()->setSystemUniforms(camera);
                            });
                    }

                    std::vector<Light*>& lights = Renderer::getSingleton()->getLights();
                    for (auto& light : lights)
                    {
                        light->onRender(camera, viewLight, 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ADD, Renderer::getSingleton()->getLightProgram(), [=]() {
                                Renderer::getSingleton()->setSystemUniforms(camera);

                                bgfx::setTexture(0, Renderer::getSingleton()->getAlbedoMapUniform(), gbufferTex[0]);
                                bgfx::setTexture(1, Renderer::getSingleton()->getNormalMapUniform(), gbufferTex[1]);
                                bgfx::setTexture(2, Renderer::getSingleton()->getMRAMapUniform(), gbufferTex[2]);
                                bgfx::setTexture(3, Renderer::getSingleton()->getLightmapUniform(), gbufferTex[3]);
                                bgfx::setTexture(4, Renderer::getSingleton()->getDepthMapUniform(), gbufferTex[4]);

                                bgfx::setUniform(Renderer::getSingleton()->getInvViewProjUniform(), glm::value_ptr(invViewProj), 1);
                            }
                        );
                    }

                    Renderer::getSingleton()->setSystemUniforms(camera);
                    bgfx::setTexture(0, Renderer::getSingleton()->getAlbedoMapUniform(), gbufferTex[0]);
                    bgfx::setTexture(1, Renderer::getSingleton()->getDepthMapUniform(), gbufferTex[4]);
                    bgfx::setTexture(2, Renderer::getSingleton()->getLightColorMapUniform(), lightBufferTex);
                    bgfx::setTexture(3, Renderer::getSingleton()->getLightmapUniform(), gbufferTex[3]);
                    bgfx::setState(0
                        | BGFX_STATE_WRITE_RGB
                        | BGFX_STATE_WRITE_A
                        | BGFX_STATE_WRITE_Z
                        | BGFX_STATE_DEPTH_TEST_ALWAYS);
                    Primitives::screenSpaceQuad();
                    bgfx::submit(viewCombine, Renderer::getSingleton()->getCombineProgram());

                    for (auto it = renderables.begin(); it != renderables.end(); ++it)
                    {
                        Renderable* comp = *it;
                        if (comp->getRenderQueue() == 1)
                            continue;

                        if (comp == this)
                            continue;

                        AxisAlignedBox aab1 = getBounds();
                        AxisAlignedBox aab2 = comp->getBounds();

                        if (aab1.isFinite() && aab2.isFinite())
                        {
                            if (!Mathf::intersects(t->getPosition(), reflectionsDistance, aab2))
                                continue;

                            /*if (aab1.getCenter().y > aab2.getCenter().y)
                                continue;*/
                        }

                        if (!comp->checkCullingMask(camera->getCullingMask()))
                            continue;

                        if (!comp->isAlwaysVisible())
                        {
                            if (!camera->isVisible(comp->getBounds()))
                                continue;
                        }

                        comp->onRender(camera, viewForward, 0
                            | BGFX_STATE_WRITE_RGB
                            | BGFX_STATE_WRITE_A
                            | BGFX_STATE_WRITE_Z
                            | BGFX_STATE_DEPTH_TEST_LEQUAL
                            | BGFX_STATE_CULL_CCW,
                            { bgfx::kInvalidHandle }, static_cast<int>(RenderMode::Forward), [=]() {
                                Renderer::getSingleton()->setSystemUniforms(camera);
                            });
                    }
                }
                //
            }
        }

        Renderer::getSingleton()->frame();
        ///

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
                                for (auto& light : lights)
                                {
                                    if (light->getLightType() == LightType::Directional)
                                    {
                                        if (!light->submitUniforms())
                                            continue;

                                        break;
                                    }
                                }
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

                //Bind system uniforms
                bgfx::setUniform(Renderer::getInvModelUniform(), glm::value_ptr(invTrans), 1);
                if (reflections && (reflectSkybox || reflectObjects) && renderTarget != nullptr)
                    bgfx::setTexture(reflectionTexReg, u_reflectionTexture, renderTarget->getColorTextureHandle());
                else
                    bgfx::setTexture(reflectionTexReg, u_reflectionTexture, Texture::getNullTexture()->getHandle());

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