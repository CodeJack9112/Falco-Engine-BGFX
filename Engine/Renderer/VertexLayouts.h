#pragma once

#include <bgfx/bgfx.h>

namespace GX
{
	class VertexLayouts
	{
	public:
		static bgfx::VertexLayout verticesOnlyVertexLayout;
		static bgfx::VertexLayout primitiveVertexLayout;
		static bgfx::VertexLayout subMeshVertexLayout;
		static bgfx::VertexLayout terrainVertexLayout;
		static bgfx::VertexLayout waterVertexLayout;
		static bgfx::VertexLayout particleVertexLayout;

		static void init();
	};
}