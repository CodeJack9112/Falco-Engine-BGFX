#include "MonoScriptEditor.h"

#include <boost/algorithm/string.hpp>

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/APIManager.h"
#include "../Engine/Core/GameObject.h"

#include "../Engine/Components/MonoScript.h"

#include "../Engine/Assets/AudioClip.h"
#include "../Engine/Assets/VideoClip.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Assets/Prefab.h"
#include "../Engine/Assets/Shader.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/AnimationClip.h"
#include "../Engine/Assets/Cubemap.h"
#include "../Engine/Assets/Font.h"

#include "../Engine/Math/Mathf.h"
#include "../Engine/Classes/StringConverter.h"

#include "Property.h"
#include "PropVector2.h"
#include "PropVector3.h"
#include "PropVector4.h"
#include "PropInt.h"
#include "PropFloat.h"
#include "PropString.h"
#include "PropBool.h"
#include "PropGameObject.h"
#include "PropAsset.h"
#include "PropColorPicker.h"

#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"
#include "../Classes/Undo.h"

namespace GX
{
	MonoScriptEditor::MonoScriptEditor()
	{
		setEditorName("MonoScriptEditor");
	}

	MonoScriptEditor::~MonoScriptEditor()
	{
	}

	void MonoScriptEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		MonoScript* script = (MonoScript*)comps[0];

		componentsIcons["FalcoEngine.Transform"] = "Inspector/transform.png";
		componentsIcons["FalcoEngine.Animation"] = "Inspector/animation.png";
		componentsIcons["FalcoEngine.AudioListener"] = "Inspector/audio_listener.png";
		componentsIcons["FalcoEngine.AudioSource"] = "Inspector/audio_source.png";
		componentsIcons["FalcoEngine.VideoPlayer"] = "Inspector/video_player.png";
		componentsIcons["FalcoEngine.NavMeshAgent"] = "Inspector/navmesh_agent.png";
		componentsIcons["FalcoEngine.NavMeshObstacle"] = "Inspector/navmesh_obstacle.png";
		componentsIcons["FalcoEngine.Spline"] = "Inspector/spline.png";
		componentsIcons["FalcoEngine.BoxCollider"] = "Inspector/box_collider.png";
		componentsIcons["FalcoEngine.CapsuleCollider"] = "Inspector/capsule_collider.png";
		componentsIcons["FalcoEngine.MeshCollider"] = "Inspector/mesh_collider.png";
		componentsIcons["FalcoEngine.SphereCollider"] = "Inspector/sphere_collider.png";
		componentsIcons["FalcoEngine.TerrainCollider"] = "Inspector/terrain_collider.png";
		componentsIcons["FalcoEngine.ConeTwistJoint"] = "Inspector/fixed_joint.png";
		componentsIcons["FalcoEngine.FixedJoint"] = "Inspector/fixed_joint.png";
		componentsIcons["FalcoEngine.FreeJoint"] = "Inspector/free_joint.png";
		componentsIcons["FalcoEngine.HingeJoint"] = "Inspector/hinge_joint.png";
		componentsIcons["FalcoEngine.Rigidbody"] = "Inspector/rigidbody.png";
		componentsIcons["FalcoEngine.Vehicle"] = "Inspector/vehicle.png";
		componentsIcons["FalcoEngine.Camera"] = "Toolbar/camera.png";
		componentsIcons["FalcoEngine.Light"] = "Hierarchy/light.png";
		componentsIcons["FalcoEngine.MeshRenderer"] = "Hierarchy/mesh_renderer.png";
		componentsIcons["FalcoEngine.ParticleSystem"] = "Inspector/particle_system.png";
		componentsIcons["FalcoEngine.Terrain"] = "Inspector/terrain.png";
		componentsIcons["FalcoEngine.Water"] = "Inspector/water.png";
		componentsIcons["FalcoEngine.Button"] = "Hierarchy/button.png";
		componentsIcons["FalcoEngine.Canvas"] = "Hierarchy/canvas.png";
		componentsIcons["FalcoEngine.Image"] = "Hierarchy/image.png";
		componentsIcons["FalcoEngine.Mask"] = "Hierarchy/mask.png";
		componentsIcons["FalcoEngine.Text"] = "Hierarchy/text.png";
		componentsIcons["FalcoEngine.TextInput"] = "Hierarchy/text_input.png";

