#include "DecalRenderer.h"

#include "../glm/glm.hpp"
#include "../glm/gtc/type_ptr.hpp"

#include "../Math/Mathf.h"

#include "../Core/GameObject.h"

#include "../Assets/Shader.h"
#include "../Assets/Texture.h"
#include "../Assets/Material.h"

#include "../Components/Transform.h"
#include "../Components/Camera.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Primitives.h"

namespace GX
{
	std::string DecalRenderer::COMPONENT_TYPE = "DecalRenderer";

	DecalRenderer::DecalRenderer() : Component(nullptr), Renderable()
	{
	}

	DecalRenderer::~DecalRenderer()
	{
	}

    void DecalRenderer::onAttach()
    {
        Component::onAttach();

        transform = getGameObject()->getTransform();

        attach();
    }

    void DecalRenderer::onDetach()
    {
        Component::onDetach();

        detach();
    }

	bool DecalRenderer::isTransparent()
	{
		return false;
	}

	AxisAlignedBox DecalRenderer::getBounds(bool world)
	{
        glm::vec3 scale = glm::vec3(1.0f);

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
                glm::vec3 _min = -scale;
                glm::vec3 _max = scale;

                cachedAAB = AxisAlignedBox(_min, _max);
                cachedAAB.transform(mtx);

                prevTransform = mtx;

                return cachedAAB;
            }
            else
                return cachedAAB;
        }
        else
        {
            scale = transform->getScale();

            glm::vec3 _min = -scale;
            glm::vec3 _max = scale;

            return AxisAlignedBox(_min, _max);
        }
	}

    Component* DecalRenderer::onClone()
    {
        DecalRenderer* newComponent = new DecalRenderer();
        newComponent->enabled = enabled;
        newComponent->material = material;

        return newComponent;
    }

    void DecalRenderer::onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback)
    {
        if (transform == nullptr ||
            material == nullptr ||
            program.idx != bgfx::kInvalidHandle ||
            renderMode != static_cast<int>(RenderMode::Deferred))
            return;

        Shader* shader = Renderer::getDefaultShader();

        if (material != nullptr && material->isLoaded() && material->getShader() != nullptr)
            shader = material->getShader();

        if (shader == nullptr || !shader->isLoaded())
            return;

        glm::mat4x4 mtx = transform->getTransformMatrix();
        glm::vec3 scale = transform->getScale();

        glm::mat4x4 m_projector_view = glm::lookAt(transform->getPosition(), transform->getPosition() - transform->getForward(), glm::cross(transform->getRight(), transform->getForward()));
        glm::mat4x4 m_projector_proj = glm::ortho(-scale.x, scale.x, -scale.y, scale.y, 0.1f, scale.z);
        glm::mat4x4 m_projector_view_proj = m_projector_proj * m_projector_view;

        uint64_t decalState = 0;

        Transform* cameraTransform = camera->getTransform();
        if (cameraTransform != nullptr)
        {
            if (Mathf::intersects(cameraTransform->getPosition(), 0.05f, getBounds()))
            {
                decalState = BGFX_STATE_CULL_CW;
            }
            else
            {
                decalState = BGFX_STATE_CULL_CCW | BGFX_STATE_DEPTH_TEST_LEQUAL;
            }
        }

        int passCount = 1;

        if (shader != nullptr && shader->isLoaded())
            passCount = shader->getPassCount();

        for (int j = 0; j < passCount; ++j)
        {
            Pass* pass = nullptr;

            if (shader != nullptr && shader->isLoaded())
                pass = shader->getPass(j);

            ProgramVariant* pv = nullptr;
            if (material != nullptr && material->isLoaded() && pass != nullptr)
                pv = pass->getProgramVariant(material->getDefinesStringHash());

            uint64_t passState = state;

            if (pv != nullptr)
                passState = pv->getRenderState(state);

            Primitives::cube();

            //Set transform
            bgfx::setTransform(glm::value_ptr(mtx), 1);

            // Set render states
            bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | decalState);

            // Pass uniforms
            if (pv != nullptr)
                material->submitUniforms(pv, camera);

            //Bind system uniforms
            bgfx::setUniform(Renderer::getSingleton()->getViewProjUniform(), glm::value_ptr(m_projector_view_proj), 1);
            bgfx::setUniform(Renderer::getSingleton()->getInvViewProjUniform(), glm::value_ptr(glm::inverse(m_projector_view_proj)), 1);

            if (preRenderCallback != nullptr)
                preRenderCallback();

            // Render
            bgfx::ProgramHandle ph = { bgfx::kInvalidHandle };

            if (pv != nullptr)
                ph = pv->programHandle;

            bgfx::submit(view, ph);
        }
    }
}