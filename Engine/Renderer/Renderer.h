#pragma once

#include <string>
#include <vector>
#include <functional>
#include <bx/thread.h>
#include <bgfx/bgfx.h>
#include "../glm/glm.hpp"

#include "Color.h"

class MaskedOcclusionCulling;
class CullingThreadpool;

namespace GX
{
	class Camera;
	class Light;
	class Shader;
	class ProjectSettings;
	class Transform;
	class Renderable;
	class Canvas;
	class UIElement;
	class Material;
	class RenderTexture;
	class GameObject;
	class Frustum;
	class Cubemap;

	#define MAX_LIGHTS 8

	#define RENDER_UI_PASS_ID			1023
	#define RENDER_SHADOW_1_PASS_ID		0
	#define RENDER_SHADOW_2_PASS_ID		1
	#define RENDER_SHADOW_3_PASS_ID		2
	#define RENDER_SHADOW_4_PASS_ID		3

	#define NUM_LIGHT_VIEWS				4

	#define RENDER_SKYBOX_PASS_ID		0
	#define RENDER_GEOMETRY_PASS_ID		1
	#define RENDER_DECAL_PASS_ID		2
	#define RENDER_LIGHT_PASS_ID		3
	#define RENDER_SCENE_PASS_ID		4
	#define RENDER_FORWARD_PASS_ID		5
	#define RENDER_BACKBUFFER_PASS_ID	6
	#define RENDER_OVERLAY_PASS_ID		7
	#define RENDER_OUTLINE_PASS_ID		8
	#define RENDER_FINAL_PASS_ID		9

	#define NUM_VIEWS 10

	struct TransientRenderable
	{
	public:
		TransientRenderable() = default;
		~TransientRenderable() { triangles.clear(); }

		Transform* attachedTransform = nullptr;
		glm::mat4x4 transform = glm::identity<glm::mat4x4>();
		std::vector<glm::vec3> triangles;
	};

	enum class SkyModel
	{
		Box,
		Sphere
	};

	class Renderer
	{
		friend class Camera;
		friend class Light;
		friend class Renderable;
		friend class MeshRenderer;
		friend class Terrain;
		friend class Primitives;
		friend class Canvas;
		friend class UIElement;
		friend class Water;

	private:
		static Renderer singleton;

		void* sdlWindow = nullptr;
		void* context = nullptr;

		Color uiClearColor = Color::White;

		struct RenderCallbackInfo
		{
			std::function<void(int viewId, int viewLayer, Camera* camera)> cb = nullptr;
			int queue = 0;
			std::string id = "";
		};

		struct PostRenderCallbackInfo
		{
			std::function<void()> cb = nullptr;
			std::string id = "";
		};

		std::vector<bx::Thread*> renderThreads;
		std::vector<RenderCallbackInfo> renderCallbacks;
		std::function<void()> uiCallback = nullptr;
		std::vector<PostRenderCallbackInfo> postRenderCallbacks;
		static std::vector<std::function<void()>> onEndUpdateCallbacks;

		int width = 0;
		int height = 0;

		bool isRunning = false;

		std::vector<Camera*> cameras;
		std::vector<Light*> lights;
		std::vector<Renderable*> renderables;
		std::vector<Canvas*> canvases;
		std::vector<UIElement*> uiElements;
		std::vector<TransientRenderable> transientRenderables;

		float cpuTime = 0;
		float gpuTime = 0;
		float frameMs = 0;
		uint32_t numDrawCalls = 0;
		uint32_t numTriangles = 0;
		int64_t gpuMemUsed = 0;

		float lastCpuTime = 0;
		float lastGpuTime = 0;
		float lastFrameMs = 0;
		uint32_t lastNumDrawCalls = 0;
		uint32_t lastNumTriangles = 0;

		ProjectSettings* projectSettings = nullptr;
		
		bool shadowSettingsChanged = false;
		bool shadowsEnabled = true;

		Cubemap* environmentMap = nullptr;

