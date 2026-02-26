#pragma once

#include <functional>
#include <algorithm>
#include <vector>
#include <map>
#include <string>

#include "../Engine/glm/mat4x4.hpp"
#include "../Engine/glm/vec2.hpp"
#include "../Engine/glm/vec3.hpp"
#include "../Engine/glm/vec4.hpp"
#include "../Engine/glm/gtc/quaternion.hpp"

namespace GX
{
	struct UndoData
	{
	public:
		~UndoData();

		std::string actionName = "";

		std::function<void(UndoData* data)> undoAction;
		std::function<void(UndoData* data)> redoAction;
		std::function<void(UndoData* data)> destroyAction;

		std::vector<std::vector<void*>> objectData;
		std::vector<std::map<void*, void*>> object2Data;
		std::vector<std::map<void*, glm::mat4x4>> matrixData;
		std::vector<std::map<void*, glm::vec2>> vec2Data;
		std::vector<std::map<void*, glm::vec3>> vec3Data;
		std::vector<std::map<void*, glm::vec4>> vec4Data;
		std::vector<std::map<void*, std::map<int, glm::vec3>>> vec3ListData;
		std::vector<std::map<void*, glm::quat>> quatData;
		std::vector<std::map<void*, std::string>> stringData;
		std::vector<std::map<void*, bool>> boolData;
		std::vector<std::map<void*, int>> intData;
		std::vector<std::map<void*, float>> floatData;
		std::map<void*, std::vector<std::string>> string2Data;
		std::map<void*, std::vector<glm::vec2>> vec22Data;
		std::map<void*, std::vector<float>> float2Data;
		std::map<void*, std::vector<bool>> bool2Data;

		bool hasObject(void* object);
	};

	class Undo
	{
	private:
		static int stackPos;
		static std::vector<UndoData*> undoStack;
		static std::map<void*, bool> deletedObjects;
		static std::function<void()> onUndoAdded;

	public:
		static UndoData* addUndo(std::string name);
		static void removeUndo(UndoData* data);
		static void clearUndo();

		static void doUndo();
		static void doRedo();

		static bool isUndoAvailable();
		static bool isRedoAvailable();

		static std::string getLastUndoName();
		static std::string getLastRedoName();

		static bool getObjectDeleted(void* object);
		static void setObjectDeleted(void* object, bool value);

		static void setOnUndoAddedCallback(std::function<void()> callback) { onUndoAdded = callback; }

		static int getCurrentPosition() { return stackPos; }

		static const std::vector<UndoData*>& getStack() { return undoStack; }
	};
}