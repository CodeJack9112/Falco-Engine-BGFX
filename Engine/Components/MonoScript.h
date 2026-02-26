#pragma once

#include "Component.h"

#include "../glm/vec2.hpp"
#include "../glm/vec3.hpp"
#include "../glm/vec4.hpp"

#include "../Engine/Serialization/Data/SVector.h"

#include <map>
#include <string>
#include <vector>

namespace GX
{
	class MonoScript : public Component
	{
		friend class APIManager;
		friend class Scene;
		friend class API_GameObject;

	public:
		struct MonoFieldInfo;

		struct ValueVariant : public Archive
		{
			friend struct MonoFieldInfo;

		protected:
			SVector2 svec2Val;
			SVector3 svec3Val;
			SVector4 svec4Val;

		public:
			virtual void serialize(Serializer* s)
			{
				Archive::serialize(s);
				data(stringVal);
				data(boolVal);
				data(intVal);
				data(floatVal);
				data(objectVal);
				data(svec2Val);
				data(svec3Val);
				data(svec4Val);
				data(stringArrVal);
			}

			ValueVariant() {};
			ValueVariant(std::string strVal) { stringVal = strVal; }
			ValueVariant(bool bVal) { boolVal = bVal; }
			ValueVariant(int iVal) { intVal = iVal; }
			ValueVariant(float fVal) { floatVal = fVal; }
			ValueVariant(glm::vec2 vecVal) { vec2Val = vecVal; }
			ValueVariant(glm::vec3 vecVal) { vec3Val = vecVal; }
			ValueVariant(glm::vec4 vecVal) { vec4Val = vecVal; }

			std::string stringVal = "";
			std::vector<std::string> stringArrVal;
			bool boolVal = false;
			int intVal = 0;
			float floatVal = 0;
			std::string objectVal = "[None]";
			glm::vec2 vec2Val = glm::vec2(0, 0);
			glm::vec3 vec3Val = glm::vec3(0, 0, 0);
			glm::vec4 vec4Val = glm::vec4(0, 0, 0, 0);
		};

		struct MonoFieldInfo : public Archive
		{
		public:
			virtual void serialize(Serializer* s)
			{
				Archive::serialize(s);
				data(fieldName);
				data(fieldType);
				data(fieldValue);
			}

			std::string fieldName = "";
			std::string fieldType = "";
			ValueVariant fieldValue;

			std::string serialize();
			void deserialize(std::string value);
		};

		MonoScript();
		virtual ~MonoScript();

		typedef std::vector<MonoScript::MonoFieldInfo> FieldList; //Store field info

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual void setEnabled(bool value);
		virtual void onSceneLoaded();
		virtual void onManagedObjectDestroyed();
		virtual Component* onClone();
		virtual void onRefresh();
		virtual void onRebindObject(std::string oldObj, std::string newObj);
		virtual void onStateChanged();

		FieldList getFields();
		MonoScript::MonoFieldInfo* getField(std::string name); //Delete it after use!
		void setField(MonoScript::MonoFieldInfo value);
		void setClassFromName(std::string className);
		std::string getClassName();

		static std::vector<std::string>& getComponentList() { return componentList; }

		bool isInitialized() { return startExecuted; }

	protected:
		bool startExecuted = false;
		static std::vector<std::string> componentList;

	private:
		std::string managedClassName = "";

		std::vector<MonoFieldInfo> serializedFields;
		void addSerializedField(MonoScript::MonoFieldInfo value) { serializedFields.push_back(value); }
		void sendSerializedFieldsToMono();
	};
}