		//System uniforms
		static bgfx::UniformHandle uLightPosition;
		static bgfx::UniformHandle uLightDirection;
		static bgfx::UniformHandle uLightColor;
		static bgfx::UniformHandle uLightRadius;
		static bgfx::UniformHandle uLightIntensity;
		static bgfx::UniformHandle uLightType;
		static bgfx::UniformHandle uLightRenderMode;
		static bgfx::UniformHandle uLightShadowBias;
		static bgfx::UniformHandle uLightCastShadows;
		static bgfx::UniformHandle uCamPos;
		static bgfx::UniformHandle uCamDir;
		static bgfx::UniformHandle uCamFov;
		static bgfx::UniformHandle uClipPlane;
		static bgfx::UniformHandle uCamClearColor;
		static bgfx::UniformHandle uCamView;
		static bgfx::UniformHandle uCamInvView;
		static bgfx::UniformHandle uCamProj;
		static bgfx::UniformHandle uCamInvProj;
		static bgfx::UniformHandle uNormalMatrix;
		static bgfx::UniformHandle uShadowMapTexelSize;
		static bgfx::UniformHandle uShadowSamplingParams;
		static bgfx::UniformHandle uAmbientColor;
		static bgfx::UniformHandle uShadowMaps[];
		static bgfx::UniformHandle uShadowMtx;
		static bgfx::UniformHandle uBoneMtx;
		static bgfx::UniformHandle uGpuSkinning;
		static bgfx::UniformHandle uTime;
		static bgfx::UniformHandle uScaledTime;
		static bgfx::UniformHandle uInvModel;
		static bgfx::UniformHandle uScreenParams;
		static bgfx::UniformHandle uUseFXAA;
		static bgfx::UniformHandle uFogEnabled;
		static bgfx::UniformHandle uFogParams;
		static bgfx::UniformHandle uFogColor;
		static bgfx::UniformHandle uLightmap;
		static bgfx::UniformHandle uHasLightmap;
		static bgfx::UniformHandle uEnvMap;
		static bgfx::UniformHandle uNormal;
		static bgfx::UniformHandle uGIParams;

		//Deferred
		bgfx::UniformHandle u_albedoMap;
		bgfx::UniformHandle u_normalMap;
		bgfx::UniformHandle u_mraMap;
		bgfx::UniformHandle u_depthMap;
		bgfx::UniformHandle u_lightColor;
		bgfx::UniformHandle u_VP;
		bgfx::UniformHandle u_invVP;
		bgfx::UniformHandle u_aspect;
		bgfx::UniformHandle u_forwardColor;
		bgfx::UniformHandle u_forwardDepth;
		bgfx::UniformHandle u_color;
		bgfx::UniformHandle u_scale;
		bgfx::UniformHandle u_texture;

		bgfx::ProgramHandle lightPH = { bgfx::kInvalidHandle };
		bgfx::ProgramHandle combinePH = { bgfx::kInvalidHandle };
		bgfx::ProgramHandle shadowCasterPH = { bgfx::kInvalidHandle };
		bgfx::ProgramHandle simplePH = { bgfx::kInvalidHandle };
		bgfx::ProgramHandle simpleTexturePH = { bgfx::kInvalidHandle };
		bgfx::ProgramHandle terrainTreeBillboardPH = { bgfx::kInvalidHandle };
		bgfx::ProgramHandle terrainTreeBillboardLightPH = { bgfx::kInvalidHandle };
		bgfx::ProgramHandle transparentPH = { bgfx::kInvalidHandle };
		bgfx::ProgramHandle outlinePH = { bgfx::kInvalidHandle };
		bgfx::ProgramHandle outlineFinalPH = { bgfx::kInvalidHandle };
		bgfx::ProgramHandle cameraBackBufferPH = { bgfx::kInvalidHandle };
		bgfx::ProgramHandle fxaaPH = { bgfx::kInvalidHandle };
		bgfx::ProgramHandle skyboxPH = { bgfx::kInvalidHandle };

		RenderTexture* backBuffer = nullptr;

		uint64_t defaultRenderState = 0
			| BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_CULL_CW;

		uint64_t shadowRenderState = 0
			//| BGFX_STATE_WRITE_RGB
			//| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_CULL_CW;

		uint64_t lightRenderState = 0
			| BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_BLEND_ADD;

		uint64_t combineRenderState = 0
			| BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z;

		SkyModel skyModel = SkyModel::Box;
		Color ambientColor = Color(0.7f, 0.7f, 0.7f, 1.0f);
		bool giEnabled = true;
		float giIntensity = 1.0f;
		Material* skyMaterial = nullptr;
		Shader* lightShader = nullptr;

		bool fogEnabled = false;
		bool fogIncludeSkybox = false;
		float fogStartDistance = 10.0f;
		float fogEndDistance = 100.0f;
		float fogDensity = 0.25f;
		Color fogColor = Color::White;
		int fogType = 0; // Linear, Exp, Exp2

		static Material* defaultMaterial;
		static Material* defaultStandardMaterial;
		static Shader* defaultShader;

		void createResources();
		void destroyResources();

		void collectStats();