		//List fields
		MonoScript::FieldList fields = script->getFields();

		for (auto f = fields.begin(); f != fields.end(); ++f)
		{
			Property* fieldProp = nullptr;

			if (f->fieldType == "System.Int32")
			{
				fieldProp = new PropInt(this, CP_UNI(f->fieldName), f->fieldValue.intVal);
				((PropInt*)fieldProp)->setOnChangeCallback([=](Property* prop, int val) { onChangeInt(prop, val); });
			}

			if (f->fieldType == "System.Single")
			{
				fieldProp = new PropFloat(this, CP_UNI(f->fieldName), f->fieldValue.floatVal);
				((PropFloat*)fieldProp)->setOnChangeCallback([=](Property* prop, float val) { onChangeFloat(prop, val); });
			}

			if (f->fieldType == "System.Boolean")
			{
				fieldProp = new PropBool(this, CP_UNI(f->fieldName), f->fieldValue.boolVal);
				((PropBool*)fieldProp)->setOnChangeCallback([=](Property* prop, bool val) { onChangeBool(prop, val); });
			}

			if (f->fieldType == "FalcoEngine.Vector2")
			{
				fieldProp = new PropVector2(this, CP_UNI(f->fieldName), f->fieldValue.vec2Val);
				((PropVector2*)fieldProp)->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeVector2(prop, val); });
			}

			if (f->fieldType == "FalcoEngine.Vector3")
			{
				fieldProp = new PropVector3(this, CP_UNI(f->fieldName), f->fieldValue.vec3Val);
				((PropVector3*)fieldProp)->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeVector3(prop, val); });
			}

			if (f->fieldType == "FalcoEngine.Vector4")
			{
				fieldProp = new PropVector4(this, CP_UNI(f->fieldName), f->fieldValue.vec4Val);
				((PropVector4*)fieldProp)->setOnChangeCallback([=](Property* prop, glm::vec4 val) { onChangeVector4(prop, val); });
			}

			if (f->fieldType == "FalcoEngine.Quaternion")
			{
				fieldProp = new PropVector4(this, CP_UNI(f->fieldName), f->fieldValue.vec4Val);
				((PropVector4*)fieldProp)->setOnChangeCallback([=](Property* prop, glm::vec4 val) { onChangeVector4(prop, val); });
			}

			if (f->fieldType == "FalcoEngine.Color")
			{
				glm::vec4 vec4 = f->fieldValue.vec4Val;
				Color color = Color(vec4.x, vec4.y, vec4.z, vec4.w);
				fieldProp = new PropColorPicker(this, CP_UNI(f->fieldName), color);
				((PropColorPicker*)fieldProp)->setOnChangeCallback([=](Property* prop, Color val) { onChangeColor(prop, val); });
			}

			if (f->fieldType == "System.String")
			{
				fieldProp = new PropString(this, CP_UNI(f->fieldName), CP_UNI(f->fieldValue.stringVal));
				((PropString*)fieldProp)->setOnChangeCallback([=](Property* prop, std::string val) { onChangeString(prop, val); });
			}

			if (f->fieldType == "System.String[]")
			{
				int arrSize = f->fieldValue.stringArrVal.size();

				fieldProp = new Property(this, f->fieldName + " (" + std::to_string(arrSize) + ")");

				PropInt* strArrSize = new PropInt(this, "Size", arrSize);
				strArrSize->setOnChangeCallback([=](Property* prop, int val) { onChangeStringArraySize(prop, val); });
				strArrSize->setIsDraggable(false);
				fieldProp->addChild(strArrSize);

				for (int j = 0; j < f->fieldValue.stringArrVal.size(); ++j)
				{
					PropString* strProp = new PropString(this, std::to_string(j), CP_UNI(f->fieldValue.stringArrVal[j]));
					strProp->setOnChangeCallback([=](Property* prop, std::string val) { onChangeStringArray(prop, val, j); });
					fieldProp->addChild(strProp);
				}
			}

			if (f->fieldType == "FalcoEngine.GameObject")
			{
				GameObject* node = Engine::getSingleton()->getGameObject(f->fieldValue.objectVal);
				fieldProp = new PropGameObject(this, CP_UNI(f->fieldName), node);

				fieldProp->setSupportedFormats({ "::GameObject" });
				fieldProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropGameObject(prop, from); });
				((PropGameObject*)fieldProp)->setImage(MainWindow::loadEditorIcon("Hierarchy/empty.png"));
			}

			//Assets
			if (f->fieldType == "FalcoEngine.AudioClip")
			{
				AudioClip* asset = AudioClip::load(Engine::getSingleton()->getAssetsPath(), f->fieldValue.objectVal, false);
				fieldProp = new PropAsset(this, CP_UNI(f->fieldName), asset);

				fieldProp->setSupportedFormats(Engine::getAudioFileFormats());
				fieldProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropAudioClip(prop, from); });
			}

			if (f->fieldType == "FalcoEngine.VideoClip")
			{
				VideoClip* asset = VideoClip::load(Engine::getSingleton()->getAssetsPath(), f->fieldValue.objectVal, false);
				fieldProp = new PropAsset(this, CP_UNI(f->fieldName), asset);

				fieldProp->setSupportedFormats(Engine::getVideoFileFormats());
				fieldProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropVideoClip(prop, from); });
			}

			if (f->fieldType == "FalcoEngine.Material")
			{
				Material* asset = Material::load(Engine::getSingleton()->getAssetsPath(), f->fieldValue.objectVal);
				fieldProp = new PropAsset(this, CP_UNI(f->fieldName), asset);

				fieldProp->setSupportedFormats({ "material" });
				fieldProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropMaterial(prop, from); });
			}

			if (f->fieldType == "FalcoEngine.Prefab")
			{
				Prefab* asset = Prefab::load(Engine::getSingleton()->getAssetsPath(), f->fieldValue.objectVal, false);
				fieldProp = new PropAsset(this, CP_UNI(f->fieldName), asset);

				fieldProp->setSupportedFormats({ "prefab" });
				fieldProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropPrefab(prop, from); });
			}

			if (f->fieldType == "FalcoEngine.Shader")
			{
				Shader* asset = Shader::load(Engine::getSingleton()->getAssetsPath(), f->fieldValue.objectVal);
				fieldProp = new PropAsset(this, CP_UNI(f->fieldName), asset);

				fieldProp->setSupportedFormats({ "shader" });
				fieldProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropShader(prop, from); });
			}

			if (f->fieldType == "FalcoEngine.Texture")
			{
				Texture* asset = Texture::load(Engine::getSingleton()->getAssetsPath(), f->fieldValue.objectVal, true, Texture::CompressionMethod::Default, false, false);
				fieldProp = new PropAsset(this, CP_UNI(f->fieldName), asset);

				fieldProp->setSupportedFormats(Engine::getImageFileFormats());
				fieldProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropTexture(prop, from); });
			}

			if (f->fieldType == "FalcoEngine.AnimationClip")
			{
				AnimationClip* asset = AnimationClip::load(Engine::getSingleton()->getAssetsPath(), f->fieldValue.objectVal);
				fieldProp = new PropAsset(this, CP_UNI(f->fieldName), asset);

				fieldProp->setSupportedFormats({ "animation" });
				fieldProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropAnimationClip(prop, from); });
			}

			if (f->fieldType == "FalcoEngine.Cubemap")
			{
				Cubemap* asset = Cubemap::load(Engine::getSingleton()->getAssetsPath(), f->fieldValue.objectVal);
				fieldProp = new PropAsset(this, CP_UNI(f->fieldName), asset);

				fieldProp->setSupportedFormats({ "cubemap" });
				fieldProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropCubemap(prop, from); });
			}

			if (f->fieldType == "FalcoEngine.Font")
			{
				Font* asset = Font::load(Engine::getSingleton()->getAssetsPath(), f->fieldValue.objectVal);
				fieldProp = new PropAsset(this, CP_UNI(f->fieldName), asset);

				fieldProp->setSupportedFormats({ "ttf", "otf" });
				fieldProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropFont(prop, from); });
			}

			//Components
			auto& compList = MonoScript::getComponentList();

			if (std::find(compList.begin(), compList.end(), f->fieldType) != compList.end())
			{
				GameObject* node = Engine::getSingleton()->getGameObject(f->fieldValue.objectVal);
				fieldProp = new PropGameObject(this, CP_UNI(f->fieldName), node);
				fieldProp->setSupportedFormats({ "::GameObject" });
				fieldProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropGameObject(prop, from); });
				if (componentsIcons.find(f->fieldType) != componentsIcons.end())
					((PropGameObject*)fieldProp)->setImage(MainWindow::loadEditorIcon(componentsIcons[f->fieldType]));
			}

			std::string _fldType = boost::replace_all_copy(f->fieldType, "FalcoEngine.", "");

			MonoClass* _classType = mono_class_from_name(APIManager::getSingleton()->getImage(), "", CP_UNI(f->fieldType).c_str());
			MonoClass* _classType2 = mono_class_from_name(APIManager::getSingleton()->getBaseImage(), "FalcoEngine", CP_UNI(_fldType).c_str());
			MonoClass* systemObjectClass = mono_class_from_name(APIManager::getSingleton()->getSystemImage(), "System", "Object");
			bool objClass = false;

			if (_classType != nullptr)
			{
				if (mono_class_is_subclass_of(_classType, APIManager::getSingleton()->behaviour_class, false))
				{
					GameObject* node = Engine::getSingleton()->getGameObject(f->fieldValue.objectVal);
					fieldProp = new PropGameObject(this, CP_UNI(f->fieldName), node);
					fieldProp->setSupportedFormats({ "::GameObject" });
					fieldProp->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onDropGameObject(prop, from); });
				}

				if (mono_class_is_subclass_of(_classType, systemObjectClass, false))
					objClass = true;
			}
			if (_classType2 != nullptr)
			{
				if (mono_class_is_subclass_of(_classType2, systemObjectClass, false))
					objClass = true;
			}

			if (objClass)
			{
				fieldProp->setOnClearCallback([=](Property* prop)
					{
						if (dynamic_cast<PropAsset*>(prop))
						{
							((PropAsset*)prop)->setValue(nullptr);
							setAssetField(prop, nullptr);
						}
						if (dynamic_cast<PropGameObject*>(prop))
						{
							((PropGameObject*)prop)->setValue(nullptr);
							setGameObjectField(prop, nullptr);
						}
					}
				);
			}

			if (fieldProp == nullptr)
				fieldProp = new PropString(this, CP_UNI(f->fieldName), "");

			MonoScript::MonoFieldInfo* propData = new MonoScript::MonoFieldInfo();
			*propData = *f;

			fieldProp->setUserData(static_cast<void*>(propData));
			
			addProperty(fieldProp);
		}

		fields.clear();
	}

	void MonoScriptEditor::addUndo(void* data)
	{
		MonoScript::MonoFieldInfo* fieldInfo = (MonoScript::MonoFieldInfo*)data;

		//Add undo
		UndoData* undoData = Undo::addUndo("Edit object property");
		undoData->stringData.resize(2);

		for (auto& it : components)
		{
			//0
			MonoScript* monoScript = (MonoScript*)it;
			auto* field = monoScript->getField(fieldInfo->fieldName);
			if (field == nullptr)
				continue;

			undoData->stringData[0][monoScript] = field->serialize();

			delete field;

			//1
			undoData->stringData[1][monoScript] = fieldInfo->serialize();
		}

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				MonoScript::MonoFieldInfo field;
				field.deserialize(d.second);

				((MonoScript*)d.first)->setField(field);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[1])
			{
				MonoScript::MonoFieldInfo field;
				field.deserialize(d.second);

				((MonoScript*)d.first)->setField(field);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//
	}

	void MonoScriptEditor::onChangeInt(Property* prop, int val)
	{
		MonoScript::MonoFieldInfo* fieldInfo = static_cast<MonoScript::MonoFieldInfo*>(prop->getUserData());
		fieldInfo->fieldValue = MonoScript::ValueVariant(val);

		addUndo(fieldInfo);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			MonoScript* monoScript = (MonoScript*)*it;
			monoScript->setField(*fieldInfo);
		}
	}

	void MonoScriptEditor::onChangeFloat(Property* prop, float val)
	{
		MonoScript::MonoFieldInfo* fieldInfo = static_cast<MonoScript::MonoFieldInfo*>(prop->getUserData());
		fieldInfo->fieldValue = MonoScript::ValueVariant(val);

		addUndo(fieldInfo);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			MonoScript* monoScript = (MonoScript*)*it;
			monoScript->setField(*fieldInfo);
		}
	}

	void MonoScriptEditor::onChangeBool(Property* prop, bool val)
	{
		MonoScript::MonoFieldInfo* fieldInfo = static_cast<MonoScript::MonoFieldInfo*>(prop->getUserData());
		fieldInfo->fieldValue = MonoScript::ValueVariant(val);

		addUndo(fieldInfo);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			MonoScript* monoScript = (MonoScript*)*it;
			monoScript->setField(*fieldInfo);
		}
	}

	void MonoScriptEditor::onChangeString(Property* prop, std::string val)
	{
		MonoScript::MonoFieldInfo* fieldInfo = static_cast<MonoScript::MonoFieldInfo*>(prop->getUserData());
		fieldInfo->fieldValue = MonoScript::ValueVariant(val);

		addUndo(fieldInfo);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			MonoScript* monoScript = (MonoScript*)*it;
			monoScript->setField(*fieldInfo);
		}
	}

	void MonoScriptEditor::onChangeStringArray(Property* prop, std::string val, int index)
	{
		MonoScript::MonoFieldInfo* fieldInfo = static_cast<MonoScript::MonoFieldInfo*>(prop->parent->getUserData());
		fieldInfo->fieldValue.stringArrVal[index] = CP_SYS(val);

		addUndo(fieldInfo);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			MonoScript* monoScript = (MonoScript*)*it;
			monoScript->setField(*fieldInfo);
		}
	}

	void MonoScriptEditor::onChangeStringArraySize(Property* prop, int size)
	{
		if (size >= 0 && size <= INT_MAX)
		{
			MonoScript::MonoFieldInfo* fieldInfo = static_cast<MonoScript::MonoFieldInfo*>(prop->parent->getUserData());
			fieldInfo->fieldValue.stringArrVal.resize(size);

			addUndo(fieldInfo);

			for (auto it = components.begin(); it != components.end(); ++it)
			{
				MonoScript* monoScript = (MonoScript*)*it;
				monoScript->setField(*fieldInfo);
			}
		}

		updateEditor();
	}

	void MonoScriptEditor::onChangeVector2(Property* prop, glm::vec2 val)
	{
		MonoScript::MonoFieldInfo* fieldInfo = static_cast<MonoScript::MonoFieldInfo*>(prop->getUserData());
		fieldInfo->fieldValue = MonoScript::ValueVariant(val);

		addUndo(fieldInfo);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			MonoScript* monoScript = (MonoScript*)*it;
			monoScript->setField(*fieldInfo);
		}
	}

	void MonoScriptEditor::onChangeVector3(Property* prop, glm::vec3 val)
	{
		MonoScript::MonoFieldInfo* fieldInfo = static_cast<MonoScript::MonoFieldInfo*>(prop->getUserData());
		fieldInfo->fieldValue = MonoScript::ValueVariant(val);

		addUndo(fieldInfo);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			MonoScript* monoScript = (MonoScript*)*it;
			monoScript->setField(*fieldInfo);
		}
	}

	void MonoScriptEditor::onChangeVector4(Property* prop, glm::vec4 val)
	{
		MonoScript::MonoFieldInfo* fieldInfo = static_cast<MonoScript::MonoFieldInfo*>(prop->getUserData());
		fieldInfo->fieldValue = MonoScript::ValueVariant(val);

		addUndo(fieldInfo);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			MonoScript* monoScript = (MonoScript*)*it;
			monoScript->setField(*fieldInfo);
		}
	}

	void MonoScriptEditor::onChangeColor(Property* prop, Color val)
	{
		MonoScript::MonoFieldInfo* fieldInfo = static_cast<MonoScript::MonoFieldInfo*>(prop->getUserData());
		fieldInfo->fieldValue = MonoScript::ValueVariant(glm::vec4(val[0], val[1], val[2], val[3]));

		addUndo(fieldInfo);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			MonoScript* monoScript = (MonoScript*)*it;
			monoScript->setField(*fieldInfo);
		}
	}

	void MonoScriptEditor::onDropGameObject(TreeNode* prop, TreeNode* from)
	{
		std::string objName = from->name;

		GameObject* _node = Engine::getSingleton()->getGameObject(objName);

		setGameObjectField(prop, _node);
	}

	void MonoScriptEditor::setAssetField(TreeNode* prop, Asset* asset)
	{
		((PropAsset*)prop)->setValue(asset);

		MonoScript::MonoFieldInfo* fieldInfo = static_cast<MonoScript::MonoFieldInfo*>(prop->getUserData());
		if (asset != nullptr && asset->getManagedObject() != nullptr)
		{
			fieldInfo->fieldValue = MonoScript::ValueVariant();
			fieldInfo->fieldValue.objectVal = asset->getName();
		}
		else
		{
			fieldInfo->fieldValue = MonoScript::ValueVariant();
			fieldInfo->fieldValue.objectVal = "[None]";
		}

		addUndo(fieldInfo);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			MonoScript* monoScript = (MonoScript*)*it;
			monoScript->setField(*fieldInfo);
		}
	}

	void MonoScriptEditor::setGameObjectField(TreeNode* prop, GameObject* gameObject)
	{
		((PropGameObject*)prop)->setValue(gameObject);

		MonoScript::MonoFieldInfo* fieldInfo = static_cast<MonoScript::MonoFieldInfo*>(prop->getUserData());
		if (gameObject != nullptr && gameObject->getManagedObject() != nullptr)
		{
			fieldInfo->fieldValue = MonoScript::ValueVariant();
			fieldInfo->fieldValue.objectVal = gameObject->getGuid();
		}
		else
		{
			fieldInfo->fieldValue = MonoScript::ValueVariant();
			fieldInfo->fieldValue.objectVal = "[None]";
		}

		addUndo(fieldInfo);

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			MonoScript* monoScript = (MonoScript*)*it;
			monoScript->setField(*fieldInfo);
		}
	}

	void MonoScriptEditor::onDropAudioClip(TreeNode* prop, TreeNode* from)
	{
		std::string objName = from->getPath();

		AudioClip* _node = AudioClip::load(Engine::getSingleton()->getAssetsPath(), objName);
		setAssetField(prop, _node);
	}

	void MonoScriptEditor::onDropVideoClip(TreeNode* prop, TreeNode* from)
	{
		std::string objName = from->getPath();

		VideoClip* _node = VideoClip::load(Engine::getSingleton()->getAssetsPath(), objName);
		setAssetField(prop, _node);
	}

	void MonoScriptEditor::onDropMaterial(TreeNode* prop, TreeNode* from)
	{
		std::string objName = from->getPath();

		Material* _node = Material::load(Engine::getSingleton()->getAssetsPath(), objName);
		setAssetField(prop, _node);
	}

	void MonoScriptEditor::onDropPrefab(TreeNode* prop, TreeNode* from)
	{
		std::string objName = from->getPath();

		Prefab* _node = Prefab::load(Engine::getSingleton()->getAssetsPath(), objName);
		setAssetField(prop, _node);
	}

	void MonoScriptEditor::onDropShader(TreeNode* prop, TreeNode* from)
	{
		std::string objName = from->getPath();

		Shader* _node = Shader::load(Engine::getSingleton()->getAssetsPath(), objName);
		setAssetField(prop, _node);
	}

	void MonoScriptEditor::onDropTexture(TreeNode* prop, TreeNode* from)
	{
		std::string objName = from->getPath();

		Texture* _node = Texture::load(Engine::getSingleton()->getAssetsPath(), objName);
		setAssetField(prop, _node);
	}

	void MonoScriptEditor::onDropAnimationClip(TreeNode* prop, TreeNode* from)
	{
		std::string objName = from->getPath();

		AnimationClip *_node = AnimationClip::load(Engine::getSingleton()->getAssetsPath(), objName);
		setAssetField(prop, _node);
	}

	void MonoScriptEditor::onDropCubemap(TreeNode* prop, TreeNode* from)
	{
		std::string objName = from->getPath();

		Cubemap* _node = Cubemap::load(Engine::getSingleton()->getAssetsPath(), objName);
		setAssetField(prop, _node);
	}

	void MonoScriptEditor::onDropFont(TreeNode* prop, TreeNode* from)
	{
		std::string objName = from->getPath();

		Font* _node = Font::load(Engine::getSingleton()->getAssetsPath(), objName);
		setAssetField(prop, _node);
	}
}