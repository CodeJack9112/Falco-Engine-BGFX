#include "Renderer.h"

#ifndef _WIN32
#include <string.h>
#endif

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <SDL2/SDL_syswm.h>

#define FREEIMAGE_LIB
#include "../FreeImage/include/FreeImage.h"
#undef FREEIMAGE_LIB

#include <GL/gl.h>

#undef None

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>
#include <bx/timer.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <bx/math.h>
#include <bx/string.h>
#include <bgfx/bgfx.h>

#include "Primitives.h"
#include "RenderTexture.h"
#include "Frustum.h"
#include "VertexLayouts.h"
#include "../Core/Engine.h"
#include "../Core/Debug.h"
#include "../Core/APIManager.h"
#include "../UI/imgui_impl_sdl.h"
#include "../UI/imgui_impl_bgfx.h"
#include "../Classes/IO.h"
#include "../Classes/Helpers.h"
#include "../Classes/StringConverter.h"
#include "../Classes/GUIDGenerator.h"
#include "../Core/GameObject.h"
#include "../Components/Component.h"
#include "../Components/Transform.h"
#include "../Components/Camera.h"
#include "../Components/Renderable.h"
#include "../Components/Light.h"
#include "../Components/Canvas.h"
#include "../Components/UIElement.h"
#include "../Components/Mask.h"
#include "../Components/MonoScript.h"
#include "../Components/MeshRenderer.h"
#include "../Assets/Texture.h"
#include "../Assets/Cubemap.h"
#include "../Assets/Shader.h"
#include "../Assets/Material.h"
#include "../Assets/Mesh.h"
#include "../Serialization/Settings/ProjectSettings.h"
#include "../Math/Mathf.h"
#include "../Math/Raycast.h"
#include "../Core/Time.h"

#include "../Classes/brtshaderc.h"

#include "SystemShaders/ShadowCaster.h"
#include "SystemShaders/DefaultShader.h"
#include "SystemShaders/DeferredCombine.h"
#include "SystemShaders/SimpleShader.h"
#include "SystemShaders/SimpleTextureShader.h"
#include "SystemShaders/TerrainTreeBillboardShader.h"
#include "SystemShaders/TerrainTreeBillboardLightShader.h"
#include "SystemShaders/TransparentShader.h"
#include "SystemShaders/OutlineShader.h"
#include "SystemShaders/OutlineFinalShader.h"
#include "SystemShaders/CameraBackBuffer.h"
#include "SystemShaders/FXAA.h"
#include "SystemShaders/Skybox.h"

#include "../OcclusionCulling/CullingThreadpool.h"

constexpr auto SHADOW_AAB_VAL = 9000;

namespace GX
{
	Renderer Renderer::singleton;

	//System uniforms
	bgfx::UniformHandle Renderer::uLightPosition = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uLightDirection = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uLightColor = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uLightRadius = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uLightType = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uLightRenderMode = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uLightIntensity = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uLightCastShadows = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uCamPos = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uCamDir = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uCamFov = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uCamClearColor = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uCamView = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uCamInvView = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uCamProj = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uCamInvProj = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uClipPlane = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uNormalMatrix = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uShadowMaps[] = { { bgfx::kInvalidHandle }, { bgfx::kInvalidHandle }, { bgfx::kInvalidHandle }, { bgfx::kInvalidHandle } };
	bgfx::UniformHandle Renderer::uShadowMtx = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uShadowMapTexelSize = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uShadowSamplingParams = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uAmbientColor = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uLightShadowBias = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uBoneMtx = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uGpuSkinning = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uTime = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uScaledTime = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uInvModel = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uScreenParams = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uUseFXAA = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uFogEnabled = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uFogParams = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uFogColor = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uLightmap = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uHasLightmap = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uEnvMap = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uGIParams = { bgfx::kInvalidHandle };
	bgfx::UniformHandle Renderer::uNormal = { bgfx::kInvalidHandle };

	Material* Renderer::defaultMaterial = nullptr;
	Material* Renderer::defaultStandardMaterial = nullptr;
	Shader* Renderer::defaultShader = nullptr;

	Renderer::Renderer()
	{
		
	}

	Renderer::~Renderer()
	{
		
	}

	uint64_t Renderer::getRenderState(Camera* camera, uint64_t defaultState)
	{
		uint64_t state = defaultState;

		if (camera->getInvertCulling())
		{
			state &= ~BGFX_STATE_CULL_CW;
			state |= BGFX_STATE_CULL_CCW;
		}

		return state;
	}

	void Renderer::init(void* window, void* ctx)
	{
		sdlWindow = window;
		context = ctx;
		
		std::string version = Helper::getVersion();

		bgfx::PlatformData pd;
		pd.ndt = NULL;

		SDL_SysWMinfo wmi;
		SDL_VERSION(&wmi.version);
		if (!SDL_GetWindowWMInfo((SDL_Window*)window, &wmi))
		{
			std::cout << "SDL init error!" << std::endl;
		}

	#ifdef _WIN32
		pd.nwh = wmi.info.win.window;
	#else
		pd.ndt = wmi.info.x11.display;
		pd.nwh = (void*)(uintptr_t)wmi.info.x11.window;
	#endif
		pd.context = ctx;

		bgfx::setPlatformData(pd);

		bgfx::renderFrame();

		int w = 0;
		int h = 0;

		SDL_GetWindowSize((SDL_Window*)window, &w, &h);

		bgfx::Init init;
		init.type = bgfx::RendererType::OpenGL;
		init.resolution.width = w;
		init.resolution.height = h;

		bgfx::init(init);

		width = w;
		height = h;

		//ImGUI
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		//if (!Engine::getSingleton()->getAssetsPath().empty())
		//	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui_Implbgfx_Init(RENDER_UI_PASS_ID);
		ImGui_ImplSDL2_InitForOpenGL((SDL_Window*)window, nullptr);

		Texture::createNullTexture();
		Cubemap::createNullCubemap();

		if (!Engine::getSingleton()->getAssetsPath().empty())
		{
			///--------------------------------------------------------///
			///-------------------------LOG----------------------------///

			const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
			const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model

			GLint nTotalMemoryInKB = 0;
			glGetIntegerv(0x9048, &nTotalMemoryInKB);
			std::string gpuMemTotal = std::to_string(nTotalMemoryInKB / 1000);

			std::string vendorName = vendor != nullptr ? (const char*)vendor : "Unknown";
			std::string gpuName = renderer != nullptr ? (const char*)renderer : "Unknown";

			Debug::log("Renderer initialized", Debug::DbgColorGreen);
			Debug::log("--Version " + version);
			Debug::log("--GPU Vendor: " + vendorName);
			Debug::log("--GPU Name: " + gpuName);
			Debug::log("--GPU Memory total: " + gpuMemTotal + "MB");
			Debug::log("\n");

			///--------------------------------------------------------///
			///--------------------------------------------------------///

			projectSettings = Engine::getSingleton()->getSettings();
			
			createResources();

			/*SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			int numCPU = sysinfo.dwNumberOfProcessors;
			numCPU = numCPU >= 2 ? 2 : 1;*/

			moc = MaskedOcclusionCulling::Create();
			//auto impl = moc->GetImplementation();
			//cmoc = new CullingThreadpool(numCPU, numCPU, numCPU);
			//cmoc->SetBuffer(moc);

			const int _width = 1280, _height = 960;
			moc->SetResolution(_width, _height);
			moc->SetNearClipPlane(0.1f);

			//cmoc->WakeThreads();
		}

		isRunning = true;
	}

	void Renderer::createResources()
	{
		VertexLayouts::init();

		//Compile shadow caster shader
		const bgfx::Memory* memVsh = shaderc::compileShaderFromSources(shaderc::ST_VERTEX, "/", shaders::shadowCasterVertex.c_str(), "", shaders::shadowCasterVarying.c_str());
		const bgfx::Memory* memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, "/", shaders::shadowCasterFragment.c_str(), "", shaders::shadowCasterVarying.c_str());
		bgfx::ShaderHandle vsh = bgfx::createShader(memVsh);
		bgfx::ShaderHandle fsh = bgfx::createShader(memFsh);
		shadowCasterPH = bgfx::createProgram(vsh, fsh, true);

