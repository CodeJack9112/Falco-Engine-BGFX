#pragma once

#include <vector>
#include <functional>

#include "../glm/vec2.hpp"
#include "../glm/vec3.hpp"

#include "Component.h"

#include "../Math/AxisAlignedBox.h"

namespace carve
{
	namespace poly
	{
		class Polyhedron;
	}

	namespace interpolate
	{
		template<typename attr_t>
		class FaceVertexAttr;

		template<typename attr_t>
		class FaceAttr;
	}
}

namespace GX
{
	class Material;

	class CSGBrush : public Component
	{
	public:
		enum class BrushType
		{
			Cube,
			Sphere,
			Cone,
			Cylinder,
			Custom
		};

		enum class BrushOperation
		{
			Add,
			Subtract
		};

		struct uv_t
		{
			float u = 0.0f;
			float v = 0.0f;

			uv_t() : u(0.0f), v(0.0f) { }
			uv_t(float _u, float _v) : u(_u), v(_v) { }
		};

		struct FaceInfo
		{
			std::vector<uint32_t> indices;
			std::vector<glm::vec2> texCoords;
			glm::vec2 texCoordsScale = glm::vec2(1.0f);
			glm::vec2 texCoordsOffset = glm::vec2(0.0f);
			float texCoordsRotation = 0.0f;
			bool smoothNormals = true;

			Material* material = nullptr;
		};

	private:
		BrushType brushType = BrushType::Cube;
		BrushOperation brushOperation = BrushOperation::Add;

		bool castShadows = true;

		AxisAlignedBox bounds = AxisAlignedBox::BOX_NULL;
		AxisAlignedBox localBounds = AxisAlignedBox::BOX_NULL;

		std::vector<glm::vec3> vertices;
		std::vector<FaceInfo> faces;

		carve::poly::Polyhedron* brushPtr = nullptr;

		int segments = 16;
		int stacks = 16;

		void destroy();

		void makeCube();
		void makeSphere();
		void makeCone();
		void makeCylinder();

	public:
		CSGBrush();
		virtual ~CSGBrush();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType() { return COMPONENT_TYPE; }

		virtual void onAttach();
		virtual void onStateChanged();
		virtual Component* onClone();

		void rebuild();

		carve::poly::Polyhedron* getBrushPtr() { return brushPtr; }

		BrushOperation getBrushOperation() { return brushOperation; }
		void setBrushOperation(BrushOperation value);

		BrushType getBrushType() { return brushType; }
		void setBrushType(BrushType value);

		bool getCastShadows() { return castShadows; }
		void setCastShadows(bool value);

		AxisAlignedBox& getBounds() { return bounds; }
		AxisAlignedBox& getLocalBounds() { return localBounds; }

		void bind
		(
			carve::interpolate::FaceVertexAttr<uv_t>* fv_uv,
			carve::interpolate::FaceAttr<Material*>* f_material,
			carve::interpolate::FaceAttr<int>* f_layer,
			carve::interpolate::FaceAttr<bool>* f_castShadows,
			carve::interpolate::FaceAttr<bool>* f_smoothNormals,
			carve::interpolate::FaceAttr<size_t>* f_brushId
		);

		std::vector<glm::vec3>& getVertices() { return vertices; }
		std::vector<FaceInfo>& getFaces() { return faces; }

		Material* getMaterial(int faceIndex);
		void setMaterial(int faceIndex, Material* value);

		glm::vec2 getUV(int faceIndex, int vertIndex);
		void setUV(int faceIndex, int vertIndex, glm::vec2 value);

		glm::vec2 getUVScale(int faceIndex);
		void setUVScale(int faceIndex, glm::vec2 value);

		glm::vec2 getUVOffset(int faceIndex);
		void setUVOffset(int faceIndex, glm::vec2 value);

		float getUVRotation(int faceIndex);
		void setUVRotation(int faceIndex, float value);

		bool getSmoothNormals(int faceIndex);
		void setSmoothNormals(int faceIndex, bool value);

		int getNumSegments() { return segments; }
		void setNumSegments(int value);

		int getNumStacks() { return stacks; }
		void setNumStacks(int value);
	};

	CSGBrush::uv_t operator*(double s, const CSGBrush::uv_t& t);
	CSGBrush::uv_t& operator+=(CSGBrush::uv_t& t1, const CSGBrush::uv_t& t2);
}