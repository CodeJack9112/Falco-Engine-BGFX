#pragma once

#include "Component.h"
#include "Renderable.h"

#include <map>

#include <bgfx/bgfx.h>

#include "../Math/AxisAlignedBox.h"

#include "../glm/glm.hpp"

namespace GX
{
	class Camera;
	class Transform;
	class Material;
	class Mesh;
	class SubMesh;
	class Texture;

	struct BoneLink
	{
	public:
		int boneIndex = 0;
		std::string boneName = "";
		size_t boneNameHash = 0;
		GameObject * target = nullptr;
	};

	class MeshRenderer : public Component, public Renderable
	{
	private:
		Mesh* mesh = nullptr;

		std::vector<Material*> sharedMaterials;
		std::vector<Material*> materials;

		std::vector<std::vector<BoneLink>> boneLinks;
		
		AxisAlignedBox cachedAAB = AxisAlignedBox::BOX_NULL;
		glm::mat4x4 prevTransform = glm::mat4x4(FLT_MAX);

		std::string rootObjectGuid = "";
		GameObject * rootObject = nullptr;
		Transform* transform = nullptr;

		bool is_skinned = false;

		std::vector<Texture*> lightmaps;
		uint8_t lightmapSize = 0; //0 - default

		glm::mat4x4 boneMatrices[128];
		void calcBoneData(int subMeshIndex, SubMesh * subMesh);

		void applyMaterials();

		std::map<Camera*, std::vector<bool>> occluded;

		std::vector<std::vector<glm::vec4>> lodVertexBuffer;
		std::vector<std::vector<bool>> lodVertexBufferMtxCache;

		float lodMaxDistance = 50.0f;
		bool cullOverMaxDistance = false;

	public:
		MeshRenderer();
		virtual ~MeshRenderer();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();

		void setMesh(Mesh * meshPtr, bool setMaterials = true);
		Mesh* getMesh() { return mesh; }

		std::vector<std::vector<glm::vec4>>& getLodVertexBuffer() { return lodVertexBuffer; }
		std::vector<std::vector<bool>>& getLodVertexBufferMtxCache() { return lodVertexBufferMtxCache; }

		virtual void onRender(Camera * camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback); // Rendering function
		virtual void onSceneLoaded();
		virtual Component* onClone();
		virtual void onRebindObject(std::string oldObj, std::string newObj);
		virtual void onRefresh();
		virtual void onAttach();
		virtual void onDetach();
		virtual bool isAlwaysVisible() { return isSkinned(); }
		virtual bool isTransparent();
		virtual bool isStatic();
		virtual bool getSkipRendering();
		virtual AxisAlignedBox getBounds(bool world = true);
		virtual bool checkCullingMask(LayerMask& mask);

		void addSharedMaterial(Material* mat);
		void removeSharedMaterial(Material* mat);
		void removeSharedMaterial(int index);
		int getSharedMaterialsCount() { return (int)sharedMaterials.size(); }
		Material* getSharedMaterial(int index);
		void setSharedMaterial(int index, Material* material);
		Material* getMaterial(int index);

		std::vector<std::vector<BoneLink>>& getBoneLinks() { return boneLinks; }
		void resetBoneLinks();

		GameObject* getRootObject() { return rootObject; }
		void setRootObject(GameObject* value);

		std::string getRootObjectGuid() { return rootObjectGuid; }
		void setRootObjectGuid(std::string value) { rootObjectGuid = value; }

		int getLightmapSize() { return lightmapSize; }
		void setLightmapSize(int value) { lightmapSize = value; }

		float getLodMaxDistance() { return lodMaxDistance; }
		void setLodMaxDistance(float value) { lodMaxDistance = value; }

		bool getCullOverMaxDistance() { return cullOverMaxDistance; }
		void setCullOverMaxDistance(bool value) { cullOverMaxDistance = value; }

		bool isSkinned();

		void reloadLightmaps();
	};
}