#include "VertexLayouts.h"

namespace GX
{
	bgfx::VertexLayout VertexLayouts::verticesOnlyVertexLayout;
	bgfx::VertexLayout VertexLayouts::primitiveVertexLayout;
	bgfx::VertexLayout VertexLayouts::subMeshVertexLayout;
	bgfx::VertexLayout VertexLayouts::terrainVertexLayout;
	bgfx::VertexLayout VertexLayouts::waterVertexLayout;
	bgfx::VertexLayout VertexLayouts::particleVertexLayout;

	void VertexLayouts::init()
	{
		verticesOnlyVertexLayout = bgfx::VertexLayout();
		verticesOnlyVertexLayout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.end();

		primitiveVertexLayout = bgfx::VertexLayout();
		primitiveVertexLayout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.end();

		subMeshVertexLayout = bgfx::VertexLayout();
		subMeshVertexLayout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, true, true)
			.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float, true, true)
			.add(bgfx::Attrib::Weight, 4, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Indices, 4, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();

		terrainVertexLayout = bgfx::VertexLayout();
		terrainVertexLayout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, true, true)
			.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float, true, true)
			.end();

		waterVertexLayout = bgfx::VertexLayout();
		waterVertexLayout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, true, true)
			.end();

		particleVertexLayout = bgfx::VertexLayout();
		particleVertexLayout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, true, true)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
	}
}