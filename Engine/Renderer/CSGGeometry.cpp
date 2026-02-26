#include "CSGGeometry.h"

#include "../glm/gtc/type_ptr.hpp"

#include "../Assets/Material.h"
#include "../Assets/Shader.h"
#include "../Assets/Mesh.h"
#include "../Assets/Texture.h"
#include "../Assets/Scene.h"
#include "../Assets/Material.h"

#include "../Components/MeshRenderer.h"
#include "../Components/Transform.h"
#include "../Components/Camera.h"
#include "../Components/Light.h"
#include "../Components/CSGModel.h"
#include "../Components/CSGBrush.h"

#include "../Core/Engine.h"
#include "../Core/GameObject.h"
#include "../Core/PhysicsManager.h"
#include "../Core/Debug.h"

#include "../Renderer/Renderer.h"
#include "../Renderer/VertexLayouts.h"

#include "../Classes/IO.h"
#include "../Classes/md5.h"
#include "../Classes/xatlas.h"
#include "../Classes/GUIDGenerator.h"

#include "../Serialization/Scene/SCSGGeometry.h"

#include <carve/interpolator.hpp>
#include <carve/csg_triangulator.hpp>
#include <carve/carve.hpp>
#include <carve/csg.hpp>
#include <carve/input.hpp>

#include "../Bullet/include/btBulletCollisionCommon.h"
#include "../Bullet/include/btBulletDynamicsCommon.h"

namespace GX
{
    CSGGeometry CSGGeometry::singleton;
    Material* CSGGeometry::defaultMaterial = nullptr;

    //Batch

    CSGGeometry::SubMesh::SubMesh()
    {

    }

    CSGGeometry::SubMesh::~SubMesh()
    {
        unload();
    }

    void CSGGeometry::SubMesh::load()
    {
        if (!bgfx::isValid(m_vbh) && vertexBuffer.size() > 0)
        {
            m_vbh = bgfx::createVertexBuffer(
                bgfx::makeRef(vertexBuffer.data(), vertexBuffer.size() * sizeof(VertexBuffer)),
                VertexLayouts::subMeshVertexLayout
            );
        }

        if (!bgfx::isValid(m_ibh) && indexBuffer.size() > 0)
        {
            m_ibh = bgfx::createIndexBuffer(
                bgfx::makeRef(indexBuffer.data(), indexBuffer.size() * sizeof(uint32_t)), BGFX_BUFFER_INDEX32
            );
        }

        //vertexBuffer.clear();
        //indexBuffer.clear();

        attach();
    }

    void CSGGeometry::SubMesh::unload()
    {
        if (bgfx::isValid(m_vbh))
            bgfx::destroy(m_vbh);

        if (bgfx::isValid(m_ibh))
            bgfx::destroy(m_ibh);

        m_vbh = { bgfx::kInvalidHandle };
        m_ibh = { bgfx::kInvalidHandle };

        vertexBuffer.clear();
        indexBuffer.clear();
        idBuffer.clear();

        cachedAAB = AxisAlignedBox::BOX_NULL;

        layer = 0;

        if (lightmap != nullptr)
            lightmap->unload();

        lightmap = nullptr;

        detach();
    }

    AxisAlignedBox CSGGeometry::SubMesh::getBounds(bool world)
    {
        return cachedAAB;
    }

    bool CSGGeometry::SubMesh::isTransparent()
    {
        bool transp = false;

        Shader* shader = nullptr;

        if (material != nullptr && material->isLoaded())
            shader = material->getShader();

        if (shader != nullptr)
        {
            for (int j = 0; j < shader->getPassCount(); ++j)
            {
                Pass* pass = shader->getPass(j);
                ProgramVariant* pv = nullptr;
                if (material != nullptr && material->isLoaded() && pass != nullptr)
                    pv = pass->getProgramVariant(material->getDefinesStringHash());
                if (pv != nullptr && pv->blendMode == BlendMode::Alpha)
                {
                    transp = true;
                    break;
                }
            }
        }
 
        return transp;
    }

    bool CSGGeometry::SubMesh::checkCullingMask(LayerMask& mask)
    {
        return mask.getLayer(layer);
    }

