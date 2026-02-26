#pragma once

#include "Component.h"

#include <bgfx/bgfx.h>

#include "../glm/glm.hpp"

#include "../Math/Ray.h"
#include "../Math/Mathf.h"
#include "../Math/AxisAlignedBox.h"
#include "../Renderer/Color.h"
#include "../Core/LayerMask.h"

namespace GX
{
	class RenderTexture;
	class Renderer;
	class Frustum;
	class Transform;
	class Texture;

	enum class ProjectionType
	{
		Perspective,
		Orthographic
	};

	class Camera : public Component
	{
		friend class Renderer;
		friend class Primitives;
		friend class API_Graphics;
		friend class Material;

	public:
		enum class ClearFlags
		{
			Skybox,
			SolidColor,
			DepthOnly,
			DontClear
		};

	private:
		float fovy = 75;
		float znear = 0.1f;
		float zfar = 1000.0f;

		float viewport_l = 0.0f;	//0..1
		float viewport_t = 0.0f;	//0..1
		float viewport_w = 1.0f;	//0..1
		float viewport_h = 1.0f;	//0..1
		float screenOffset_left = 0.0f;
		float screenOffset_top = 0.0f;
		float orthographicSize = 5.0f;

		int depth = 0;

		bool editorCamera = false;

		ProjectionType projectionType = ProjectionType::Perspective;
		LayerMask cullingMask;
		ClearFlags clearFlags = ClearFlags::Skybox;

		Color clearColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
		RenderTexture* renderTarget = nullptr;
		Frustum* frustum = nullptr;

		Transform* transform = nullptr;

		glm::mat4x4 prevTransform = glm::identity<glm::mat4x4>();
		
		//Frame buffers
		bgfx::FrameBufferHandle gbuffer = { bgfx::kInvalidHandle };
		bgfx::FrameBufferHandle decalBuffer = { bgfx::kInvalidHandle };
		bgfx::FrameBufferHandle lightBuffer = { bgfx::kInvalidHandle };
		bgfx::FrameBufferHandle outlineBuffer = { bgfx::kInvalidHandle };

		//Diffuse, normals, metallic+roughness+ao+specular, lightmap, depth
		bgfx::TextureHandle gbufferTex[5] = { bgfx::kInvalidHandle, bgfx::kInvalidHandle, bgfx::kInvalidHandle, bgfx::kInvalidHandle, bgfx::kInvalidHandle };
		bgfx::TextureHandle lightBufferTex = { bgfx::kInvalidHandle };

		bgfx::TextureHandle outlineBufferTex = { bgfx::kInvalidHandle };
		bgfx::TextureHandle outlineDepthTex = { bgfx::kInvalidHandle };

		RenderTexture* backBuffer = nullptr;
		RenderTexture* sceneBuffer = nullptr;
		RenderTexture* postProcessRt = nullptr;

		Texture* gbDiffuseTexture = nullptr;
		Texture* gbNormalsTexture = nullptr;
		Texture* gbMRASTexture = nullptr;
		Texture* gbLightmapTexture = nullptr;
		Texture* gbDepthTexture = nullptr;

		bool flipH = false;
		bool flipV = false;

		void destroyFrameBuffers();
		void resetFrameBuffers();

		glm::mat4x4 injectedViewMatrix = glm::identity<glm::mat4x4>();
		glm::mat4x4 injectedProjectionMatrix = glm::identity<glm::mat4x4>();
		bool injectView = false;
		bool injectProj = false;

		bool invertCulling = false;
		bool occlusionCulling = false;

		static float dynAutoRes;
		static int dynAutoResFrames;
		static int dynAutoResFramesToUpdate;

		static Camera* apiCamera;
		static int apiViewId;

		static void updateDynamicResolution();

	public:
		Camera();
		virtual ~Camera();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual void onAttach();
		virtual void onDetach();
		virtual Component* onClone();

		void initResources();
		void destroyResources();

		int getDepth() { return depth; }
		void setDepth(int value) { depth = value; }

		float getFOVy() { return fovy; }
		void setFOVy(float value);

		float getNear() { return znear; }
		void setNear(float value);

		float getFar() { return zfar; }
		void setFar(float value);

