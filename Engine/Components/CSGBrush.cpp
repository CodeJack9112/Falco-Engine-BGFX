#include "CSGBrush.h"

#include <carve/carve.hpp>
#include <carve/csg.hpp>
#include <carve/input.hpp>
#include <carve/interpolator.hpp>

#include "../glm/mat4x4.hpp"

#include "../Core/APIManager.h"
#include "../Core/GameObject.h"

#include "Transform.h"
#include "../Renderer/CSGGeometry.h"
#include "../Math/Mathf.h"

namespace GX
{
	std::string CSGBrush::COMPONENT_TYPE = "CSGBrush";

	CSGBrush::CSGBrush() : Component(nullptr)
	{
		makeCube();
	}

	CSGBrush::~CSGBrush()
	{
		
	}

	CSGBrush::uv_t operator*(double s, const CSGBrush::uv_t& t)
	{
		return CSGBrush::uv_t(t.u * s, t.v * s);
	}

	CSGBrush::uv_t& operator+=(CSGBrush::uv_t& t1, const CSGBrush::uv_t& t2)
	{
		t1.u += t2.u;
		t1.v += t2.v;
		return t1;
	}

	Component* CSGBrush::onClone()
	{
		CSGBrush* newComponent = new CSGBrush();
		newComponent->brushType = brushType;
		newComponent->brushOperation = brushOperation;
		newComponent->castShadows = castShadows;
		newComponent->segments = segments;
		newComponent->stacks = stacks;
		newComponent->vertices = vertices;
		newComponent->faces = faces;

		newComponent->rebuild();

		return newComponent;
	}

	void CSGBrush::destroy()
	{
		if (brushPtr != nullptr)
			delete brushPtr;

		brushPtr = nullptr;

		bounds = AxisAlignedBox::BOX_NULL;
		localBounds = AxisAlignedBox::BOX_NULL;
	}

	void CSGBrush::makeCube()
	{
		vertices.clear();
		faces.clear();

		vertices.push_back(glm::vec3(+1.0f, +1.0f, +1.0f));
		vertices.push_back(glm::vec3(-1.0f, +1.0f, +1.0f));
		vertices.push_back(glm::vec3(-1.0f, -1.0f, +1.0f));
		vertices.push_back(glm::vec3(+1.0f, -1.0f, +1.0f));
		vertices.push_back(glm::vec3(+1.0f, +1.0f, -1.0f));
		vertices.push_back(glm::vec3(-1.0f, +1.0f, -1.0f));
		vertices.push_back(glm::vec3(-1.0f, -1.0f, -1.0f));
		vertices.push_back(glm::vec3(+1.0f, -1.0f, -1.0f));

		FaceInfo face1, face2, face3, face4, face5, face6;

		face1.indices = { 0, 1, 2, 3 };
		face2.indices = { 7, 6, 5, 4 };
		face3.indices = { 0, 4, 5, 1 };
		face4.indices = { 2, 6, 7, 3 };
		face5.indices = { 3, 7, 4, 0 };
		face6.indices = { 1, 5, 6, 2 };

		face1.texCoords = { {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f} };
		face2.texCoords = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		face3.texCoords = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		face4.texCoords = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		face5.texCoords = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		face6.texCoords = { {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f} };

		faces.push_back(face1); //+Z
		faces.push_back(face2); //-Z
		faces.push_back(face3); //+Y
		faces.push_back(face4); //-Y
		faces.push_back(face5); //+X
		faces.push_back(face6); //-X
	}

