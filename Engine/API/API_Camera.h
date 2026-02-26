#pragma once

#include "API.h"

namespace GX
{
	class API_Camera
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Camera::INTERNAL_getViewMatrix", (void*)getViewMatrix);
			mono_add_internal_call("FalcoEngine.Camera::INTERNAL_getProjectionMatrix", (void*)getProjectionMatrix);
			mono_add_internal_call("FalcoEngine.Camera::INTERNAL_worldToScreenPoint", (void*)worldToScreenPoint);
			mono_add_internal_call("FalcoEngine.Camera::INTERNAL_screenToWorldPoint", (void*)screenToWorldPoint);
			mono_add_internal_call("FalcoEngine.Camera::INTERNAL_isObjectVisible", (void*)isObjectVisible);
			mono_add_internal_call("FalcoEngine.Camera::INTERNAL_GetGBufferTexture", (void*)getGBufferTexture);

			mono_add_internal_call("FalcoEngine.Camera::get_nearClipDistance", (void*)getNearClipDistance);
			mono_add_internal_call("FalcoEngine.Camera::set_nearClipDistance", (void*)setNearClipDistance);
			mono_add_internal_call("FalcoEngine.Camera::get_farClipDistance", (void*)getFarClipDistance);
			mono_add_internal_call("FalcoEngine.Camera::set_farClipDistance", (void*)setFarClipDistance);
			mono_add_internal_call("FalcoEngine.Camera::get_renderTarget", (void*)getRenderTarget);
			mono_add_internal_call("FalcoEngine.Camera::set_renderTarget", (void*)setRenderTarget);
			mono_add_internal_call("FalcoEngine.Camera::get_fov", (void*)getFov);
			mono_add_internal_call("FalcoEngine.Camera::set_fov", (void*)setFov);
		}

	private:
		//getViewMatrix
		static void getViewMatrix(MonoObject* this_ptr, bool ownFrustumOnly, API::Matrix4 * matrix);

		//getViewMatrix
		static void getProjectionMatrix(MonoObject* this_ptr, API::Matrix4* matrix);

		//worldToScreenPoint
		static void worldToScreenPoint(MonoObject* this_ptr, API::Vector3 * ref_world, API::Vector3* out_screen);

		//screenToWorldPoint
		static void screenToWorldPoint(MonoObject* this_ptr, API::Vector3 * ref_screen, API::Vector3* out_world);

		//getNearClipDistance
		static float getNearClipDistance(MonoObject* this_ptr);

		//setNearClipDistance
		static void setNearClipDistance(MonoObject* this_ptr, float value);

		//getFarClipDistance
		static float getFarClipDistance(MonoObject* this_ptr);

		//setFarClipDistance
		static void setFarClipDistance(MonoObject* this_ptr, float value);

		//isObjectVisible
		static bool isObjectVisible(MonoObject* this_ptr, MonoObject * obj);

		//getRenderTarget
		static MonoObject* getRenderTarget(MonoObject* this_ptr);

		//setRenderTarget
		static void setRenderTarget(MonoObject* this_ptr, MonoObject* value);
		
		//get GBuffer Texture
		static MonoObject* getGBufferTexture(MonoObject* this_ptr, int value);

		//getFov
		static float getFov(MonoObject* this_ptr);

		//setFov
		static void setFov(MonoObject* this_ptr, float value);
	};
}