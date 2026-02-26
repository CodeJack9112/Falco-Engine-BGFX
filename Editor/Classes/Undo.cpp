#include "Undo.h"

namespace GX
{
	int Undo::stackPos = -1;
	std::vector<UndoData*> Undo::undoStack;
	std::map<void*, bool> Undo::deletedObjects;
	std::function<void()> Undo::onUndoAdded = nullptr;

	UndoData::~UndoData()
	{
		auto& stack = Undo::getStack();

		auto it = std::find(stack.begin(), stack.end(), this);
		if (it != stack.end())
		{
			int pos = std::distance(stack.begin(), it);
			if (pos > Undo::getCurrentPosition())
			{
				if (destroyAction != nullptr)
					destroyAction(this);
			}
		}

		for (auto& it : matrixData)
			it.clear();

		matrixData.clear();

		for (auto& it : objectData)
			it.clear();

		objectData.clear();

		for (auto& it : object2Data)
			it.clear();

		object2Data.clear();

		for (auto& it : stringData)
			it.clear();

		stringData.clear();

		for (auto& it : boolData)
			it.clear();

		boolData.clear();

		for (auto& it : intData)
			it.clear();

		intData.clear();

		for (auto& it : vec2Data)
			it.clear();

		vec2Data.clear();

		for (auto& it : vec3Data)
			it.clear();

		vec3Data.clear();

		for (auto& it : vec4Data)
			it.clear();

		vec4Data.clear();

		for (auto& it : quatData)
			it.clear();

		quatData.clear();

		for (auto& it : floatData)
			it.clear();

		floatData.clear();

		for (auto& it : vec3ListData)
		{
			for (auto& tt : it)
				tt.second.clear();

			it.clear();
		}

		vec3ListData.clear();

		for (auto& it : string2Data)
			it.second.clear();

		string2Data.clear();

		for (auto& it : vec22Data)
			it.second.clear();

		vec22Data.clear();

		for (auto& it : float2Data)
			it.second.clear();

		float2Data.clear();

		for (auto& it : bool2Data)
			it.second.clear();

		bool2Data.clear();
	}

	bool UndoData::hasObject(void* object)
	{
		bool value = false;

		/*if (destroyAction != nullptr)
			destroyAction(this);*/

		for (auto& it : matrixData)
		{
			if (value)
				break;

			for (auto& tt : it)
			{
				if (tt.first == object)
				{
					value = true;
					break;
				}
			}
		}

		if (value)
			return value;

		for (auto& it : objectData)
		{
			if (value)
				break;

			auto tt = std::find(it.begin(), it.end(), object);
			if (tt != it.end())
			{
				value = true;
				break;
			}
		}

		if (value)
			return value;

		for (auto& it : object2Data)
		{
			if (value)
				break;

			auto tt = it.find(object);
			if (tt != it.end())
			{
				value = true;
				break;
			}
		}

		if (value)
			return value;

		for (auto& it : stringData)
		{
			if (value)
				break;

			auto tt = it.find(object);
			if (tt != it.end())
			{
				value = true;
				break;
			}
		}

		if (value)
			return value;

		for (auto& it : boolData)
		{
			if (value)
				break;

			auto tt = it.find(object);
			if (tt != it.end())
			{
				value = true;
				break;
			}
		}

		if (value)
			return value;

		for (auto& it : intData)
		{
			if (value)
				break;

			auto tt = it.find(object);
			if (tt != it.end())
			{
				value = true;
				break;
			}
		}

		if (value)
			return value;

		for (auto& it : vec2Data)
		{
			if (value)
				break;

			auto tt = it.find(object);
			if (tt != it.end())
			{
				value = true;
				break;
			}
		}

		if (value)
			return value;

		for (auto& it : vec3Data)
		{
			if (value)
				break;

			auto tt = it.find(object);
			if (tt != it.end())
			{
				value = true;
				break;
			}
		}

		if (value)
			return value;

		for (auto& it : vec4Data)
		{
			if (value)
				break;

			auto tt = it.find(object);
			if (tt != it.end())
			{
				value = true;
				break;
			}
		}

		if (value)
			return value;

		for (auto& it : quatData)
		{
			if (value)
				break;

			auto tt = it.find(object);
			if (tt != it.end())
			{
				value = true;
				break;
			}
		}

		if (value)
			return value;

		for (auto& it : floatData)
		{
			if (value)
				break;

			auto tt = it.find(object);
			if (tt != it.end())
			{
				value = true;
				break;
			}
		}

		if (value)
			return value;

		for (auto& it : vec3ListData)
		{
			auto tt = it.find(object);
			if (tt != it.end())
			{
				value = true;
				break;
			}
		}

		if (value)
			return value;

		auto _tt = string2Data.find(object);
		if (_tt != string2Data.end())
			value = true;

		if (value)
			return value;

		auto _kt = vec22Data.find(object);
		if (_kt != vec22Data.end())
			value = true;

		auto _ft = float2Data.find(object);
		if (_ft != float2Data.end())
			value = true;

		auto _bt = bool2Data.find(object);
		if (_bt != bool2Data.end())
			value = true;

		return value;
	}

	UndoData* Undo::addUndo(std::string name)
	{
		if (stackPos + 1 < undoStack.size())
		{
			for (int i = stackPos + 1; i < undoStack.size(); ++i)
			{
				delete undoStack[i];
			}

			while (undoStack.size() > stackPos + 1)
				undoStack.erase(undoStack.end() - 1);
		}

		UndoData* data = new UndoData();
		data->actionName = name;

		undoStack.push_back(data);
		stackPos = undoStack.size() - 1;

		if (onUndoAdded != nullptr)
			onUndoAdded();

		return data;
	}

	void Undo::removeUndo(UndoData* data)
	{
		auto it = std::find(undoStack.begin(), undoStack.end(), data);
		if (it != undoStack.end())
		{
			int pos = std::distance(undoStack.begin(), it);

			delete* it;
			undoStack.erase(it);

			if (pos <= stackPos)
				--stackPos;
		}
	}

	void Undo::clearUndo()
	{
		for (auto& data : undoStack)
			delete data;

		undoStack.clear();
		deletedObjects.clear();

		stackPos = -1;
	}

	void Undo::doUndo()
	{
		if (isUndoAvailable())
		{
			UndoData* data = undoStack[stackPos];
			data->undoAction(data);

			--stackPos;
		}
	}

	void Undo::doRedo()
	{
		if (isRedoAvailable())
		{
			++stackPos;

			UndoData* data = undoStack[stackPos];
			data->redoAction(data);
		}
	}

	bool Undo::isUndoAvailable()
	{
		return stackPos >= 0;
	}

	bool Undo::isRedoAvailable()
	{
		return stackPos + 1 < undoStack.size();
	}

	std::string Undo::getLastUndoName()
	{
		if (isUndoAvailable())
			return undoStack[stackPos]->actionName;

		return "";
	}

	std::string Undo::getLastRedoName()
	{
		if (isRedoAvailable())
			return undoStack[stackPos + 1]->actionName;

		return "";
	}

	bool Undo::getObjectDeleted(void* object)
	{
		return deletedObjects[object];
	}

	void Undo::setObjectDeleted(void* object, bool value)
	{
		deletedObjects[object] = value;
	}
}