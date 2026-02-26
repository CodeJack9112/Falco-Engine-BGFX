#include "API_Graphics.h"

#include "../Renderer/Renderer.h"
#include "../Core/APIManager.h"
#include "../Renderer/Primitives.h"
#include "../Assets/Material.h"
#include "../Assets/Shader.h"
#include "../Renderer/RenderTexture.h"
#include "../Components/Camera.h"

#include "../glm/glm.hpp"
#include "../glm/gtc/type_ptr.hpp"

namespace GX
{
    void API_Graphics::blitRt(MonoObject* material, MonoObject* renderTarget, bool copyToBackBuffer)
    {
        if (renderTarget == nullptr)
            return;

        RenderTexture* rt = nullptr;
        mono_field_get_value(renderTarget, APIManager::getSingleton()->object_ptr_field, reinterpret_cast<void*>(&rt));

        if (rt == nullptr)
            return;

        int width = rt->getWidth();
        int height = rt->getHeight();
        int viewId = Camera::getApiViewId();

        Camera* camera = Camera::getApiCamera();

        uint16_t cullState = 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
        if (camera->getClearFlags() == Camera::ClearFlags::SolidColor)
            cullState = 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL;
        if (camera->getClearFlags() == Camera::ClearFlags::DontClear)
            cullState = 0;

        glm::mat4x4 finalProj = glm::orthoRH(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f);
        bgfx::setViewClear(viewId, cullState, 0x00000000, 1.0f, 0);
        bgfx::setViewRect(viewId, 0, 0, width, height);
        bgfx::setViewTransform(viewId, NULL, glm::value_ptr(finalProj));
        bgfx::setViewFrameBuffer(viewId, rt->getFrameBufferHandle());

        Material* mat = nullptr;
        mono_field_get_value(material, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&mat));

        if (mat == nullptr)
            return;

        Shader* shader = nullptr;

        if (mat != nullptr && mat->isLoaded())
            shader = mat->getShader();

        if (shader == nullptr || !shader->isLoaded())
            return;

        int passCount = 1;

        if (shader != nullptr && shader->isLoaded())
            passCount = shader->getPassCount();

        for (int j = 0; j < passCount; ++j)
        {
            Pass* pass = nullptr;

            if (shader != nullptr && shader->isLoaded())
                pass = shader->getPass(j);

            ProgramVariant* pv = nullptr;
            if (mat != nullptr && mat->isLoaded() && pass != nullptr)
                pv = pass->getProgramVariant(mat->getDefinesStringHash());

            uint64_t passState = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA;

            // Set render states.
            bgfx::setState(passState);

            // Pass uniforms
            if (pv != nullptr)
            {
                mat->submitUniforms(pv, camera);

                Renderer::getSingleton()->setSystemUniforms(camera);

                // Render
                Primitives::screenSpaceQuad();
                bgfx::submit(viewId, pv->programHandle);
            }
        }

        Renderer::getSingleton()->frame();

        if (copyToBackBuffer)
            camera->postProcessRt = rt;
    }
}