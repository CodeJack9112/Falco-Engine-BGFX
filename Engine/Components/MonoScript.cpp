#include "MonoScript.h"

#include <sstream>

#include "../../Mono/include/mono/metadata/attrdefs.h"

#include "../Core/APIManager.h"
#include "../Core/Engine.h"
#include "../Core/GameObject.h"
#include "../Classes/StringConverter.h"

#include "../Assets/AudioClip.h"
#include "../Assets/VideoClip.h"
#include "../Assets/Material.h"
#include "../Assets/Prefab.h"
#include "../Assets/Shader.h"
#include "../Assets/Texture.h"
#include "../Assets/AnimationClip.h"
#include "../Assets/Cubemap.h"
#include "../Assets/Font.h"

#include "../Components/Transform.h"
#include "../Components/Animation.h"
#include "../Components/AudioListener.h"
#include "../Components/AudioSource.h"
#include "../Components/VideoPlayer.h"
#include "../Components/NavMeshAgent.h"
#include "../Components/NavMeshObstacle.h"
#include "../Components/Spline.h"
#include "../Components/BoxCollider.h"
#include "../Components/CapsuleCollider.h"
#include "../Components/MeshCollider.h"
#include "../Components/SphereCollider.h"
#include "../Components/TerrainCollider.h"
#include "../Components/ConeTwistJoint.h"
#include "../Components/FixedJoint.h"
#include "../Components/FreeJoint.h"
#include "../Components/HingeJoint.h"
#include "../Components/RigidBody.h"
#include "../Components/Vehicle.h"
#include "../Components/Camera.h"
#include "../Components/Light.h"
#include "../Components/MeshRenderer.h"
#include "../Components/ParticleSystem.h"
#include "../Components/Terrain.h"
#include "../Components/Water.h"
#include "../Components/Button.h"
#include "../Components/Canvas.h"
#include "../Components/Image.h"
#include "../Components/Mask.h"
#include "../Components/Text.h"
#include "../Components/TextInput.h"

namespace GX
{
	std::string MonoScript::COMPONENT_TYPE = "MonoScript";

	std::vector<std::string> MonoScript::componentList =
	{
		"FalcoEngine.Transform",
		"FalcoEngine.Animation",
		"FalcoEngine.AudioListener",
		"FalcoEngine.AudioSource",
		"FalcoEngine.VideoPlayer",
		"FalcoEngine.NavMeshAgent",
		"FalcoEngine.NavMeshObstacle",
		"FalcoEngine.Spline",
		"FalcoEngine.BoxCollider",
		"FalcoEngine.CapsuleCollider",
		"FalcoEngine.MeshCollider",
		"FalcoEngine.SphereCollider",
		"FalcoEngine.TerrainCollider",
		"FalcoEngine.ConeTwistJoint",
		"FalcoEngine.FixedJoint",
		"FalcoEngine.FreeJoint",
		"FalcoEngine.HingeJoint",
		"FalcoEngine.Rigidbody",
		"FalcoEngine.Vehicle",
		"FalcoEngine.Camera",
		"FalcoEngine.Light",
		"FalcoEngine.MeshRenderer",
		"FalcoEngine.ParticleSystem",
		"FalcoEngine.Terrain",
		"FalcoEngine.Water",
		"FalcoEngine.Button",
		"FalcoEngine.Canvas",
		"FalcoEngine.Image",
		"FalcoEngine.Mask",
		"FalcoEngine.Text",
		"FalcoEngine.TextInput",
	};

	MonoScript::MonoScript() : Component(nullptr)
	{
		
	}

	MonoScript::~MonoScript()
	{
		if (Engine::getSingleton()->getIsRuntimeMode())
			APIManager::getSingleton()->execute(managedObject, "OnDestroy");
	}

	std::string MonoScript::MonoFieldInfo::serialize()
	{
		fieldValue.svec2Val = fieldValue.vec2Val;
		fieldValue.svec3Val = fieldValue.vec3Val;
		fieldValue.svec4Val = fieldValue.vec4Val;

		std::stringstream ofs;
		BinarySerializer s;
		s.serialize(&ofs, this, "");
		
		return ofs.str();
	}

