#pragma once

#include "../../Mono/include/mono/metadata/object.h"

#include <string>
#include <vector>
#include <functional>
#include <map>

namespace GX
{
	class GameObject;
	class MonoScript;

	class APIManager
	{
		friend class MonoScript;
		friend class API_SceneManager;

	public:
		enum CompileConfiguration { Debug, Release };

	private:
		static APIManager singleton;

		MonoDomain * domain = nullptr;
		MonoAssembly * assembly = nullptr;
		MonoImage * image = nullptr;

		MonoAssembly * base_assembly = nullptr;
		MonoImage * base_image = nullptr;

		MonoAssembly* system_assembly = nullptr;
		MonoImage * system_image = nullptr;

		bool startExecuted = false;
		bool initialized = false;
		bool libraryOpened = false;

		std::vector<MonoClass*> behaviourClasses;

		std::string getStringProperty(const char *propertyName, MonoClass *classType, MonoObject *classObject);
		void retrieveClassesAndFields();

		std::function<void(std::string)> logCallback = nullptr;
		std::function<void()> beginCompileCallback = nullptr;
		std::function<void()> endCompileCallback = nullptr;

		std::string lastLog = "";
		std::string sceneToLoad = "";
		std::vector<GameObject*> objectsToDestroy;

		float fixedTimeStep = 0.01f;
		float fixedTimeSimulated = 0.0f;
		float currentTime = 0.0f;

		std::vector<MonoAssembly*> externalAssemblies;

		void logLine(std::string line, bool& tab_found, std::string& msg_block);

		void destroyNodes();
		void createObjects();
		void cleanup();
		void compileLibrary(CompileConfiguration config = CompileConfiguration::Debug);
		void openLibrary();
		void closeLibrary();
		void reportException(MonoObject* except);

	public:
		typedef std::vector<std::pair<std::string, std::string>> FunctionList; //Store class name and function name
		typedef std::vector<MonoScript*> ScriptList;

		APIManager();
		~APIManager();

		static APIManager* getSingleton() { return &singleton; }

		void init();
		void open();
		void close();
		void reload();
		void compile(CompileConfiguration config);

		void start();
		void update();
		void fixedUpdate();

		void execute(std::string methodName, void ** params = nullptr, std::string stringParams = "");
		void execute(GameObject * root, std::string methodName, void ** params = nullptr, std::string stringParams = "");
		void executeForNode(GameObject* node, std::string methodName, std::string className = "", void** params = nullptr, std::string stringParams = "");
		void execute(MonoObject* object, std::string methodName, void** params = nullptr, std::string stringParams = "");
		void executeStatic(MonoClass* klass, std::string methodName, void** params = nullptr);
		void setLogCallback(std::function<void(std::string)> callback) { logCallback = callback; }
		void setBeginCompileCallback(std::function<void()> callback) { beginCompileCallback = callback; }
		void setEndCompileCallback(std::function<void()> callback) { endCompileCallback = callback; }
		std::string getLastLog() { return lastLog; }

		void addDestroyObject(GameObject* node);
		void loadScene(std::string scene) { sceneToLoad = scene; }
		void checkSceneToLoad();

		bool isInitialized() { return initialized && libraryOpened; }

		MonoDomain * getDomain() { return domain; }
		MonoImage * getImage() { return image; }
		MonoImage * getBaseImage() { return base_image; }
		MonoImage * getSystemImage() { return system_image; }
		FunctionList getFunctions(GameObject* node);
		std::vector<MonoClass*>& getBehaviourClasses() { return behaviourClasses; }
		MonoClass * findClass(std::string name);

		std::vector<MonoClass*> getAssemblyClassList(MonoImage* image);

	public:
		//Classes
		MonoClass* behaviour_class = nullptr;
		MonoClass* transform_class = nullptr;
		MonoClass* rigidbody_class = nullptr;
		MonoClass* vector2_class = nullptr;
		MonoClass* vector3_class = nullptr;
		MonoClass* vector4_class = nullptr;
		MonoClass* color_class = nullptr;
		MonoClass* quaternion_class = nullptr;
		MonoClass* input_class = nullptr;
		MonoClass* gameobject_class = nullptr;
		MonoClass* animation_class = nullptr;
		MonoClass* animationclip_class = nullptr;
		MonoClass* audiosource_class = nullptr;
		MonoClass* audiolistener_class = nullptr;
		MonoClass* audioclip_class = nullptr;
		MonoClass* videoclip_class = nullptr;
		MonoClass* navmeshagent_class = nullptr;
		MonoClass* navmeshobstacle_class = nullptr;
		MonoClass* component_class = nullptr;
		MonoClass* font_class = nullptr;
		MonoClass* asset_class = nullptr;
		MonoClass* object_class = nullptr;
		MonoClass* material_class = nullptr;
		MonoClass* shader_class = nullptr;
		MonoClass* camera_class = nullptr;
		MonoClass* uielement_class = nullptr;
		MonoClass* canvas_class = nullptr;
		MonoClass* text_class = nullptr;
		MonoClass* image_class = nullptr;
		MonoClass* button_class = nullptr;
		MonoClass* text_input_class = nullptr;
		MonoClass* mask_class = nullptr;
		MonoClass* texture_class = nullptr;
		MonoClass* cubemap_class = nullptr;
		MonoClass* collision_class = nullptr;
		MonoClass* collider_class = nullptr;
		MonoClass* meshcollider_class = nullptr;
		MonoClass* boxcollider_class = nullptr;
		MonoClass* spherecollider_class = nullptr;
		MonoClass* capsulecollider_class = nullptr;
		MonoClass* vehicle_class = nullptr;
		MonoClass* meshrenderer_class = nullptr;
		MonoClass* prefab_class = nullptr;
		MonoClass* rendertexture_class = nullptr;
		MonoClass* spline_class = nullptr;
		MonoClass* videoplayer_class = nullptr;
		MonoClass* light_class = nullptr;
		MonoClass* mesh_class = nullptr;
		MonoClass* submesh_class = nullptr;
		MonoClass* conetwistjoint_class = nullptr;
		MonoClass* fixedjoint_class = nullptr;
		MonoClass* freejoint_class = nullptr;
		MonoClass* hingejoint_class = nullptr;
		MonoClass* particlesystem_class = nullptr;
		MonoClass* particleemitter_class = nullptr;

		MonoClass* string_class = nullptr;
		MonoClass* int_class = nullptr;

		MonoClassField* gameobject_ptr_field = nullptr;
		MonoClassField* component_ptr_field = nullptr;
		MonoClassField* asset_ptr_field = nullptr;
		MonoClassField* object_ptr_field = nullptr;

		MonoClassField* vector2_x = nullptr;
		MonoClassField* vector2_y = nullptr;

		MonoClassField* vector3_x = nullptr;
		MonoClassField* vector3_y = nullptr;
		MonoClassField* vector3_z = nullptr;

		MonoClassField* vector4_x = nullptr;
		MonoClassField* vector4_y = nullptr;
		MonoClassField* vector4_z = nullptr;
		MonoClassField* vector4_w = nullptr;

		MonoClassField* quaternion_x = nullptr;
		MonoClassField* quaternion_y = nullptr;
		MonoClassField* quaternion_z = nullptr;
		MonoClassField* quaternion_w = nullptr;

		MonoClassField* color_r = nullptr;
		MonoClassField* color_g = nullptr;
		MonoClassField* color_b = nullptr;
		MonoClassField* color_a = nullptr;

		MonoClassField* collision_other = nullptr;
		MonoClassField* collision_point = nullptr;
		MonoClassField* collision_normal = nullptr;
	};
}