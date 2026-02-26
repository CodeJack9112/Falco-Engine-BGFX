#include "Camera.h"

#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <glm/gtc/type_ptr.hpp>

#include "Transform.h"

#include "../Core/Engine.h"
#include "../Core/GameObject.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/RenderTexture.h"
#include "../Components/Renderable.h"
#include "../Components/Light.h"
#include "../Components/MonoScript.h"
#include "../Math/Mathf.h"
#include "../Math/Plane.h"
#include "../Core/APIManager.h"
#include "../Renderer/Frustum.h"
#include "../Renderer/Primitives.h"
#include "../Core/Time.h"
#include "../Assets/Texture.h"
#include "../Assets/Shader.h"

namespace GX
{
	std::string Camera::COMPONENT_TYPE = "Camera";
	float Camera::dynAutoRes = 1.0f;
	int Camera::dynAutoResFrames = 1.0f;
	int Camera::dynAutoResFramesToUpdate = 60;

	Camera* Camera::apiCamera = nullptr;
	int Camera::apiViewId = 0;

	std::string Camera::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Camera::Camera() : Component(APIManager::getSingleton()->camera_class)
	{
		frustum = new Frustum();
	}

	Camera::~Camera()
	{
		delete frustum;
		frustum = nullptr;
	}

	void Camera::initResources()
	{
		int width = Renderer::getSingleton()->getWidth() * viewport_w;
		int height = Renderer::getSingleton()->getHeight() * viewport_h;

		backBuffer = new RenderTexture(width, height);
		sceneBuffer = new RenderTexture(width, height);

		gbDiffuseTexture = new Texture();
		gbNormalsTexture = new Texture();
		gbMRASTexture = new Texture();
		gbLightmapTexture = new Texture();
		gbDepthTexture = new Texture();
		
		resetFrameBuffers();
	}

	void Camera::destroyResources()
	{
		if (backBuffer != nullptr)
			delete backBuffer;

		if (sceneBuffer != nullptr)
			delete sceneBuffer;

		if (gbDiffuseTexture != nullptr)
			delete gbDiffuseTexture;

		if (gbNormalsTexture != nullptr)
			delete gbNormalsTexture;

		if (gbMRASTexture != nullptr)
			delete gbMRASTexture;

		if (gbLightmapTexture != nullptr)
			delete gbLightmapTexture;

		if (gbDepthTexture != nullptr)
			delete gbDepthTexture;

		backBuffer = nullptr;
		sceneBuffer = nullptr;
		gbDiffuseTexture = nullptr;
		gbNormalsTexture = nullptr;
		gbMRASTexture = nullptr;
		gbLightmapTexture = nullptr;
		gbDepthTexture = nullptr;
		
		destroyFrameBuffers();
	}

	void Camera::onAttach()
	{
		Component::onAttach();

		transform = getGameObject()->getTransform();

		std::vector<Camera*>& cameras = Renderer::getSingleton()->getCameras();

		cameras.push_back(this);

		if (Engine::getSingleton()->getIsRuntimeMode() || cameras.size() == 1)
			initResources();
	}

	void Camera::onDetach()
	{
		Component::onDetach();

		std::vector<Camera*>& cameras = Renderer::getSingleton()->getCameras();
		auto it = std::find(cameras.begin(), cameras.end(), this);

		if (it != cameras.end())
		{
			cameras.erase(it);
			destroyResources();
		}
	}

	Component* Camera::onClone()
	{
		Camera* newComponent = new Camera();
		newComponent->fovy = fovy;
		newComponent->znear = znear;
		newComponent->zfar = zfar;
		newComponent->viewport_l = viewport_l;
		newComponent->viewport_t = viewport_t;
		newComponent->viewport_w = viewport_w;
		newComponent->viewport_h = viewport_h;
		newComponent->screenOffset_left = screenOffset_left;
		newComponent->screenOffset_top = screenOffset_top;
		newComponent->depth = depth;
		newComponent->clearColor = clearColor;
		newComponent->renderTarget = renderTarget;
		newComponent->projectionType = projectionType;
		newComponent->orthographicSize = orthographicSize;
		newComponent->cullingMask = cullingMask;
		newComponent->clearFlags = clearFlags;

		return newComponent;
	}

	void Camera::setFOVy(float value)
	{
		fovy = value;
		if (fovy < 1.0f) fovy = 1.0f;
		if (fovy > 179.0f) fovy = 179.0f;
	}

	void Camera::setNear(float value)
	{
		znear = value;

		if (znear < 0.0f)
			znear = 0.0f;

		prevTransform = glm::identity<glm::mat4x4>();
	}

	void Camera::setFar(float value)
	{
		zfar = value;

		if (zfar > 10000.0f)
			zfar = 10000.0f;

		if (zfar < 1.0f)
			zfar = 1.0f;

		prevTransform = glm::identity<glm::mat4x4>();
	}

	Transform* Camera::getTransform()
	{
		return transform;
	}

