#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec2.hpp"

namespace GX
{
	class Property;
	class TreeNode;
	class Camera;

	class CSGBrushEditor : public ComponentEditor
	{
	public:
		CSGBrushEditor();
		virtual ~CSGBrushEditor();

		virtual void init(std::vector<Component*> comps);

		std::string cbId = "";
		std::vector<Property*> faceProps;

		Property* facesProp = nullptr;

		bool face0Hovered = false;
		bool face1Hovered = false;
		bool face2Hovered = false;
		bool face3Hovered = false;
		bool face4Hovered = false;
		bool face5Hovered = false;

		void updateFaceProps();
		void renderGizmos(int viewId, int viewLayer, Camera* camera);

	private:
		void onChangeBrushType(Property* prop, int val);
		void onChangeBrushOperation(Property* prop, int val);
		void onChangeCastShadows(Property* prop, bool val);
		void onChangeNumSegments(Property* prop, int val);
		void onChangeNumStacks(Property* prop, int val);
		void onChangeUV(Property* prop, glm::vec2 val, int faceId, int vertId);
		void onChangeUVScale(Property* prop, glm::vec2 val, int faceId);
		void onChangeUVOffset(Property* prop, glm::vec2 val, int faceId);
		void onChangeUVRotation(Property* prop, float val, int faceId);
		void onDropMaterial(TreeNode* from, int matIdx);

		void onDropMasterMaterial(TreeNode* from);
		void onDropMaterialRange(TreeNode* from, int start, int end);
		void onChangeMasterUVScale(Property* prop, glm::vec2 val);
		void onChangeMasterUVOffset(Property* prop, glm::vec2 val);
		void onChangeMasterUVRotation(Property* prop, float val);
		void onChangeMasterSmoothNormals(Property* prop, bool val);
	};
}