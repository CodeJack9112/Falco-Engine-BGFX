#pragma once

#include <vector>
#include <algorithm>

#include "Component.h"
#include "Renderable.h"

#include "../glm/glm.hpp"

#include "../Serialization/Assets/STerrainData.h"

#define MAX_TERRAIN_TEXTURES 5

namespace GX
{
    class Material;
    class Texture;
    class Terrain;
    class Mesh;

    struct TerrainTextureData
    {
    public:
        Texture* diffuseTexture = nullptr;
        Texture* normalTexture = nullptr;
        float worldSize = 100.0f;
    };

    struct TerrainTreeData
    {
        friend class Terrain;

    public:
        class TreeMesh : public Renderable
        {
            friend class Terrain;
            friend struct TerrainTreeData;

        private:
            struct PosTexCoord0Vertex
            {
                glm::vec3 pos = glm::vec3(0);
                glm::vec2 uv = glm::vec2(0);
            };

            TerrainTreeData* parent = nullptr;
            AxisAlignedBox cachedAAB = AxisAlignedBox::BOX_NULL;
            glm::mat4x4 cachedTransform = glm::mat4x4(FLT_MAX);
            glm::mat4x4 getTransform();

        public:
            TreeMesh(TerrainTreeData* _parent);
            virtual ~TreeMesh();

            glm::vec3 position = glm::vec3(0.0f);
            float scale = 1.0f;
            float rotation = 0.0f;

            virtual AxisAlignedBox getBounds(bool world = true);
            virtual bool isTransparent() { return false; }
            virtual bool getCastShadows();
            virtual void onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback);
            virtual bool checkCullingMask(LayerMask& mask);

            void updateTransform();
        };

    private:
        struct ModelMeshData
        {
        public:
            Mesh* mesh = nullptr;
            std::vector<Material*> materials;
            glm::mat4x4 transform = glm::identity<glm::mat4x4>();
        };

        Terrain* terrain = nullptr;

        std::string name = "";
        float minScale = 0.85f;
        float maxScale = 1.25f;

        std::vector<ModelMeshData> meshList;
        bgfx::TextureHandle impostorTexture = { bgfx::kInvalidHandle };

        std::vector<TreeMesh*> meshes;

        AxisAlignedBox bounds = AxisAlignedBox::BOX_NULL;
        void destroyImpostorTexture();
        void updateImpostorTexture();

    public:
        TerrainTreeData(Terrain* _terrain) { terrain = _terrain; }
        ~TerrainTreeData();

        void updateBounds();
        AxisAlignedBox getBounds() { return bounds; }

        std::string getName() { return name; }
        void setName(std::string value);

        float& getMinScale() { return minScale; }
        void setMinScale(float value);

        float& getMaxScale() { return maxScale; }
        void setMaxScale(float value);

        bgfx::TextureHandle getImpostorTexture() { return impostorTexture; }

        bool addTree(glm::vec2 pos, float density);
        void addTree(glm::vec3 position, float rotation, float scale);
        bool removeTree(glm::vec2 pos, float density);
        const std::vector<TreeMesh*>& getMeshes() { return meshes; }