	void CSGBrush::makeSphere()
	{
		Material* mat = nullptr;
		if (faces.size() > 0)
			mat = faces[0].material;

		vertices.clear();
		faces.clear();

		uint32_t meridians = segments;
		uint32_t parallels = stacks;

		vertices.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f));
		for (uint32_t j = 0; j < parallels - 1; ++j)
		{
			double polar = M_PI * double(j + 1) / double(parallels);
			double sp = std::sin(polar);
			double cp = std::cos(polar);

			for (uint32_t i = 0; i < meridians; ++i)
			{
				double azimuth = 2.0 * M_PI * double(i) / double(meridians);
				double sa = std::sin(azimuth);
				double ca = std::cos(azimuth);
				double x = sp * ca;
				double y = cp;
				double z = sp * sa;

				vertices.emplace_back(glm::vec3(x, y, z));
			}
		}
		vertices.emplace_back(glm::vec3(0.0f, -1.0f, 0.0f));

		float m = 1.0f / (float)meridians;
		float p = 1.0f / (float)parallels;
		
		for (uint32_t i = 0; i < meridians; ++i)
		{
			uint32_t a = i + 1;
			uint32_t b = (i + 1) % meridians + 1;

			float jj = 1.0f / (float)parallels * (float)(parallels - 1);
			float ii = 1.0f / (float)meridians * (float)i;
			
			FaceInfo face;
			face.material = mat;
			face.indices = { 0, b, a };
			face.texCoords = { {1.0f - (ii + m),	1.0f},
								{1.0f - (ii + m),	jj},
								{1.0f - ii,			jj} };

			faces.push_back(face);
		}

		for (uint32_t j = 0; j < parallels - 2; ++j)
		{
			uint32_t aStart = j * meridians + 1;
			uint32_t bStart = (j + 1) * meridians + 1;

			for (uint32_t i = 0; i < meridians; ++i)
			{
				uint32_t a = aStart + i;
				uint32_t a1 = aStart + (i + 1) % meridians;
				uint32_t b = bStart + i;
				uint32_t b1 = bStart + (i + 1) % meridians;

				float jj = 1.0f / (float)parallels * (float)(j + 1);
				float ii = 1.0f / (float)meridians * (float)i;

				FaceInfo face;
				face.material = mat;
				face.indices = { a, a1, b1, b };
				face.texCoords = {  {1.0f - ii,			1.0f - jj},
									{1.0f - (ii + m),	1.0f - jj},
									{1.0f - (ii + m),	1.0f - (jj + p)},
									{1.0f - ii,			1.0f - (jj + p)} };

				faces.push_back(face);
			}
		}

		for (uint32_t i = 0; i < meridians; ++i)
		{
			uint32_t a = i + meridians * (parallels - 2) + 1;
			uint32_t b = (i + 1) % meridians + meridians * (parallels - 2) + 1;

			uint32_t i0 = (uint32_t)vertices.size() - 1;

			float ii = 1.0f / (float)meridians * (float)i;

			FaceInfo face;
			face.material = mat;
			face.indices = { i0, a, b };
			face.texCoords = { {1.0f - (ii + m),	0.0f},
								{1.0f - ii,			p},
								{1.0f - (ii + m),	p} };

			faces.push_back(face);
		}
	}

	void CSGBrush::makeCone()
	{
		Material* mat = nullptr;
		if (faces.size() > 0)
			mat = faces[0].material;

		vertices.clear();
		faces.clear();

		float radius = 1.0f;

		// add vertices subdividing a circle
		for (uint32_t i = 0; i < segments; i++)
		{
			float ratio = (float)i / (float)segments;
			float r = ratio * (M_PI * 2.0);
			float x = std::cos(r) * radius;
			float y = std::sin(r) * radius;
			vertices.push_back(glm::vec3(x, -1.0f, y));
		}

		std::reverse(vertices.begin(), vertices.end());

		// add the tip of the cone
		vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
		vertices.push_back(glm::vec3(0.0f, -1.0f, 0.0f));

		float m = 1.0f / (float)segments;

		// generate triangular faces
		for (uint32_t i = 0; i < segments; i++)
		{
			uint32_t ii = (i + 1) % segments;

			float _ii = 1.0f / (float)segments * (float)i;

			FaceInfo face0;
			face0.material = mat;
			face0.indices = { (uint32_t)vertices.size() - 2, i, ii };
			face0.texCoords = { {_ii + m,	1.0f},
								{_ii,		0.0f},
								{_ii + m,	0.0f}};

			faces.push_back(face0);

			FaceInfo face1;
			face1.material = mat;
			face1.indices = { ii, i, (uint32_t)vertices.size() - 1 };
			face1.texCoords = { {_ii + m,	0.0f},
								{_ii,		0.0f},
								{_ii,		1.0f} };

			faces.push_back(face1);
		}
	}

	void CSGBrush::makeCylinder()
	{
		Material* mat = nullptr;
		if (faces.size() > 0)
			mat = faces[0].material;

		vertices.clear();
		faces.clear();

		uint32_t meridians = segments;
		uint32_t parallels = 3;

		vertices.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f));
		for (uint32_t j = 0; j < parallels - 1; ++j)
		{
			double polar = M_PI / 2.0;
			double sp = std::sin(polar);
			double cp = j == 0 ? 1.0 : -1.0;

			for (uint32_t i = 0; i < meridians; ++i)
			{
				double azimuth = 2.0 * M_PI * double(i) / double(meridians);
				double sa = std::sin(azimuth);
				double ca = std::cos(azimuth);
				double x = sp * ca;
				double y = cp;
				double z = sp * sa;

				vertices.emplace_back(glm::vec3(x, y, z));
			}
		}
		vertices.emplace_back(glm::vec3(0.0f, -1.0f, 0.0f));

		float m = 1.0f / (float)meridians;
		float p = 1.0f / (float)parallels;

		for (uint32_t i = 0; i < meridians; ++i)
		{
			uint32_t a = i + 1;
			uint32_t b = (i + 1) % meridians + 1;

			float jj = 1.0f / (float)parallels * (float)(parallels - 1);
			float ii = 1.0f / (float)meridians * (float)i;
			
			FaceInfo face;
			face.material = mat;
			face.indices = { 0, b, a };
			face.texCoords = { {1.0f - (ii + m),	1.0f},
								{1.0f - (ii + m),	jj},
								{1.0f - ii,			jj} };

			faces.push_back(face);
		}

		for (uint32_t j = 0; j < parallels - 2; ++j)
		{
			uint32_t aStart = j * meridians + 1;
			uint32_t bStart = (j + 1) * meridians + 1;

			for (uint32_t i = 0; i < meridians; ++i)
			{
				uint32_t a = aStart + i;
				uint32_t a1 = aStart + (i + 1) % meridians;
				uint32_t b = bStart + i;
				uint32_t b1 = bStart + (i + 1) % meridians;

				float jj = 1.0f / (float)parallels * (float)(j + 1);
				float ii = 1.0f / (float)meridians * (float)i;
				
				FaceInfo face;
				face.material = mat;
				face.indices = { a, a1, b1, b };
				face.texCoords = { {1.0f - ii,			1.0f - jj},
									{1.0f - (ii + m),	1.0f - jj},
									{1.0f - (ii + m),	1.0f - (jj + p)},
									{1.0f - ii,			1.0f - (jj + p)} };

				faces.push_back(face);
			}
		}

		for (uint32_t i = 0; i < meridians; ++i)
		{
			uint32_t a = i + meridians * (parallels - 2) + 1;
			uint32_t b = (i + 1) % meridians + meridians * (parallels - 2) + 1;

			uint32_t i0 = (uint32_t)vertices.size() - 1;

			float ii = 1.0f / (float)meridians * (float)i;

			FaceInfo face;
			face.material = mat;
			face.indices = { i0, a, b };
			face.texCoords = { {1.0f - (ii + m),	0.0f},
								{1.0f - ii,			p},
								{1.0f - (ii + m),	p} };

			faces.push_back(face);
		}
	}

	void CSGBrush::setBrushOperation(BrushOperation value)
	{
		brushOperation = value;
		rebuild();
	}

	void CSGBrush::setBrushType(BrushType value)
	{
		brushType = value;

		if (brushType == BrushType::Cube)
			makeCube();
		else if (brushType == BrushType::Sphere)
			makeSphere();
		else if (brushType == BrushType::Cone)
			makeCone();
		else if (brushType == BrushType::Cylinder)
			makeCylinder();

		rebuild();
	}

	void CSGBrush::setCastShadows(bool value)
	{
		castShadows = value;
		rebuild();
	}

	void CSGBrush::onAttach()
	{
		Component::onAttach();
		rebuild();
	}

	void CSGBrush::onStateChanged()
	{
		Component::onStateChanged();
		rebuild();
	}

	void CSGBrush::bind
	(
		carve::interpolate::FaceVertexAttr<uv_t>* fv_uv,
		carve::interpolate::FaceAttr<Material*>* f_material,
		carve::interpolate::FaceAttr<int>* f_layer,
		carve::interpolate::FaceAttr<bool>* f_castShadows,
		carve::interpolate::FaceAttr<bool>* f_smoothNormals,
		carve::interpolate::FaceAttr<size_t>* f_brushId
	)
	{
		for (int i = 0; i < faces.size(); ++i)
		{
			FaceInfo& face = faces[i];
			
			for (int j = 0; j < face.texCoords.size(); ++j)
			{
				glm::vec2 uv = face.texCoords[j];

				uv.x = uv.x * face.texCoordsScale.x + face.texCoordsOffset.x;
				uv.y = uv.y * face.texCoordsScale.y + face.texCoordsOffset.y;

				uv = Mathf::rotateUV(uv, Mathf::fDeg2Rad * face.texCoordsRotation);

				fv_uv->setAttribute(&brushPtr->faces[i], j, uv_t(uv.x, uv.y));
			}

			f_material->setAttribute(&brushPtr->faces[i], faces[i].material);
			f_layer->setAttribute(&brushPtr->faces[i], gameObject->getLayer());
			f_castShadows->setAttribute(&brushPtr->faces[i], castShadows);
			f_smoothNormals->setAttribute(&brushPtr->faces[i], faces[i].smoothNormals);
			f_brushId->setAttribute(&brushPtr->faces[i], gameObject->getGuidHash());
		}
	}

	Material* CSGBrush::getMaterial(int faceIndex)
	{
		if (faceIndex < faces.size())
			return faces[faceIndex].material;

		return nullptr;
	}

	void CSGBrush::setMaterial(int faceIndex, Material* value)
	{
		if (faceIndex < faces.size())
			faces[faceIndex].material = value;
	}

	glm::vec2 CSGBrush::getUV(int faceIndex, int vertIndex)
	{
		if (faceIndex < faces.size())
		{
			FaceInfo& face = faces[faceIndex];

			if (vertIndex < face.texCoords.size())
				return face.texCoords[vertIndex];
		}

		return glm::vec2(0.0f);
	}

	void CSGBrush::setUV(int faceIndex, int vertIndex, glm::vec2 value)
	{
		if (faceIndex < faces.size())
		{
			FaceInfo& face = faces[faceIndex];

			if (vertIndex < face.texCoords.size())
				face.texCoords[vertIndex] = value;
		}
	}

	glm::vec2 CSGBrush::getUVScale(int faceIndex)
	{
		if (faceIndex < faces.size())
		{
			FaceInfo& face = faces[faceIndex];
			return face.texCoordsScale;
		}

		return glm::vec2(1.0f);
	}

	void CSGBrush::setUVScale(int faceIndex, glm::vec2 value)
	{
		if (faceIndex < faces.size())
		{
			FaceInfo& face = faces[faceIndex];
			face.texCoordsScale = value;
		}
	}

	glm::vec2 CSGBrush::getUVOffset(int faceIndex)
	{
		if (faceIndex < faces.size())
		{
			FaceInfo& face = faces[faceIndex];
			return face.texCoordsOffset;
		}

		return glm::vec2(0.0f);
	}

	void CSGBrush::setUVOffset(int faceIndex, glm::vec2 value)
	{
		if (faceIndex < faces.size())
		{
			FaceInfo& face = faces[faceIndex];
			face.texCoordsOffset = value;
		}
	}

	float CSGBrush::getUVRotation(int faceIndex)
	{
		if (faceIndex < faces.size())
		{
			FaceInfo& face = faces[faceIndex];
			return face.texCoordsRotation;
		}

		return 0.0f;
	}

	void CSGBrush::setUVRotation(int faceIndex, float value)
	{
		if (faceIndex < faces.size())
		{
			FaceInfo& face = faces[faceIndex];
			face.texCoordsRotation = value;
		}
	}

	bool CSGBrush::getSmoothNormals(int faceIndex)
	{
		if (faceIndex < faces.size())
		{
			FaceInfo& face = faces[faceIndex];
			return face.smoothNormals;
		}

		return false;
	}

	void CSGBrush::setSmoothNormals(int faceIndex, bool value)
	{
		if (faceIndex < faces.size())
		{
			FaceInfo& face = faces[faceIndex];
			face.smoothNormals = value;
		}
	}

	void CSGBrush::setNumSegments(int value)
	{
		if (value < 3 || value > 64)
			return;

		segments = value;
		setBrushType(brushType);
	}

	void CSGBrush::setNumStacks(int value)
	{
		if (value < 3 || value > 64)
			return;

		stacks = value;
		setBrushType(brushType);
	}

	void CSGBrush::rebuild()
	{
		destroy();

		carve::input::PolyhedronData data;

		Transform* t = nullptr;
		glm::mat4x4 mtx = glm::identity<glm::mat4x4>();

		if (gameObject != nullptr)
		{
			t = gameObject->getTransform();
			mtx = t->getTransformMatrix();

			for (auto& v : vertices)
			{
				glm::vec4 lp = glm::vec4(v, 1.0f);
				glm::vec3 p = mtx * lp;

				localBounds.merge(lp);
				bounds.merge(p);

				data.addVertex(carve::geom::VECTOR(p.x, p.y, p.z));
			}

			for (int i = 0; i < faces.size(); ++i)
			{
				FaceInfo& face = faces[i];

				if (face.indices.size() == 3)
					data.addFace(face.indices[0], face.indices[1], face.indices[2]);
				else if (face.indices.size() == 4)
					data.addFace(face.indices[0], face.indices[1], face.indices[2], face.indices[3]);
			}
		}

		brushPtr = data.create();
	}
}