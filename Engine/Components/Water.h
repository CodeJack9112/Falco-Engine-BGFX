#pragma once

#include "Component.h"
#include "Renderable.h"

#include "../glm/glm.hpp"

namespace GX
{
    class Material;
    class RenderTexture;
    class Transform;

    class Water : public Renderable, public Component
    {
    public:
        struct VertexBuffer
        {
            glm::vec3 position = glm::vec3(0, 0, 0);
            glm::vec3 normal = glm::vec3(0, 0, 0);
            glm::vec3 tangent = glm::vec3(0, 0, 0);
            glm::vec3 bitangent = glm::vec3(0, 0, 0);
            glm::vec2 texcoord0 = glm::vec2(0, 0);
        };

    private:
        Transform* transform = nullptr;

        AxisAlignedBox bounds = AxisAlignedBox::Extent::EXTENT_INFINITE;
        AxisAlignedBox cachedAAB = AxisAlignedBox::Extent::EXTENT_INFINITE;
        glm::mat4x4 prevTransform = glm::mat4x4(FLT_MAX);

        bgfx::VertexBufferHandle vbh = { bgfx::kInvalidHandle };
        bgfx::IndexBufferHandle ibh = { bgfx::kInvalidHandle };

        int size = 120;
        bool reflections = true;
        bool reflectSkybox = true;
        bool reflectObjects = true;
        float reflectionsDistance = 100;
        int reflectionsQuality = 1; // 0 - low, 1 - normal, 2 = high

        VertexBuffer* vertices = nullptr;
        uint32_t* indices = nullptr;
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;

        Material* material = nullptr;
        Transform* cameraTransform = nullptr;
        RenderTexture* renderTarget = nullptr;
        bgfx::FrameBufferHandle gbuffer = { bgfx::kInvalidHandle };
        bgfx::FrameBufferHandle lightBuffer = { bgfx::kInvalidHandle };
        bgfx::TextureHandle gbufferTex[5] = { bgfx::kInvalidHandle, bgfx::kInvalidHandle, bgfx::kInvalidHandle, bgfx::kInvalidHandle, bgfx::kInvalidHandle };
        bgfx::TextureHandle lightBufferTex = { bgfx::kInvalidHandle };

        bgfx::UniformHandle u_reflectionTexture;
        int reflectionTexReg = 0;

        void destroy();
        void create();
        void recreate();

        void updateReflectionsCamera();
        void createReflections();
        void destroyReflections();

    public:
        Water();
        virtual ~Water();

        static std::string COMPONENT_TYPE;
        virtual std::string getComponentType() { return COMPONENT_TYPE; }

        virtual AxisAlignedBox getBounds(bool world = true);
        virtual bool isTransparent() { return false; }
        virtual void onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback);
        virtual bool checkCullingMask(LayerMask& mask);
        virtual void onAttach();
        virtual void onDetach();
        virtual Component* onClone();
        virtual void onScreenResized(int w, int h);

        Material* getMaterial() { return material; }
        void setMaterial(Material* value) { material = value; }

        int32_t getSize() { return size; }
        void setSize(int32_t value);

        bool getReflections() { return reflections; }
        void setReflections(bool value);

        bool getReflectSkybox() { return reflectSkybox; }
        void setReflectSkybox(bool value) { reflectSkybox = value; }

        bool getReflectObjects() { return reflectObjects; }
        void setReflectObjects(bool value) { reflectObjects = value; }

        float getReflectionsDistance() { return reflectionsDistance; }
        void setReflectionsDistance(float value) { reflectionsDistance = value; }

        int getReflectionsQuality() { return reflectionsQuality; }
        void setReflectionsQuality(int value);

        uint32_t getIndexCount() { return indexCount; }
        uint32_t getVertexCount() { return vertexCount; }
    };
}