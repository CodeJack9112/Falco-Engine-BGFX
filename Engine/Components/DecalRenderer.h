#pragma once

#include "Component.h"
#include "Renderable.h"

#include "../Renderer/Color.h"

namespace GX
{
	class Material;
	class Transform;

	class DecalRenderer : public Component, public Renderable
	{
	private:
		AxisAlignedBox cachedAAB = AxisAlignedBox::BOX_NULL;
		glm::mat4x4 prevTransform = glm::mat4x4(FLT_MAX);

		Transform* transform = nullptr;

		Color color = Color::White;
		Material* material = nullptr;

	public:
		DecalRenderer();
		virtual ~DecalRenderer();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType() { return COMPONENT_TYPE; }

		virtual void onAttach();
		virtual void onDetach();
		virtual bool isTransparent();
		virtual bool isDecal() { return true; }
		virtual AxisAlignedBox getBounds(bool world = true);
		virtual Component* onClone();
		virtual bool getCastShadows() { return false; }
		virtual void onRender(Camera* camera, int view, uint64_t state, bgfx::ProgramHandle program, int renderMode, std::function<void()> preRenderCallback);

		Color getColor() { return color; }
		void setColor(Color value) { color = value; }

		Material* getMaterial() { return material; }
		void setMaterial(Material* value) { material = value; }
	};
}