#pragma once

#include <vector>

#include "VertexBuffer.h"

#include "../Components/Renderable.h"
#include "../Math/AxisAlignedBox.h"

class btBvhTriangleMeshShape;
class btRigidBody;
class btDefaultMotionState;
class btTriangleMesh;

namespace GX
{
    class Material;
    class Texture;
    class CSGModel;
    class CSGBrush;
    class GameObject;
    class Transform;
    class Component;

    class CSGGeometry
    {
    public:
        class SubMesh : public Renderable
        {
            friend class CSGGeometry;

        private:
            void* parent = nullptr;
            Material* material = nullptr;

            AxisAlignedBox cachedAAB = AxisAlignedBox::BOX_NULL;

            std::string guid = "";

            std::vector<VertexBuffer> vertexBuffer;
            std::vector<uint32_t> indexBuffer;
            std::vector<unsigned long long> idBuffer;

            bgfx::VertexBufferHandle m_vbh = { bgfx::kInvalidHandle };
            bgfx::IndexBufferHandle m_ibh = { bgfx::kInvalidHandle };

            Texture* lightmap = nullptr;

            size_t layer = 0;

            int numFaces = 0;

        public:
            SubMesh();
            virtual ~SubMesh();

            virtual AxisAlignedBox getBounds(bool world = true);
            virtual bool isTransparent();
            virtual bool isStatic() { return true; }
            virtual bool checkCullingMask(LayerMask& mask);
            virtual void onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback);

            Material* getMaterial() { return material; }
            void setMaterial(Material* value) { material = value; }

            std::vector<VertexBuffer>& getVertexBuffer() { return vertexBuffer; }
            std::vector<uint32_t>& getIndexBuffer() { return indexBuffer; }
            std::vector<unsigned long long>& getIdBuffer() { return idBuffer; }

            bgfx::VertexBufferHandle& getVertexBufferHandle() { return m_vbh; }
            bgfx::IndexBufferHandle& getIndexBufferHandle() { return m_ibh; }

            const size_t& getLayer() { return layer; }
            const std::string& getGuid() { return guid; }

            const int& getNumFaces() { return numFaces; }

            Texture* getLightmap() { return lightmap; }

            void reloadLightmap();

            void load();
            void unload();
        };

        struct Model
        {
        public:
            ~Model();

            CSGModel* component = nullptr;
            CSGGeometry* parent = nullptr;
            std::vector<SubMesh*> subMeshes;
            btTriangleMesh* colMesh = nullptr;
            btBvhTriangleMeshShape* collider = nullptr;
            btRigidBody* rigidbody = nullptr;
            btDefaultMotionState* motionState = nullptr;

            void clear();
            void reloadLightmaps();
            void deleteColliders();
            void updateColliders();

            bool _needRebuild = false;
        };

    private:
        static CSGGeometry singleton;

        CSGBrush* nullBrush = nullptr;

        static Material* defaultMaterial;

        std::vector<Model*> models;

        void loadMaterial();

    public:
        CSGGeometry();
        ~CSGGeometry();

        static CSGGeometry* getSingleton() { return &singleton; }
        static Material* getDefaultMaterial() { return defaultMaterial; }

        std::vector<Model*>& getModels() { return models; }

        void rebuild(CSGModel* target, bool buildUvs = false);
        void rebuild(CSGBrush* brush, bool buildUvs = false);
        void rebuild(std::vector<Component*> brushes, bool buildUvs = false);
        void clear();
        void reloadLightmaps();
        
        bool loadFromFile(std::string location, std::string name, CSGModel* comp);
        void saveToFile(std::string location, std::string name, CSGModel* comp);

        CSGModel* getModel(Transform* t);
        CSGModel* getModel(GameObject* obj);
        CSGModel* getModel(CSGBrush* brush);
    };
}