		//Compile renderable shader
		memVsh = shaderc::compileShaderFromSources(shaderc::ST_VERTEX, "/", shaders::outlineVertex.c_str(), "", shaders::outlineVarying.c_str());
		memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, "/", shaders::outlineFragment.c_str(), "", shaders::outlineVarying.c_str());
		vsh = bgfx::createShader(memVsh);
		fsh = bgfx::createShader(memFsh);
		outlinePH = bgfx::createProgram(vsh, fsh, true);

		//Compile simple shader
		memVsh = shaderc::compileShaderFromSources(shaderc::ST_VERTEX, "/", shaders::simpleVertex.c_str(), "", shaders::simpleVarying.c_str());
		memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, "/", shaders::simpleFragment.c_str(), "", shaders::simpleVarying.c_str());
		vsh = bgfx::createShader(memVsh);
		fsh = bgfx::createShader(memFsh);
		simplePH = bgfx::createProgram(vsh, fsh, true);

		//Compile simple texture shader
		memVsh = shaderc::compileShaderFromSources(shaderc::ST_VERTEX, "/", shaders::simpleTextureVertex.c_str(), "", shaders::simpleTextureVarying.c_str());
		memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, "/", shaders::simpleTextureFragment.c_str(), "", shaders::simpleTextureVarying.c_str());
		vsh = bgfx::createShader(memVsh);
		fsh = bgfx::createShader(memFsh);
		simpleTexturePH = bgfx::createProgram(vsh, fsh, true);

		//Compile terrain tree billboard shader
		memVsh = shaderc::compileShaderFromSources(shaderc::ST_VERTEX, "/", shaders::terrainTreeBillboardLightVertex.c_str(), "", shaders::terrainTreeBillboardLightVarying.c_str());
		memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, "/", shaders::terrainTreeBillboardFragment.c_str(), "", shaders::terrainTreeBillboardLightVarying.c_str());
		vsh = bgfx::createShader(memVsh);
		fsh = bgfx::createShader(memFsh);
		terrainTreeBillboardPH = bgfx::createProgram(vsh, fsh, true);

		//Compile terrain tree billboard light shader
		memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, "/", shaders::terrainTreeBillboardLightFragment.c_str(), "", shaders::terrainTreeBillboardLightVarying.c_str());
		fsh = bgfx::createShader(memFsh);
		terrainTreeBillboardLightPH = bgfx::createProgram(vsh, fsh, true);

		//Compile billboard shader
		memVsh = shaderc::compileShaderFromSources(shaderc::ST_VERTEX, "/", shaders::transparentVertex.c_str(), "", shaders::transparentVarying.c_str());
		memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, "/", shaders::transparentFragment.c_str(), "", shaders::transparentVarying.c_str());
		vsh = bgfx::createShader(memVsh);
		fsh = bgfx::createShader(memFsh);
		transparentPH = bgfx::createProgram(vsh, fsh, true);

		//Compile combine shader
		memVsh = shaderc::compileShaderFromSources(shaderc::ST_VERTEX, "/", shaders::deferredCombineVertex.c_str(), "", shaders::deferredCombineVarying.c_str());
		memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, "/", shaders::deferredCombineFragment.c_str(), "", shaders::deferredCombineVarying.c_str());
		vsh = bgfx::createShader(memVsh);
		fsh = bgfx::createShader(memFsh);
		combinePH = bgfx::createProgram(vsh, fsh, true);

		//Compile final shader
		memVsh = shaderc::compileShaderFromSources(shaderc::ST_VERTEX, "/", shaders::outlineFinalVertex.c_str(), "", shaders::outlineFinalVarying.c_str());
		memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, "/", shaders::outlineFinalFragment.c_str(), "", shaders::outlineFinalVarying.c_str());
		vsh = bgfx::createShader(memVsh);
		fsh = bgfx::createShader(memFsh);
		outlineFinalPH = bgfx::createProgram(vsh, fsh, true);

		//Camera back buffer shader
		memVsh = shaderc::compileShaderFromSources(shaderc::ST_VERTEX, "/", shaders::cameraBackBufferVertex.c_str(), "", shaders::cameraBackBufferVarying.c_str());
		memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, "/", shaders::cameraBackBufferFragment.c_str(), "", shaders::cameraBackBufferVarying.c_str());
		vsh = bgfx::createShader(memVsh);
		fsh = bgfx::createShader(memFsh);
		cameraBackBufferPH = bgfx::createProgram(vsh, fsh, true);

		//FXAA shader
		memVsh = shaderc::compileShaderFromSources(shaderc::ST_VERTEX, "/", shaders::fxaaVertex.c_str(), "", shaders::fxaaVarying.c_str());
		memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, "/", shaders::fxaaFragment.c_str(), "", shaders::fxaaVarying.c_str());
		vsh = bgfx::createShader(memVsh);
		fsh = bgfx::createShader(memFsh);
		fxaaPH = bgfx::createProgram(vsh, fsh, true);

		//Skybox shader
		memVsh = shaderc::compileShaderFromSources(shaderc::ST_VERTEX, "/", shaders::skyboxVertex.c_str(), "", shaders::skyboxVarying.c_str());
		memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, "/", shaders::skyboxFragment.c_str(), "", shaders::skyboxVarying.c_str());
		vsh = bgfx::createShader(memVsh);
		fsh = bgfx::createShader(memFsh);
		skyboxPH = bgfx::createProgram(vsh, fsh, true);

		//Light shader
		lightShader = Shader::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Shaders/StandardDeferredLight.shader");
		lightShader->compile("");
		if (lightShader->getPassCount() > 0)
			lightPH = lightShader->getPass(0)->getProgramHandle("");

		uLightPosition = bgfx::createUniform("u_lightPosition", bgfx::UniformType::Vec4, 1);
		uLightDirection = bgfx::createUniform("u_lightDirection", bgfx::UniformType::Vec4, 1);
		uLightColor = bgfx::createUniform("u_lightColor", bgfx::UniformType::Vec4, 1);
		uLightRadius = bgfx::createUniform("u_lightRadius", bgfx::UniformType::Vec4, 1);
		uLightIntensity = bgfx::createUniform("u_lightIntensity", bgfx::UniformType::Vec4, 1);
		uLightType = bgfx::createUniform("u_lightType", bgfx::UniformType::Vec4, 1);
		uLightRenderMode = bgfx::createUniform("u_lightRenderMode", bgfx::UniformType::Vec4, 1);
		uLightShadowBias = bgfx::createUniform("u_lightShadowBias", bgfx::UniformType::Vec4, 1);
		uLightCastShadows = bgfx::createUniform("u_lightCastShadows", bgfx::UniformType::Vec4, 1);

		uCamPos = bgfx::createUniform("u_camPos", bgfx::UniformType::Vec4, 1);
		uCamDir = bgfx::createUniform("u_camDir", bgfx::UniformType::Vec4, 1);
		uCamFov = bgfx::createUniform("u_camFov", bgfx::UniformType::Vec4, 1);
		uCamClearColor = bgfx::createUniform("u_clearColor", bgfx::UniformType::Vec4, 1);
		uCamView = bgfx::createUniform("u_camView", bgfx::UniformType::Mat4, 1);
		uCamProj = bgfx::createUniform("u_camProj", bgfx::UniformType::Mat4, 1);
		uCamInvView = bgfx::createUniform("u_camInvView", bgfx::UniformType::Mat4, 1);
		uCamInvProj = bgfx::createUniform("u_camInvProj", bgfx::UniformType::Mat4, 1);
		uClipPlane = bgfx::createUniform("u_clipPlane", bgfx::UniformType::Vec4, 1);
		uScreenParams = bgfx::createUniform("u_screenParams", bgfx::UniformType::Vec4, 1);
		uUseFXAA = bgfx::createUniform("u_useFXAA", bgfx::UniformType::Vec4, 1);
		uFogEnabled = bgfx::createUniform("u_fogEnabled", bgfx::UniformType::Vec4, 1);
		uFogParams = bgfx::createUniform("u_fogParams", bgfx::UniformType::Vec4, 1);
		uFogColor = bgfx::createUniform("u_fogColor", bgfx::UniformType::Vec4, 1);

		uNormalMatrix = bgfx::createUniform("u_normalMatrix", bgfx::UniformType::Mat3, 1);

		uShadowMaps[0] = bgfx::createUniform("u_shadowMap0", bgfx::UniformType::Sampler, 1);
		uShadowMaps[1] = bgfx::createUniform("u_shadowMap1", bgfx::UniformType::Sampler, 1);
		uShadowMaps[2] = bgfx::createUniform("u_shadowMap2", bgfx::UniformType::Sampler, 1);
		uShadowMaps[3] = bgfx::createUniform("u_shadowMap3", bgfx::UniformType::Sampler, 1);

		uShadowMtx = bgfx::createUniform("u_shadowMatrix", bgfx::UniformType::Mat4, 4);
		uBoneMtx = bgfx::createUniform("u_boneMatrix", bgfx::UniformType::Mat4, 128);
		uGpuSkinning = bgfx::createUniform("u_skinned", bgfx::UniformType::Vec4, 1);
		uTime = bgfx::createUniform("u_time", bgfx::UniformType::Vec4, 1);
		uScaledTime = bgfx::createUniform("u_timeScaled", bgfx::UniformType::Vec4, 1);
		uInvModel = bgfx::createUniform("u_invModel", bgfx::UniformType::Mat4, 1);

		uShadowMapTexelSize = bgfx::createUniform("u_shadowMapTexelSize", bgfx::UniformType::Vec4, 1);
		uShadowSamplingParams = bgfx::createUniform("u_shadowSamplingParams", bgfx::UniformType::Vec4, 1);

		uAmbientColor = bgfx::createUniform("u_ambientColor", bgfx::UniformType::Vec4, 1);
		uEnvMap = bgfx::createUniform("u_envMap", bgfx::UniformType::Sampler, 1);
		uGIParams = bgfx::createUniform("u_giParams", bgfx::UniformType::Vec4, 1);
		//uGIMode = bgfx::createUniform("u_giMode", bgfx::UniformType::Vec4, 1);

		//Deferred
		u_albedoMap = bgfx::createUniform("u_albedoMap", bgfx::UniformType::Sampler, 1);
		u_normalMap = bgfx::createUniform("u_normalMap", bgfx::UniformType::Sampler, 1);
		u_mraMap = bgfx::createUniform("u_mraMap", bgfx::UniformType::Sampler, 1);
		u_depthMap = bgfx::createUniform("u_depthMap", bgfx::UniformType::Sampler, 1);
		u_lightColor = bgfx::createUniform("u_lightColor", bgfx::UniformType::Sampler, 1);
		u_VP = bgfx::createUniform("u_VP", bgfx::UniformType::Mat4, 1);
		u_invVP = bgfx::createUniform("u_invVP", bgfx::UniformType::Mat4, 1);
		u_aspect = bgfx::createUniform("u_aspect", bgfx::UniformType::Vec4, 1);
		u_forwardColor = bgfx::createUniform("u_forwardColor", bgfx::UniformType::Sampler, 1);
		u_forwardDepth = bgfx::createUniform("u_forwardDepth", bgfx::UniformType::Sampler, 1);

		//Other
		u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4, 1);
		u_scale = bgfx::createUniform("u_scale", bgfx::UniformType::Vec4, 1);

		u_texture = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler, 1);
		uLightmap = bgfx::createUniform("u_lightMap", bgfx::UniformType::Sampler, 1);
		uHasLightmap = bgfx::createUniform("u_hasLightMap", bgfx::UniformType::Vec4, 1);

		uNormal = bgfx::createUniform("u_normal", bgfx::UniformType::Vec4, 1);

		if (Engine::getSingleton()->getIsRuntimeMode())
			backBuffer = new RenderTexture(width, height);

		defaultShader = Shader::create("system/shaders/", "DefaultShader");
		defaultShader->loadFromSource(shaders::defaultShader);
		defaultShader->compile("");
		defaultShader->setPersistent(true);

		defaultMaterial = Material::create("system/materials/1/", "DefaultMaterial");
		defaultMaterial->load();
		defaultMaterial->setShader(defaultShader);
		defaultMaterial->setPersistent(true);
		defaultMaterial->compileShader();

		Shader* standardShader = Shader::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Shaders/StandardDeferred.shader");

		defaultStandardMaterial = Material::create("system/materials/2/", "DefaultMaterial");
		defaultStandardMaterial->load();
		defaultStandardMaterial->setShader(standardShader);
		defaultStandardMaterial->setPersistent(true);
	}

	void Renderer::destroyResources()
	{
		if (backBuffer != nullptr)
			delete backBuffer;

		backBuffer = nullptr;

		if (bgfx::isValid(shadowCasterPH))
			bgfx::destroy(shadowCasterPH);

		if (bgfx::isValid(outlinePH))
			bgfx::destroy(outlinePH);
		
		if (bgfx::isValid(simplePH))
			bgfx::destroy(simplePH);
		
		if (bgfx::isValid(simpleTexturePH))
			bgfx::destroy(simpleTexturePH);
		
		if (bgfx::isValid(terrainTreeBillboardPH))
			bgfx::destroy(terrainTreeBillboardPH);
		
		if (bgfx::isValid(terrainTreeBillboardLightPH))
			bgfx::destroy(terrainTreeBillboardLightPH);
		
		if (bgfx::isValid(transparentPH))
			bgfx::destroy(transparentPH);
		
		if (bgfx::isValid(combinePH))
			bgfx::destroy(combinePH);
		
		if (bgfx::isValid(outlineFinalPH))
			bgfx::destroy(outlineFinalPH);
		
		if (bgfx::isValid(cameraBackBufferPH))
			bgfx::destroy(cameraBackBufferPH);
		
		if (bgfx::isValid(fxaaPH))
			bgfx::destroy(fxaaPH);
		
		if (bgfx::isValid(skyboxPH))
			bgfx::destroy(skyboxPH);

		deleteEnvironmentMap();

		shadowCasterPH = { bgfx::kInvalidHandle };
		outlinePH = { bgfx::kInvalidHandle };
		simplePH = { bgfx::kInvalidHandle };
		simpleTexturePH = { bgfx::kInvalidHandle };
		terrainTreeBillboardPH = { bgfx::kInvalidHandle };
		terrainTreeBillboardLightPH = { bgfx::kInvalidHandle };
		transparentPH = { bgfx::kInvalidHandle };
		combinePH = { bgfx::kInvalidHandle };
		outlineFinalPH = { bgfx::kInvalidHandle };
		cameraBackBufferPH = { bgfx::kInvalidHandle };
		fxaaPH = { bgfx::kInvalidHandle };
		skyboxPH = { bgfx::kInvalidHandle };
	}

	void Renderer::deleteEnvironmentMap()
	{
		if (environmentMap != nullptr && environmentMap->isLoaded())
			delete environmentMap;

		environmentMap = nullptr;
	}

	void Renderer::setGIEnabled(bool value)
	{
		giEnabled = value;
		if (!giEnabled)
			deleteEnvironmentMap();
		else
			updateEnvironmentMap();
	}

	void Renderer::shutdown()
	{
		Texture::destroyNullTexture();
		Cubemap::destroyNullCubemap();

		isRunning = false;

		if (sdlWindow != nullptr)
		{
			ImGui_Implbgfx_Shutdown();
			ImGui_ImplSDL2_Shutdown();
		}

		destroyResources();

		bgfx::shutdown();

		while (bgfx::RenderFrame::NoContext != bgfx::renderFrame()) {};

		uiCallback = nullptr;
		renderCallbacks.clear();

		cameras.clear();
	}

	void Renderer::setSize(int w, int h)
	{
		width = w;
		height = h;

		uint32_t flags = BGFX_RESET_MAXANISOTROPY;

		if (Engine::getSingleton()->getSettings()->getVSync())
			flags |= BGFX_RESET_VSYNC;
		
		bgfx::reset(width, height, flags);

		if (backBuffer != nullptr)
			backBuffer->reset(width, height);

		resetFrameBuffers();

		std::vector<GameObject*> gameObjects = Engine::getSingleton()->getGameObjects();
		for (auto& obj : gameObjects)
		{
			std::vector<Component*>& components = obj->getComponents();
			for (auto& cc : components)
				cc->onScreenResized(w, h);
		}
	}

	int Renderer::getWidth()
	{
		return width;
	}

	int Renderer::getHeight()
	{
		return height;
	}

	void Renderer::resetFrameBuffers()
	{
		int camIndex = 0;

		for (auto it = cameras.begin(); it != cameras.end(); ++it, ++camIndex)
		{
			if (!Engine::getSingleton()->getIsRuntimeMode())
			{
				if (camIndex > 0)
					break;
			}

			if (!(*it)->getEnabled() ||
				(*it)->gameObject == nullptr ||
				!(*it)->gameObject->getActive())
				continue;

			(*it)->resetFrameBuffers();
		}
	}

	void Renderer::collectStats()
	{
		//Update stats
		const bgfx::Stats* stats = bgfx::getStats();
		double toMsCpu = 1000.0 / stats->cpuTimerFreq;
		cpuTime += double(stats->cpuTimeEnd - stats->cpuTimeBegin) * toMsCpu;

		double toMsGpu = 1000.0 / stats->gpuTimerFreq;
		gpuTime += double(stats->gpuTimeEnd - stats->gpuTimeBegin) * toMsGpu;

		frameMs += double(stats->cpuTimeFrame) * toMsCpu;
		numDrawCalls += stats->numDraw;
		numTriangles += stats->numPrims[bgfx::Topology::TriList];

		gpuMemUsed = (stats->textureMemoryUsed + stats->rtMemoryUsed) / 1000000;
	}

	std::string Renderer::addRenderCallback(std::function<void(int viewId, int viewLayer, Camera* camera)> callback, int queue)
	{
		RenderCallbackInfo inf;
		inf.cb = callback;
		inf.queue = queue;
		inf.id = GUIDGenerator::genGuid();

		renderCallbacks.push_back(inf);

		std::sort(renderCallbacks.begin(), renderCallbacks.end(), [=](RenderCallbackInfo& cb1, RenderCallbackInfo& cb2) -> bool
			{
				return cb1.queue > cb2.queue;
			}
		);

		return inf.id;
	}

	void Renderer::removeRenderCallback(std::string id)
	{
		auto it = std::find_if(renderCallbacks.begin(), renderCallbacks.end(), [=](RenderCallbackInfo& inf) -> bool
			{
				return inf.id == id;
			}
		);

		if (it != renderCallbacks.end())
			renderCallbacks.erase(it);
	}

	std::string Renderer::addPostRenderCallback(std::function<void()> callback)
	{
		PostRenderCallbackInfo inf;
		inf.cb = callback;
		inf.id = GUIDGenerator::genGuid();

		postRenderCallbacks.push_back(inf);

		return inf.id;
	}

	void Renderer::removePostRenderCallback(std::string id)
	{
		auto it = std::find_if(postRenderCallbacks.begin(), postRenderCallbacks.end(), [=](PostRenderCallbackInfo& inf) -> bool
			{
				return inf.id == id;
			}
		);

		if (it != postRenderCallbacks.end())
			postRenderCallbacks.erase(it);
	}

	void Renderer::uiProcessEvent(const void* event)
	{
		ImGui_ImplSDL2_ProcessEvent((const SDL_Event*)event);
	}

	void splitFrustum(float* _splits, uint8_t _numSplits, float _near, float _far, float _splitWeight = 0.75f)
	{
		const float l = _splitWeight;
		const float ratio = _far / _near;
		const int8_t numSlices = _numSplits * 2;
		const float numSlicesf = float(numSlices);

		// First slice.
		_splits[0] = 0.25f;

		for (uint8_t nn = 2, ff = 1; nn < numSlices; nn += 2, ff += 2)
		{
			float si = float(int8_t(ff)) / numSlicesf;

			const float nearp = l * (_near * bx::pow(ratio, si)) + (1 - l) * (_near + (_far - _near) * si);
			_splits[nn] = nearp;          //near
			_splits[ff] = nearp * 1.005f; //far from previous split
		}

		// Last slice.
		_splits[numSlices - 1] = _far;
	}

	void worldSpaceFrustumCorners(float* _corners24f, float _near, float _far, float _projWidth, float _projHeight, const float* _invViewMtx)
	{
		// Define frustum corners in view space.
		const float nw = _near * _projWidth;
		const float nh = _near * _projHeight;
		const float fw = _far * _projWidth;
		const float fh = _far * _projHeight;

		const uint8_t numCorners = 8;
		const bx::Vec3 corners[numCorners] =
		{
			{ -nw,  nh, _near },
			{  nw,  nh, _near },
			{  nw, -nh, _near },
			{ -nw, -nh, _near },
			{ -fw,  fh, _far  },
			{  fw,  fh, _far  },
			{  fw, -fh, _far  },
			{ -fw, -fh, _far  },
		};

		// Convert them to world space.
		float(*out)[3] = (float(*)[3])_corners24f;
		for (uint8_t ii = 0; ii < numCorners; ++ii)
		{
			bx::store(&out[ii], bx::mul(corners[ii], _invViewMtx));
		}
	}

	void Renderer::renderUI(std::vector<GameObject*>& gameObjects, glm::vec2 sizeOffset, bool updateMouse)
	{
		std::vector<Canvas*>& canvases = Renderer::getSingleton()->getCanvases();
		ImGuiWindow* win = ImGui::GetCurrentWindow();
		for (auto canv = canvases.begin(); canv != canvases.end(); ++canv)
		{
			Canvas* canvas = *canv;
			if (!canvas->getEnabled() || !canvas->getGameObject()->getActive())
				continue;

			canvas->setWindow(win);

			glm::vec2 refSize = canvas->getRefScreenSize();
			float delta = canvas->getScreenMatchSide();
			float zoom = 1.0f;

			if (canvas->getMode() == CanvasMode::ScaleToScreenSize)
			{
				zoom = (((win->Size.x + sizeOffset.x) / refSize.x) * (1.0f - delta)) + (((win->Size.y + sizeOffset.y) / refSize.y) * delta);
				canvas->setZoom(zoom);
			}

			std::vector<UIElement*>& uiElements = canvas->getUIElements();
			std::sort(uiElements.begin(), uiElements.end(), [=](UIElement* elem1, UIElement* elem2) -> bool
				{
					auto e1 = std::find(gameObjects.begin(), gameObjects.end(), elem1->getGameObject());
					auto e2 = std::find(gameObjects.begin(), gameObjects.end(), elem2->getGameObject());
					int64_t eidx1 = std::distance(gameObjects.begin(), e1) + (int)elem1->getTransform()->getPosition().z;
					int64_t eidx2 = std::distance(gameObjects.begin(), e2) + (int)elem2->getTransform()->getPosition().z;
					return eidx1 < eidx2;
				}
			);

			for (auto elem = uiElements.begin(); elem != uiElements.end(); ++elem)
			{
				UIElement* element = *elem;

				if (!element->getEnabled() || !element->getGameObject()->getActive())
					continue;

				if (element->getCanvas() == nullptr)
					continue;

				int pushCount = 0;
				Transform* par = element->getTransform()->getParent();
				while (par != nullptr)
				{
					GameObject* ob = par->getGameObject();
					Mask* mask = (Mask*)ob->getComponent(Mask::COMPONENT_TYPE);
					if (mask != nullptr)
					{
						UIElement::Properties& props = mask->getProperties();
						props.isEditor = false;

						++pushCount;
						glm::vec4 rect = mask->getRect();
						win->DrawList->PushClipRect(ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w), true);
					}

					par = par->getParent();
				}

				element->clearClipRects();
				UIElement::Properties& props = element->getProperties();

				auto& clipRects = props.clipRects;
				auto& clipRects1 = win->DrawList->_ClipRectStack;
				for (auto& clip : clipRects1)
					clipRects.push_back(glm::vec4(clip.x, clip.y, clip.z, clip.w));

				props.isEditor = false;
				element->onUpdate(Time::getDeltaTime());

				element->onRender(win->DrawList);

				while (pushCount > 0)
				{
					--pushCount;
					win->DrawList->PopClipRect();
				}
			}

			if (canvas->getMode() == CanvasMode::ScaleToScreenSize)
			{
				float dimX = ((win->Size.x / refSize.x) / zoom) * delta;
				float dimY = ((win->Size.y / refSize.y) / zoom) * (1.0f - delta);
				canvas->setRealScreenSize(glm::vec2(refSize.x * ((1.0f - delta) + dimX), refSize.y * (delta + dimY)), win);
			}
			else
				canvas->setRealScreenSize(glm::vec2(win->Size.x, win->Size.y), win);
		}
	}

	void Renderer::frame()
	{
		SDL_GL_MakeCurrent((SDL_Window*)sdlWindow, context);
		bgfx::frame();
	}

	//Single thread rendering
	void Renderer::renderFrame()
	{
		cpuTime = 0;
		gpuTime = 0;
		frameMs = 0;
		numDrawCalls = 0;
		numTriangles = 0;

		clearTransientRenderables();

		Camera::updateDynamicResolution();

		std::vector<GameObject*> gameObjects = Engine::getSingleton()->getGameObjects();
		std::vector<Camera*>& cameras = Renderer::getSingleton()->getCameras();
		std::vector<Light*>& lights = Renderer::getSingleton()->getLights();

		glm::mat4x4 screenView = glm::identity<glm::mat4x4>();
		glm::mat4x4 screenProj = glm::orthoRH(
			0.0f
			, 1.0f
			, 1.0f
			, 0.0f
			, 0.0f
			, 100.0f
		);

		std::sort(cameras.begin(), cameras.end(), [=](Camera* cam1, Camera* cam2) -> bool
			{
				return cam1->getDepth() < cam2->getDepth();
			}
		);

		//-->Render UI begin
		bgfx::setViewClear(RENDER_UI_PASS_ID, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, Color::packRGBA(uiClearColor), 1.0f, 0);
		bgfx::setViewRect(RENDER_UI_PASS_ID, 0, 0, width, height);

		bgfx::setViewFrameBuffer(RENDER_UI_PASS_ID, BGFX_INVALID_HANDLE);

		ImGui_Implbgfx_NewFrame();
		ImGui_ImplSDL2_NewFrame();

		ImGui::NewFrame();

		if (uiCallback != nullptr)
			uiCallback();

		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			bool isEditor = false;
			for (auto& cam : cameras)
			{
				if (cam->getIsEditorCamera())
				{
					isEditor = true;
					break;
				}
			}

			if (!isEditor)
			{
				bool open = true;
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
				ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
				ImGui::SetNextWindowSize(ImVec2((float)width, (float)height), ImGuiCond_Always);
				ImGui::Begin("BackbufferOutput", &open, ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoBringToFrontOnFocus
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoDecoration
					| ImGuiWindowFlags_NoDocking
					| ImGuiWindowFlags_NoFocusOnAppearing
					| ImGuiWindowFlags_NoInputs
					| ImGuiWindowFlags_NoMouseInputs
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoNav
					| ImGuiWindowFlags_NoNavFocus
					| ImGuiWindowFlags_NoNavInputs
					| ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoSavedSettings
					| ImGuiWindowFlags_NoScrollbar
					| ImGuiWindowFlags_NoTitleBar
				);

				if (backBuffer != nullptr)
					ImGui::Image((void*)backBuffer->getColorTextureHandle().idx, ImVec2((float)width, (float)height), ImVec2(0, 1), ImVec2(1, 0));

				renderUI(gameObjects);

				ImGui::SetCursorPos(ImVec2(0, 0));

				APIManager::getSingleton()->execute("OnGUI");

				ImGui::End();
				ImGui::PopStyleVar(3);
			}
		}

		ImGui::Render();

		ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());

		if (lights.size() > 0 || renderables.size() > 0)
			frame();

		//-->Render UI end

		//----------Render point and spot light shadows----------//

		renderPointAndSpotLightShadows();

		//-------------------------------------------------------//

		int numLightsWithShadows = getNumActiveLightsWithShadows();
		int viewLayer = numLightsWithShadows * NUM_LIGHT_VIEWS;

		int camIndex = 0;
		for (auto c = cameras.begin(); c != cameras.end(); c++, ++camIndex)
		{
			Camera* camera = *c;

			if (!Engine::getSingleton()->getIsRuntimeMode())
			{
				if (camIndex > 0)
					break;
			}

			camera->renderFrame();
		}

		for (auto& obj : gameObjects)
		{
			if (!obj->getActive())
				continue;

			std::vector<Component*>& components = obj->getComponents();
			for (auto& cc : components)
			{
				if (cc->isUiComponent())
					continue;

				if (cc->getEnabled())
					cc->onUpdate(Time::getDeltaTime());
			}
		}

		for (auto& cb : postRenderCallbacks)
			cb.cb();

		lastCpuTime = cpuTime;
		lastGpuTime = gpuTime;
		lastFrameMs = frameMs;
		lastNumDrawCalls = numDrawCalls;
		lastNumTriangles = numTriangles;
	}

	void Renderer::renderObjects(Camera* camera, int viewLayer, int renderQueue, const glm::mat4x4& view, const glm::mat4x4& proj, const glm::mat4x4& skyMtx)
	{
		if (bgfx::getCaps()->limits.maxFBAttachments > 1 && bgfx::isValid(combinePH))
		{
			//-----------Render skybox-----------//

			if (renderQueue == 0)
				renderSkybox(RENDER_SKYBOX_PASS_ID + viewLayer, camera, skyMtx);

			//-----------Render forward-----------//

			std::vector<Renderable*> transparentObjects;

			//Render opaque

			for (auto it = renderables.begin(); it != renderables.end(); ++it)
			{
				Renderable* comp = *it;

				if (comp->getSkipRendering() || comp->isDecal())
					continue;

				if (comp->getRenderQueue() != renderQueue)
					continue;

				if (!comp->checkCullingMask(camera->getCullingMask()))
					continue;

				if (!comp->isAlwaysVisible())
				{
					if (!camera->isVisible(comp->getBounds()))
						continue;
				}

				if (!comp->isTransparent())
				{
					comp->onRender(camera, RENDER_FORWARD_PASS_ID + viewLayer, getRenderState(camera, defaultRenderState), { bgfx::kInvalidHandle }, static_cast<int>(RenderMode::Forward), [=]() {
						setSystemUniforms(camera);
						});
				}
				else
					transparentObjects.push_back(comp);
			}

			//Sort transparent
			glm::vec3 cameraPos = camera->getTransform()->getPosition();
			std::sort(transparentObjects.begin(), transparentObjects.end(), [=](Renderable* rend1, Renderable* rend2) -> bool
				{
					glm::vec3 pos1 = rend1->getBounds().getCenter();
					glm::vec3 pos2 = rend2->getBounds().getCenter();

					float dist1 = glm::distance(pos1, cameraPos);
					float dist2 = glm::distance(pos2, cameraPos);

					return dist1 > dist2;
				}
			);

			//Render transparent
			for (auto it = transparentObjects.begin(); it != transparentObjects.end(); ++it)
			{
				Renderable* comp = *it;

				comp->onRender(camera, RENDER_FORWARD_PASS_ID + viewLayer, getRenderState(camera, defaultRenderState), { bgfx::kInvalidHandle }, static_cast<int>(RenderMode::Forward), [=]() {
					setSystemUniforms(camera);
					});
			}

			transparentObjects.clear();
		}
	}

	void Renderer::renderRenderCallbacks(Camera* camera, int viewLayer)
	{
		if (bgfx::getCaps()->limits.maxFBAttachments > 1 && bgfx::isValid(combinePH))
		{
			for (auto cb = renderCallbacks.begin(); cb != renderCallbacks.end(); ++cb)
			{
				if (cb->cb != nullptr)
					cb->cb(RENDER_FORWARD_PASS_ID + viewLayer, viewLayer, camera);
			}
		}
	}

	void Renderer::renderPointAndSpotLightShadows()
	{
		if (projectSettings != nullptr && projectSettings->getShadowsEnabled() && shadowsEnabled)
		{
			int lightIndex = 0;

			for (auto ll = lights.begin(); ll != lights.end(); ++ll)
			{
				Light* light = *ll;

				if (!light->getCastShadows())
					continue;
				if (light->getLightRenderMode() == LightRenderMode::Baked)
					continue;
				if (!light->getEnabled())
					continue;
				if (!light->getGameObject()->getActive())
					continue;

				int viewLayer = lightIndex * NUM_LIGHT_VIEWS;
				++lightIndex;

				if (light->getLightType() == LightType::Directional)
					continue;

				bgfx::setViewClear(RENDER_SHADOW_1_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
				bgfx::setViewClear(RENDER_SHADOW_2_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
				bgfx::setViewClear(RENDER_SHADOW_3_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
				bgfx::setViewClear(RENDER_SHADOW_4_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);

				Transform* lightTransform = light->getGameObject()->getTransform();
				glm::vec3 lightPos = lightTransform->getPosition();

				bool lightVisible = false;
				int camIndex = 0;
				for (auto c = cameras.begin(); c != cameras.end(); c++, ++camIndex)
				{
					if (!Engine::getSingleton()->getIsRuntimeMode())
					{
						if (camIndex > 0)
							break;
					}

					if (!(*c)->getEnabled())
						continue;

					if ((*c)->gameObject == nullptr || !(*c)->gameObject->getActive())
						continue;

					Frustum* camFrustum = (*c)->getFrustum();
					if (camFrustum->sphereInFrustum(lightPos, light->getRadius() * 0.5f))
					{
						lightVisible = true;
						break;
					}
				}

				if (!lightVisible)
					continue;

				int shadowMapSize = light->getShadowMapSize();

				glm::mat4x4 lightView[4];
				glm::mat4x4 lightProj = glm::identity<glm::mat4x4>();
				glm::mat4x4 mtxYpr[4];

				glm::vec3 lightDir = lightTransform->getForward();
				glm::vec3 lightRight = lightTransform->getRight();

				glm::vec3 eye = glm::vec3(0, 0, 0);
				glm::vec3 at = eye + lightDir;
				lightView[0] = glm::lookAtRH(eye, at, glm::vec3(0, 1, 0));

				const int maxNumSplits = 4;
				int numSplits = 1;
				glm::mat4x4 mShadowMtx[4];
				for (int iii = 0; iii < 4; ++iii)
					mShadowMtx[iii] = glm::identity<glm::mat4x4>();

				glm::vec4 splitDistances = glm::vec4(0, 0, 0, 0);

				if (light->getLightType() == LightType::Point)
				{
					numSplits = 4;

					float ypr[4][3] =
					{
						{ bx::toRad(180.0f), bx::toRad(-27.36780516f), bx::toRad(0.0f) },
						{ bx::toRad(0.0f), bx::toRad(-27.36780516f), bx::toRad(0.0f) },
						{ bx::toRad(90.0f), bx::toRad(27.36780516f), bx::toRad(0.0f) },
						{ bx::toRad(-90.0f), bx::toRad(27.36780516f), bx::toRad(0.0f) },
					};

					const float fovx = 143.98570868f + 7.8f + 0; //-20..20
					const float fovy = 125.26438968f + 3.0f + 0; //-20..20
					const float aspect = bx::tan(bx::toRad(fovx * 0.5f)) / bx::tan(bx::toRad(fovy * 0.5f));

					lightProj = glm::perspectiveRH(fovy, aspect, 0.25f, 250.0f);

					for (uint8_t ii = 0; ii < 4; ++ii)
					{
						glm::mat4x4 mtxTmp = glm::yawPitchRoll(ypr[ii][0], ypr[ii][1], ypr[ii][2]);

						float tmp[3] =
						{
							-glm::dot(lightPos, glm::make_vec3(&mtxTmp[0].x)),
							-glm::dot(lightPos, glm::make_vec3(&mtxTmp[1].x)),
							-glm::dot(lightPos, glm::make_vec3(&mtxTmp[2].x))
						};

						mtxYpr[ii] = glm::transpose(mtxTmp);

						memcpy(glm::value_ptr(lightView[ii]), glm::value_ptr(mtxYpr[ii]), 12 * sizeof(float));
						lightView[ii][3].x = tmp[0];
						lightView[ii][3].y = tmp[1];
						lightView[ii][3].z = tmp[2];
						lightView[ii][3].w = 1.0f;
					}

					const float s = 1.0f; //sign
					float zadd = 0.5f;

					const float mtxCropBias[4][16] =
					{
						{ // D3D: Green, OGL: Blue
							0.25f,    0.0f, 0.0f, 0.0f,
							0.0f, s * 0.25f, 0.0f, 0.0f,
							0.0f,    0.0f, 0.5f, 0.0f,
							0.25f,   0.25f, zadd, 1.0f,
						},
						{ // D3D: Yellow, OGL: Red
							0.25f,    0.0f, 0.0f, 0.0f,
							0.0f, s * 0.25f, 0.0f, 0.0f,
							0.0f,    0.0f, 0.5f, 0.0f,
							0.75f,   0.25f, zadd, 1.0f,
						},
						{ // D3D: Blue, OGL: Green
							0.25f,    0.0f, 0.0f, 0.0f,
							0.0f, s * 0.25f, 0.0f, 0.0f,
							0.0f,    0.0f, 0.5f, 0.0f,
							0.25f,   0.75f, zadd, 1.0f,
						},
						{ // D3D: Red, OGL: Yellow
							0.25f,    0.0f, 0.0f, 0.0f,
							0.0f, s * 0.25f, 0.0f, 0.0f,
							0.0f,    0.0f, 0.5f, 0.0f,
							0.75f,   0.75f, zadd, 1.0f,
						},
					};

					static const uint8_t cropBiasIndices[4] = { 2, 3, 0, 1 };

					for (uint8_t ii = 0; ii < 4; ++ii)
					{
						uint8_t biasIndex = cropBiasIndices[ii];

						float mtxTmp[16];
						bx::mtxMul(mtxTmp, glm::value_ptr(mtxYpr[ii]), glm::value_ptr(lightProj));
						bx::mtxMul(glm::value_ptr(mShadowMtx[ii]), mtxTmp, mtxCropBias[biasIndex]); //mtxYprProjBias
					}
				}

				if (light->getLightType() == LightType::Spot)
				{
					numSplits = 1;

					const float fovy = 130.0f * Mathf::fDeg2Rad;
					const float aspect = 1.0f;

					lightProj = glm::perspectiveRH(fovy, aspect, 0.25f, 250.0f);
					lightView[0] = glm::lookAtRH(lightPos, lightPos + lightDir, glm::vec3(0, 1, 0));

					const float ymul = 0.5f;
					float zadd = 0.5f;

					const float mtxBias[16] =
					{
						0.5f, 0.0f, 0.0f, 0.0f,
						0.0f, ymul, 0.0f, 0.0f,
						0.0f, 0.0f, 0.5f, 0.0f,
						0.5f, 0.5f, zadd, 1.0f,
					};

					float mtxTmp[16];

					bx::mtxMul(mtxTmp, glm::value_ptr(lightProj), mtxBias);
					bx::mtxMul(glm::value_ptr(mShadowMtx[0]), glm::value_ptr(lightView[0]), mtxTmp); //lViewProjCropBias
				}

				int numShadowMaps = light->getNumShadowMaps();
				if (numShadowMaps > 0)
				{
					if (light->getLightType() == LightType::Point)
					{
						const uint16_t h = shadowMapSize / 2; //half size
						bgfx::setViewRect(RENDER_SHADOW_1_PASS_ID + viewLayer, 0, 0, h, h);
						bgfx::setViewRect(RENDER_SHADOW_2_PASS_ID + viewLayer, h, 0, h, h);
						bgfx::setViewRect(RENDER_SHADOW_3_PASS_ID + viewLayer, 0, h, h, h);
						bgfx::setViewRect(RENDER_SHADOW_4_PASS_ID + viewLayer, h, h, h, h);

						for (int split = 0; split < 4; ++split)
						{
							bgfx::setViewTransform(RENDER_SHADOW_1_PASS_ID + split + viewLayer, glm::value_ptr(lightView[split]), glm::value_ptr(lightProj));
							bgfx::setViewFrameBuffer(RENDER_SHADOW_1_PASS_ID + split + viewLayer, light->shadowMapFB[0]);
							bgfx::touch(RENDER_SHADOW_1_PASS_ID + split + viewLayer);
						}
					}

					if (light->getLightType() == LightType::Spot)
					{
						bgfx::setViewRect(RENDER_SHADOW_1_PASS_ID + viewLayer, 0, 0, shadowMapSize, shadowMapSize);
						bgfx::setViewTransform(RENDER_SHADOW_1_PASS_ID + viewLayer, glm::value_ptr(lightView[0]), glm::value_ptr(lightProj));
						bgfx::setViewFrameBuffer(RENDER_SHADOW_1_PASS_ID + viewLayer, light->shadowMapFB[0]);

						bgfx::touch(RENDER_SHADOW_1_PASS_ID + viewLayer);
					}
				}

				light->mShadowMtx[0] = mShadowMtx[0];
				light->mShadowMtx[1] = mShadowMtx[1];
				light->mShadowMtx[2] = mShadowMtx[2];
				light->mShadowMtx[3] = mShadowMtx[3];

				for (int split = 0; split < numSplits; ++split)
					light->mShadowFrustum[split]->calculateFrustum(lightView[split], lightProj);

				for (auto it = renderables.begin(); it != renderables.end(); ++it)
				{
					Renderable* comp = *it;

					if (comp->getSkipRendering() || comp->isDecal())
						continue;

					if (!comp->getCastShadows())
						continue;

					if (light->getLightRenderMode() == LightRenderMode::Mixed)
					{
						if (comp->isStatic())
							continue;
					}

					AxisAlignedBox aab = comp->getBounds();

					if (!Mathf::intersects(lightPos, light->getRadius(), aab))
						continue;

					bool isVisible = false;
					camIndex = 0;
					for (auto cam = cameras.begin(); cam != cameras.end(); ++cam, ++camIndex)
					{
						Camera* camera = *cam;

						if (!Engine::getSingleton()->getIsRuntimeMode())
						{
							if (camIndex > 0)
								break;
						}

						if (!camera->getEnabled())
							continue;

						if (camera->gameObject == nullptr || !camera->gameObject->getActive())
							continue;

						if (!comp->isAlwaysVisible())
						{
							if (comp->checkCullingMask(camera->getCullingMask()))
							{
								if (Mathf::intersects(camera->getTransform()->getPosition(), projectSettings->getShadowDistance(), aab))
								{
									if (camera->isVisible(aab, 2.2f))
									{
										isVisible = true;
										break;
									}
								}
							}
						}
						else
						{
							if (comp->checkCullingMask(camera->getCullingMask()))
							{
								if (Mathf::intersects(camera->getTransform()->getPosition(), projectSettings->getShadowDistance(), aab))
								{
									isVisible = true;
									break;
								}
							}
						}

						if (!Engine::getSingleton()->getIsRuntimeMode())
							break;
					}

					if (!isVisible)
						continue;

					for (uint8_t ii = 0; ii < numSplits; ++ii)
					{
						if (!light->mShadowFrustum[ii]->sphereInFrustum(aab.getCenter(), aab.getRadius()))
							continue;

						/*Camera lightCamera;
						lightCamera.injectView = true;
						lightCamera.injectProj = true;
						lightCamera.injectedViewMatrix = lightView[ii];
						lightCamera.injectedProjectionMatrix = lightProj;
						lightCamera.transform = lightTransform;*/

						const uint8_t viewId = RENDER_SHADOW_1_PASS_ID + viewLayer + ii;
						comp->onRender(nullptr, viewId, shadowRenderState, shadowCasterPH, static_cast<int>(RenderMode::Forward), [=]()
							{
								setSystemUniforms(nullptr);
								light->submitUniforms();
							}
						);
					}
				}
			}
		}
	}

	void Renderer::renderDirectionalLightShadows(Camera* camera, const glm::mat4x4& invView)
	{
		if (projectSettings != nullptr && projectSettings->getShadowsEnabled() && shadowsEnabled)
		{
			const float projHeight = bx::tan(bx::toRad(camera->getFOVy()) * 0.5f);
			const float projWidth = projHeight * camera->getAspectRatio();

			int lightIndex = 0;

			for (auto ll = lights.begin(); ll != lights.end(); ++ll)
			{
				Light* light = *ll;

				if (!light->getCastShadows())
					continue;
				if (light->getLightRenderMode() == LightRenderMode::Baked)
					continue;
				if (!light->getEnabled())
					continue;
				if (!light->getGameObject()->getActive())
					continue;

				int viewLayer = lightIndex * NUM_LIGHT_VIEWS;
				++lightIndex;

				if (light->getLightType() != LightType::Directional)
					continue;

				bgfx::setViewClear(RENDER_SHADOW_1_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
				bgfx::setViewClear(RENDER_SHADOW_2_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
				bgfx::setViewClear(RENDER_SHADOW_3_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
				bgfx::setViewClear(RENDER_SHADOW_4_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);

				Transform* lightTransform = light->getGameObject()->getTransform();

				int shadowMapSize = light->getShadowMapSize();

				glm::mat4x4 lightView = glm::identity<glm::mat4x4>();
				glm::mat4x4 lightProj[4];

				glm::vec3 camPos = camera->getTransform()->getPosition();
				glm::vec3 lightDir = lightTransform->getForward();

				glm::vec3 eye = camPos;
				glm::vec3 at = eye + lightDir;
				lightView = glm::lookAtRH(eye, at, glm::vec3(0, 1, 0));

				const int maxNumSplits = 4;
				int numSplits = 1;
				glm::mat4x4 mShadowMtx[4];
				for (int iii = 0; iii < 4; ++iii)
					mShadowMtx[iii] = glm::identity<glm::mat4x4>();
				glm::vec4 splitDistances = glm::vec4(0, 0, 0, 0);

				numSplits = projectSettings->getShadowCascadesCount();
				float splitSlices[maxNumSplits * 2];
				splitFrustum(splitSlices
					, uint8_t(numSplits)
					, -0.25f
					, -250.0f
					, 0.6f
				);

				// Update uniforms.
				for (uint8_t ii = 0, ff = 1; ii < numSplits; ++ii, ff += 2)
				{
					// This lags for 1 frame, but it's not a problem.
					splitDistances[ii] = splitSlices[ff];
				}

				glm::mat4x4 mtxProj = glm::orthoRH(
					1.0f
					, -1.0f
					, 1.0f
					, -1.0f
					, -2500.0f
					, 2500.0f
				);

				const uint8_t numCorners = 8;
				float frustumCorners[maxNumSplits][numCorners][3];
				for (uint8_t ii = 0, nn = 0, ff = 1; ii < numSplits; ++ii, nn += 2, ff += 2)
				{
					// Compute frustum corners for one split in world space.
					worldSpaceFrustumCorners((float*)frustumCorners[ii], splitSlices[nn], splitSlices[ff], projWidth, projHeight, glm::value_ptr(invView));

					bx::Vec3 min = { SHADOW_AAB_VAL, SHADOW_AAB_VAL, SHADOW_AAB_VAL };
					bx::Vec3 max = { -SHADOW_AAB_VAL, -SHADOW_AAB_VAL, -SHADOW_AAB_VAL };

					for (uint8_t jj = 0; jj < numCorners; ++jj)
					{
						// Transform to light space.
						const bx::Vec3 xyz = bx::mul(bx::load<bx::Vec3>(frustumCorners[ii][jj]), glm::value_ptr(lightView));

						// Update bounding box.
						min = bx::min(min, xyz);
						max = bx::max(max, xyz);
					}

					const bx::Vec3 minproj = bx::mulH(min, glm::value_ptr(mtxProj));
					const bx::Vec3 maxproj = bx::mulH(max, glm::value_ptr(mtxProj));

					float scalex = 2.0f / (maxproj.x - minproj.x);
					float scaley = 2.0f / (maxproj.y - minproj.y);

					//if (m_settings.m_stabilize)
					//{
					const float quantizer = 64.0f;
					scalex = quantizer / bx::ceil(quantizer / scalex);
					scaley = quantizer / bx::ceil(quantizer / scaley);
					//}

					float offsetx = 0.5f * (maxproj.x + minproj.x) * scalex;
					float offsety = 0.5f * (maxproj.y + minproj.y) * scaley;

					//if (m_settings.m_stabilize)
					//{
					const float halfSize = shadowMapSize * 0.5f;
					offsetx = bx::ceil(offsetx * halfSize) / halfSize;
					offsety = bx::ceil(offsety * halfSize) / halfSize;
					//}

					float mtxCrop[16];
					bx::mtxIdentity(mtxCrop);
					mtxCrop[0] = scalex;
					mtxCrop[5] = scaley;
					mtxCrop[12] = offsetx;
					mtxCrop[13] = offsety;

					bx::mtxMul(glm::value_ptr(lightProj[ii]), mtxCrop, glm::value_ptr(mtxProj));
				}

				const float ymul = 0.5f;
				float zadd = 0.5f;

				const float mtxBias[16] =
				{
					0.5f, 0.0f, 0.0f, 0.0f,
					0.0f, ymul, 0.0f, 0.0f,
					0.0f, 0.0f, 0.5f, 0.0f,
					0.5f, 0.5f, zadd, 1.0f,
				};

				for (uint8_t ii = 0; ii < numSplits; ++ii)
				{
					float mtxTmp[16];

					bx::mtxMul(mtxTmp, glm::value_ptr(lightProj[ii]), mtxBias);
					bx::mtxMul(glm::value_ptr(mShadowMtx[ii]), glm::value_ptr(lightView), mtxTmp); //lViewProjCropBias
				
					bgfx::setViewRect(RENDER_SHADOW_1_PASS_ID + ii + viewLayer, 0, 0, shadowMapSize, shadowMapSize);
					bgfx::setViewTransform(RENDER_SHADOW_1_PASS_ID + ii + viewLayer, glm::value_ptr(lightView), glm::value_ptr(lightProj[ii]));
					bgfx::setViewFrameBuffer(RENDER_SHADOW_1_PASS_ID + ii + viewLayer, light->shadowMapFB[ii]);
					bgfx::touch(RENDER_SHADOW_1_PASS_ID + ii + viewLayer);
				}

				light->mShadowMtx[0] = mShadowMtx[0];
				light->mShadowMtx[1] = mShadowMtx[1];
				light->mShadowMtx[2] = mShadowMtx[2];
				light->mShadowMtx[3] = mShadowMtx[3];

				for (int split = 0; split < numSplits; ++split)
					light->mShadowFrustum[split]->calculateFrustum(lightView, lightProj[split]);

				for (auto it = renderables.begin(); it != renderables.end(); ++it)
				{
					Renderable* comp = *it;

					if (comp->getSkipRendering() || comp->isDecal())
						continue;

					if (!comp->getCastShadows())
						continue;

					if (light->getLightRenderMode() == LightRenderMode::Mixed)
					{
						if (comp->isStatic())
							continue;
					}

					if (!comp->checkCullingMask(camera->getCullingMask()))
						continue;

					/*if (!comp->isAlwaysVisible())
					{
						if (!camera->isVisible(comp->getBounds(), 2.2f))
							continue;
					}*/

					AxisAlignedBox aab = comp->getBounds();

					if (aab.isNull() || aab.isInfinite())
						continue;

					if (!Mathf::intersects(camPos, projectSettings->getShadowDistance(), aab))
						continue;

					for (uint8_t ii = 0; ii < numSplits; ++ii)
					{
						if (!light->mShadowFrustum[ii]->sphereInFrustum(aab.getCenter(), aab.getRadius()))
							continue;

						/*Camera lightCamera;
						lightCamera.injectView = true;
						lightCamera.injectProj = true;
						lightCamera.injectedViewMatrix = lightView;
						lightCamera.injectedProjectionMatrix = lightProj[ii];
						lightCamera.transform = lightTransform;*/

						const uint8_t viewId = RENDER_SHADOW_1_PASS_ID + viewLayer + ii;
						comp->onRender(nullptr, viewId, shadowRenderState, shadowCasterPH, static_cast<int>(RenderMode::Forward), [=]()
							{
								setSystemUniforms(camera);
								light->submitUniforms();
							}
						);
					}
				}
			}
		}
	}

	void Renderer::renderSkybox(int skyView, Camera* camera, const glm::mat4x4& skyMtx)
	{
		if (camera != nullptr)
		{
			if (camera->getClearFlags() != Camera::ClearFlags::Skybox)
				return;
		}

		//-->Render skybox begin
		if (skyMaterial != nullptr)
		{
			Shader* shader = nullptr;
			if (skyMaterial->isLoaded())
				shader = skyMaterial->getShader();

			if (shader != nullptr && shader->isLoaded())
			{
				int passCount = shader->getPassCount();

				for (int j = 0; j < passCount; ++j)
				{
					Pass* pass = shader->getPass(j);

					ProgramVariant* pv = nullptr;
					if (pass != nullptr)
						pv = pass->getProgramVariant(skyMaterial->getDefinesStringHash());

					uint64_t passState = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;

					if (pv != nullptr)
						passState = pv->getRenderState(passState);

					// Pass uniforms
					if (pv != nullptr)
						skyMaterial->submitUniforms(pv, camera);

					//Bind system uniforms
					setSystemUniforms(camera);

					// Render
					bgfx::ProgramHandle ph = { bgfx::kInvalidHandle };
					if (pv != nullptr)
						ph = pv->programHandle;

					if (skyModel == SkyModel::Box)
						Primitives::skybox(skyMtx, skyView, passState, ph, camera);
					else
						Primitives::sphere(skyMtx, 48, skyView, passState, ph, camera);
				}
			}
		}
		//-->Render skybox end
	}

	void Renderer::setSystemUniforms(Camera* camera)
	{
		if (camera != nullptr)
		{
			Transform* cameraTransform = camera->getTransform();

			Color clearColor = camera->getClearColor();
			if (skyMaterial != nullptr)
				clearColor[3] = 0.0f;
			else
				clearColor[3] = 1.0f;

			float w = width;
			float h = height;

			if (camera->sceneBuffer != nullptr)
			{
				w = camera->sceneBuffer->getWidth();
				h = camera->sceneBuffer->getHeight();
			}

			//if (Engine::getSingleton()->getIsRuntimeMode())
			//{
			RenderTexture* rt = camera->getRenderTarget();
			if (rt != nullptr)
			{
				w = rt->getWidth();
				h = rt->getHeight();
			}
			//}

			bgfx::setUniform(uCamPos, glm::value_ptr(glm::vec4(cameraTransform->getPosition(), 0.0f)), 1);
			bgfx::setUniform(uCamDir, glm::value_ptr(glm::vec4(cameraTransform->getForward(), 0.0f)), 1);
			bgfx::setUniform(uCamFov, glm::value_ptr(glm::vec4(camera->getFOVy(), 1.0f / camera->getFOVy(), 0.0f, 0.0f)), 1);
			bgfx::setUniform(uCamView, glm::value_ptr(camera->getViewMatrix()), 1);
			bgfx::setUniform(uCamProj, glm::value_ptr(camera->getProjectionMatrix()), 1);
			bgfx::setUniform(uCamInvView, glm::value_ptr(glm::inverse(camera->getViewMatrix())), 1);
			bgfx::setUniform(uCamInvProj, glm::value_ptr(glm::inverse(camera->getProjectionMatrix())), 1);
			bgfx::setUniform(uCamClearColor, clearColor.ptr(), 1);
			bgfx::setUniform(uClipPlane, glm::value_ptr(glm::vec4(camera->getNear(), camera->getFar(), 0.0f, 0.0f)), 1);
			bgfx::setUniform(uScreenParams, glm::value_ptr(glm::vec4(w, h, width, height)), 1);
		}

		bgfx::setUniform(uAmbientColor, ambientColor.ptr(), 1);
		bgfx::setUniform(uTime, glm::value_ptr(glm::vec4(Time::getTimeSinceLevelStart(), 0.0f, 0.0f, 0.0f)), 1);
		bgfx::setUniform(uScaledTime, glm::value_ptr(glm::vec4(Time::getTimeSinceLevelStartScaled(), 0.0f, 0.0f, 0.0f)), 1);

		bgfx::setUniform(uUseFXAA, glm::value_ptr(glm::vec4(projectSettings->getFXAA() ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f)), 1);
		bgfx::setUniform(uFogEnabled, glm::value_ptr(glm::vec4(fogEnabled ? 1.0f : 0.0f, fogIncludeSkybox ? 1.0f : 0.0f, 0.0f, 0.0f)), 1);
		bgfx::setUniform(uFogParams, glm::value_ptr(glm::vec4(fogStartDistance, fogEndDistance, fogDensity, fogType)), 1);
		bgfx::setUniform(uFogColor, fogColor.ptr(), 1);

		if (environmentMap != nullptr)
			bgfx::setTexture(9, uEnvMap, environmentMap->getHandle());
		else
			bgfx::setTexture(9, uEnvMap, Cubemap::getNullCubemap()->getHandle());

		float enbl = giEnabled ? 1.0f : 0.0f;
		bgfx::setUniform(uGIParams, glm::value_ptr(glm::vec4(enbl, giIntensity, 0.0f, 0.0f)), 1);
	}

	int Renderer::getOutlineViewId()
	{
		return RENDER_OUTLINE_PASS_ID;
	}

	int Renderer::getFinalViewId()
	{
		return RENDER_FINAL_PASS_ID;
	}

	int Renderer::getSceneViewId()
	{
		return RENDER_SCENE_PASS_ID;
	}

	int Renderer::getOverlayViewId()
	{
		return RENDER_OVERLAY_PASS_ID;
	}

	int Renderer::getNumViewsUsed()
	{
		int numLightsWithShadows = getNumActiveLightsWithShadows();

		return numLightsWithShadows * NUM_LIGHT_VIEWS + NUM_VIEWS;
	}

	Light* Renderer::getFirstLight()
	{
		Light* light = nullptr;

		//Search directional light first
		for (auto& l : lights)
		{
			if (l->getLightType() != LightType::Directional)
				continue;

			if (l->gameObject->getActive() && l->enabled)
			{
				light = l;
				break;
			}
		}

		//Use other if not found
		if (light == nullptr)
		{
			for (auto& l : lights)
			{
				if (l->getLightType() == LightType::Directional)
					continue;

				if (l->gameObject->getActive() && l->enabled)
				{
					light = l;
					break;
				}
			}
		}

		return light;
	}

	int Renderer::getNumActiveLightsWithShadows()
	{
		int lightsWithShadowsCount = 0;

		for (auto& light : lights)
		{
			if (!light->getCastShadows())
				continue;
			if (light->getLightRenderMode() == LightRenderMode::Baked)
				continue;
			if (!light->getEnabled())
				continue;
			if (!light->getGameObject()->getActive())
				continue;

			++lightsWithShadowsCount;
		}

		return lightsWithShadowsCount;
	}

	void Renderer::setSkyModel(SkyModel value)
	{
		skyModel = value;
		updateEnvironmentMap();
	}

	void Renderer::setSkyMaterial(Material* value)
	{
		skyMaterial = value;
		updateEnvironmentMap();
	}

	void Renderer::calculateVisibility(const glm::mat4x4& view, const glm::mat4x4& proj, Frustum* frustum, float maxDistance)
	{
		moc->ClearBuffer();

		std::vector<GameObject*> staticNodes = Engine::getSingleton()->getGameObjects();

		glm::mat4x4 viewProj = proj * view;
		glm::vec3 cameraPos = glm::vec3(view[3][0], view[3][1], view[3][2]);

		for (auto it = staticNodes.begin(); it != staticNodes.end(); ++it)
		{
			GameObject* obj = *it;
			Transform* t = obj->getTransform();

			if (!obj->getActive())
				continue;

			if (!obj->getOcclusionStatic())
				continue;

			MeshRenderer* rend = (MeshRenderer*)obj->getComponent(MeshRenderer::COMPONENT_TYPE);
			if (rend != nullptr && rend->getEnabled())
			{
				if (rend->isSkinned())
					continue;

				if (rend->isTransparent())
					continue;

				if (frustum != nullptr)
				{
					if (!frustum->sphereInFrustum(t->getPosition(), rend->getBounds().getRadius() * 2.2f))
						continue;
				}

				if (glm::distance(t->getPosition(), cameraPos) > maxDistance)
					continue;

				Mesh* mesh = rend->getMesh();

				if (mesh != nullptr)
				{
					glm::mat4x4 mtx = viewProj * t->getTransformMatrix();

					auto& lodVbuf = rend->getLodVertexBuffer();
					auto& lodVbufMtx = rend->getLodVertexBufferMtxCache();

					for (int sm = 0; sm < mesh->getSubMeshCount(); ++sm)
					{
						SubMesh* subMesh = mesh->getSubMesh(sm);

						auto& vbuf = subMesh->getVertexBuffer();
						auto& ibuf = subMesh->getIndexBuffer();

						int maxLod = subMesh->getLodLevelsCount() - 1;

						if (maxLod >= 0)
						{
							auto& lodIbuf = subMesh->getLodIndexBuffer(maxLod);

							for (int i = 0; i < lodVbufMtx[sm].size(); ++i)
								lodVbufMtx[sm][i] = false;

							for (int i = 0; i < lodIbuf.size(); ++i)
							{
								if (!lodVbufMtx[sm][lodIbuf[i]])
								{
									glm::vec4 pos = mtx * glm::vec4(vbuf[lodIbuf[i]].position, 1.0);
									lodVbuf[sm][lodIbuf[i]] = pos;
									lodVbufMtx[sm][lodIbuf[i]] = true;
								}
							}

							moc->RenderTriangles(&lodVbuf[sm][0].x, lodIbuf.data(), lodIbuf.size() / 3);
						}

						if (maxLod < 0)
						{
							for (int i = 0; i < vbuf.size(); ++i)
							{
								glm::vec4 pos = mtx * glm::vec4(vbuf[i].position, 1.0);
								lodVbuf[sm][i] = pos;
							}

							moc->RenderTriangles(&lodVbuf[sm][0].x, ibuf.data(), ibuf.size() / 3);
						}
					}
				}
			}
		}

		//cmoc->Flush();
	}

	void Renderer::updateEnvironmentMap()
	{
		deleteEnvironmentMap();

		const bgfx::Caps* caps = bgfx::getCaps();
		if ((BGFX_CAPS_TEXTURE_BLIT | BGFX_CAPS_TEXTURE_READ_BACK) == (caps->supported & (BGFX_CAPS_TEXTURE_BLIT | BGFX_CAPS_TEXTURE_READ_BACK)))
		{
			int envSize = 512;

			uint32_t tflags = BGFX_SAMPLER_MIN_ANISOTROPIC
				| BGFX_SAMPLER_MAG_ANISOTROPIC
				| BGFX_SAMPLER_U_CLAMP
				| BGFX_SAMPLER_V_CLAMP;

			bgfx::TextureHandle tex0 = bgfx::createTexture2D(envSize, envSize, true, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tflags);
			bgfx::TextureHandle tex1 = bgfx::createTexture2D(envSize, envSize, true, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tflags);
			bgfx::TextureHandle tex2 = bgfx::createTexture2D(envSize, envSize, true, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tflags);
			bgfx::TextureHandle tex3 = bgfx::createTexture2D(envSize, envSize, true, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tflags);
			bgfx::TextureHandle tex4 = bgfx::createTexture2D(envSize, envSize, true, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tflags);
			bgfx::TextureHandle tex5 = bgfx::createTexture2D(envSize, envSize, true, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tflags);

			bgfx::TextureHandle m_rb0 = bgfx::createTexture2D(envSize, envSize, true, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | tflags);
			bgfx::TextureHandle m_rb1 = bgfx::createTexture2D(envSize, envSize, true, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | tflags);
			bgfx::TextureHandle m_rb2 = bgfx::createTexture2D(envSize, envSize, true, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | tflags);
			bgfx::TextureHandle m_rb3 = bgfx::createTexture2D(envSize, envSize, true, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | tflags);
			bgfx::TextureHandle m_rb4 = bgfx::createTexture2D(envSize, envSize, true, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | tflags);
			bgfx::TextureHandle m_rb5 = bgfx::createTexture2D(envSize, envSize, true, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | tflags);

			bgfx::FrameBufferHandle envFb0 = bgfx::createFrameBuffer(1, &tex0, true);
			bgfx::FrameBufferHandle envFb1 = bgfx::createFrameBuffer(1, &tex1, true);
			bgfx::FrameBufferHandle envFb2 = bgfx::createFrameBuffer(1, &tex2, true);
			bgfx::FrameBufferHandle envFb3 = bgfx::createFrameBuffer(1, &tex3, true);
			bgfx::FrameBufferHandle envFb4 = bgfx::createFrameBuffer(1, &tex4, true);
			bgfx::FrameBufferHandle envFb5 = bgfx::createFrameBuffer(1, &tex5, true);

			int v1 = getNumViewsUsed() + 1;

			bgfx::setViewClear(v1, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, Color::packRGBA(ambientColor), 1.0f, 0);
			bgfx::setViewRect(v1, 0, 0, envSize, envSize);

			float aspect = width / height;
			glm::mat4 proj = glm::perspectiveRH(90.0f * Mathf::fDeg2Rad, aspect, 0.1f, 1000.0f);
			proj = glm::scale(proj, glm::vec3(-1, 1, 1));

			glm::mat4x4 view = glm::identity<glm::mat4x4>();

			if (skyMaterial != nullptr)
			{
				glm::mat4x4 skyMtx = glm::identity<glm::mat4x4>();
				skyMtx = glm::scale(skyMtx, glm::vec3(100.0f, 100.0f, 100.0f));

				//+X
				bgfx::setViewFrameBuffer(v1, envFb0);
				view = glm::identity<glm::mat4x4>();
				view *= glm::mat4_cast(glm::angleAxis(-90.0f * Mathf::fDeg2Rad, glm::vec3(0, 1, 0)));
				bgfx::setViewTransform(v1, glm::value_ptr(view), glm::value_ptr(proj));
				renderSkybox(v1, nullptr, skyMtx);
				frame();

				//-X
				bgfx::setViewFrameBuffer(v1, envFb1);
				view = glm::identity<glm::mat4x4>();
				view *= glm::mat4_cast(glm::angleAxis(90.0f * Mathf::fDeg2Rad, glm::vec3(0, 1, 0)));
				bgfx::setViewTransform(v1, glm::value_ptr(view), glm::value_ptr(proj));
				renderSkybox(v1, nullptr, skyMtx);
				frame();

				//+Y
				bgfx::setViewFrameBuffer(v1, envFb2);
				view = glm::identity<glm::mat4x4>();
				view *= glm::mat4_cast(glm::angleAxis(90.0f * Mathf::fDeg2Rad, glm::vec3(1, 0, 0)));
				bgfx::setViewTransform(v1, glm::value_ptr(view), glm::value_ptr(proj));
				renderSkybox(v1, nullptr, skyMtx);
				frame();

				//-Y
				bgfx::setViewFrameBuffer(v1, envFb3);
				view = glm::identity<glm::mat4x4>();
				view *= glm::mat4_cast(glm::angleAxis(-90.0f * Mathf::fDeg2Rad, glm::vec3(1, 0, 0)));
				bgfx::setViewTransform(v1, glm::value_ptr(view), glm::value_ptr(proj));
				renderSkybox(v1, nullptr, skyMtx);
				frame();

				//+Z
				bgfx::setViewFrameBuffer(v1, envFb4);
				view = glm::identity<glm::mat4x4>();
				bgfx::setViewTransform(v1, glm::value_ptr(view), glm::value_ptr(proj));
				renderSkybox(v1, nullptr, skyMtx);
				frame();

				//-Z
				bgfx::setViewFrameBuffer(v1, envFb5);
				view = glm::identity<glm::mat4x4>();
				view *= glm::mat4_cast(glm::angleAxis(180.0f * Mathf::fDeg2Rad, glm::vec3(0, 1, 0)));
				bgfx::setViewTransform(v1, glm::value_ptr(view), glm::value_ptr(proj));
				renderSkybox(v1, nullptr, skyMtx);
				frame();
			}
			else
			{
				bgfx::setViewFrameBuffer(v1, envFb0); bgfx::touch(v1); frame();
				bgfx::setViewFrameBuffer(v1, envFb1); bgfx::touch(v1); frame();
				bgfx::setViewFrameBuffer(v1, envFb2); bgfx::touch(v1); frame();
				bgfx::setViewFrameBuffer(v1, envFb3); bgfx::touch(v1); frame();
				bgfx::setViewFrameBuffer(v1, envFb4); bgfx::touch(v1); frame();
				bgfx::setViewFrameBuffer(v1, envFb5); bgfx::touch(v1); frame();
			}

			int ds = envSize * envSize * 4;

			environmentMap = Cubemap::create("system/GI/", "environmentMap");

			int v2 = v1 + 1;

			uint8_t* data0 = new uint8_t[ds];
			uint8_t* data1 = new uint8_t[ds];
			uint8_t* data2 = new uint8_t[ds];
			uint8_t* data3 = new uint8_t[ds];
			uint8_t* data4 = new uint8_t[ds];
			uint8_t* data5 = new uint8_t[ds];

			bgfx::blit(v2, m_rb0, 0, 0, tex0);
			bgfx::blit(v2, m_rb1, 0, 0, tex1);
			bgfx::blit(v2, m_rb2, 0, 0, tex2);
			bgfx::blit(v2, m_rb3, 0, 0, tex3);
			bgfx::blit(v2, m_rb4, 0, 0, tex4);
			bgfx::blit(v2, m_rb5, 0, 0, tex5);

			bgfx::readTexture(m_rb0, data0);
			bgfx::readTexture(m_rb1, data1);
			bgfx::readTexture(m_rb2, data2);
			bgfx::readTexture(m_rb3, data3);
			bgfx::readTexture(m_rb4, data4);
			bgfx::readTexture(m_rb5, data5);

			frame();
			frame();

			bgfx::destroy(m_rb0);
			bgfx::destroy(m_rb1);
			bgfx::destroy(m_rb2);
			bgfx::destroy(m_rb3);
			bgfx::destroy(m_rb4);
			bgfx::destroy(m_rb5);

			bgfx::destroy(envFb0);
			bgfx::destroy(envFb1);
			bgfx::destroy(envFb2);
			bgfx::destroy(envFb3);
			bgfx::destroy(envFb4);
			bgfx::destroy(envFb5);

			Texture* cubeTex0 = Texture::create("system/GI/", "env_texture0", envSize, envSize, 1, GX::Texture::TextureType::Texture2D, bgfx::TextureFormat::BGRA8, data0, ds, true, true);
			Texture* cubeTex1 = Texture::create("system/GI/", "env_texture1", envSize, envSize, 1, GX::Texture::TextureType::Texture2D, bgfx::TextureFormat::BGRA8, data1, ds, true, true);
			Texture* cubeTex2 = Texture::create("system/GI/", "env_texture2", envSize, envSize, 1, GX::Texture::TextureType::Texture2D, bgfx::TextureFormat::BGRA8, data2, ds, true, true);
			Texture* cubeTex3 = Texture::create("system/GI/", "env_texture3", envSize, envSize, 1, GX::Texture::TextureType::Texture2D, bgfx::TextureFormat::BGRA8, data3, ds, true, true);
			Texture* cubeTex4 = Texture::create("system/GI/", "env_texture4", envSize, envSize, 1, GX::Texture::TextureType::Texture2D, bgfx::TextureFormat::BGRA8, data4, ds, true, true);
			Texture* cubeTex5 = Texture::create("system/GI/", "env_texture5", envSize, envSize, 1, GX::Texture::TextureType::Texture2D, bgfx::TextureFormat::BGRA8, data5, ds, true, true);

			environmentMap->setTexture(0, cubeTex0, false);
			environmentMap->setTexture(1, cubeTex1, false);
			environmentMap->setTexture(2, cubeTex2, false);
			environmentMap->setTexture(3, cubeTex3, false);
			environmentMap->setTexture(4, cubeTex4, false);
			environmentMap->setTexture(5, cubeTex5, false);

			environmentMap->updateCubemap();

			cubeTex0->freeData();
			cubeTex1->freeData();
			cubeTex2->freeData();
			cubeTex3->freeData();
			cubeTex4->freeData();
			cubeTex5->freeData();

			delete cubeTex0;
			delete cubeTex1;
			delete cubeTex2;
			delete cubeTex3;
			delete cubeTex4;
			delete cubeTex5;

			delete[] data0;
			delete[] data1;
			delete[] data2;
			delete[] data3;
			delete[] data4;
			delete[] data5;

			environmentMap->setTexture(0, nullptr, false);
			environmentMap->setTexture(1, nullptr, false);
			environmentMap->setTexture(2, nullptr, false);
			environmentMap->setTexture(3, nullptr, false);
			environmentMap->setTexture(4, nullptr, false);
			environmentMap->setTexture(5, nullptr, false);

			frame();
		}
	}
}