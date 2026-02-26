#pragma once

#include <functional>
#include <bgfx/bgfx.h>

#include "../Math/AxisAlignedBox.h"
#include "../Core/LayerMask.h"

namespace GX
{
    class Transform;
    class Camera;

    class Renderable
    {
    protected:
        bool castShadows = true;
        int renderQueue = 0;

        void attach();
        void detach();

    public:
        Renderable();
        virtual ~Renderable();

        virtual AxisAlignedBox getBounds(bool world = true) = 0;
        virtual bool isTransparent() = 0;
        virtual void onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback) = 0;
        virtual bool isAlwaysVisible() { return false; }

        virtual bool checkCullingMask(LayerMask& mask) { return true; }
        virtual bool getCastShadows() { return castShadows; }
        void setCastShadows(bool value) { castShadows = value; }
        virtual bool isStatic() { return false; }
        virtual bool isDecal() { return false; }
        virtual bool getSkipRendering() { return false; }

        int getRenderQueue() { return renderQueue; }
        void setRenderQueue(int value) { renderQueue = value; }
    };
}