        void destroy();
    };

    struct TerrainDetailMeshData
    {
        friend class Terrain;

    public:
        struct DetailMesh
        {
        public:
            glm::vec3 position = glm::vec3(0.0f);
            float scale = 1.0f;
            float rotation = 0.0f;
        };

        class Batch : public Renderable
        {
            friend class Terrain;
            friend struct TerrainDetailMeshData;

        private:
            struct SubBatch
            {
            public:
                struct VertexBuffer
                {
                    glm::vec3 position = glm::vec3(0, 0, 0);
                    glm::vec3 normal = glm::vec3(0, 0, 0);
                    glm::vec3 tangent = glm::vec3(0, 0, 0);
                    glm::vec3 bitangent = glm::vec3(0, 0, 0);
                    glm::vec2 texcoord0 = glm::vec2(0, 0);
                    glm::vec2 texcoord1 = glm::vec2(0, 0);
                };

                VertexBuffer* vertices = nullptr;
                uint32_t* indices = nullptr;
                uint32_t vertexCount = 0;
                uint32_t indexCount = 0;
                bgfx::VertexBufferHandle vbh = { bgfx::kInvalidHandle };
                bgfx::IndexBufferHandle ibh = { bgfx::kInvalidHandle };
            };

            TerrainDetailMeshData* parent = nullptr;
            AxisAlignedBox bounds = AxisAlignedBox::BOX_NULL;
            AxisAlignedBox cachedAAB = AxisAlignedBox::BOX_NULL;
            glm::mat4x4 prevTransform = glm::mat4x4(FLT_MAX);

            std::vector<DetailMesh> meshes;
            std::vector<std::vector<SubBatch>> subBatches;

            static void releaseMemory(void* _ptr, void* _userData);

        public:
            Batch(TerrainDetailMeshData* _parent);
            virtual ~Batch();

            virtual AxisAlignedBox getBounds(bool world = true);
            virtual bool isTransparent() { return false; }
            virtual bool getCastShadows();
            virtual void onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback);
            virtual bool checkCullingMask(LayerMask& mask);

            void destroy();
            void init();
            void update();

            std::vector<DetailMesh>& getMeshes() { return meshes; }
        };

    private:
        struct ModelMeshData
        {
        public:
            Mesh* mesh = nullptr;
            std::vector<Material*> materials;
            glm::mat4x4 transform = glm::identity<glm::mat4x4>();
        };

        Terrain* terrain = nullptr;

        std::string name = "";
        float minScale = 0.85f;
        float maxScale = 1.25f;

        int batchSize = 128;

        std::vector<ModelMeshData> meshList;
        bgfx::TextureHandle impostorTexture = { bgfx::kInvalidHandle };

        std::vector<Batch*> batches;

        AxisAlignedBox bounds = AxisAlignedBox::BOX_NULL;
        void destroyImpostorTexture();
        void updateImpostorTexture();

    public:
        TerrainDetailMeshData(Terrain* _terrain) { terrain = _terrain; }
        ~TerrainDetailMeshData();

        void updateBounds();
        AxisAlignedBox getBounds() { return bounds; }

        std::string getName() { return name; }
        void setName(std::string value);

        float& getMinScale() { return minScale; }
        void setMinScale(float value);

        float& getMaxScale() { return maxScale; }
        void setMaxScale(float value);

        bgfx::TextureHandle getImpostorTexture() { return impostorTexture; }

        bool addMesh(std::vector<glm::vec2> positions, float density);
        void addMesh(glm::vec3 position, float rotation, float scale);
        bool removeMesh(std::vector<glm::vec2> positions, float density);
        const std::vector<Batch*>& getBatches() { return batches; }

        void destroy();
    };

    struct TerrainGrassData
    {
        friend class Terrain;
        friend class Batch;

    public:
        struct GrassMesh
        {
        public:
            glm::vec3 position = glm::vec3(0.0f);
            float width = 0.0f;
            float height = 0.0f;
            float rotation = 0.0f;
        };

        class Batch : public Renderable
        {
            friend class Terrain;
            friend struct TerrainGrassData;

        private:
            struct VertexBuffer
            {
                glm::vec3 position = glm::vec3(0, 0, 0);
                glm::vec3 normal = glm::vec3(0, 0, 0);
                glm::vec3 tangent = glm::vec3(0, 0, 0);
                glm::vec3 bitangent = glm::vec3(0, 0, 0);
                glm::vec2 texcoord0 = glm::vec2(0, 0);
                glm::vec2 texcoord1 = glm::vec2(0, 0);
            };

            VertexBuffer* vertices = nullptr;
            uint32_t* indices = nullptr;
            uint32_t vertexCount = 0;
            uint32_t indexCount = 0;
            bgfx::VertexBufferHandle vbh = { bgfx::kInvalidHandle };
            bgfx::IndexBufferHandle ibh = { bgfx::kInvalidHandle };

            AxisAlignedBox bounds = AxisAlignedBox::BOX_NULL;
            AxisAlignedBox cachedAAB = AxisAlignedBox::BOX_NULL;
            glm::mat4x4 prevTransform = glm::mat4x4(FLT_MAX);

            TerrainGrassData* parent = nullptr;

            std::vector<GrassMesh> meshes;

        public:
            Batch(TerrainGrassData* grassData);
            virtual ~Batch();

            void init();
            void destroy();

            static void releaseMemory(void* _ptr, void* _userData);

            virtual AxisAlignedBox getBounds(bool world = true);
            virtual bool isTransparent() { return false; }
            virtual bool getCastShadows();
            virtual void onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback);
            virtual bool checkCullingMask(LayerMask& mask);
            void update();

            std::vector<GrassMesh>& getMeshes() { return meshes; }
        };

    private:
        Terrain* terrain = nullptr;
        Material* material = nullptr;
        glm::vec2 minSize = glm::vec2(1.0f, 1.0f);
        glm::vec2 maxSize = glm::vec2(2.0f, 2.0f);

        int batchSize = 2048;

        std::vector<Batch*> batches;

        void destroy();

    public:
        TerrainGrassData(Terrain* _terrain);
        ~TerrainGrassData();

        Material* getMaterial() { return material; }
        void setMaterial(Material* value);

        glm::vec2& getMinSize() { return minSize; }
        void setMinSize(glm::vec2 value);

        glm::vec2& getMaxSize() { return maxSize; }
        void setMaxSize(glm::vec2 value);

        bool addGrass(std::vector<glm::vec2> positions, float density);
        void addGrass(glm::vec3 position, float rotation, float width, float height);
        bool removeGrass(std::vector<glm::vec2> positions, float density);

        const std::vector<TerrainGrassData::Batch*>& getBatches() { return batches; }
    };

    class Terrain : public Renderable, public Component
    {
        friend struct TerrainTreeData;
        friend struct TerrainDetailMeshData;

    public:
        struct VertexBuffer
        {
            glm::vec3 position = glm::vec3(0, 0, 0);
            glm::vec3 normal = glm::vec3(0, 0, 0);
            glm::vec3 tangent = glm::vec3(0, 0, 0);
            glm::vec3 bitangent = glm::vec3(0, 0, 0);
            glm::vec2 texcoord0 = glm::vec2(0, 0);
            glm::vec2 texcoord1 = glm::vec2(0, 0);
        };

    private:
        static bgfx::UniformHandle textureDiffuseHandles[MAX_TERRAIN_TEXTURES];
        static bgfx::UniformHandle textureNormalHandles[MAX_TERRAIN_TEXTURES];
        static bgfx::UniformHandle textureSplatHandles[MAX_TERRAIN_TEXTURES];
        static bgfx::UniformHandle textureSizesHandle;
        static bgfx::UniformHandle textureCountHandle;
        static bgfx::UniformHandle albedoTextureHandle;

        bool isDirty = true;

        Transform* transform = nullptr;

        AxisAlignedBox bounds = AxisAlignedBox::Extent::EXTENT_INFINITE;
        AxisAlignedBox cachedAAB = AxisAlignedBox::Extent::EXTENT_INFINITE;
        glm::mat4x4 prevTransform = glm::mat4x4(FLT_MAX);

        bgfx::DynamicVertexBufferHandle vbh = { bgfx::kInvalidHandle };
        bgfx::DynamicIndexBufferHandle ibh = { bgfx::kInvalidHandle };

        float* heightMap = nullptr;
        VertexBuffer* vertices = nullptr;
        uint32_t* indices = nullptr;
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;

        Material* material = nullptr;
        std::string filePath = "";

        /* PROPERTIES */
        int size = 128;
        int worldSize = 256;
        float treeImpostorStartDistance = 90.0f;
        float treeDrawDistance = 250.0f;
        float detailMeshesDrawDistance = 40.0f;
        float grassDrawDistance = 50.0f;
        bool drawGrass = true;
        bool drawTrees = true;
        bool drawDetailMeshes = true;
        bool grassCastShadows = true;
        bool treesCastShadows = true;
        bool detailMeshesCastShadows = true;

        std::vector<Texture*> splatTextures;

        std::vector<TerrainTextureData> textureList;
        std::vector<TerrainTreeData*> treeList;
        std::vector<TerrainDetailMeshData*> detailMeshList;
        std::vector<TerrainGrassData*> grassList;

        void destroy();
        void create();
        void recreate();
        void updateTerrainMesh();
        void updateTerrain();
        void updateTreesTransforms();

        static void releaseData(void* _ptr, void* _userData);

    public:
        Terrain();
        virtual ~Terrain();

        static std::string COMPONENT_TYPE;
        virtual std::string getComponentType() { return COMPONENT_TYPE; }

        virtual AxisAlignedBox getBounds(bool world = true);
        virtual bool isTransparent() { return false; }
        virtual bool isStatic();
        virtual void onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback);
        virtual bool checkCullingMask(LayerMask& mask);
        virtual void onAttach();
        virtual void onDetach();
        virtual void onRefresh();
        virtual Component* onClone();

        Material* getMaterial() { return material; }
        void setMaterial(Material* value) { material = value; }

        int32_t getSize() { return size; }
        void setSize(int32_t value);

        int32_t getWorldSize() { return worldSize; }
        void setWorldSize(int32_t value);

        float getScale() { return (float)worldSize / (float)size; }

        float getTreeImpostorStartDistance() { return treeImpostorStartDistance; }
        void setTreeImpostorStartDistance(float value) { treeImpostorStartDistance = value; }

        float getTreeDrawDistance() { return treeDrawDistance; }
        void setTreeDrawDistance(float value) { treeDrawDistance = value; }

        float getDetailMeshesDrawDistance() { return detailMeshesDrawDistance; }
        void setDetailMeshesDrawDistance(float value) { detailMeshesDrawDistance = value; }

        float getGrassDrawDistance() { return grassDrawDistance; }
        void setGrassDrawDistance(float value) { grassDrawDistance = value; }

        std::vector<TerrainTextureData>& getTextures() { return textureList; }
        std::vector<TerrainTreeData*>& getTrees() { return treeList; }
        std::vector<TerrainDetailMeshData*>& getDetailMeshes() { return detailMeshList; }
        std::vector<TerrainGrassData*>& getGrass() { return grassList; }
        std::vector<Texture*>& getSplatTextures() { return splatTextures; }

        float* getHeightMap() { return heightMap; }
        VertexBuffer* getVertexBuffer() { return vertices; }
        uint32_t* getIndexBuffer() { return indices; }

        float getHeight(uint32_t x, uint32_t y);
        void setHeight(uint32_t x, uint32_t y, float height);

        float getHeightAtWorldPos(glm::vec3 worldPos);
        float getHeightAtWorldPosUnclamped(glm::vec3 worldPos);
        void setHeightAtWorldPos(glm::vec3 worldPos, float height);

        void setDrawGrass(bool value) { drawGrass = value; }
        bool getDrawGrass() { return drawGrass; }

        void setDrawTrees(bool value) { drawTrees = value; }
        bool getDrawTrees() { return drawTrees; }

        void setDrawDetailMeshes(bool value) { drawDetailMeshes = value; }
        bool getDrawDetailMeshes() { return drawDetailMeshes; }

        void setGrassCastShadows(bool value) { grassCastShadows = value; }
        bool getGrassCastShadows() { return grassCastShadows; }

        void setTreesCastShadows(bool value) { treesCastShadows = value; }
        bool getTreesCastShadows() { return treesCastShadows; }

        void setDetailMeshesCastShadows(bool value) { detailMeshesCastShadows = value; }
        bool getDetailMeshesCastShadows() { return detailMeshesCastShadows; }

        bool getIsDirty() { return isDirty; }
        void setIsDirty() { isDirty = true; }

        TerrainGrassData* addGrassData();
        void removeGrassData(int index);

        TerrainTreeData* addTreeData();
        void removeTreeData(int index);

        TerrainDetailMeshData* addDetailMeshData();
        void removeDetailMeshData(int index);

        void updatePositions(/*glm::vec3 _min, glm::vec3 _max*/);

        uint32_t getIndexCount() { return indexCount; }
        uint32_t getVertexCount() { return vertexCount; }

        std::string getFilePath() { return filePath; }

        void save();
        void save(std::string path);
        void load(std::string path);

        STerrainGrassData serializeGrass(TerrainGrassData* grass);
        TerrainGrassData* deserializeGrass(STerrainGrassData sGrass);

        STerrainTreeData serializeTree(TerrainTreeData* tree);
        TerrainTreeData* deserializeTree(STerrainTreeData sTree);

        STerrainDetailMeshData serializeDetailMesh(TerrainDetailMeshData* detailMesh);
        TerrainDetailMeshData* deserializeDetailMesh(STerrainDetailMeshData sDetailMesh);

        STerrainData serialize();
        void deserialize(STerrainData data, std::string path);
    };
}