#include "API_GameObject.h"

#include <string>

#include "../../Mono/include/mono/metadata/reflection.h"

#include "../Core/Engine.h"
#include "../Core/GameObject.h"
#include "../Core/APIManager.h"

#include "../Components/Transform.h"
#include "../Components/MonoScript.h"
#include "../Components/Animation.h"
#include "../Components/AudioListener.h"
#include "../Components/AudioSource.h"
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

#include "../Classes/StringConverter.h"
#include "../Serialization/Settings/ProjectSettings.h"

namespace GX
{
	void API_GameObject::ctor(MonoObject* this_ptr)
	{
		GameObject* obj = Engine::getSingleton()->createGameObject();
		obj->setName("New Game Object");
		obj->setManagedObject(this_ptr);
	}

	MonoString * API_GameObject::getName(MonoObject * this_ptr)
	{
		GameObject* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		std::string _str = CP_UNI(node->getName());
		MonoString * str = mono_string_new(APIManager::getSingleton()->getDomain(), _str.c_str());

		return str;
	}

	MonoString * API_GameObject::getUniqueName(MonoObject* this_ptr)
	{
		GameObject* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		std::string _str = CP_UNI(node->getGuid());
		MonoString* str = mono_string_new(APIManager::getSingleton()->getDomain(), _str.c_str());

		return str;
	}

	MonoString* API_GameObject::getTag(MonoObject* this_ptr)
	{
		GameObject* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		auto tags = Engine::getSingleton()->getSettings()->getTags();
		tags.insert(tags.begin(), "Untagged");

		std::string _tag = CP_UNI("Untagged");

		if (node->getTag() >= 0 && node->getTag() < tags.size())
			_tag = CP_UNI(tags[node->getTag()]);

		MonoString* str = mono_string_new(APIManager::getSingleton()->getDomain(), _tag.c_str());

		return str;
	}

	void API_GameObject::setTag(MonoObject* this_ptr, MonoString* value)
	{
		GameObject* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		std::string _tag = (const char*)mono_string_to_utf8((MonoString*)value);
		_tag = CP_SYS(_tag);

		auto tags = Engine::getSingleton()->getSettings()->getTags();
		tags.insert(tags.begin(), "Untagged");

		auto it = find(tags.begin(), tags.end(), _tag);

		if (it != tags.end())
		{
			int tag = (int)distance(tags.begin(), it);
			node->setTag(tag);
		}
	}

	int API_GameObject::getLayer(MonoObject* this_ptr)
	{
		GameObject* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		return node->getLayer();
	}

	void API_GameObject::setLayer(MonoObject* this_ptr, int value)
	{
		GameObject* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		node->setLayer(value);
	}

	bool API_GameObject::getEnabled(MonoObject * this_ptr)
	{
		GameObject * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		return node->getEnabled();
	}

	void API_GameObject::setEnabled(MonoObject * this_ptr, bool visible)
	{
		GameObject * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		if (node->getEnabled() != visible)
		{
			bool prevVisible = node->getEnabled();
			node->setEnabled(visible);

			if (prevVisible == false && visible && (GameObject*)node->getActive())
			{
				auto scripts = node->getMonoScripts();
				for (auto it = scripts.begin(); it != scripts.end(); ++it)
				{
					if ((*it)->getEnabled())
					{
						if (!(*it)->startExecuted)
						{
							(*it)->startExecuted = true;
							APIManager::getSingleton()->execute((MonoObject*)(*it)->getManagedObject(), "Start");
						}
					}
				}
			}
		}
	}

	bool API_GameObject::getActive(MonoObject* this_ptr)
	{
		GameObject* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		return node->getActive();
	}

	MonoObject * API_GameObject::getTransform(MonoObject * this_ptr)
	{
		GameObject * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			Transform* transform = node->getTransform();
			if (transform != nullptr)
				return transform->getManagedObject();
		}

