#pragma once

#include "../glm/vec2.hpp"
#include "../glm/vec3.hpp"
#include "../glm/vec4.hpp"

namespace GX
{
    struct VertexBuffer
    {
        // 3d space position
        glm::vec3 position = glm::vec3(0, 0, 0);
        glm::vec3 normal = glm::vec3(0, 0, 0);
        glm::vec3 tangent = glm::vec3(0, 0, 0);
        glm::vec3 bitangent = glm::vec3(0, 0, 0);
        glm::vec2 texcoord0 = glm::vec2(0, 0);
        glm::vec2 texcoord1 = glm::vec2(0, 0);
        glm::vec4 blendWeights = glm::vec4(0, 0, 0, 0);
        glm::vec4 blendIndices = glm::vec4(0, 0, 0, 0);

        // Color value
        uint32_t color = 0xffffffff;
    };
}