	glm::mat4x4 Camera::getViewMatrix()
	{
		if (!injectView)
		{
			glm::vec3 eye = transform->getPosition();
			glm::vec3 at = eye + transform->getForward();

			glm::mat4 view = glm::identity<glm::mat4x4>();
			glm::vec3 up = glm::cross(transform->getRight(), transform->getForward());

			view = glm::lookAtRH(eye, at, -up);

			return view;
		}
		else
			return injectedViewMatrix;
	}

	glm::mat4x4 Camera::getProjectionMatrix(ProjectionType type)
	{
		if (type == ProjectionType::Perspective)
		{
			float aspect = getAspectRatio();
			glm::mat4 proj = glm::identity<glm::mat4x4>();

			proj = glm::perspectiveRH(fovy * Mathf::fDeg2Rad, aspect, znear, zfar);

			if (flipH || flipV)
				proj = glm::scale(proj, glm::vec3(flipH ? -1 : 1, flipV ? -1 : 1, 1));

			return proj;
		}
		else
		{
			glm::mat4 proj = glm::identity<glm::mat4x4>();
			float aspect = getAspectRatio();

			proj = glm::orthoRH(-orthographicSize * aspect, orthographicSize * aspect, -orthographicSize, orthographicSize, znear, zfar);

			if (flipH || flipV)
				proj = glm::scale(proj, glm::vec3(flipH ? -1 : 1, flipV ? -1 : 1, 1));

			return proj;
		}
	}

	glm::mat4x4 Camera::getProjectionMatrix()
	{
		if (!injectProj)
		{
			return getProjectionMatrix(projectionType);
		}
		else
			return injectedProjectionMatrix;
	}

	glm::mat4x4 Camera::makeViewMatrix(glm::vec3 position, glm::vec3 lookAt, glm::vec3 up)
	{
		glm::vec3 eye = position;
		glm::vec3 at = lookAt;

		glm::mat4 view = glm::identity<glm::mat4x4>();

		view = glm::lookAtRH(eye, lookAt, up);

		return view;
	}

	glm::mat4x4 Camera::makeProjectionMatrix(float fovy, float aspect, float znear, float zfar)
	{
		glm::mat4 proj = glm::identity<glm::mat4x4>();
		proj = glm::perspectiveRH(fovy * Mathf::fDeg2Rad, aspect, znear, zfar);

		return proj;
	}

	void Camera::injectViewMatrix(glm::mat4x4 mtx)
	{
		injectView = true;
		injectedViewMatrix = mtx;
	}

	void Camera::injectProjectionMatrix(glm::mat4x4 mtx)
	{
		injectProj = true;
		injectedProjectionMatrix = mtx;
	}

	float Camera::getAspectRatio()
	{
		float w = (float)Renderer::getSingleton()->getWidth();
		float h = (float)Renderer::getSingleton()->getHeight();

		if (renderTarget != nullptr)
		{
			w = (float)renderTarget->getWidth();
			h = (float)renderTarget->getHeight();
		}
		else if (backBuffer != nullptr)
		{
			w = (float)backBuffer->getWidth();
			h = (float)backBuffer->getHeight();
		}

		return (w * viewport_w) / (h * viewport_h);
	}

	void Camera::setRenderTarget(RenderTexture* rt, int layer)
	{
		renderTarget = rt;
	}

	Ray Camera::getCameraToViewportRay(float screenX, float screenY)
	{
		float width1 = Renderer::getSingleton()->getWidth();
		float height1 = Renderer::getSingleton()->getHeight();

		if (renderTarget != nullptr)
		{
			width1 = renderTarget->getWidth();
			height1 = renderTarget->getHeight();
		}
		else if (backBuffer != nullptr)
		{
			width1 = backBuffer->getWidth();
			height1 = backBuffer->getHeight();
		}

		float width = width1 * viewport_w;
		float height = height1 * viewport_h;

		float mx = (screenX - width1 * screenOffset_left);
		float my = (screenY - height1 * screenOffset_top);

		glm::mat4x4 mViewProjInverse;
		mViewProjInverse = glm::inverse(getProjectionMatrix() * getViewMatrix());

		float mox = ((mx - 0) / width) * 2.f - 1.f;
		//float moy = ((my - 0) / height) * 2.f - 1.f;
		float moy = (1.f - ((my - 0) / height)) * 2.f - 1.f;

		glm::vec4 rayOrigin = mViewProjInverse * glm::vec4(mox, moy, 0.f, 1.f);
		rayOrigin *= 1.f / rayOrigin.w;
		glm::vec4 rayEnd = mViewProjInverse * glm::vec4(mox, moy, 1.f - FLT_EPSILON, 1.f);
		rayEnd *= 1.f / rayEnd.w;
		glm::vec4 rayDir = glm::normalize(rayEnd - rayOrigin);

		return Ray(rayOrigin, rayDir);
	}

	void Camera::setProjectionType(ProjectionType value)
	{
		projectionType = value;
		prevTransform = glm::identity<glm::mat4x4>();
	}

	void Camera::setOrthographicSize(float value)
	{
		orthographicSize = value;
		prevTransform = glm::identity<glm::mat4x4>();
	}

