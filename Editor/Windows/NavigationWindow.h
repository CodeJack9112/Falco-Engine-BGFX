#pragma once

#include <bgfx/bgfx.h>

namespace GX
{
	class DialogProgress;
	class Camera;

	class NavigationWindow
	{
	private:
		bool opened = true;

		DialogProgress* progressDialog = nullptr;

		int prevVerticesCount = 0;
		bgfx::DynamicVertexBufferHandle vbh = { bgfx::kInvalidHandle };

		void drawNavMesh(int viewId, int viewLayer, Camera* camera);

		void onChangeWalkableSlopeAngle(float prevVal, float newVal);
		void onChangeWalkableHeight(float prevVal, float newVal);
		void onChangeWalkableClimb(float prevVal, float newVal);
		void onChangeWalkableRadius(float prevVal, float newVal);
		void onChangeMaxEdgeLength(float prevVal, float newVal);
		void onChangeMaxSimplificationError(float prevVal, float newVal);
		void onChangeMinRegionArea(float prevVal, float newVal);
		void onChangeMergeRegionArea(float prevVal, float newVal);
		void onChangeCellSize(float prevVal, float newVal);
		void onChangeCellHeight(float prevVal, float newVal);

	public:
		NavigationWindow();
		~NavigationWindow();

		void init();
		void update();
		void show(bool show) { opened = show; }
		bool getVisible() { return opened; }
	};
}