		uint64_t getRenderState(Camera* camera, uint64_t defaultState);

		void renderPointAndSpotLightShadows();
		void renderDirectionalLightShadows(Camera* camera, const glm::mat4x4& invView);
		void renderSkybox(int skyView, Camera* camera, const glm::mat4x4& skyMtx);
		void renderObjects(Camera* camera, int viewLayer, int renderQueue, const glm::mat4x4& view, const glm::mat4x4& proj, const glm::mat4x4& skyMtx);
		void renderRenderCallbacks(Camera* camera, int viewLayer);

		//Occlusion culling
		MaskedOcclusionCulling* moc = nullptr;
		CullingThreadpool* cmoc = nullptr;

		void calculateVisibility(const glm::mat4x4& view, const glm::mat4x4& proj, Frustum* frustum = nullptr, float maxDistance = FLT_MAX);

	public:
		static Renderer* getSingleton() { return &singleton; }

		Renderer();
		~Renderer();

		void init(void* window, void* ctx);
		void shutdown();
		
		void setSize(int w, int h);

		int getWidth();
		int getHeight();

		void* getSdlWindow() { return sdlWindow; }
		void* getContext() { return context; }

		Color getUIClearColor() { return uiClearColor; }
		void setUIClearColor(Color value) { uiClearColor = value; }

		std::string addRenderCallback(std::function<void(int viewId, int viewLayer, Camera* camera)> callback, int queue = 0);
		void removeRenderCallback(std::string id);
		void setUICallback(std::function<void()> callback) { uiCallback = callback; }
		std::string addPostRenderCallback(std::function<void()> callback);
		void removePostRenderCallback(std::string id);
		std::function<void()> getUICallback() { return uiCallback; }
		void uiProcessEvent(const void* event);
		bool getIsRunning() { return isRunning; }
		std::vector<Camera*>& getCameras() { return cameras; }
		std::vector<Light*>& getLights() { return lights; }
		std::vector<Canvas*>& getCanvases() { return canvases; }
		std::vector<UIElement*>& getUIElement() { return uiElements; }

		void setSystemUniforms(Camera* camera);
		void renderUI(std::vector<GameObject*>& gameObjects, glm::vec2 sizeOffset = glm::vec2(0.0f), bool updateMouse = true);
		void renderFrame();
		void frame();

		float getCpuTime() { return lastCpuTime; }
		float getGpuTime() { return lastGpuTime; }
		float getFrameTime() { return lastFrameMs; }
		uint32_t getNumDrawCalls() { return lastNumDrawCalls; }
		uint32_t getNumTriangles() { return lastNumTriangles; }
		int64_t getGpuMemoryUsed() { return gpuMemUsed; }

		int getNumActiveLightsWithShadows();
		
		SkyModel getSkyModel() { return skyModel; }
		void setSkyModel(SkyModel value);

		Color& getAmbientColor() { return ambientColor; }
		void setAmbientColor(Color value) { ambientColor = value; }

		Material* getSkyMaterial() { return skyMaterial; }
		void setSkyMaterial(Material* value);

		bgfx::ProgramHandle getTransparentProgram() { return transparentPH; }
		bgfx::ProgramHandle getOutlineProgram() { return outlinePH; }
		bgfx::ProgramHandle getSimpleProgram() { return simplePH; }
		bgfx::ProgramHandle getSimpleTextureProgram() { return simpleTexturePH; }
		bgfx::ProgramHandle getTerrainTreeBillboardProgram() { return terrainTreeBillboardPH; }
		bgfx::ProgramHandle getTerrainTreeBillboardLightProgram() { return terrainTreeBillboardLightPH; }
		bgfx::ProgramHandle getShadowCasterProgram() { return shadowCasterPH; }
		bgfx::ProgramHandle getLightProgram() { return lightPH; }
		bgfx::ProgramHandle getCombineProgram() { return combinePH; }
		bgfx::ProgramHandle getSkyboxProgram() { return skyboxPH; }

		static bgfx::UniformHandle getAlbedoMapUniform() { return singleton.u_albedoMap; }
		static bgfx::UniformHandle getNormalMapUniform() { return singleton.u_normalMap; }
		static bgfx::UniformHandle getMRAMapUniform() { return singleton.u_mraMap; }
		static bgfx::UniformHandle getDepthMapUniform() { return singleton.u_depthMap; }
		static bgfx::UniformHandle getLightColorMapUniform() { return singleton.u_lightColor; }
		static bgfx::UniformHandle getViewProjUniform() { return singleton.u_VP; }
		static bgfx::UniformHandle getInvViewProjUniform() { return singleton.u_invVP; }
		static bgfx::UniformHandle getAspectUniform() { return singleton.u_aspect; }