		return nullptr;
	}

	MonoObject * API_GameObject::getRigidbody(MonoObject * this_ptr)
	{
		GameObject * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			RigidBody* body = ((RigidBody*)node->getComponent(RigidBody::COMPONENT_TYPE));
			if (body != nullptr)
				return body->getManagedObject();
		}

		return nullptr;
	}

	MonoObject * API_GameObject::getAnimation(MonoObject * this_ptr)
	{
		GameObject * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			Animation* animList = ((Animation*)node->getComponent(Animation::COMPONENT_TYPE));
			if (animList != nullptr)
				return animList->getManagedObject();
		}

		return nullptr;
	}

	MonoObject * API_GameObject::getAudioSource(MonoObject * this_ptr)
	{
		GameObject * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			AudioSource* audio = ((AudioSource*)node->getComponent(AudioSource::COMPONENT_TYPE));
			if (audio != nullptr)
				return audio->getManagedObject();
		}

		return nullptr;
	}

	MonoObject * API_GameObject::getNavMeshAgent(MonoObject * this_ptr)
	{
		GameObject * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			NavMeshAgent* agent = ((NavMeshAgent*)node->getComponent(NavMeshAgent::COMPONENT_TYPE));
			if (agent != nullptr)
				return agent->getManagedObject();
		}

		return nullptr;
	}

	MonoArray* API_GameObject::getComponents(MonoObject* this_ptr)
	{
		GameObject* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			std::vector<Component*>& components = node->getComponents();

			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->component_class, components.size());
			for (int i = 0; i < components.size(); ++i)
				mono_array_setref(arr, i, components[i]->managedObject);

			return arr;
		}

		return nullptr;
	}

	MonoObject * API_GameObject::findGameObject(MonoObject * name)
	{
		std::string _name = (const char*)mono_string_to_utf8((MonoString*)name);
		_name = CP_SYS(_name);

		GameObject * node = Engine::getSingleton()->findGameObject(_name);
		if (node != nullptr)
			return node->getManagedObject();

		return nullptr;
	}

	MonoObject * API_GameObject::getGameObject(MonoObject* name)
	{
		std::string _name = (const char*)mono_string_to_utf8((MonoString*)name);
		_name = CP_SYS(_name);

		GameObject* node = Engine::getSingleton()->getGameObject(_name);
		return node->getManagedObject();

		return nullptr;
	}

	MonoObject * API_GameObject::getMonoBehaviour(MonoObject * this_ptr, MonoObject * className)
	{
		GameObject * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		std::string _className = (const char*)mono_string_to_utf8((MonoString*)className);
		_className = CP_SYS(_className);
		MonoClass* _class = APIManager::getSingleton()->findClass(_className);

		MonoObject* result = nullptr;
		if (_class != nullptr)
		{
			auto scripts = node->getMonoScripts();
			for (auto it = scripts.begin(); it != scripts.end(); ++it)
			{
				if ((*it)->getManagedClass() == _class)
				{
					result = (*it)->getManagedObject();
					break;
				}
			}
		}

		return result;
	}

	MonoObject * API_GameObject::getMonoBehaviourT(MonoObject * this_ptr, MonoReflectionType * type)
	{
		GameObject * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		MonoType * _type = mono_reflection_type_get_type(type);
		MonoClass * _class = mono_type_get_class(_type);
		
		MonoObject* result = nullptr;
		if (_class != nullptr)
		{
			auto scripts = node->getMonoScripts();
			for (auto it = scripts.begin(); it != scripts.end(); ++it)
			{
				if ((*it)->getManagedClass() == _class)
				{
					result = (*it)->getManagedObject();
					break;
				}
			}
		}

		return result;
	}

	MonoObject * API_GameObject::getComponentT(MonoObject * this_ptr, MonoReflectionType * type)
	{
		GameObject * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		MonoType * _type = mono_reflection_type_get_type(type);
		MonoClass * _class = mono_type_get_class(_type);
		std::string _className = mono_class_get_name(_class);
		_className = CP_SYS(_className);

		Component * component = node->getComponent(_className);
		if (component != nullptr)
			return component->getManagedObject();

		return nullptr;
	}

	MonoObject* API_GameObject::addComponentT(MonoObject* this_ptr, MonoReflectionType* type)
	{
		GameObject* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		MonoType* _type = mono_reflection_type_get_type(type);
		MonoClass* _class = mono_type_get_class(_type);
		std::string _className = mono_class_get_name(_class);
		_className = CP_SYS(_className);

		Component* component = nullptr;

		if (_className == "Animation")
		{
			Component* _comp = node->getComponent(Animation::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				Animation* comp = new Animation();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "AudioListener")
		{
			Component* _comp = node->getComponent(AudioListener::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				AudioListener* comp = new AudioListener();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "AudioSource")
		{
			AudioSource* comp = new AudioSource();
			node->addComponent(comp);

			component = comp;
		}

		if (_className == "NavMeshAgent")
		{
			Component* _comp = node->getComponent(NavMeshAgent::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				NavMeshAgent* comp = new NavMeshAgent();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "NavMeshObstacle")
		{
			NavMeshObstacle* comp = new NavMeshObstacle();
			node->addComponent(comp);

			component = comp;
		}

		if (_className == "Spline")
		{
			Component* _comp = node->getComponent(Spline::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				Spline* comp = new Spline();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "BoxCollider")
		{
			BoxCollider* comp = new BoxCollider();
			node->addComponent(comp);

			component = comp;
		}

		if (_className == "CapsuleCollider")
		{
			CapsuleCollider* comp = new CapsuleCollider();
			node->addComponent(comp);

			component = comp;
		}

		if (_className == "SphereCollider")
		{
			SphereCollider* comp = new SphereCollider();
			node->addComponent(comp);

			component = comp;
		}

		if (_className == "MeshCollider")
		{
			Component* _comp = node->getComponent(MeshCollider::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				MeshCollider* comp = new MeshCollider();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "TerrainCollider")
		{
			Component* _comp = node->getComponent(TerrainCollider::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				TerrainCollider* comp = new TerrainCollider();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "ConeTwistJoint")
		{
			Component* _comp = node->getComponent(ConeTwistJoint::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				ConeTwistJoint* comp = new ConeTwistJoint();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "FixedJoint")
		{
			Component* _comp = node->getComponent(FixedJoint::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				FixedJoint* comp = new FixedJoint();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "FreeJoint")
		{
			Component* _comp = node->getComponent(FreeJoint::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				FreeJoint* comp = new FreeJoint();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "HingeJoint")
		{
			Component* _comp = node->getComponent(HingeJoint::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				HingeJoint* comp = new HingeJoint();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "Rigidbody")
		{
			Component* _comp = node->getComponent(RigidBody::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				RigidBody* comp = new RigidBody();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "Vehicle")
		{
			Component* _comp = node->getComponent(Vehicle::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				Vehicle* comp = new Vehicle();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "Camera")
		{
			Component* _comp = node->getComponent(Camera::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				Camera* comp = new Camera();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "Light")
		{
			Component* _comp = node->getComponent(Light::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				Light* comp = new Light();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "MeshRenderer")
		{
			Component* _comp = node->getComponent(MeshRenderer::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				MeshRenderer* comp = new MeshRenderer();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "ParticleSystem")
		{
			Component* _comp = node->getComponent(ParticleSystem::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				ParticleSystem* comp = new ParticleSystem();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "Terrain")
		{
			Component* _comp = node->getComponent(Terrain::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				Terrain* comp = new Terrain();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "Water")
		{
			Component* _comp = node->getComponent(Water::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				Water* comp = new Water();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "Button")
		{
			Component* _comp = node->getComponent(Button::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				Button* comp = new Button();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "Canvas")
		{
			Component* _comp = node->getComponent(Canvas::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				Canvas* comp = new Canvas();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "Image")
		{
			Component* _comp = node->getComponent(Image::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				Image* comp = new Image();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "Mask")
		{
			Component* _comp = node->getComponent(Mask::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				Mask* comp = new Mask();
				node->addComponent(comp);

				component = comp;
			}
		}

		if (_className == "Text")
		{
			Component* _comp = node->getComponent(Text::COMPONENT_TYPE);
			if (_comp != nullptr)
			{
				component = _comp;
			}
			else
			{
				Text* comp = new Text();
				node->addComponent(comp);

				component = comp;
			}
		}

		MonoClass* _classType = mono_class_from_name(APIManager::getSingleton()->getImage(), "", CP_UNI(_className).c_str());
		if (_classType != nullptr)
		{
			if (mono_class_is_subclass_of(_classType, APIManager::getSingleton()->behaviour_class, false))
			{
				MonoScript* script = new MonoScript();
				script->setClassFromName(_className);
				node->addComponent(script);

				component = script;
			}
		}

		if (component != nullptr)
			return component->managedObject;

		return nullptr;
	}

	MonoObject* API_GameObject::clone(MonoObject* this_ptr)
	{
		GameObject* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		GameObject* obj = node->clone();

		return obj->getManagedObject();
	}
}