	float Camera::getDynamicResolution()
	{
		ProjectSettings* settings = Engine::getSingleton()->getSettings();
		if (settings->getUseDynamicResolution())
		{
			if (settings->getAutoDynamicResolution())
				return dynAutoRes;
			else
				return settings->getDynamicResolution();
		}
		else
			return 1.0f;
	}

	void Camera::updateDynamicResolution()
	{
		int fps = Time::getFramesPerSecond();
		ProjectSettings* settings = Engine::getSingleton()->getSettings();
		if (!settings->getUseDynamicResolution())
		{
			dynAutoRes = 1.0f;
			return;
		}

		if (!settings->getAutoDynamicResolution())
		{
			dynAutoRes = 1.0f;
			return;
		}

		if (Time::getTimeSinceLevelStart() > 1.0f)
		{
			int targetFPS = settings->getAutoDynamicResolutionTargetFPS();

			if (fps < targetFPS)
			{
				if (dynAutoRes >= 0.5f)
				{
					if (dynAutoResFrames < dynAutoResFramesToUpdate)
					{
						++dynAutoResFrames;
					}
					else
					{
						dynAutoResFrames = 0;
						dynAutoRes -= 0.05f;
						if (dynAutoRes < 0.5f)
							dynAutoRes = 0.5f;

						Renderer::getSingleton()->resetFrameBuffers();
					}
				}
			}

			if (fps > targetFPS + 30)
			{
				if (dynAutoRes <= 0.95)
				{
					if (dynAutoResFrames < dynAutoResFramesToUpdate)
					{
						++dynAutoResFrames;
					}
					else
					{
						dynAutoResFrames = 0;
						dynAutoRes += 0.05f;
						if (dynAutoRes > 1.0f)
							dynAutoRes = 1.0f;

						Renderer::getSingleton()->resetFrameBuffers();
					}
				}
			}
		}
	}

	void Camera::destroyFrameBuffers()
	{
		if (bgfx::isValid(outlineBuffer))
			bgfx::destroy(outlineBuffer);

		if (bgfx::isValid(gbuffer))
			bgfx::destroy(gbuffer);

		if (bgfx::isValid(decalBuffer))
			bgfx::destroy(decalBuffer);

		if (bgfx::isValid(lightBuffer))
			bgfx::destroy(lightBuffer);

		gbufferTex[0].idx = bgfx::kInvalidHandle;
		gbufferTex[1].idx = bgfx::kInvalidHandle;
		gbufferTex[2].idx = bgfx::kInvalidHandle;
		gbufferTex[3].idx = bgfx::kInvalidHandle;
		gbufferTex[4].idx = bgfx::kInvalidHandle;

		outlineBufferTex.idx = bgfx::kInvalidHandle;
		outlineDepthTex.idx = bgfx::kInvalidHandle;
		lightBufferTex.idx = bgfx::kInvalidHandle;

		outlineBuffer.idx = bgfx::kInvalidHandle;
		gbuffer.idx = bgfx::kInvalidHandle;
		decalBuffer.idx = bgfx::kInvalidHandle;
		lightBuffer.idx = bgfx::kInvalidHandle;
	}