    void CSGGeometry::SubMesh::onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback)
    {
        Shader* shader = Renderer::getDefaultShader();

        if (material != nullptr && material->isLoaded() && material->getShader() != nullptr)
            shader = material->getShader();

        if (program.idx == bgfx::kInvalidHandle)
        {
            if (shader == nullptr || !shader->isLoaded())
                return;

            if (renderMode != static_cast<int>(shader->getRenderMode()))
                return;
        }

        //

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

            int iterationCount = 1;

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

                if (program.idx == bgfx::kInvalidHandle)
                {
                    if (pv != nullptr)
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
                                Light* light = Renderer::getSingleton()->getFirstLight();
                                if (light != nullptr)
                                    light->submitUniforms();
                            }
                        }
                    }
                }

                if (pv != nullptr)
                    passState = pv->getRenderState(state);

                glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
                glm::mat4x4 invMtx = glm::inverse(mtx);

                bgfx::setTransform(glm::value_ptr(mtx));

                // Set vertex and index buffer.
                bgfx::setVertexBuffer(0, m_vbh);
                bgfx::setIndexBuffer(m_ibh);

                // Set render states.
                bgfx::setState(passState);

                // Pass uniforms
                if (program.idx == bgfx::kInvalidHandle)
                {
                    if (pv != nullptr)
                        material->submitUniforms(pv, camera);
                }
                else
                {
                    bgfx::UniformHandle tex = Renderer::getSingleton()->getAlbedoMapUniform();
                    auto& uniforms = material->getUniforms();
                    bool textureWasSet = false;
                    for (auto& u : uniforms)
                    {
                        if (u.getType() == UniformType::Sampler2D)
                        {
                            const UniformVariant* var = pv->getUniform(u.getNameHash());
                            if (var != nullptr)
                            {
                                Texture* tx = u.getValue<Sampler2DDef>().second;
                                if (tx != nullptr)
                                {
                                    bgfx::setTexture(0, tex, tx->getHandle());
                                    textureWasSet = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (!textureWasSet)
                    {
                        Texture* nullTexture = Texture::getNullTexture();
                        bgfx::setTexture(0, tex, nullTexture->getHandle());
                    }
                }

                glm::mat3x3 nrmMtx = mtx;

                //Bind system uniforms
                bgfx::setUniform(Renderer::getNormalMatrixUniform(), glm::value_ptr(nrmMtx), 1);
                bgfx::setUniform(Renderer::getGpuSkinningUniform(), glm::value_ptr(glm::vec4(0.0f)), 1);
                bgfx::setUniform(Renderer::getInvModelUniform(), glm::value_ptr(invMtx), 1);

                bool useLightmaps = lightmap != nullptr && ((Model*)parent)->_needRebuild == false;
                bgfx::setUniform(Renderer::getHasLightmapUniform(), glm::value_ptr(glm::vec4(useLightmaps ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f)), 1);

                if (useLightmaps)
                    bgfx::setTexture(8, Renderer::getLightmapUniform(), lightmap->getHandle());

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

    void CSGGeometry::SubMesh::reloadLightmap()
    {
        if (lightmap != nullptr)
            lightmap->unload();

        lightmap = nullptr;

        std::string loadedScene = Scene::getLoadedScene();
        std::string dirPath = IO::GetFilePath(loadedScene) + IO::GetFileName(loadedScene) + "/Lightmaps/";
        std::string fileName = dirPath + guid + ".jpg";

        std::string location = Engine::getSingleton()->getAssetsPath();

        Texture* _lightmap = Texture::load(location, fileName, true, Texture::CompressionMethod::Default, false, false);
        if (_lightmap != nullptr && _lightmap->isLoaded())
            lightmap = _lightmap;
    }

    //CSG geometry

    CSGGeometry::CSGGeometry()
    {
        nullBrush = new CSGBrush();
    }

    CSGGeometry::~CSGGeometry()
    {
        if (nullBrush != nullptr)
            delete nullBrush;

        nullBrush = nullptr;
    }

    void CSGGeometry::loadMaterial()
    {
        if (defaultMaterial == nullptr)
            defaultMaterial = Material::create("system/csg/materials/", "default");

        if (!defaultMaterial->isLoaded())
            defaultMaterial->load();

        defaultMaterial->setPersistent(true);

        Shader* shader = Shader::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Shaders/StandardDeferred.shader");
        defaultMaterial->setShader(shader);

        Texture* tex1 = Texture::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Textures/CSG/Wall.png");

        auto param1 = defaultMaterial->getUniform<Sampler2DDef>("albedoMap");
        defaultMaterial->setUniform("HAS_ALBEDO_MAP", true, true);
        defaultMaterial->setUniform("roughnessVal", 0.8f, false);

        defaultMaterial->setUniform<Sampler2DDef>("albedoMap", std::make_pair(param1.first, tex1), false);
    }

    void CSGGeometry::Model::deleteColliders()
    {
        if (colMesh != nullptr)
            delete colMesh;

        if (collider != nullptr)
            delete collider;

        if (motionState != nullptr)
            delete motionState;

        if (rigidbody != nullptr)
        {
            PhysicsManager::getSingleton()->getWorld()->removeRigidBody(rigidbody);
            delete rigidbody;
        }

        colMesh = nullptr;
        collider = nullptr;
        rigidbody = nullptr;
        motionState = nullptr;
    }

    void CSGGeometry::Model::updateColliders()
    {
        //Delete old
        deleteColliders();

        if (!Engine::getSingleton()->getIsRuntimeMode())
            return;

        //Create new
        btTransform startTransform;
        btVector3 localInertia(0, 0, 0);
        startTransform.setIdentity();
        startTransform.setOrigin(btVector3(0, 0, 0));

        //Create collider
        colMesh = new btTriangleMesh();

        for (int sm = 0; sm < subMeshes.size(); ++sm)
        {
            CSGGeometry::SubMesh* subMesh = subMeshes[sm];

            std::vector<VertexBuffer>& vbuf = subMesh->getVertexBuffer();
            std::vector<uint32_t>& ibuf = subMesh->getIndexBuffer();

            for (int i = 0; i < ibuf.size(); i += 3)
            {
                glm::vec3 pt1 = vbuf[ibuf[i]].position;
                glm::vec3 pt2 = vbuf[ibuf[i + 1]].position;
                glm::vec3 pt3 = vbuf[ibuf[i + 2]].position;

                colMesh->addTriangle(
                    btVector3(btScalar(pt1.x), btScalar(pt1.y), btScalar(pt1.z)),
                    btVector3(btScalar(pt2.x), btScalar(pt2.y), btScalar(pt2.z)),
                    btVector3(btScalar(pt3.x), btScalar(pt3.y), btScalar(pt3.z)));
            }
        }

        if (colMesh->getNumTriangles() > 0)
        {
            collider = new btBvhTriangleMeshShape(colMesh, true);
            collider->recalcLocalAabb();
        }
        //

        motionState = new btDefaultMotionState(startTransform);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, collider, localInertia);
        rigidbody = new btRigidBody(rbInfo);

        rigidbody->setActivationState(DISABLE_DEACTIVATION);
        rigidbody->setMassProps(0, localInertia);
        rigidbody->setDamping(0.0f, 0.0f);
        rigidbody->updateInertiaTensor();
        rigidbody->clearForces();

        PhysicsManager::getSingleton()->getWorld()->addRigidBody(rigidbody);

        rigidbody->setFriction(0.85f);
        rigidbody->setRestitution(0.25f);
    }

    void CSGGeometry::Model::reloadLightmaps()
    {
        for (auto* subMesh : subMeshes)
            subMesh->reloadLightmap();
    }

    CSGGeometry::Model::~Model()
    {
        clear();
    }

    void CSGGeometry::Model::clear()
    {
        for (auto subMesh : subMeshes)
            delete subMesh;

        subMeshes.clear();

        deleteColliders();

        Renderer::getSingleton()->frame();
    }

    void CSGGeometry::rebuild(CSGModel* target, bool buildUvs)
    {
        loadMaterial();

        auto& objects = Engine::getSingleton()->getGameObjects();

        for (auto& object : objects)
        {
            CSGModel* model = (CSGModel*)object->getComponent(CSGModel::COMPONENT_TYPE);
            if (model == nullptr)
                continue;

            if (target != nullptr)
            {
                if (target != model)
                    continue;
            }

            Transform* modelTrans = object->getTransform();

            Model* modelData = nullptr;
            auto it = std::find_if(models.begin(), models.end(), [=](Model* m) -> bool { return m->component == model; });
            if (it != models.end())
            {
                modelData = *it;
            }
            else
            {
                modelData = new Model();
                modelData->component = model;
                modelData->parent = this;

                models.push_back(modelData);
            }

            modelData->clear();

            //CSG
            carve::csg::CSG csg;

            carve::interpolate::FaceVertexAttr<CSGBrush::uv_t> fv_uv;
            carve::interpolate::FaceAttr<Material*> f_material;
            carve::interpolate::FaceAttr<int> f_layer;
            carve::interpolate::FaceAttr<bool> f_castShadows;
            carve::interpolate::FaceAttr<bool> f_smoothNormals;
            carve::interpolate::FaceAttr<size_t> f_brushId;

            carve::poly::Polyhedron* csgGeom = nullptr;

            nullBrush->rebuild();

            //Bind attributes
            std::vector<Transform*> nstack;
            for (auto trans : modelTrans->getChildren())
            {
                if (trans->getGameObject()->getComponent(CSGModel::COMPONENT_TYPE) != nullptr)
                    continue;

                nstack.push_back(trans);
            }

            while (nstack.size() > 0)
            {
                Transform* child = *nstack.begin();
                nstack.erase(nstack.begin());

                //
                GameObject* chObj = child->getGameObject();
                if (!chObj->getActive())
                    continue;

                if (chObj->getComponent(CSGModel::COMPONENT_TYPE) != nullptr)
                    continue;

                CSGBrush* brush = (CSGBrush*)chObj->getComponent(CSGBrush::COMPONENT_TYPE);

                if (brush != nullptr)
                {
                    if (!brush->getEnabled())
                        continue;

                    brush->bind(&fv_uv, &f_material, &f_layer, &f_castShadows, &f_smoothNormals, &f_brushId);
                }
                //

                int j = 0;
                for (auto it = child->getChildren().begin(); it != child->getChildren().end(); ++it, ++j)
                {
                    Transform* ch = *it;
                    nstack.insert(nstack.begin() + j, ch);
                }
            }

            fv_uv.installHooks(csg);
            f_material.installHooks(csg);
            f_layer.installHooks(csg);
            f_castShadows.installHooks(csg);
            f_smoothNormals.installHooks(csg);
            f_brushId.installHooks(csg);

            csg.hooks.registerHook(new carve::csg::CarveTriangulatorWithImprovement(), carve::csg::CSG::Hooks::PROCESS_OUTPUT_FACE_BIT);

            //Compute CSG
            nstack.clear();
            for (auto trans : modelTrans->getChildren())
            {
                if (trans->getGameObject()->getComponent(CSGModel::COMPONENT_TYPE) != nullptr)
                    continue;

                nstack.push_back(trans);
            }

            while (nstack.size() > 0)
            {
                Transform* child = *nstack.begin();
                nstack.erase(nstack.begin());

                //
                GameObject* chObj = child->getGameObject();
                if (!chObj->getActive())
                    continue;

                if (chObj->getComponent(CSGModel::COMPONENT_TYPE) != nullptr)
                    continue;

                CSGBrush* brush = (CSGBrush*)chObj->getComponent(CSGBrush::COMPONENT_TYPE);

                if (brush != nullptr)
                {
                    if (!brush->getEnabled())
                        continue;

                    glm::mat4x4 mtx = child->getTransformMatrix();
                    glm::quat nrmMtx = child->getRotation();

                    carve::poly::Polyhedron* brushPtr = brush->getBrushPtr();
                    carve::poly::Polyhedron* prevCSG = csgGeom;
                    carve::csg::CSG::OP op = carve::csg::CSG::OP::UNION;

                    if (brush->getBrushOperation() == CSGBrush::BrushOperation::Subtract)
                        op = carve::csg::CSG::OP::A_MINUS_B;

                    try
                    {
                        if (csgGeom == nullptr)
                            csgGeom = csg.compute(nullBrush->getBrushPtr(), brushPtr, op);
                        else
                            csgGeom = csg.compute(csgGeom, brushPtr, op);
                    }
                    catch (carve::exception e)
                    {
                        prevCSG = nullptr;
                        //Debug::log("CSG build error", Debug::DbgColorRed);
                    }

                    if (prevCSG != nullptr && prevCSG != nullBrush->getBrushPtr())
                        delete prevCSG;
                }
                //

                int j = 0;
                for (auto it = child->getChildren().begin(); it != child->getChildren().end(); ++it, ++j)
                {
                    Transform* ch = *it;
                    nstack.insert(nstack.begin() + j, ch);
                }
            }

            int maxSubMeshSize = 512;

            //Build meshes
            if (csgGeom != nullptr)
            {
                for (size_t i = 0; i < csgGeom->faces.size(); ++i)
                {
                    auto* f = &csgGeom->faces[i];

                    Material* mat = defaultMaterial;
                    int layer = 0;
                    bool castShadows = true;
                    bool smoothNormals = false;
                    size_t brushId = 0;

                    if (f_material.hasAttribute(f))
                    {
                        Material* mt = f_material.getAttribute(f);
                        if (mt != nullptr)
                            mat = mt;
                    }

                    if (f_layer.hasAttribute(f))
                        layer = f_layer.getAttribute(f);

                    if (f_castShadows.hasAttribute(f))
                        castShadows = f_castShadows.getAttribute(f);

                    if (f_smoothNormals.hasAttribute(f))
                        smoothNormals = f_smoothNormals.getAttribute(f);

                    if (f_brushId.hasAttribute(f))
                        brushId = f_brushId.getAttribute(f);

                    CSGGeometry::SubMesh* subMesh = nullptr;

                    auto it = std::find_if(modelData->subMeshes.begin(), modelData->subMeshes.end(), [=](CSGGeometry::SubMesh* b) -> bool
                        {
                            bool match = b->getMaterial() == mat &&
                                b->getLayer() == layer &&
                                b->getCastShadows() == castShadows &&
                                b->getNumFaces() < maxSubMeshSize;

                            return match;
                        }
                    );

                    if (it != modelData->subMeshes.end())
                    {
                        subMesh = *it;
                    }
                    else
                    {
                        //Create new
                        subMesh = new CSGGeometry::SubMesh();
                        subMesh->setMaterial(mat);
                        subMesh->parent = modelData;
                        subMesh->setCastShadows(castShadows);
                        subMesh->layer = layer;
                        subMesh->guid = md5(GUIDGenerator::genGuid());

                        modelData->subMeshes.push_back(subMesh);
                    }

                    subMesh->numFaces += 1;

                    auto& subMeshVb = subMesh->getVertexBuffer();
                    auto& subMeshIb = subMesh->getIndexBuffer();
                    auto& subMeshId = subMesh->getIdBuffer();

                    int idx = subMeshIb.size();

                    for (size_t j = 0; j < 3; ++j)
                    {
                        carve::geom3d::Vector v = f->vertex(j)->v;
                        CSGBrush::uv_t uv = CSGBrush::uv_t(0, 0);

                        if (fv_uv.hasAttribute(f, j))
                            uv = fv_uv.getAttribute(f, j);

                        GX::VertexBuffer vbuf;
                        vbuf.position = glm::vec3(v.x, v.y, v.z);
                        vbuf.texcoord0 = glm::vec2(uv.u, uv.v);
                        vbuf.normal = glm::vec3(smoothNormals);

                        subMesh->cachedAAB.merge(vbuf.position);

                        subMeshVb.push_back(vbuf);
                        subMeshIb.push_back(idx);
                        subMeshId.push_back(brushId);

                        ++idx;
                    }
                }

                if (csgGeom != nullptr && csgGeom != nullBrush->getBrushPtr())
                    delete csgGeom;
            }

            for (auto* subMesh : modelData->subMeshes)
            {
                auto& subMeshVb = subMesh->getVertexBuffer();
                auto& subMeshIb = subMesh->getIndexBuffer();

                //Calculate normals + tangents + bitangents
                for (uint32_t i = 0; i < subMeshIb.size(); i += 3)
                {
                    //Normals
                    uint32_t id0 = subMeshIb[i + 0];
                    uint32_t id1 = subMeshIb[i + 1];
                    uint32_t id2 = subMeshIb[i + 2];

                    glm::vec3 v1 = subMeshVb[id0].position;
                    glm::vec3 v2 = subMeshVb[id1].position;
                    glm::vec3 v3 = subMeshVb[id2].position;

                    glm::vec3 normalA = glm::normalize(glm::cross(v2 - v1, v3 - v1));

                    subMeshVb[id0].normal = normalA;
                    subMeshVb[id1].normal = normalA;
                    subMeshVb[id2].normal = normalA;

                    //Tangent space
                    glm::vec3 deltaPos = glm::vec3(0);
                    if (v1 == v2)
                        deltaPos = v3 - v1;
                    else
                        deltaPos = v2 - v1;

                    glm::vec2 uv0 = subMeshVb[id0].texcoord0;
                    glm::vec2 uv1 = subMeshVb[id1].texcoord0;
                    glm::vec2 uv2 = subMeshVb[id2].texcoord0;

                    glm::vec2 deltaUV1 = uv1 - uv0;
                    glm::vec2 deltaUV2 = uv2 - uv0;

                    glm::vec3 tan = glm::vec3(0); // tangent
                    glm::vec3 bin = glm::vec3(0); // binormal

                    // avoid division with 0
                    if (deltaUV1.s != 0)
                        tan = deltaPos / deltaUV1.s;
                    else
                        tan = deltaPos / 1.0f;

                    tan = glm::normalize(tan - glm::dot(normalA, tan) * normalA);
                    bin = glm::normalize(glm::cross(tan, normalA));

                    subMeshVb[id0].tangent = tan;
                    subMeshVb[id0].bitangent = bin;
                    subMeshVb[id1].tangent = tan;
                    subMeshVb[id1].bitangent = bin;
                    subMeshVb[id2].tangent = tan;
                    subMeshVb[id2].bitangent = bin;
                }

                if (buildUvs)
                {
                    auto _subMeshVb = subMesh->getVertexBuffer();
                    auto _subMeshIb = subMesh->getIndexBuffer();
                    auto _subMeshId = subMesh->getIdBuffer();

                    std::vector<glm::vec3> vertices;
                    std::vector<glm::vec3> normals;
                    std::vector<glm::vec2> uvs;

                    vertices.resize(_subMeshIb.size());
                    normals.resize(_subMeshIb.size());
                    uvs.resize(_subMeshIb.size());

                    for (int k = 0; k < _subMeshVb.size(); ++k)
                    {
                        vertices[k] = _subMeshVb[k].position;
                        normals[k] = _subMeshVb[k].normal;
                        uvs[k] = _subMeshVb[k].texcoord0;
                    }

                    //Gen lightmap UVs
                    xatlas::Atlas* atlas = xatlas::Create();
                    xatlas::MeshDecl mesh;
                    mesh.indexCount = _subMeshIb.size();
                    mesh.indexFormat = xatlas::IndexFormat::UInt32;
                    mesh.indexData = _subMeshIb.data();

                    mesh.vertexCount = vertices.size();
                    mesh.vertexPositionData = vertices.data();
                    mesh.vertexPositionStride = sizeof(float) * 3;
                    mesh.vertexNormalData = normals.data();
                    mesh.vertexNormalStride = sizeof(float) * 3;
                    mesh.vertexUvData = uvs.data();
                    mesh.vertexUvStride = sizeof(float) * 2;

                    xatlas::ChartOptions chartOptions = xatlas::ChartOptions();
                    xatlas::PackOptions packOptions = xatlas::PackOptions();
                    packOptions.padding = 4.0f;

                    xatlas::AddMesh(atlas, mesh);
                    xatlas::Generate(atlas, chartOptions, packOptions);

                    std::vector<int> vertIndexes;
                    std::vector<glm::vec2> lightmapUVs;

                    if (atlas->meshCount > 0)
                    {
                        xatlas::Mesh output_mesh = atlas->meshes[0];

                        _subMeshIb.resize(output_mesh.indexCount);
                        _subMeshVb.resize(output_mesh.vertexCount);
                        _subMeshId.resize(output_mesh.vertexCount);

                        lightmapUVs.resize(output_mesh.vertexCount);
                        vertIndexes.resize(output_mesh.vertexCount);

                        for (int j = 0; j < output_mesh.indexCount; ++j)
                        {
                            int idx = output_mesh.indexArray[j];
                            _subMeshIb[j] = idx;
                        }

                        for (int j = 0; j < output_mesh.vertexCount; ++j)
                        {
                            xatlas::Vertex vertex = output_mesh.vertexArray[j];

                            float x = vertex.uv[0] / atlas->width;
                            float y = vertex.uv[1] / atlas->height;

                            vertIndexes[j] = vertex.xref;
                            lightmapUVs[j] = glm::vec2(x, y);
                        }

                        for (int jj = 0; jj < vertIndexes.size(); ++jj)
                        {
                            uint32_t j = vertIndexes[jj];

                            _subMeshVb[jj] = subMesh->getVertexBuffer()[j];
                            _subMeshVb[jj].texcoord1 = lightmapUVs[jj];

                            _subMeshId[jj] = subMesh->getIdBuffer()[j];
                        }

                        subMesh->vertexBuffer = _subMeshVb;
                        subMesh->indexBuffer = _subMeshIb;
                        subMesh->idBuffer = _subMeshId;
                    }

                    xatlas::Destroy(atlas);

                    _subMeshVb.clear();
                    _subMeshIb.clear();
                    _subMeshId.clear();

                    lightmapUVs.clear();
                    vertIndexes.clear();

                    vertices.clear();
                    normals.clear();
                    uvs.clear();

                    subMesh->reloadLightmap();
                }

                subMesh->load();
            }

            if (modelData->subMeshes.size() > 0)
            {
                modelData->_needRebuild = !buildUvs;
                modelData->updateColliders();
            }
            else
            {
                modelData->_needRebuild = false;
            }
        }

        std::vector<Model*> del;
        for (auto model : models)
        {
            size_t hash = model->component->getGameObject()->getGuidHash();

            if (Engine::getSingleton()->getGameObject(hash) == nullptr)
                model->clear();

            if (model->subMeshes.size() == 0)
                del.push_back(model);
        }

        for (auto d : del)
        {
            auto it = std::find(models.begin(), models.end(), d);
            if (it != models.end())
                models.erase(it);

            delete d;
        }

        del.clear();

        Renderer::getSingleton()->frame();
    }

    void CSGGeometry::rebuild(CSGBrush* brush, bool buildUvs)
    {
        CSGModel* model = CSGGeometry::getSingleton()->getModel(brush);
        if (model != nullptr)
        {
            rebuild(model, buildUvs);
        }
    }

    void CSGGeometry::rebuild(std::vector<Component*> brushes, bool buildUvs)
    {
        std::vector<CSGModel*> models;
        for (auto& brush : brushes)
        {
            if (brush->getComponentType() != CSGBrush::COMPONENT_TYPE)
            {
                if (brush->getComponentType() == CSGModel::COMPONENT_TYPE)
                    models.push_back((CSGModel*)brush);

                continue;
            }

            CSGModel* model = getModel((CSGBrush*)brush);
            if (model != nullptr)
            {
                auto it = std::find(models.begin(), models.end(), model);
                if (it == models.end())
                    models.push_back(model);
            }
        }

        for (auto model : models)
            rebuild(model, buildUvs);
    }

    void CSGGeometry::clear()
    {
        for (auto* model : models)
            delete model;

        models.clear();
    }
    
    void CSGGeometry::reloadLightmaps()
    {
        for (auto* model : models)
            model->reloadLightmaps();
    }

    bool CSGGeometry::loadFromFile(std::string location, std::string name, CSGModel* comp)
    {
        loadMaterial();

        auto it = std::find_if(models.begin(), models.end(), [=](Model* m) -> bool { return m->component == comp; });
        if (it != models.end())
            (*it)->clear();

        SCSGGeometry data;
        bool loaded = data.load(location, name);

        if (!loaded)
            return false;

        Model* model = new CSGGeometry::Model();
        model->component = comp;

        for (auto& _subMesh : data.subMeshes)
        {
            Material* mat = Material::load(Engine::getSingleton()->getAssetsPath(), _subMesh.material);

            if (mat == nullptr || !mat->isLoaded())
                mat = defaultMaterial;

            CSGGeometry::SubMesh* subMesh = new CSGGeometry::SubMesh();

            subMesh->parent = model;

            subMesh->material = mat;
            subMesh->guid = _subMesh.guid;
            subMesh->setCastShadows(_subMesh.castShadows);
            subMesh->layer = _subMesh.layer;

            subMesh->vertexBuffer.resize(_subMesh.vertexBuffer.size());

            for (int i = 0; i < _subMesh.vertexBuffer.size(); ++i)
            {
                SCSGSubMeshVertexBuffer& v = _subMesh.vertexBuffer[i];

                VertexBuffer vb;
                vb.bitangent = v.bitangent.getValue();
                vb.color = Color::packABGR(v.color.getValue());
                vb.normal = v.normal.getValue();
                vb.position = v.position.getValue();
                vb.tangent = v.tangent.getValue();
                vb.texcoord0 = v.texcoord0.getValue();
                vb.texcoord1 = v.texcoord1.getValue();

                subMesh->vertexBuffer[i] = vb;

                subMesh->cachedAAB.merge(vb.position);
            }

            subMesh->indexBuffer = _subMesh.indexBuffer;
            subMesh->idBuffer = _subMesh.idBuffer;

            model->subMeshes.push_back(subMesh);
        }

        models.push_back(model);

        for (auto* subMesh : model->subMeshes)
        {
            subMesh->load();
            subMesh->reloadLightmap();
        }

        model->_needRebuild = false;

        data.clear();

        model->updateColliders();

        return true;
    }

    void CSGGeometry::saveToFile(std::string location, std::string name, CSGModel* comp)
    {
        if (models.size() == 0)
            return;

        Model* modelData = nullptr;
        auto it = std::find_if(models.begin(), models.end(), [=](Model* m) -> bool { return m->component == comp; });
        if (it != models.end())
            modelData = *it;

        if (modelData == nullptr)
            return;

        if (modelData->subMeshes.size() == 0)
            return;

        SCSGGeometry data;

        for (auto* subMesh : modelData->subMeshes)
        {
            std::string mat = "";
            if (subMesh->material != nullptr &&
                subMesh->material != defaultMaterial)
            {
                mat = subMesh->material->getName();
            }

            SCSGSubMesh _subMesh;

            _subMesh.material = mat;
            _subMesh.guid = subMesh->guid;
            _subMesh.castShadows = subMesh->getCastShadows();
            _subMesh.layer = subMesh->layer;

            _subMesh.vertexBuffer.resize(subMesh->vertexBuffer.size());

            for (int i = 0; i < subMesh->vertexBuffer.size(); ++i)
            {
                VertexBuffer& vb = subMesh->vertexBuffer[i];
                SCSGSubMeshVertexBuffer v;

                v.bitangent = vb.bitangent;
                v.color = Color::unpackABGR(vb.color);
                v.normal = vb.normal;
                v.position = vb.position;
                v.tangent = vb.tangent;
                v.texcoord0 = vb.texcoord0;
                v.texcoord1 = vb.texcoord1;

                _subMesh.vertexBuffer[i] = v;
            }

            _subMesh.indexBuffer = subMesh->indexBuffer;
            _subMesh.idBuffer = subMesh->idBuffer;

            data.subMeshes.push_back(_subMesh);
        }

        data.save(location, name);

        data.clear();
    }

    CSGModel* CSGGeometry::getModel(Transform* t)
    {
        CSGModel* model = nullptr;

        Transform* p = t->getParent();
        while (p != nullptr)
        {
            GameObject* obj = p->getGameObject();
            CSGModel* comp = (CSGModel*)obj->getComponent(CSGModel::COMPONENT_TYPE);
            if (comp != nullptr)
            {
                model = comp;
                break;
            }

            p = p->getParent();
        }

        return model;
    }

    CSGModel* CSGGeometry::getModel(GameObject* obj)
    {
        return getModel(obj->getTransform());
    }

    CSGModel* CSGGeometry::getModel(CSGBrush* brush)
    {
        GameObject* obj = brush->getGameObject();

        return getModel(obj->getTransform());
    }
}