		static int getApiViewId() { return apiViewId; }
		static Camera* getApiCamera() { return apiCamera; }

		//Normalized value 0..1
		float getViewportWidth() { return viewport_w; }
		//Normalized value 0..1
		void setViewportWidth(float value) { viewport_w = Mathf::clamp(value, 0.001f, 1.0f); }

		//Normalized value 0..1
		float getViewportHeight() { return viewport_h; }
		//Normalized value 0..1
		void setViewportHeight(float value) { viewport_h = Mathf::clamp(value, 0.001f, 1.0f); }

		//Normalized value 0..1
		float getViewportLeft() { return viewport_l; }
		//Normalized value 0..1
		void setViewportLeft(float value) { viewport_l = Mathf::clamp(value, 0.0f, 1.0f); }

		//Normalized value 0..1
		float getViewportTop() { return viewport_t; }
		//Normalized value 0..1
		void setViewportTop(float value) { viewport_t = Mathf::clamp(value, 0.0f, 1.0f); }

		ProjectionType getProjectionType() { return projectionType; }
		void setProjectionType(ProjectionType value);

		float getOrthographicSize() { return orthographicSize; }
		void setOrthographicSize(float value);

		float getScreenOffsetLeft() { return screenOffset_left; }
		void setScreenOffsetLeft(float value) { screenOffset_left = value; }

		float getScreenOffsetTop() { return screenOffset_top; }
		void setScreenOffsetTop(float value) { screenOffset_top = value; }
		
		bool getOcclusionCulling() { return occlusionCulling; }
		void setOcclusionCulling(bool value) { occlusionCulling = value; }

		static float getDynamicResolution();

		Transform* getTransform();
		void setTransform(Transform* value) { transform = value; }

		glm::mat4x4 getViewMatrix();
		glm::mat4x4 getProjectionMatrix(ProjectionType type);
		glm::mat4x4 getProjectionMatrix();

		static glm::mat4x4 makeViewMatrix(glm::vec3 position, glm::vec3 lookAt, glm::vec3 up);
		static glm::mat4x4 makeProjectionMatrix(float fovy, float aspect, float znear, float zfar);

		void injectViewMatrix(glm::mat4x4 mtx);
		void injectProjectionMatrix(glm::mat4x4 mtx);
		void injectViewMatrix(bool inject) { injectView = inject; }
		void injectProjectionMatrix(bool inject) { injectProj = inject; }

		bool getFlipHorizontal() { return flipH; }
		void setFlipHorizontal(bool value) { flipH = value; }

		bool getFlipVertical() { return flipV; }
		void setFlipVertical(bool value) { flipV = value; }

		bool getInvertCulling() { return invertCulling; }
		void setInvertCulling(bool value) { invertCulling = value; }

		float getAspectRatio();
		LayerMask& getCullingMask() { return cullingMask; }
		ClearFlags getClearFlags() { return clearFlags; }
		void setClearFlags(ClearFlags value) { clearFlags = value; }

		void setClearColor(Color color) { clearColor = color; }
		Color getClearColor() { return clearColor; }

		RenderTexture* getRenderTarget() { return renderTarget; }
		void setRenderTarget(RenderTexture * rt, int layer = 0);

		RenderTexture* getBackBuffer() { return backBuffer; }
		RenderTexture* getSceneBuffer() { return sceneBuffer; }

		Ray getCameraToViewportRay(float screenX, float screenY);
		glm::vec3 worldToScreenPoint(glm::vec3 point);
		glm::vec3 screenToWorldPoint(glm::vec3 point);

		Frustum* getFrustum();
		bool isVisible(AxisAlignedBox aab, float bias = 1.5f);

		Texture* getGBufferDiffuseTexture() { return gbDiffuseTexture; }
		Texture* getGBufferNormalTexture() { return gbNormalsTexture; }
		Texture* getGBufferMRASTexture() { return gbMRASTexture; }
		Texture* getGBufferLightmapTexture() { return gbLightmapTexture; }
		Texture* getGBufferDepthTexture() { return gbDepthTexture; }

		bool getIsEditorCamera() { return editorCamera; }
		void setIsEditorCamera(bool value) { editorCamera = value; }

		void renderFrame();
	};
}