	void Camera::resetFrameBuffers()
	{
		destroyFrameBuffers();

		float dynRes = getDynamicResolution();

		int _w = Renderer::getSingleton()->getWidth();
		int _h = Renderer::getSingleton()->getHeight();

		if (renderTarget != nullptr)
		{
			_w = renderTarget->getWidth();
			_h = renderTarget->getHeight();
		}

		int width = _w * dynRes;
		int height = _h * dynRes;

		if (width < 8) width = 8;
		if (height < 8) height = 8;

		if (editorCamera)
		{
			outlineBufferTex = bgfx::createTexture2D(uint16_t(width), uint16_t(height), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT
				| BGFX_SAMPLER_MIN_ANISOTROPIC
				| BGFX_SAMPLER_MAG_ANISOTROPIC
				//| BGFX_SAMPLER_MIP_POINT
				| BGFX_SAMPLER_U_CLAMP
				| BGFX_SAMPLER_V_CLAMP);
			outlineDepthTex = bgfx::createTexture2D(uint16_t(width), uint16_t(height), false, 1, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT
				| BGFX_SAMPLER_MIN_POINT
				| BGFX_SAMPLER_MAG_POINT
				| BGFX_SAMPLER_MIP_POINT
				| BGFX_SAMPLER_U_CLAMP
				| BGFX_SAMPLER_V_CLAMP);

			bgfx::Attachment outlineAt[2];
			outlineAt[0].init(outlineBufferTex);
			outlineAt[1].init(outlineDepthTex);

			outlineBuffer = bgfx::createFrameBuffer(BX_COUNTOF(outlineAt), outlineAt, true);
		}

		////

		backBuffer->reset(width, height);
		sceneBuffer->reset(width, height);

		//--------GBuffer--------//

		const uint64_t tsFlags = 0
			| BGFX_SAMPLER_MIN_ANISOTROPIC
			| BGFX_SAMPLER_MAG_ANISOTROPIC
			//| BGFX_SAMPLER_MIP_POINT
			| BGFX_SAMPLER_U_CLAMP
			| BGFX_SAMPLER_V_CLAMP
			;

		const uint64_t tsFlagsD = 0
			| BGFX_SAMPLER_MIN_POINT
			| BGFX_SAMPLER_MAG_POINT
			| BGFX_SAMPLER_MIP_POINT
			| BGFX_SAMPLER_U_CLAMP
			| BGFX_SAMPLER_V_CLAMP
			;

		bgfx::Attachment gbufferAt[5];

		//GBuffer
		gbufferTex[0] = bgfx::createTexture2D(uint16_t(width), uint16_t(height), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
		gbufferTex[1] = bgfx::createTexture2D(uint16_t(width), uint16_t(height), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
		gbufferTex[2] = bgfx::createTexture2D(uint16_t(width), uint16_t(height), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
		gbufferTex[3] = bgfx::createTexture2D(uint16_t(width), uint16_t(height), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
		gbufferTex[4] = bgfx::createTexture2D(uint16_t(width), uint16_t(height), false, 1, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT | tsFlagsD);

		gbufferAt[0].init(gbufferTex[0]);
		gbufferAt[1].init(gbufferTex[1]);
		gbufferAt[2].init(gbufferTex[2]);
		gbufferAt[3].init(gbufferTex[3]);
		gbufferAt[4].init(gbufferTex[4]);

		gbuffer = bgfx::createFrameBuffer(BX_COUNTOF(gbufferAt), gbufferAt, true);
		decalBuffer = bgfx::createFrameBuffer(BX_COUNTOF(gbufferAt), gbufferAt, false);

		//Light buffer
		lightBufferTex = bgfx::createTexture2D(uint16_t(width), uint16_t(height), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
		lightBuffer = bgfx::createFrameBuffer(1, &lightBufferTex, true);

		gbDiffuseTexture->textureHandle = gbufferTex[0]; gbDiffuseTexture->format = bgfx::TextureFormat::BGRA8;
		gbNormalsTexture->textureHandle = gbufferTex[1]; gbNormalsTexture->format = bgfx::TextureFormat::BGRA8;
		gbMRASTexture->textureHandle = gbufferTex[2]; gbMRASTexture->format = bgfx::TextureFormat::BGRA8;
		gbLightmapTexture->textureHandle = gbufferTex[3]; gbLightmapTexture->format = bgfx::TextureFormat::BGRA8;
		gbDepthTexture->textureHandle = gbufferTex[4]; gbDepthTexture->format = bgfx::TextureFormat::D24S8;

		gbDiffuseTexture->width = width; gbDiffuseTexture->originalWidth = width;
		gbDiffuseTexture->height = height; gbDiffuseTexture->originalHeight = height;

		gbNormalsTexture->width = width; gbNormalsTexture->originalWidth = width;
		gbNormalsTexture->height = height; gbNormalsTexture->originalHeight = height;

		gbMRASTexture->width = width; gbMRASTexture->originalWidth = width;
		gbMRASTexture->height = height; gbMRASTexture->originalHeight = height;

		gbLightmapTexture->width = width; gbLightmapTexture->originalWidth = width;
		gbLightmapTexture->height = height; gbLightmapTexture->originalHeight = height;

		gbDepthTexture->width = width; gbDepthTexture->originalWidth = width;
		gbDepthTexture->height = height; gbDepthTexture->originalHeight = height;

		//--------------------------------//
	}

	glm::vec3 Camera::worldToScreenPoint(glm::vec3 point)
	{
		float width = (float)backBuffer->getWidth();
		float height = (float)backBuffer->getHeight();

		glm::vec4 spPoint = getProjectionMatrix() * (getViewMatrix() * glm::vec4(point, 1.0f));

		bool isInFrustum =
			(spPoint.x < -1.0f) ||
			(spPoint.x > 1.0f) ||
			(spPoint.y < -1.0f) ||
			(spPoint.y > 1.0f);

		Plane cameraPlane = Plane(transform->getForward(), transform->getPosition());
		if (cameraPlane.getSide(point) == Plane::NEGATIVE_SIDE)
			isInFrustum = false;

		glm::vec3 spoint = glm::vec3(spPoint) / spPoint.w;
		
		glm::vec3 screenSpacePoint = glm::vec3(0);
		screenSpacePoint.x = ((spoint.x * 0.5f) + 0.5f) * width;
		screenSpacePoint.y = height - (((spoint.y * 0.5f) + 0.5f) * height);
		screenSpacePoint.z = isInFrustum ? 1.0f : -1.0f;

		return screenSpacePoint;
	}

	glm::vec3 Camera::screenToWorldPoint(glm::vec3 point)
	{
		float width = (float)backBuffer->getWidth();
		float height = (float)backBuffer->getHeight();

		float scrx = point.x / width;
		float scry = point.y / height;
		Ray ray = getCameraToViewportRay((float)(scrx), (float)(scry));
		glm::vec3 vect = ray.origin + ray.direction * point.z;

		return vect;
	}

	Frustum* Camera::getFrustum()
	{
		glm::mat4x4 mtx = transform->getTransformMatrix();
		if (mtx != prevTransform)
		{
			glm::mat4x4 view = getViewMatrix();
			glm::mat4x4 proj = getProjectionMatrix();
			frustum->calculateFrustum(view, proj);
			prevTransform = mtx;
		}

		return frustum;
	}

	bool Camera::isVisible(AxisAlignedBox aab, float bias)
	{
		if (aab.isNull())
			return false;

		Frustum* cullFrustum = getFrustum();
		if (!aab.isInfinite())
		{
			if (!cullFrustum->sphereInFrustum(aab.getCenter(), aab.getRadius() * bias))
				return false;
		}

		return true;
	}

	void Camera::renderFrame()
	{
		if (!getEnabled())
			return;

		if (gameObject == nullptr || !gameObject->getActive())
			return;

		Renderer* renderer = Renderer::getSingleton();

		int numLightsWithShadows = Renderer::getSingleton()->getNumActiveLightsWithShadows();
		int viewLayer = numLightsWithShadows * NUM_LIGHT_VIEWS;

		bgfx::touch(RENDER_SKYBOX_PASS_ID + viewLayer);
		bgfx::touch(RENDER_SCENE_PASS_ID + viewLayer);

		if (getIsEditorCamera())
		{
			bgfx::touch(RENDER_OVERLAY_PASS_ID + viewLayer);
			bgfx::touch(RENDER_OUTLINE_PASS_ID + viewLayer);
		}

		bgfx::touch(RENDER_FINAL_PASS_ID + viewLayer);

		//*
		bgfx::touch(RENDER_GEOMETRY_PASS_ID + viewLayer);
		bgfx::touch(RENDER_DECAL_PASS_ID + viewLayer);
		bgfx::touch(RENDER_LIGHT_PASS_ID + viewLayer);
		bgfx::touch(RENDER_FORWARD_PASS_ID + viewLayer);
		//*

		bgfx::setViewMode(RENDER_FORWARD_PASS_ID + viewLayer, bgfx::ViewMode::Sequential);

		float left_scale = getViewportLeft();
		float top_scale = getViewportTop();
		float width_scale = getViewportWidth();
		float height_scale = getViewportHeight();

		float _width = Renderer::getSingleton()->getWidth();
		float _height = Renderer::getSingleton()->getHeight();

		if (getRenderTarget() != nullptr)
		{
			_width = getRenderTarget()->getWidth();
			_height = getRenderTarget()->getHeight();
		}

		//-->Render 3D begin
		float viewLeft = _width * left_scale;
		float viewRight = _width * width_scale;
		float viewTop = _height * top_scale;
		float viewBottom = _height * height_scale;

		glm::mat4x4 view = getViewMatrix();
		glm::mat4x4 invView = glm::inverse(view);
		glm::mat4x4 proj = getProjectionMatrix();

		glm::mat4x4 projPerspective = getProjectionMatrix(ProjectionType::Perspective);

		Color camColor = getClearColor();
		//camColor[3] = 0.0f;

		if (getIsEditorCamera())
		{
			bgfx::setViewClear(RENDER_SKYBOX_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, Color::packRGBA(camColor), 1.0f, 0);
			bgfx::setViewRect(RENDER_SKYBOX_PASS_ID + viewLayer, 0, 0, _width, _height);
			bgfx::setViewFrameBuffer(RENDER_SKYBOX_PASS_ID + viewLayer, sceneBuffer->getFrameBufferHandle());
			bgfx::setViewTransform(RENDER_SKYBOX_PASS_ID + viewLayer, glm::value_ptr(view), glm::value_ptr(projPerspective));

			bgfx::setViewClear(RENDER_SCENE_PASS_ID + viewLayer, 0 | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);
			bgfx::setViewRect(RENDER_SCENE_PASS_ID + viewLayer, 0, 0, _width, _height);
			bgfx::setViewFrameBuffer(RENDER_SCENE_PASS_ID + viewLayer, sceneBuffer->getFrameBufferHandle());
			bgfx::setViewTransform(RENDER_SCENE_PASS_ID + viewLayer, glm::value_ptr(view), glm::value_ptr(proj));

			bgfx::setViewClear(RENDER_FORWARD_PASS_ID + viewLayer, 0, 0x00000000, 1.0f, 0);
			bgfx::setViewRect(RENDER_FORWARD_PASS_ID + viewLayer, 0, 0, _width, _height);
			bgfx::setViewFrameBuffer(RENDER_FORWARD_PASS_ID + viewLayer, sceneBuffer->getFrameBufferHandle());
			bgfx::setViewTransform(RENDER_FORWARD_PASS_ID + viewLayer, glm::value_ptr(view), glm::value_ptr(proj));

			bgfx::setViewClear(RENDER_OVERLAY_PASS_ID + viewLayer, 0, 0x00000000, 1.0f, 0);
			bgfx::setViewRect(RENDER_OVERLAY_PASS_ID + viewLayer, 0, 0, _width, _height);
			bgfx::setViewFrameBuffer(RENDER_OVERLAY_PASS_ID + viewLayer, sceneBuffer->getFrameBufferHandle());
			bgfx::setViewTransform(RENDER_OVERLAY_PASS_ID + viewLayer, glm::value_ptr(view), glm::value_ptr(proj));

			bgfx::setViewClear(RENDER_OUTLINE_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0);
			bgfx::setViewRect(RENDER_OUTLINE_PASS_ID + viewLayer, 0, 0, _width, _height);
			bgfx::setViewTransform(RENDER_OUTLINE_PASS_ID + viewLayer, glm::value_ptr(view), glm::value_ptr(proj));
			bgfx::setViewFrameBuffer(RENDER_OUTLINE_PASS_ID + viewLayer, outlineBuffer);

			glm::mat4x4 finalProj = glm::orthoRH(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f);
			bgfx::setViewClear(RENDER_FINAL_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, Color::packRGBA(camColor), 1.0f, 0);
			bgfx::setViewRect(RENDER_FINAL_PASS_ID + viewLayer, viewLeft, viewTop, viewRight, viewBottom);
			bgfx::setViewTransform(RENDER_FINAL_PASS_ID + viewLayer, NULL, glm::value_ptr(finalProj));
			if (getRenderTarget() != nullptr)
				bgfx::setViewFrameBuffer(RENDER_FINAL_PASS_ID + viewLayer, getRenderTarget()->getFrameBufferHandle());
			else if (renderer->backBuffer != nullptr)
				bgfx::setViewFrameBuffer(RENDER_FINAL_PASS_ID + viewLayer, renderer->backBuffer->getFrameBufferHandle());
		}
		else
		{
			uint64_t cullState = 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
			uint32_t clearColor = Color::packRGBA(camColor);
			if (getClearFlags() == Camera::ClearFlags::DepthOnly)
				clearColor = 0x00000000;
			if (getClearFlags() == Camera::ClearFlags::SolidColor)
				cullState = 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL;
			if (getClearFlags() == Camera::ClearFlags::DontClear)
				cullState = 0;

			bgfx::setViewClear(RENDER_SKYBOX_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, clearColor, 1.0f, 0);
			bgfx::setViewRect(RENDER_SKYBOX_PASS_ID + viewLayer, 0, 0, _width, _height);
			bgfx::setViewTransform(RENDER_SKYBOX_PASS_ID + viewLayer, glm::value_ptr(view), glm::value_ptr(projPerspective));
			bgfx::setViewFrameBuffer(RENDER_SKYBOX_PASS_ID + viewLayer, sceneBuffer->getFrameBufferHandle());

			cullState = 0 | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
			if (getClearFlags() == Camera::ClearFlags::SolidColor)
				cullState = 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL;
			if (getClearFlags() == Camera::ClearFlags::DontClear)
				cullState = 0;

			bgfx::setViewClear(RENDER_SCENE_PASS_ID + viewLayer, cullState, 0x00000000, 1.0f, 0);
			bgfx::setViewRect(RENDER_SCENE_PASS_ID + viewLayer, 0, 0, _width, _height);
			bgfx::setViewTransform(RENDER_SCENE_PASS_ID + viewLayer, glm::value_ptr(view), glm::value_ptr(proj));
			bgfx::setViewFrameBuffer(RENDER_SCENE_PASS_ID + viewLayer, sceneBuffer->getFrameBufferHandle());

			bgfx::setViewClear(RENDER_FORWARD_PASS_ID + viewLayer, 0, 0x00000000, 1.0f, 0);
			bgfx::setViewRect(RENDER_FORWARD_PASS_ID + viewLayer, 0, 0, _width, _height);
			bgfx::setViewTransform(RENDER_FORWARD_PASS_ID + viewLayer, glm::value_ptr(view), glm::value_ptr(proj));
			bgfx::setViewFrameBuffer(RENDER_FORWARD_PASS_ID + viewLayer, sceneBuffer->getFrameBufferHandle());

			glm::mat4x4 finalProj = glm::orthoRH(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f);
			bgfx::setViewClear(RENDER_FINAL_PASS_ID + viewLayer, 0, clearColor, 1.0f, 0);
			bgfx::setViewRect(RENDER_FINAL_PASS_ID + viewLayer, viewLeft, viewTop, viewRight, viewBottom);
			bgfx::setViewTransform(RENDER_FINAL_PASS_ID + viewLayer, NULL, glm::value_ptr(finalProj));
			if (getRenderTarget() != nullptr)
				bgfx::setViewFrameBuffer(RENDER_FINAL_PASS_ID + viewLayer, getRenderTarget()->getFrameBufferHandle());
			else if (renderer->backBuffer != nullptr)
				bgfx::setViewFrameBuffer(RENDER_FINAL_PASS_ID + viewLayer, renderer->backBuffer->getFrameBufferHandle());
		}

		//----------------Update occlusion data-------------//

		if (getOcclusionCulling())
		{
			if (renderer->projectSettings != nullptr)
				renderer->calculateVisibility(view, proj, getFrustum(), getFar());
		}

		//----------------Render directional shadows-------------//

		renderer->renderDirectionalLightShadows(this, invView);

		//-------------------------------------------------------//

		//---Render G-Buffer
		if (bgfx::getCaps()->limits.maxFBAttachments > 1 && bgfx::isValid(renderer->combinePH))
		{
			bgfx::setViewClear(RENDER_GEOMETRY_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);
			bgfx::setViewClear(RENDER_LIGHT_PASS_ID + viewLayer, 0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);

			bgfx::setViewRect(RENDER_GEOMETRY_PASS_ID + viewLayer, 0, 0, _width, _height);
			bgfx::setViewRect(RENDER_LIGHT_PASS_ID + viewLayer, 0, 0, _width, _height);

			if (gbuffer.idx != bgfx::kInvalidHandle)
				bgfx::setViewFrameBuffer(RENDER_GEOMETRY_PASS_ID + viewLayer, gbuffer);
			if (lightBuffer.idx != bgfx::kInvalidHandle)
				bgfx::setViewFrameBuffer(RENDER_LIGHT_PASS_ID + viewLayer, lightBuffer);

			bgfx::setViewTransform(RENDER_GEOMETRY_PASS_ID + viewLayer, glm::value_ptr(view), glm::value_ptr(proj));

			glm::mat4x4 invViewProj = glm::inverse(proj * view);

			auto& renderables = renderer->getRenderables();

			for (auto it = renderables.begin(); it != renderables.end(); ++it)
			{
				Renderable* comp = *it;

				if (comp->getSkipRendering() || comp->isDecal())
					continue;

				if (!comp->checkCullingMask(getCullingMask()))
					continue;

				if (!comp->isAlwaysVisible())
				{
					if (!isVisible(comp->getBounds()))
						continue;
				}

				comp->onRender(this, RENDER_GEOMETRY_PASS_ID + viewLayer, renderer->getRenderState(this, renderer->defaultRenderState), { bgfx::kInvalidHandle }, static_cast<int>(RenderMode::Deferred), [=]() {
					renderer->setSystemUniforms(this);
					});
			}

			if (decalBuffer.idx != bgfx::kInvalidHandle)
			{
				bgfx::setViewClear(RENDER_DECAL_PASS_ID + viewLayer, BGFX_CLEAR_NONE, 0x00000000, 1.0f, 0);
				bgfx::setViewRect(RENDER_DECAL_PASS_ID + viewLayer, 0, 0, _width, _height);
				bgfx::setViewFrameBuffer(RENDER_DECAL_PASS_ID + viewLayer, decalBuffer);
				bgfx::setViewTransform(RENDER_DECAL_PASS_ID + viewLayer, glm::value_ptr(view), glm::value_ptr(proj));
			}

			//Decals
			for (auto it = renderables.begin(); it != renderables.end(); ++it)
			{
				Renderable* comp = *it;

				if (comp->getSkipRendering() || !comp->isDecal())
					continue;

				if (!comp->checkCullingMask(getCullingMask()))
					continue;

				if (!comp->isAlwaysVisible())
				{
					if (!isVisible(comp->getBounds()))
						continue;
				}

				comp->onRender(this, RENDER_DECAL_PASS_ID + viewLayer, 0, { bgfx::kInvalidHandle }, static_cast<int>(RenderMode::Deferred), [=]() {
					renderer->setSystemUniforms(this);

					bgfx::setTexture(0, renderer->u_albedoMap, gbufferTex[0]);
					bgfx::setTexture(1, renderer->u_normalMap, gbufferTex[1]);
					bgfx::setTexture(2, renderer->u_mraMap, gbufferTex[2]);
					bgfx::setTexture(3, renderer->uLightmap, gbufferTex[3]);
					bgfx::setTexture(4, renderer->u_depthMap, gbufferTex[4]);
				});
			}

			glm::mat4x4 geomProj = glm::orthoRH(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f);
			bgfx::setViewTransform(RENDER_LIGHT_PASS_ID + viewLayer, NULL, glm::value_ptr(geomProj));
			bgfx::setViewTransform(RENDER_SCENE_PASS_ID + viewLayer, NULL, glm::value_ptr(geomProj));

			auto& lights = renderer->getLights();

			for (auto& light : lights)
			{
				//Render light
				light->onRender(this, RENDER_LIGHT_PASS_ID + viewLayer, renderer->lightRenderState, renderer->lightPH, [=]() {
					renderer->setSystemUniforms(this);

					bgfx::setTexture(0, renderer->u_albedoMap, gbufferTex[0]);
					bgfx::setTexture(1, renderer->u_normalMap, gbufferTex[1]);
					bgfx::setTexture(2, renderer->u_mraMap, gbufferTex[2]);
					bgfx::setTexture(3, renderer->uLightmap, gbufferTex[3]);
					bgfx::setTexture(4, renderer->u_depthMap, gbufferTex[4]);

					bgfx::setUniform(renderer->u_invVP, glm::value_ptr(invViewProj), 1);
				});
			}

			renderer->setSystemUniforms(this);
			bgfx::setTexture(0, renderer->u_albedoMap, gbufferTex[0]);
			bgfx::setTexture(1, renderer->u_depthMap, gbufferTex[4]);
			bgfx::setTexture(2, renderer->u_lightColor, lightBufferTex);
			bgfx::setTexture(3, renderer->uLightmap, gbufferTex[3]);
			bgfx::setTexture(4, renderer->u_normalMap, gbufferTex[1]);
			bgfx::setTexture(5, renderer->u_mraMap, gbufferTex[2]);
			bgfx::setUniform(renderer->u_invVP, glm::value_ptr(invViewProj), 1);
			bgfx::setState(renderer->combineRenderState);
			Primitives::screenSpaceQuad();
			bgfx::submit(RENDER_SCENE_PASS_ID + viewLayer, renderer->combinePH);
		}
		//---

		glm::mat4x4 skyMtx = glm::identity<glm::mat4x4>();
		skyMtx = glm::translate(skyMtx, getTransform()->getPosition());
		skyMtx = glm::scale(skyMtx, glm::vec3(100.0f, 100.0f, 100.0f));

		//-------------Render scene 0-------------//
		renderer->renderObjects(this, viewLayer, 0, view, proj, skyMtx);
		//----------------------------------------//

		//---Camera back buffer
		if (bgfx::isValid(renderer->cameraBackBufferPH))
		{
			glm::mat4x4 finalProj = glm::orthoRH(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f);
			int vid = RENDER_BACKBUFFER_PASS_ID + viewLayer;
			bgfx::setViewClear(vid, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x000000ff, 1.0f, 0);
			bgfx::setViewRect(vid, 0, 0, _width, _height);
			bgfx::setViewTransform(vid, NULL, glm::value_ptr(finalProj));
			bgfx::setViewFrameBuffer(vid, getBackBuffer()->getFrameBufferHandle());

			bgfx::setTexture(0, renderer->u_albedoMap, sceneBuffer->getColorTextureHandle());
			bgfx::setTexture(1, renderer->u_depthMap, sceneBuffer->getDepthTextureHandle());
			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_ALWAYS);
			Primitives::screenSpaceQuad();
			bgfx::submit(vid, renderer->cameraBackBufferPH);
		}
		//---

		//-------------Render scene 1-------------//
		renderer->renderObjects(this, viewLayer, 1, view, proj, skyMtx);
		//----------------------------------------//

		//-------------Render scene 2-------------//
		renderer->renderObjects(this, viewLayer, 2, view, proj, skyMtx);
		//----------------------------------------//

		renderer->renderRenderCallbacks(this, viewLayer);

		if (getIsEditorCamera())
		{
			if (renderer->outlineFinalPH.idx != bgfx::kInvalidHandle)
			{
				renderer->setSystemUniforms(this);
				bgfx::setTexture(0, renderer->u_albedoMap, sceneBuffer->getColorTextureHandle());
				bgfx::setTexture(1, renderer->u_depthMap, sceneBuffer->getDepthTextureHandle());
				bgfx::setTexture(2, renderer->u_forwardColor, outlineBufferTex);
				bgfx::setTexture(3, renderer->u_forwardDepth, outlineDepthTex);
				bgfx::setState(renderer->combineRenderState | BGFX_STATE_DEPTH_TEST_ALWAYS);
				Primitives::screenSpaceQuad();
				bgfx::submit(RENDER_FINAL_PASS_ID + viewLayer, renderer->outlineFinalPH);
			}
		}
		else
		{
			if (renderer->simpleTexturePH.idx != bgfx::kInvalidHandle)
			{
				if (sceneBuffer != nullptr)
				{
					postProcessRt = nullptr;

					apiCamera = this;
					apiViewId = RENDER_OUTLINE_PASS_ID + viewLayer;

					GameObject* node = getGameObject();
					if (node != nullptr)
					{
						std::vector<MonoScript*> scripts = node->getMonoScripts();
						for (auto scr : scripts)
						{
							if (!scr->getEnabled())
								continue;

							if (!scr->isInitialized())
								continue;

							MonoObject* obj = scr->getManagedObject();

							RenderTexture* srcRt = sceneBuffer;
							if (postProcessRt != nullptr)
								srcRt = postProcessRt;

							void* args[2] = { sceneBuffer->getManagedObject(), srcRt->getManagedObject() };
							APIManager::getSingleton()->execute(obj, "OnRenderImage", args, "RenderTexture,RenderTexture");
						}
					}
				}

				RenderTexture* finalRt = sceneBuffer;
				if (postProcessRt != nullptr)
					finalRt = postProcessRt;

				renderer->setSystemUniforms(this);
				bgfx::setTexture(0, renderer->u_albedoMap, finalRt->getColorTextureHandle());
				bgfx::setTexture(1, renderer->u_depthMap, sceneBuffer->getDepthTextureHandle());
				bgfx::setState(0
					| BGFX_STATE_WRITE_RGB
					| BGFX_STATE_WRITE_A
					| BGFX_STATE_BLEND_ALPHA);
				Primitives::screenSpaceQuad();
				bgfx::submit(RENDER_FINAL_PASS_ID + viewLayer, renderer->fxaaPH);
			}
		}
		//-->Render 3D end

		renderer->collectStats();
		renderer->frame();
	}
}