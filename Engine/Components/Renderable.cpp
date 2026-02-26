#include "Renderable.h"

#include <algorithm>
#include <vector>
#include "../Renderer/Renderer.h"

namespace GX
{
    Renderable::Renderable()
    {
        attach();
    }

    Renderable::~Renderable()
    {
        detach();
    }

    void Renderable::attach()
    {
        std::vector<Renderable*>& renderers = Renderer::getSingleton()->renderables;
        auto it = std::find(renderers.begin(), renderers.end(), this);

        if (it == renderers.end())
            Renderer::getSingleton()->renderables.push_back(this);
    }

    void Renderable::detach()
    {
        std::vector<Renderable*>& renderers = Renderer::getSingleton()->renderables;
        auto it = std::find(renderers.begin(), renderers.end(), this);

        if (it != renderers.end())
            renderers.erase(it);
    }
}