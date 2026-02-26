#pragma once

#include <vector>

#include "VertexBuffer.h"

#include "../Components/Renderable.h"
#include "../Math/AxisAlignedBox.h"

namespace GX
{
    class Material;
    class Texture;

    class BatchedGeometry
    {
    public:
        class Batch : public Renderable
        {
            friend class BatchedGeometry;

        private:
            BatchedGeometry* parent = nullptr;
            Material* material = nullptr;

            AxisAlignedBox cachedAAB = AxisAlignedBox::BOX_NULL;

            std::string guid = "";

            std::vector<VertexBuffer> vertexBuffer;
            std::vector<uint32_t> indexBuffer;

            bgfx::VertexBufferHandle m_vbh = { bgfx::kInvalidHandle };
            bgfx::IndexBufferHandle m_ibh = { bgfx::kInvalidHandle };

            Texture* lightmap = nullptr;

            bool transparent = false;
            bool lightingStatic = false;
            bool useLightmapUVs = false;
            uint8_t lightmapSize = 0; //0 - default

            size_t layer = 0;
            int index = 0;

            int numMeshes = 0;

        public:
            Batch();
            virtual ~Batch();

            virtual AxisAlignedBox getBounds(bool world = true);
            virtual bool isTransparent() { return transparent; }
            virtual bool isStatic() { return lightingStatic; }
            virtual bool checkCullingMask(LayerMask& mask);
            virtual void onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback);

            Material* getMaterial() { return material; }
            void setMaterial(Material* value) { material = value; }

            std::vector<VertexBuffer>& getVertexBuffer() { return vertexBuffer; }
            std::vector<uint32_t>& getIndexBuffer() { return indexBuffer; }

            bgfx::VertexBufferHandle& getVertexBufferHandle() { return m_vbh; }
            bgfx::IndexBufferHandle& getIndexBufferHandle() { return m_ibh; }

            const size_t& getLayer() { return layer; }
            bool getLightingStatic() { return lightingStatic; }
            const std::string& getGuid() { return guid; }

            Texture* getLightmap() { return lightmap; }
            bool getUseLightmapUVs() { return useLightmapUVs; }
            int getLightmapSize() { return lightmapSize; }

            const int& getNumMeshes() { return numMeshes; }

            void reloadLightmap();

            void load();
            void unload();
        };

    private:
        std::vector<Batch*> batches;

        static BatchedGeometry singleton;

        bool _needRebuild = false;

    public:
        BatchedGeometry();
        ~BatchedGeometry();

        static BatchedGeometry* getSingleton() { return &singleton; }

        std::vector<Batch*>& getBatches() { return batches; }

        void rebuild(bool buildUvs = false);
        void reloadLightmaps();
        void clear();

        bool needRebuild() { return _needRebuild; }

        bool loadFromFile(std::string location, std::string name);
        void saveToFile(std::string location, std::string name);
    };
}