		static bgfx::UniformHandle getNormalMatrixUniform() { return uNormalMatrix; }
		static bgfx::UniformHandle getGpuSkinningUniform() { return uGpuSkinning; }
		static bgfx::UniformHandle getAmbientColorUniform() { return uAmbientColor; }
		static bgfx::UniformHandle getColorUniform() { return singleton.u_color; }
		static bgfx::UniformHandle getInvModelUniform() { return uInvModel; }

		static bgfx::UniformHandle getLightPositionUniform() { return uLightPosition; }
		static bgfx::UniformHandle getLightDirectionUniform() { return uLightDirection; }
		static bgfx::UniformHandle getLightColorUniform() { return uLightColor; }
		static bgfx::UniformHandle getLightRadiusUniform() { return uLightRadius; }
		static bgfx::UniformHandle getLightIntensityUniform() { return uLightIntensity; }
		static bgfx::UniformHandle getLightTypeUniform() { return uLightType; }
		static bgfx::UniformHandle getLightRenderModeUniform() { return uLightRenderMode; }
		static bgfx::UniformHandle getLightShadowBiasUniform() { return uLightShadowBias; }
		static bgfx::UniformHandle getLightCastShadowsUniform() { return uLightCastShadows; }

		static bgfx::UniformHandle getCameraPositionUniform() { return uCamPos; }
		static bgfx::UniformHandle getCameraClearColorUniform() { return uCamClearColor; }
		static bgfx::UniformHandle getCameraClippingPlanesUniform() { return uClipPlane; }
		static bgfx::UniformHandle getScreenParamsUniform() { return uScreenParams; }

		static bgfx::UniformHandle getTextureUniform() { return singleton.u_texture; }
		static bgfx::UniformHandle getLightmapUniform() { return uLightmap; }
		static bgfx::UniformHandle getHasLightmapUniform() { return uHasLightmap; }
		
		static bgfx::UniformHandle getEnvMapUniform() { return uEnvMap; }
		static bgfx::UniformHandle getNormalUniform() { return uNormal; }

		void addTransientRenderable(TransientRenderable renderable) { transientRenderables.push_back(renderable); }
		void clearTransientRenderables() { transientRenderables.clear(); }
		std::vector<TransientRenderable>& getTransientRenderables() { return transientRenderables; }

		void resetFrameBuffers();

		void setShadowsEnabled(bool value) { shadowsEnabled = value; }
		bool getShadowsEnabled() { return shadowsEnabled; }

		int getOutlineViewId();
		int getFinalViewId();
		int getSceneViewId();
		int getOverlayViewId();

		int getNumViewsUsed();
		
		std::vector<Renderable*>& getRenderables() { return renderables; }
		RenderTexture* getBackBuffer() { return backBuffer; }

		static Material* getDefaultMaterial() { return defaultMaterial; }
		static Material* getDefaultStandardMaterial() { return defaultStandardMaterial; }
		static Shader* getDefaultShader() { return defaultShader; }

		Light* getFirstLight();

		MaskedOcclusionCulling* getOcclusionCullingProcessor() { return moc; }
		CullingThreadpool* getOcclusionCullingThreadpool() { return cmoc; }

		void updateEnvironmentMap();
		void deleteEnvironmentMap();

		bool getGIEnabled() { return giEnabled; }
		void setGIEnabled(bool value);

		float getGIIntensity() { return giIntensity; }
		void setGIIntensity(float value) { giIntensity = value; }

		bool getFogEnabled() { return fogEnabled; }
		void setFogEnabled(bool value) { fogEnabled = value; }
		
		bool getFogIncludeSkybox() { return fogIncludeSkybox; }
		void setFogIncludeSkybox(bool value) { fogIncludeSkybox = value; }

		float getFogStartDistance() { return fogStartDistance; }
		void setFogStartDistance(float value) { fogStartDistance = value; }

		float getFogEndDistance() { return fogEndDistance; }
		void setFogEndDistance(float value) { fogEndDistance = value; }

		float getFogDensity() { return fogDensity; }
		void setFogDensity(float value) { fogDensity = value; }

		Color getFogColor() { return fogColor; }
		void setFogColor(Color value) { fogColor = value; }

		int getFogType() { return fogType; }
		void setFogType(int value) { fogType = value; }

		Cubemap* getEnvironmentMap() { return environmentMap; }
	};
}