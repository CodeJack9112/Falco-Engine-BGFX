#pragma once

#include <string>
#include <vector>

#include <bgfx/bgfx.h>
#include "../glm/mat4x4.hpp"

#include "Component.h"

#include "../Renderer/Color.h"
#include "../Renderer/Frustum.h"

namespace GX
{
	enum class LightType
	{
		Point,
		Spot,
		Directional
	};

	enum class LightRenderMode
	{
		Realtime,
		Mixed,
		Baked
	};

	class Camera;

	class Light : public Component
	{
		friend class Renderer;
		friend class MeshRenderer;

	private:
		LightType lightType = LightType::Point;
		LightRenderMode lightRenderMode = LightRenderMode::Realtime;
		Color color = Color::White;
		float intensity = 1.0f;
		float radius = 10.0f;
		float innerRadius = 35.0f;
		float outerRadius = 45.0f;
		float bias = 0.005f;
		bool castShadows = true;

		std::vector<bgfx::FrameBufferHandle> shadowMapFB;
		std::vector<bgfx::TextureHandle> shadowTextures;

		int shadowMapSize = 1024;
		glm::mat4x4 mShadowMtx[4];
		Frustum* mShadowFrustum[4];

	public:
		Light();
		virtual ~Light();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();
		virtual void onStateChanged();
		virtual void onAttach();
		virtual void onDetach();

		void onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, std::function<void()> preRenderCallback); // Rendering function

		void destroyShadowMaps();
		void updateShadowMaps();
		bool submitUniforms();

		bgfx::TextureHandle & getShadowMap(int i);
		int getNumShadowMaps();

		int getShadowMapSize() { return shadowMapSize; }

		LightType getLightType() { return lightType; }
		void setLightType(LightType value);

		LightRenderMode getLightRenderMode() { return lightRenderMode; }
		void setLightRenderMode(LightRenderMode value);

		Color& getColor() { return color; }
		void setColor(Color value) { color = value; }

		float getRadius() { return radius; }
		void setRadius(float value) { radius = value; }

		float getIntensity() { return intensity; }
		void setIntensity(float value) { intensity = value; }

		float getInnerRadius() { return innerRadius; }
		void setInnerRadius(float value);

		float getOuterRadius() { return outerRadius; }
		void setOuterRadius(float value);

		float getBias() { return bias; }
		void setBias(float value) { bias = value; }

		bool getCastShadows() { return castShadows; }
		void setCastShadows(bool value);
	};
}