	void MonoScript::MonoFieldInfo::deserialize(std::string value)
	{
		std::stringstream ifs(value);
		BinarySerializer s;
		s.deserialize(&ifs, this, "");

		fieldValue.vec2Val = fieldValue.svec2Val.getValue();
		fieldValue.vec3Val = fieldValue.svec3Val.getValue();
		fieldValue.vec4Val = fieldValue.svec4Val.getValue();
	}

	void MonoScript::setClassFromName(std::string className)
	{
		destroyManagedObject();
		managedClassName = className;
		managedClass = APIManager::getSingleton()->findClass(className);
		createManagedObject();
	}

	std::string MonoScript::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	void MonoScript::setEnabled(bool value)
	{
		bool _enabled = enabled;
		Component::setEnabled(value);

		if (getGameObject() == nullptr)
			return;

		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			if (value == true && !_enabled)
			{
				if (getGameObject()->getActive())
				{
					if (!startExecuted)
					{
						startExecuted = true;
						APIManager::getSingleton()->execute(managedObject, "Awake");
						APIManager::getSingleton()->execute(managedObject, "Start");
					}
				}
			}
		}
	}

	void MonoScript::onSceneLoaded()
	{
		sendSerializedFieldsToMono();
	}

	void MonoScript::onManagedObjectDestroyed()
	{
		startExecuted = false;
	}

	Component* MonoScript::onClone()
	{
		MonoScript* newComponent = new MonoScript();

		newComponent->enabled = enabled;
		newComponent->setClassFromName(managedClassName);
		auto fields = getFields();
		for (auto& field : fields)
			newComponent->setField(field);

		return newComponent;
	}

	void MonoScript::onRefresh()
	{
		
	}

	void MonoScript::onRebindObject(std::string oldObj, std::string newObj)
	{
		if (serializedFields.size() > 0)
		{
			for (auto& f : serializedFields)
			{
				if (f.fieldType == "FalcoEngine.GameObject" || std::find(componentList.begin(), componentList.end(), f.fieldType) != componentList.end())
				{
					if (f.fieldValue.objectVal == oldObj)
						f.fieldValue.objectVal = newObj;
				}
			}
		}
		else
		{
			for (auto& f : getFields())
			{
				if (f.fieldType == "FalcoEngine.GameObject" || std::find(componentList.begin(), componentList.end(), f.fieldType) != componentList.end())
				{
					if (f.fieldValue.objectVal == oldObj)
					{
						f.fieldValue.objectVal = newObj;
						setField(f);
					}
				}
			}
		}
	}

	void MonoScript::onStateChanged()
	{
		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			if (managedObject != nullptr)
			{
				if (enabled)
				{
					if (getGameObject()->getActive())
					{
						if (!startExecuted)
						{
							startExecuted = true;
							APIManager::getSingleton()->execute(managedObject, "Awake");
							APIManager::getSingleton()->execute(managedObject, "Start");
						}
					}
				}
			
				APIManager::getSingleton()->execute(managedObject, "OnStateChanged", nullptr, "");
			}
		}
	}

	std::string MonoScript::getClassName()
	{
		return managedClassName;
	}

	MonoScript::MonoFieldInfo* MonoScript::getField(std::string name)
	{
		auto fields = getFields();
		auto it = std::find_if(fields.begin(), fields.end(), [=](MonoScript::MonoFieldInfo& inf) -> bool
			{
				return inf.fieldName == name;
			}
		);
		
		if (it != fields.end())
		{
			MonoScript::MonoFieldInfo* fieldInfo = new MonoScript::MonoFieldInfo(*it);
			fields.clear();
			return fieldInfo;
		}

		fields.clear();

		return nullptr;
	}

	MonoScript::FieldList MonoScript::getFields()
	{
		APIManager* mgr = APIManager::getSingleton();

		FieldList list;
		
		if (managedObject == nullptr)
			return list;

		MonoClass* baseClass = mono_object_get_class(managedObject);
		
		void* it = nullptr;
		MonoClassField* field = nullptr;

		MonoClass* parent = nullptr;

		std::vector<MonoClass*> classes = { baseClass };

		parent = mono_class_get_parent(baseClass);

		while (parent != nullptr)
		{
			classes.push_back(parent);
			parent = mono_class_get_parent(parent);
		}

		for (auto _it = classes.begin(); _it != classes.end(); ++_it)
		{
			MonoClass* cls = *_it;

			it = nullptr;
			field = nullptr;

			while (field = mono_class_get_fields(cls, &it))
			{
				uint32_t flags = mono_field_get_flags(field);
				bool _static = flags & MONO_FIELD_ATTR_STATIC;

				/*
					{ Mono attributes info: }

					MONO_FIELD_ATTR_PRIVATE :: PRIVATE;
					MONO_FIELD_ATTR_FAM_AND_ASSEM :: PROTECTED_AND_INTERNAL;
					MONO_FIELD_ATTR_ASSEMBLY :: INTERNAL;
					MONO_FIELD_ATTR_FAMILY :: PROTECTED;
					MONO_FIELD_ATTR_PUBLIC :: PUBLIC;
				*/

				if ((flags & MONO_FIELD_ATTR_FIELD_ACCESS_MASK) == MONO_FIELD_ATTR_PUBLIC && !_static)
				{
					MonoType* type = mono_field_get_type(field);
					std::string field_name = mono_field_get_name(field);
					std::string field_type = mono_type_get_name(type);

					field_name = CP_SYS(field_name);
					field_type = CP_SYS(field_type);

					MonoScript::MonoFieldInfo inf;

					inf.fieldName = field_name;
					inf.fieldType = field_type;

					if (field_type == "System.Int32")
					{
						int val;
						mono_field_get_value(managedObject, field, &val);
						inf.fieldValue = MonoScript::ValueVariant(val);
					}
					if (field_type == "System.Single")
					{
						float val;
						mono_field_get_value(managedObject, field, &val);
						inf.fieldValue = MonoScript::ValueVariant(val);
					}
					if (field_type == "System.String")
					{
						MonoObject* val = mono_field_get_value_object(mgr->domain, field, managedObject);
						const char* _str = (const char*)mono_string_to_utf8((MonoString*)val);
						if (_str != nullptr)
						{
							std::string str = CP_SYS(std::string(_str));
							inf.fieldValue = MonoScript::ValueVariant(std::string(str));
						}
						else
							inf.fieldValue = MonoScript::ValueVariant(std::string(""));
					}
					if (field_type == "System.String[]")
					{
						MonoArray* val = (MonoArray*)mono_field_get_value_object(APIManager::getSingleton()->getDomain(), field, managedObject);
						std::vector<std::string> strArr;
						if (val != nullptr)
						{
							int arrSize = (int)mono_array_length(val);
							for (int j = 0; j < arrSize; ++j)
							{
								MonoString* strVal = mono_array_get(val, MonoString*, j);
								const char* _str = (const char*)mono_string_to_utf8(strVal);
								if (_str != nullptr)
									strArr.push_back(CP_SYS(_str));
								else
									strArr.push_back("");
							}
						}

						inf.fieldValue = MonoScript::ValueVariant();
						inf.fieldValue.stringArrVal = strArr;
					}
					if (field_type == "System.Boolean")
					{
						bool val = false;
						mono_field_get_value(managedObject, field, &val);
						inf.fieldValue = MonoScript::ValueVariant(val);
					}
					if (field_type == "FalcoEngine.Vector2")
					{
						MonoObject* val = mono_field_get_value_object(mgr->domain, field, managedObject);
						if (val != nullptr)
						{
							float xval = 0;
							float yval = 0;

							mono_field_get_value(val, mgr->vector2_x, &xval);
							mono_field_get_value(val, mgr->vector2_y, &yval);

							inf.fieldValue = MonoScript::ValueVariant(glm::vec2(xval, yval));
						}
					}
					if (field_type == "FalcoEngine.Vector3")
					{
						MonoObject* val = mono_field_get_value_object(mgr->domain, field, managedObject);
						if (val != nullptr)
						{
							float xval = 0;
							float yval = 0;
							float zval = 0;

							mono_field_get_value(val, mgr->vector3_x, &xval);
							mono_field_get_value(val, mgr->vector3_y, &yval);
							mono_field_get_value(val, mgr->vector3_z, &zval);

							inf.fieldValue = MonoScript::ValueVariant(glm::vec3(xval, yval, zval));
						}
					}
					if (field_type == "FalcoEngine.Vector4")
					{
						MonoObject* val = mono_field_get_value_object(mgr->domain, field, managedObject);
						if (val != nullptr)
						{
							float xval = 0;
							float yval = 0;
							float zval = 0;
							float wval = 0;

							mono_field_get_value(val, mgr->vector4_x, &xval);
							mono_field_get_value(val, mgr->vector4_y, &yval);
							mono_field_get_value(val, mgr->vector4_z, &zval);
							mono_field_get_value(val, mgr->vector4_w, &wval);

							inf.fieldValue = MonoScript::ValueVariant(glm::vec4(xval, yval, zval, wval));
						}
					}
					if (field_type == "FalcoEngine.Quaternion")
					{
						MonoObject* val = mono_field_get_value_object(mgr->domain, field, managedObject);
						if (val != nullptr)
						{
							float xval = 0;
							float yval = 0;
							float zval = 0;
							float wval = 0;

							mono_field_get_value(val, mgr->quaternion_x, &xval);
							mono_field_get_value(val, mgr->quaternion_y, &yval);
							mono_field_get_value(val, mgr->quaternion_z, &zval);
							mono_field_get_value(val, mgr->quaternion_w, &wval);

							inf.fieldValue = MonoScript::ValueVariant(glm::vec4(xval, yval, zval, wval));
						}
					}
					if (field_type == "FalcoEngine.Color")
					{
						MonoObject* val = mono_field_get_value_object(mgr->domain, field, managedObject);
						if (val != nullptr)
						{
							float rval = 0;
							float gval = 0;
							float bval = 0;
							float aval = 0;

							mono_field_get_value(val, mgr->color_r, &rval);
							mono_field_get_value(val, mgr->color_g, &gval);
							mono_field_get_value(val, mgr->color_b, &bval);
							mono_field_get_value(val, mgr->color_a, &aval);

							inf.fieldValue = MonoScript::ValueVariant(glm::vec4(rval, gval, bval, aval));
						}
					}
					if (field_type == "FalcoEngine.GameObject")
					{
						MonoObject* val = mono_field_get_value_object(mgr->domain, field, managedObject);
						if (val != nullptr)
						{
							GameObject* objNode = nullptr;
							mono_field_get_value(val, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&objNode));
							if (objNode != nullptr)
							{
								MonoScript::ValueVariant _v;
								_v.objectVal = objNode->getGuid();
								inf.fieldValue = _v;
							}
						}
					}
					if (field_type == "FalcoEngine.AnimationClip"
						|| field_type == "FalcoEngine.AudioClip"
						|| field_type == "FalcoEngine.VideoClip"
						|| field_type == "FalcoEngine.Cubemap"
						|| field_type == "FalcoEngine.Font"
						|| field_type == "FalcoEngine.Material"
						|| field_type == "FalcoEngine.Prefab"
						|| field_type == "FalcoEngine.Shader"
						|| field_type == "FalcoEngine.Texture")
					{
						MonoObject* val = mono_field_get_value_object(mgr->domain, field, managedObject);
						if (val != nullptr)
						{
							Asset* asset = nullptr;
							mono_field_get_value(val, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&asset));
							if (asset != nullptr)
							{
								MonoScript::ValueVariant _v;
								_v.objectVal = asset->getName();
								inf.fieldValue = _v;
							}
						}
					}
					if (std::find(componentList.begin(), componentList.end(), field_type) != componentList.end())
					{
						MonoObject* val = mono_field_get_value_object(mgr->domain, field, managedObject);
						if (val != nullptr)
						{
							Component* comp = nullptr;
							mono_field_get_value(val, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&comp));
							if (comp != nullptr)
							{
								MonoScript::ValueVariant _v;
								_v.objectVal = comp->getGameObject()->getGuid();
								inf.fieldValue = _v;
							}
						}
					}

					MonoClass* _classType = mono_class_from_name(APIManager::getSingleton()->getImage(), "", CP_UNI(field_type).c_str());
					if (_classType != nullptr)
					{
						if (mono_class_is_subclass_of(_classType, APIManager::getSingleton()->behaviour_class, false))
						{
							MonoObject* val = mono_field_get_value_object(mgr->domain, field, managedObject);
							if (val != nullptr)
							{
								Component* comp = nullptr;
								mono_field_get_value(val, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&comp));
								if (comp != nullptr)
								{
									MonoScript::ValueVariant _v;
									_v.objectVal = comp->getGameObject()->getGuid();
									inf.fieldValue = _v;
								}
							}
						}
					}

					list.push_back(inf);
				}
			}
		}

		return list;
	}

	void MonoScript::sendSerializedFieldsToMono()
	{
		for (auto it = serializedFields.begin(); it != serializedFields.end(); ++it)
			setField(*it);

		serializedFields.clear();
	}

	void MonoScript::setField(MonoFieldInfo inf)
	{
		if (managedObject == nullptr)
			return;

		APIManager* mgr = APIManager::getSingleton();

		MonoScript::ValueVariant val = inf.fieldValue;
		MonoClass* _class = mono_object_get_class((MonoObject*)managedObject);

		if (_class != nullptr)
		{
			MonoClassField* _fld = mono_class_get_field_from_name(_class, CP_UNI(inf.fieldName).c_str());
			
			if (_fld == nullptr)
				return;

			MonoType* _fldType = mono_field_get_type(_fld);
			std::string _fldTypeName = mono_type_get_name(_fldType);

			if (inf.fieldType != _fldTypeName)
				return;

			uint32_t flags = mono_field_get_flags(_fld);
			bool _static = flags & MONO_FIELD_ATTR_STATIC;

			if ((flags & MONO_FIELD_ATTR_FIELD_ACCESS_MASK) == MONO_FIELD_ATTR_PUBLIC && !_static)
			{
				if (inf.fieldType == "System.Int32")
				{
					mono_field_set_value((MonoObject*)managedObject, _fld, &val.intVal);
				}
				if (inf.fieldType == "System.Single")
				{
					mono_field_set_value((MonoObject*)managedObject, _fld, &val.floatVal);
				}
				if (inf.fieldType == "System.Boolean")
				{
					mono_field_set_value((MonoObject*)managedObject, _fld, &val.boolVal);
				}
				if (inf.fieldType == "System.String")
				{
					MonoString* _str = mono_string_new(mgr->domain, CP_UNI(val.stringVal).c_str());
					mono_field_set_value((MonoObject*)managedObject, _fld, _str);
				}
				if (inf.fieldType == "System.String[]")
				{
					MonoArray* strArr = mono_array_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->string_class, val.stringArrVal.size());
					for (int j = 0; j < val.stringArrVal.size(); ++j)
					{
						MonoString* _str = mono_string_new(mgr->domain, CP_UNI(val.stringArrVal[j]).c_str());
						mono_array_set(strArr, MonoString*, j, _str);
					}
					
					mono_field_set_value((MonoObject*)managedObject, _fld, strArr);
				}
				if (inf.fieldType == "FalcoEngine.Vector2")
				{
					MonoObject* vec2 = mono_object_new(mgr->domain, mgr->vector2_class);
					mono_field_set_value(vec2, mgr->vector2_x, &inf.fieldValue.vec2Val.x);
					mono_field_set_value(vec2, mgr->vector2_y, &inf.fieldValue.vec2Val.y);

					void* _vec2 = mono_object_unbox(vec2);
					mono_field_set_value((MonoObject*)managedObject, _fld, _vec2);
				}
				if (inf.fieldType == "FalcoEngine.Vector3")
				{
					MonoObject* vec3 = mono_object_new(mgr->domain, mgr->vector3_class);
					mono_field_set_value(vec3, mgr->vector3_x, &inf.fieldValue.vec3Val.x);
					mono_field_set_value(vec3, mgr->vector3_y, &inf.fieldValue.vec3Val.y);
					mono_field_set_value(vec3, mgr->vector3_z, &inf.fieldValue.vec3Val.z);

					void* _vec3 = mono_object_unbox(vec3);
					mono_field_set_value((MonoObject*)managedObject, _fld, _vec3);
				}
				if (inf.fieldType == "FalcoEngine.Vector4")
				{
					MonoObject* vec4 = mono_object_new(mgr->domain, mgr->vector4_class);
					mono_field_set_value(vec4, mgr->vector4_x, &inf.fieldValue.vec4Val.x);
					mono_field_set_value(vec4, mgr->vector4_y, &inf.fieldValue.vec4Val.y);
					mono_field_set_value(vec4, mgr->vector4_z, &inf.fieldValue.vec4Val.z);
					mono_field_set_value(vec4, mgr->vector4_w, &inf.fieldValue.vec4Val.w);

					void* _vec4 = mono_object_unbox(vec4);
					mono_field_set_value((MonoObject*)managedObject, _fld, _vec4);
				}
				if (inf.fieldType == "FalcoEngine.Quaternion")
				{
					MonoObject* vec4 = mono_object_new(mgr->domain, mgr->quaternion_class);
					mono_field_set_value(vec4, mgr->quaternion_x, &inf.fieldValue.vec4Val.x);
					mono_field_set_value(vec4, mgr->quaternion_y, &inf.fieldValue.vec4Val.y);
					mono_field_set_value(vec4, mgr->quaternion_z, &inf.fieldValue.vec4Val.z);
					mono_field_set_value(vec4, mgr->quaternion_w, &inf.fieldValue.vec4Val.w);

					void* _vec4 = mono_object_unbox(vec4);
					mono_field_set_value((MonoObject*)managedObject, _fld, _vec4);
				}
				if (inf.fieldType == "FalcoEngine.Color")
				{
					MonoObject* color = mono_object_new(mgr->domain, mgr->color_class);
					mono_field_set_value(color, mgr->color_r, &inf.fieldValue.vec4Val.x);
					mono_field_set_value(color, mgr->color_g, &inf.fieldValue.vec4Val.y);
					mono_field_set_value(color, mgr->color_b, &inf.fieldValue.vec4Val.z);
					mono_field_set_value(color, mgr->color_a, &inf.fieldValue.vec4Val.w);

					void* _color = mono_object_unbox(color);
					mono_field_set_value((MonoObject*)managedObject, _fld, _color);
				}
				if (inf.fieldType == "FalcoEngine.GameObject")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							if (node->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, node->getManagedObject());
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				//Assets
				if (inf.fieldType == "FalcoEngine.AnimationClip")
				{
					if (val.objectVal != "[None]")
					{
						AnimationClip* asset = AnimationClip::load(Engine::getSingleton()->getAssetsPath(), val.objectVal);
						if (asset != nullptr)
						{
							if (asset->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, asset->getManagedObject());
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.AudioClip")
				{
					if (val.objectVal != "[None]")
					{
						AudioClip* asset = AudioClip::load(Engine::getSingleton()->getAssetsPath(), val.objectVal);
						if (asset != nullptr)
						{
							if (asset->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, asset->getManagedObject());
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.VideoClip")
				{
					if (val.objectVal != "[None]")
					{
						VideoClip* asset = VideoClip::load(Engine::getSingleton()->getAssetsPath(), val.objectVal);
						if (asset != nullptr)
						{
							if (asset->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, asset->getManagedObject());
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Cubemap")
				{
					if (val.objectVal != "[None]")
					{
						Cubemap* asset = Cubemap::load(Engine::getSingleton()->getAssetsPath(), val.objectVal);
						if (asset != nullptr)
						{
							if (asset->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, asset->getManagedObject());
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Font")
				{
					if (val.objectVal != "[None]")
					{
						Font* asset = Font::load(Engine::getSingleton()->getAssetsPath(), val.objectVal);
						if (asset != nullptr)
						{
							if (asset->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, asset->getManagedObject());
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Material")
				{
					if (val.objectVal != "[None]")
					{
						Material* asset = Material::load(Engine::getSingleton()->getAssetsPath(), val.objectVal);
						if (asset != nullptr)
						{
							if (asset->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, asset->getManagedObject());
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Prefab")
				{
					if (val.objectVal != "[None]")
					{
						Prefab* asset = Prefab::load(Engine::getSingleton()->getAssetsPath(), val.objectVal);
						if (asset != nullptr)
						{
							if (asset->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, asset->getManagedObject());
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Shader")
				{
					if (val.objectVal != "[None]")
					{
						Shader* asset = Shader::load(Engine::getSingleton()->getAssetsPath(), val.objectVal);
						if (asset != nullptr)
						{
							if (asset->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, asset->getManagedObject());
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Texture")
				{
					if (val.objectVal != "[None]")
					{
						Texture* asset = Texture::load(Engine::getSingleton()->getAssetsPath(), val.objectVal, true, Texture::CompressionMethod::Default, false, false);
						if (asset != nullptr)
						{
							if (asset->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, asset->getManagedObject());
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				//Components
				if (inf.fieldType == "FalcoEngine.Transform")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Transform::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Animation")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Animation::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.AudioListener")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(AudioListener::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.AudioSource")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(AudioSource::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.VideoPlayer")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(VideoPlayer::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.NavMeshAgent")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(NavMeshAgent::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.NavMeshObstacle")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(NavMeshObstacle::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Spline")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Spline::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.BoxCollider")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(BoxCollider::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.CapsuleCollider")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(CapsuleCollider::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.MeshCollider")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(MeshCollider::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.SphereCollider")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(SphereCollider::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.TerrainCollider")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(TerrainCollider::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.ConeTwistJoint")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(ConeTwistJoint::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.FixedJoint")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(FixedJoint::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.FreeJoint")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(FreeJoint::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.HingeJoint")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(HingeJoint::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Rigidbody")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(RigidBody::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Vehicle")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Vehicle::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Camera")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Camera::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Light")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Light::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.MeshRenderer")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(MeshRenderer::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.ParticleSystem")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(ParticleSystem::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Terrain")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Terrain::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Water")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Water::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Button")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Button::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Canvas")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Canvas::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Image")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Image::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Mask")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Mask::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.Text")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(Text::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}
				if (inf.fieldType == "FalcoEngine.TextInput")
				{
					if (val.objectVal != "[None]")
					{
						GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
						if (node != nullptr)
						{
							Component* comp = node->getComponent(TextInput::COMPONENT_TYPE);
							if (comp != nullptr && comp->getManagedObject() != nullptr)
								mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
					else
						mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
				}

				MonoClass* _classType = mono_class_from_name(APIManager::getSingleton()->getImage(), "", CP_UNI(inf.fieldType).c_str());
				if (_classType != nullptr)
				{
					if (mono_class_is_subclass_of(_classType, APIManager::getSingleton()->behaviour_class, false))
					{
						if (val.objectVal != "[None]")
						{
							GameObject* node = Engine::getSingleton()->getGameObject(val.objectVal);
							if (node != nullptr)
							{
								Component* comp = node->getComponent(inf.fieldType);
								if (comp != nullptr && comp->getManagedObject() != nullptr)
									mono_field_set_value((MonoObject*)managedObject, _fld, comp->getManagedObject());
								else
									mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
							}
							else
								mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
						}
						else
							mono_field_set_value((MonoObject*)managedObject, _fld, nullptr);
					}
				}
			}
		}
	}
}