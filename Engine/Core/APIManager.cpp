#include "APIManager.h"

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <boost/process.hpp>

#include "../../Mono/include/mono/jit/jit.h"
#include "../../Mono/include/mono/metadata/assembly.h"
#include "../../Mono/include/mono/metadata/debug-helpers.h"
#include "../../Mono/include/mono/metadata/threads.h"
#include "../../Mono/include/mono/metadata/mono-config.h"
#include "../../Mono/include/mono/metadata/class.h"
#include "../../Mono/include/mono/metadata/mono-gc.h"
#include "../../Mono/include/mono/metadata/attrdefs.h"

#include "../Classes/IO.h"
#include "../Classes/StringConverter.h"

#include "../Core/Engine.h"
#include "../Renderer/Renderer.h"
#include "../Core/Debug.h"
#include "../Core/Time.h"
#include "../Core/GameObject.h"
#include "../Components/Transform.h"
#include "../Components/MonoScript.h"
#include "../Classes/Helpers.h"

#include "../API/API_MonoBehaviour.h"
#include "../API/API_Transform.h"
#include "../API/API_RigidBody.h"
#include "../API/API_Quaternion.h"
#include "../API/API_Input.h"
#include "../API/API_Physics.h"
#include "../API/API_GameObject.h"
#include "../API/API_Animation.h"
#include "../API/API_AudioSource.h"
#include "../API/API_NavMeshAgent.h"
#include "../API/API_Component.h"
#include "../API/API_Prefab.h"
#include "../API/API_Screen.h"
#include "../API/API_Cursor.h"
#include "../API/API_Time.h"
#include "../API/API_SceneManager.h"
#include "../API/API_Debug.h"
#include "../API/API_Material.h"
#include "../API/API_Texture.h"
#include "../API/API_Camera.h"
#include "../API/API_Application.h"
#include "../API/API_AudioListener.h"
#include "../API/API_Shader.h"
#include "../API/API_Collider.h"
#include "../API/API_MeshCollider.h"
#include "../API/API_BoxCollider.h"
#include "../API/API_SphereCollider.h"
#include "../API/API_CapsuleCollider.h"
#include "../API/API_HingeJoint.h"
#include "../API/API_ConeTwistJoint.h"
#include "../API/API_FreeJoint.h"
#include "../API/API_FixedJoint.h"
#include "../API/API_Asset.h"
#include "../API/API_PlayerPrefs.h"
#include "../API/API_Random.h"
#include "../API/API_Vehicle.h"
#include "../API/API_MeshRenderer.h"
#include "../API/API_UIElement.h"
#include "../API/API_Image.h"
#include "../API/API_Text.h"
#include "../API/API_Button.h"
#include "../API/API_Canvas.h"
#include "../API/API_RenderTexture.h"
#include "../API/API_Graphics.h"
#include "../API/API_GUI.h"
#include "../API/API_Spline.h"
#include "../API/API_SteamAPI.h"
#include "../API/API_Light.h"
#include "../API/API_RenderSettings.h"
#include "../API/API_QualitySettings.h"
#include "../API/API_LayerMask.h"
#include "../API/API_Mesh.h"
#include "../API/API_SubMesh.h"
#include "../API/API_AudioClip.h"
#include "../API/API_VideoClip.h"
#include "../API/API_VideoPlayer.h"
#include "../API/API_TextInput.h"
#include "../API/API_ParticleSystem.h"

#include "../Assets/Scene.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#define BUFSIZE 4096

namespace GX
{
	APIManager APIManager::singleton;

	static std::map<MonoScript*, MonoScript::FieldList> fieldList;
	static std::map<MonoScript*, std::string> classList;

	APIManager::APIManager()
	{
		srand(static_cast<unsigned>(time(0)));
	}

	APIManager::~APIManager()
	{
	}

	void APIManager::init()
	{
		SetEnvironmentVariable("MONO_GC_PARAMS", "nursery-size=64m");

		mono_set_dirs(CP_UNI((Helper::ExePath() + "Mono/lib/")).c_str(), CP_UNI((Helper::ExePath() + "Mono/etc/")).c_str());
		mono_config_parse(NULL);

		domain = mono_jit_init("Falco_Engine");
		mono_domain_set(domain, false);

		if (domain == nullptr)
			Debug::logError("Failed to initialize mono runtime!");
		else
			Debug::log("Mono runtime initialized", Debug::DbgColorGreen);

		initialized = true;
	}

	void APIManager::open()
	{
		openLibrary();
	}

	void APIManager::close()
	{
		cleanup();
		closeLibrary();
	}

	void APIManager::reload()
	{
		close();
		openLibrary();
	}

	void APIManager::compile(CompileConfiguration config)
	{
		std::vector<GameObject*> objects = Engine::getSingleton()->getGameObjects();

		if (initialized && libraryOpened)
		{
			fieldList.clear();
			classList.clear();

			for (auto it = objects.begin(); it != objects.end(); ++it)
			{
				GameObject* obj = *it;
				std::vector<MonoScript*> scripts = obj->getMonoScripts();
				for (auto ct = scripts.begin(); ct != scripts.end(); ++ct)
				{
					if (!(*ct)->getClassName().empty())
						classList[*ct] = (*ct)->getClassName();

					auto fields = (*ct)->getFields();
					for (auto ff = fields.begin(); ff != fields.end(); ++ff)
					{
						if ((*ct)->getManagedObject() != nullptr)
							fieldList[*ct].push_back(*ff);
					}
				}
			}
		}

		if (initialized)
			closeLibrary();

		compileLibrary(config);

		if (initialized)
		{
			openLibrary();

			if (libraryOpened)
			{
				for (auto it = classList.begin(); it != classList.end(); ++it)
				{
					MonoScript* script = it->first;
					script->managedClassName = classList[script];
					script->managedClass = findClass(script->managedClassName);
					script->createManagedObject();
				}

				for (auto it = classList.begin(); it != classList.end(); ++it)
				{
					MonoScript* script = it->first;
					if (script->getManagedObject() != nullptr)
					{
						for (auto fl = fieldList[script].begin(); fl != fieldList[script].end(); ++fl)
							script->setField(*fl);
					}
				}
			}
		}

		objects.clear();
	}

	void APIManager::start()
	{
		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			execute("Awake");
			execute("Start");
		}
	}

	void APIManager::update()
	{
		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			execute("Update");

			destroyNodes();
			checkSceneToLoad();
		}
	}

	void APIManager::fixedUpdate()
	{
		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			float dt = Time::getDeltaTime();

			currentTime += dt * Time::getTimeScale();

			while (fixedTimeSimulated < currentTime)
			{
				execute("FixedUpdate");
				fixedTimeSimulated += fixedTimeStep;
			}
		}
	}

	void APIManager::compileLibrary(CompileConfiguration config)
	{
		//Compile the scripts
		if (beginCompileCallback != nullptr)
			beginCompileCallback();

		std::string conf = "Debug/";
		std::string debug = "-debug+";

		if (config == CompileConfiguration::Release)
		{
			conf = "Release/";
			debug = "-debug-";
		}

		std::string solutionPath = Engine::getSingleton()->getRootPath() + "Project/";
		
		IO::CreateDir(solutionPath + "bin/");
		IO::CreateDir(solutionPath + "bin/Debug/");
		IO::CreateDir(solutionPath + "bin/Release/");
		
		std::string feDll = Helper::ExePath() + "FalcoEngine.dll";

		std::string assetsPath = Engine::getSingleton()->getAssetsPath();
		
		std::string monoDirectory = Helper::ExePath() + "Mono/";
		std::string entryPointScript = solutionPath + "EntryPoint.cs";

		if (!IO::FileExists(entryPointScript))
			IO::WriteText(entryPointScript, "using System;\n\npublic static class EntryPoint\n{\n	static void main()\n	{\n	}\n}");

		//Dlls
		std::vector<std::string> externalDlls;

		std::string srcFiles = "";
		std::string dllsStrL = "";
		std::string dllsStrR = "";

		IO::listFiles(assetsPath, true, nullptr, [=, &externalDlls, &srcFiles](std::string d, std::string f) -> bool
			{
				std::string fileName = d + f;

				if (IO::GetFileExtension(fileName) == "dll")
					externalDlls.push_back(fileName);

				if (IO::GetFileExtension(f) == "cs")
					srcFiles += "\"" + fileName + "\" ";

				return true;
			}
		);

		srcFiles += "\"" + entryPointScript + "\"";

		int i = 0;
		for (auto dll = externalDlls.begin(); dll != externalDlls.end(); ++dll, ++i)
		{
#ifdef _WIN32
			dllsStrL += " -lib:\"" + *dll + "\"";
			dllsStrR += " -r:\"" + *dll + "\"";
#else
			dllsStrL += " -L \"" + *dll + "\"";
			dllsStrR += " -r \"" + *dll + "\"";
#endif

			if (i < externalDlls.size() - 1)
			{
				dllsStrL += " ";
				dllsStrR += " ";
			}
		}

		externalDlls.clear();

	#ifdef _WIN32
		std::string command = "\"" + monoDirectory +
			"bin/mono.exe\" \"" + monoDirectory +
			"lib/mono/4.5/mcs.exe\" -codepage:65001 --stacktrace " + debug +
			" -lib:\"" + feDll + "\"" + dllsStrL + " -r:\"" + feDll + "\"" + dllsStrR + " -target:library -out:\"" +
			solutionPath + "bin/" + conf + "MainAssembly.dll\" " + srcFiles;
	#else
		std::string command = "\"" + monoDirectory +
			"lib/mono/4.5/mcs.exe\" -nowarn:\"CS8029\" -codepage:65001 --stacktrace " + debug +
			" -L \"" + feDll + "\"" + dllsStrL + " -r \"" + feDll + "\"" + dllsStrR + " -target:library -o \"" +
			solutionPath + "bin/" + conf + "MainAssembly.dll\" " + srcFiles;
	#endif

	#ifdef _WIN32
		std::wstring ws = StringConvert::s2ws(command, GetACP());
		ws.push_back(0);

		HANDLE g_hChildStd_OUT_Rd = NULL;
		HANDLE g_hChildStd_OUT_Wr = NULL;
		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		if (CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		{
			PROCESS_INFORMATION piProcInfo;
			STARTUPINFOW siStartInfo;
			BOOL bSuccess = FALSE;

			ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
			ZeroMemory(&siStartInfo, sizeof(STARTUPINFOW));

			siStartInfo.cb = sizeof(STARTUPINFOW);
			siStartInfo.hStdError = g_hChildStd_OUT_Wr;
			siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
			siStartInfo.hStdInput = NULL;
			siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

			bSuccess = CreateProcessW(
				NULL,
				&ws[0],
				NULL,
				NULL,
				TRUE,
				NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, // | CREATE_UNICODE_ENVIRONMENT
				NULL,
				NULL,
				&siStartInfo,
				&piProcInfo);

			DWORD dwRead;
			HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

			CloseHandle(g_hChildStd_OUT_Wr);
			lastLog = "";

			bool tab_found = false;

			std::string msg_block = "";

			for (;;)
			{
				CHAR chBuf[BUFSIZE] = {0};
				bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
				
				if (!bSuccess || dwRead <= 0)
					break;

				std::string str = std::string(chBuf).substr(0, dwRead);
				str = StringConvert::cp_convert(str, 866, GetACP());

				std::vector<std::string> results;
				boost::split(results, str, [](char c) { return c == '\n'; });

				for (auto& line : results)
				{
					std::string _line = boost::replace_all_copy(line, "\n", "");
					_line = boost::replace_all_copy(_line, "\r", "");

					if (_line.empty()) continue;

					logLine(_line + "\n", tab_found, msg_block);
				}
				
				lastLog += str;
			}

			if (!msg_block.empty())
			{
				if (logCallback != nullptr)
					logCallback(msg_block);

				msg_block = "";
			}

			CloseHandle(piProcInfo.hProcess);
			CloseHandle(piProcInfo.hThread);
		}
		
		Sleep(1000);
	#else
		std::string line = "";
		lastLog = "";

		bool tab_found = false;

		std::string msg_block = "";
		
		boost::process::ipstream pipe_stream;
		boost::process::ipstream pipe_stream_err;
		boost::process::child c(command, boost::process::std_out > pipe_stream, boost::process::std_err > pipe_stream_err);

		while (pipe_stream_err && std::getline(pipe_stream_err, line) && !line.empty())
		{
			if (line.find("CS8029") != std::string::npos)
				continue;

			boost::replace_all(line, "\r", "");
			logLine(line + "\n", tab_found, msg_block);

			lastLog += line + "\n";
		}

		while (pipe_stream && std::getline(pipe_stream, line) && !line.empty())
		{
			if (line.find("CS8029") != std::string::npos)
				continue;

			boost::replace_all(line, "\r", "");
			logLine(line + "\n", tab_found, msg_block);

			lastLog += line + "\n";
		}

		if (!msg_block.empty())
		{
			if (logCallback != nullptr)
				logCallback(msg_block);

			msg_block = "";
		}

		c.wait();

		sleep(1);
	#endif

		if (endCompileCallback != nullptr)
			endCompileCallback();
	}

	void APIManager::logLine(std::string line, bool& tab_found, std::string& msg_block)
	{
		std::string fn = Engine::getSingleton()->getAssetsPath();
		std::string _line = boost::replace_all_copy(line, fn, "");

		if (_line.find('\t') == std::string::npos)
		{
			if (tab_found)
			{
				tab_found = false;

				if (logCallback != nullptr)
					logCallback(msg_block);

				msg_block = "";
			}
			else
			{
				if (!msg_block.empty())
				{
					if (logCallback != nullptr)
						logCallback(msg_block);

					msg_block = "";
				}
			}

			msg_block += _line;
		}
		else
		{
			tab_found = true;
			msg_block += _line;
		}
	}

	void APIManager::openLibrary()
	{
		domain = mono_domain_create_appdomain(const_cast<char*>("Falco_Engine"), NULL);
		mono_domain_set(domain, 0);

		//Register API
		API_Input::Register();
		API_GameObject::Register();
		API_MonoBehaviour::Register();
		API_Quaternion::Register();
		API_Transform::Register();
		API_Physics::Register();
		API_RigidBody::Register();
		API_Animation::Register();
		API_AudioSource::Register();
		API_NavMeshAgent::Register();
		API_Component::Register();
		API_Prefab::Register();
		API_Screen::Register();
		API_Cursor::Register();
		API_Time::Register();
		API_SceneManager::Register();
		API_Debug::Register();
		API_Material::Register();
		API_Texture::Register();
		API_Camera::Register();
		API_Application::Register();
		API_AudioListener::Register();
		API_Shader::Register();
		API_Collider::Register();
		API_MeshCollider::Register();
		API_BoxCollider::Register();
		API_SphereCollider::Register();
		API_CapsuleCollider::Register();
		API_HingeJoint::Register();
		API_ConeTwistJoint::Register();
		API_FreeJoint::Register();
		API_FixedJoint::Register();
		API_Asset::Register();
		API_PlayerPrefs::Register();
		API_Random::Register();
		API_Vehicle::Register();
		API_MeshRenderer::Register();
		API_UIElement::Register();
		API_Image::Register();
		API_Text::Register();
		API_Button::Register();
		API_Canvas::Register();
		API_RenderTexture::Register();
		API_Graphics::Register();
		API_GUI::Register();
		API_Spline::Register();
		API_SteamAPI::Register();
		API_Light::Register();
		API_RenderSettings::Register();
		API_QualitySettings::Register();
		API_LayerMask::Register();
		API_Mesh::Register();
		API_SubMesh::Register();
		API_AudioClip::Register();
		API_VideoClip::Register();
		API_VideoPlayer::Register();
		API_TextInput::Register();
		API_ParticleSystem::Register();

		//Base lib
		std::string libDirectory = Engine::getSingleton()->getAssemblyPath() + "MainAssembly.dll";
		std::string fp = Helper::ExePath();

	#ifdef _WIN32
		int nCodePage = GetACP();
	#else
		int nCodePage = 65001;
	#endif
		if (base_assembly == nullptr)
		{
			std::string _path = fp + "FalcoEngine.dll";

			base_assembly = mono_domain_assembly_open(domain, StringConvert::cp_convert(_path.c_str(), nCodePage, 65001).c_str());

			if (base_assembly != nullptr)
			{
				base_image = mono_assembly_get_image(base_assembly);
				if (!base_image)
				{
					Debug::logError("Base assembly load failed");
				}
			}
			else
				Debug::logError("Base assembly open failed");
		}

		//App lib
		assembly = mono_domain_assembly_open(domain, StringConvert::cp_convert(libDirectory.c_str(), nCodePage, 65001).c_str());

		if (assembly != nullptr)
		{
			image = mono_assembly_get_image(assembly);
			if (image != nullptr)
			{
				retrieveClassesAndFields();
				behaviourClasses = getAssemblyClassList(image);
			}
			else
				Debug::logError("Assembly load failed");
		}
		else
			Debug::logError("Assembly open failed");

		//System lib
		MonoImageOpenStatus stat;
		MonoAssemblyName* n = mono_assembly_name_new("mscorlib");
		system_assembly = mono_assembly_load(n, nullptr, &stat);

		system_image = mono_assembly_get_image(system_assembly);
		mono_assembly_name_free(n);

		//Get system classes
		string_class = mono_class_from_name(system_image, "System", "String");
		int_class = mono_class_from_name(system_image, "System", "Int32");

		//External libs
		std::vector<std::string> externalDlls;

		bool recursive = true;
		std::string libPath = Engine::getSingleton()->getAssetsPath();
		if (!IO::isDir(libPath))
		{
			libPath = Engine::getSingleton()->getRootPath();
			recursive = false;
		}

		IO::listFiles(libPath, recursive, nullptr, [=, &externalDlls](std::string d, std::string f) -> bool
			{
				if (boost::to_lower_copy(f) != "falcoengine.dll" &&
					boost::to_lower_copy(f) != "mainassembly.dll" &&
					boost::to_lower_copy(f) != "mscorlib.dll")
				{
					std::string fileName = d + f;
					if (boost::to_lower_copy(IO::GetFileExtension(fileName)) == "dll")
						externalDlls.push_back(fileName);
				}

				return true;
			}
		);

		for (auto& dll : externalDlls)
		{
			MonoAssembly* lib = mono_domain_assembly_open(domain, StringConvert::cp_convert(dll, nCodePage, 65001).c_str());
			if (lib != nullptr)
				externalAssemblies.push_back(lib);
		}

		externalDlls.clear();

		libraryOpened = assembly != nullptr;
	}

	void APIManager::closeLibrary()
	{
		if (domain != nullptr)
			mono_thread_attach(domain);

		behaviourClasses.clear();
		
		cleanup();

		if (assembly != nullptr)
			mono_assembly_close(assembly);

		if (base_assembly != nullptr)
			mono_assembly_close(base_assembly);

		if (system_assembly != nullptr)
			mono_assembly_close(system_assembly);

		for (auto& lib : externalAssemblies)
			mono_assembly_close(lib);

		externalAssemblies.clear();

		assembly = nullptr;
		base_assembly = nullptr;
		system_assembly = nullptr;

		domain = nullptr;
		image = nullptr;
		base_image = nullptr;
		system_image = nullptr;

		if (domain != nullptr)
		{
			mono_jit_cleanup(domain);
		}

		domain = nullptr;

		libraryOpened = false;
	}

	void APIManager::cleanup()
	{
		std::vector<GameObject*> objects = Engine::getSingleton()->getGameObjects();
		
		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			GameObject* obj = *it;
			std::vector<MonoScript*> scripts = obj->getMonoScripts();
			for (auto ct = scripts.begin(); ct != scripts.end(); ++ct)
				(*ct)->destroyManagedObject();
		}
	}

	APIManager::FunctionList APIManager::getFunctions(GameObject * node)
	{
		std::vector<std::pair<std::string, std::string>> list;

		if (node != nullptr)
		{
			std::vector<MonoScript*> scripts = node->getMonoScripts();
			for (auto it = scripts.begin(); it != scripts.end(); ++it)
			{
				MonoObject * obj = (MonoObject*)(*it)->managedObject;
				MonoClass * cls = mono_object_get_class(obj);
				std::string class_name = mono_class_get_name(cls);
				class_name = CP_SYS(class_name);

				void * iter = nullptr;
				MonoMethod * method = nullptr;

				while (method = mono_class_get_methods(cls, &iter))
				{
					std::string method_name = mono_method_get_name(method);

					if (method_name != ".ctor" && method_name != "BeginFrame" && method_name != "EndFrame" && method_name != "Start" && method_name != "OnGUI" && method_name != "Update" && method_name != "FixedUpdate")
					{
						list.push_back(make_pair(class_name, method_name));
					}
				}
			}
		}

		return list;
	}

	MonoClass * APIManager::findClass(std::string name)
	{
		auto it = find_if(behaviourClasses.begin(), behaviourClasses.end(), [name](MonoClass* __class) -> bool { return mono_class_get_name(__class) == CP_UNI(name); });

		if (it != behaviourClasses.end())
			return *it;

		return nullptr;
	}

	void APIManager::retrieveClassesAndFields()
	{
		//Get mono classes
		behaviour_class = mono_class_from_name(base_image, "FalcoEngine", "MonoBehaviour");
		transform_class = mono_class_from_name(base_image, "FalcoEngine", "Transform");
		rigidbody_class = mono_class_from_name(base_image, "FalcoEngine", "Rigidbody");
		vector2_class = mono_class_from_name(base_image, "FalcoEngine", "Vector2");
		vector3_class = mono_class_from_name(base_image, "FalcoEngine", "Vector3");
		vector4_class = mono_class_from_name(base_image, "FalcoEngine", "Vector4");
		color_class = mono_class_from_name(base_image, "FalcoEngine", "Color");
		quaternion_class = mono_class_from_name(base_image, "FalcoEngine", "Quaternion");
		input_class = mono_class_from_name(base_image, "FalcoEngine", "Input");
		gameobject_class = mono_class_from_name(base_image, "FalcoEngine", "GameObject");
		animation_class = mono_class_from_name(base_image, "FalcoEngine", "Animation");
		animationclip_class = mono_class_from_name(base_image, "FalcoEngine", "AnimationClip");
		audiosource_class = mono_class_from_name(base_image, "FalcoEngine", "AudioSource");
		audiolistener_class = mono_class_from_name(base_image, "FalcoEngine", "AudioListener");
		audioclip_class = mono_class_from_name(base_image, "FalcoEngine", "AudioClip");
		videoclip_class = mono_class_from_name(base_image, "FalcoEngine", "VideoClip");
		navmeshagent_class = mono_class_from_name(base_image, "FalcoEngine", "NavMeshAgent");
		navmeshobstacle_class = mono_class_from_name(base_image, "FalcoEngine", "NavMeshObstacle");
		component_class = mono_class_from_name(base_image, "FalcoEngine", "Component");
		font_class = mono_class_from_name(base_image, "FalcoEngine", "Font");
		asset_class = mono_class_from_name(base_image, "FalcoEngine", "Asset");
		object_class = mono_class_from_name(base_image, "FalcoEngine", "Object");
		material_class = mono_class_from_name(base_image, "FalcoEngine", "Material");
		shader_class = mono_class_from_name(base_image, "FalcoEngine", "Shader");
		camera_class = mono_class_from_name(base_image, "FalcoEngine", "Camera");
		uielement_class = mono_class_from_name(base_image, "FalcoEngine", "UIElement");
		canvas_class = mono_class_from_name(base_image, "FalcoEngine", "Canvas");
		text_class = mono_class_from_name(base_image, "FalcoEngine", "Text");
		image_class = mono_class_from_name(base_image, "FalcoEngine", "Image");
		button_class = mono_class_from_name(base_image, "FalcoEngine", "Button");
		text_input_class = mono_class_from_name(base_image, "FalcoEngine", "TextInput");
		mask_class = mono_class_from_name(base_image, "FalcoEngine", "Mask");
		texture_class = mono_class_from_name(base_image, "FalcoEngine", "Texture");
		cubemap_class = mono_class_from_name(base_image, "FalcoEngine", "Cubemap");
		collision_class = mono_class_from_name(base_image, "FalcoEngine", "Collision");
		collider_class = mono_class_from_name(base_image, "FalcoEngine", "Collider");
		meshcollider_class = mono_class_from_name(base_image, "FalcoEngine", "MeshCollider");
		boxcollider_class = mono_class_from_name(base_image, "FalcoEngine", "BoxCollider");
		spherecollider_class = mono_class_from_name(base_image, "FalcoEngine", "SphereCollider");
		capsulecollider_class = mono_class_from_name(base_image, "FalcoEngine", "CapsuleCollider");
		vehicle_class = mono_class_from_name(base_image, "FalcoEngine", "Vehicle");
		meshrenderer_class = mono_class_from_name(base_image, "FalcoEngine", "MeshRenderer");
		prefab_class = mono_class_from_name(base_image, "FalcoEngine", "Prefab");
		rendertexture_class = mono_class_from_name(base_image, "FalcoEngine", "RenderTexture");
		spline_class = mono_class_from_name(base_image, "FalcoEngine", "Spline");
		videoplayer_class = mono_class_from_name(base_image, "FalcoEngine", "VideoPlayer");
		light_class = mono_class_from_name(base_image, "FalcoEngine", "Light");
		mesh_class = mono_class_from_name(base_image, "FalcoEngine", "Mesh");
		submesh_class = mono_class_from_name(base_image, "FalcoEngine", "SubMesh");
		conetwistjoint_class = mono_class_from_name(base_image, "FalcoEngine", "ConeTwistJoint");
		fixedjoint_class = mono_class_from_name(base_image, "FalcoEngine", "FixedJoint");
		freejoint_class = mono_class_from_name(base_image, "FalcoEngine", "FreeJoint");
		hingejoint_class = mono_class_from_name(base_image, "FalcoEngine", "HingeJoint");
		particlesystem_class = mono_class_from_name(base_image, "FalcoEngine", "ParticleSystem");
		particleemitter_class = mono_class_from_name(base_image, "FalcoEngine", "ParticleEmitter");

		//Get mono fields
		gameobject_ptr_field = mono_class_get_field_from_name(gameobject_class, "this_ptr");
		component_ptr_field = mono_class_get_field_from_name(component_class, "this_ptr");
		asset_ptr_field = mono_class_get_field_from_name(asset_class, "asset_ptr");
		object_ptr_field = mono_class_get_field_from_name(object_class, "object_ptr");

		vector2_x = mono_class_get_field_from_name(vector2_class, "x");
		vector2_y = mono_class_get_field_from_name(vector2_class, "y");

		vector3_x = mono_class_get_field_from_name(vector3_class, "x");
		vector3_y = mono_class_get_field_from_name(vector3_class, "y");
		vector3_z = mono_class_get_field_from_name(vector3_class, "z");

		vector4_x = mono_class_get_field_from_name(vector4_class, "x");
		vector4_y = mono_class_get_field_from_name(vector4_class, "y");
		vector4_z = mono_class_get_field_from_name(vector4_class, "z");
		vector4_w = mono_class_get_field_from_name(vector4_class, "w");

		quaternion_x = mono_class_get_field_from_name(quaternion_class, "x");
		quaternion_y = mono_class_get_field_from_name(quaternion_class, "y");
		quaternion_z = mono_class_get_field_from_name(quaternion_class, "z");
		quaternion_w = mono_class_get_field_from_name(quaternion_class, "w");

		color_r = mono_class_get_field_from_name(color_class, "r");
		color_g = mono_class_get_field_from_name(color_class, "g");
		color_b = mono_class_get_field_from_name(color_class, "b");
		color_a = mono_class_get_field_from_name(color_class, "a");

		collision_other = mono_class_get_field_from_name(collision_class, "other");
		collision_point = mono_class_get_field_from_name(collision_class, "point");
		collision_normal = mono_class_get_field_from_name(collision_class, "normal");
	}

	std::vector<MonoClass*> APIManager::getAssemblyClassList(MonoImage * image)
	{
		std::vector<MonoClass*> class_list;

		const MonoTableInfo* table_info = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);

		int rows = mono_table_info_get_rows(table_info);

		/* For each row, get some of its values */
		for (int i = 0; i < rows; i++)
		{
			MonoClass* _class = nullptr;
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(table_info, i, cols, MONO_TYPEDEF_SIZE);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			const char* name_space = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			_class = mono_class_from_name(image, name_space, name);

			if (_class != nullptr)
			{
				if (mono_class_is_subclass_of(_class, behaviour_class, false))
				{
					if (name != "<Module>" && name != "MonoBehaviour")
						class_list.push_back(_class);
				}
			}
		}

		return class_list;
	}

	std::string APIManager::getStringProperty(const char *propertyName, MonoClass *classType, MonoObject *classObject)
	{
		if (classObject == nullptr)
			return "";

		MonoProperty *messageProperty;
		MonoMethod *messageGetter;
		MonoString *messageString;

		messageProperty = mono_class_get_property_from_name(classType, propertyName);
		if (messageProperty != nullptr)
		{
			messageGetter = mono_property_get_get_method(messageProperty);
			if (messageGetter == nullptr)
				return "";

			messageString = (MonoString*)mono_runtime_invoke(messageGetter, classObject, NULL, NULL);
			if (messageString == nullptr)
				return "";

			std::string str = mono_string_to_utf8(messageString);
			str = CP_SYS(str);

			return str;
		}

		return "";
	}

	void APIManager::execute(std::string methodName, void ** params, std::string stringParams)
	{
		if (Scene::getLoadedScene().empty())
			return;

		std::vector<Transform*> roots = Engine::getSingleton()->getRootTransforms();

		for (auto it = roots.begin(); it != roots.end(); ++it)
			execute((*it)->getGameObject(), methodName, params, stringParams);
	}

	void APIManager::execute(GameObject* root, std::string methodName, void** params, std::string stringParams)
	{
		std::vector<Transform*> nstack;
		nstack.push_back(root->getTransform());

		while (nstack.size() > 0)
		{
			Transform* child = *nstack.begin();
			nstack.erase(nstack.begin());

			if (!child->getGameObject()->getActive())
				continue;

			//
			bool destroyed = false;
			std::vector<MonoScript*> scripts = child->getGameObject()->getMonoScripts();
			for (auto it : scripts)
			{
				if (!it->enabled)
					continue;

				if (methodName == "Start")
				{
					if (!it->startExecuted)
						it->startExecuted = true;
				}

				execute(it->managedObject, methodName, params, stringParams);

				auto _it = std::find(objectsToDestroy.begin(), objectsToDestroy.end(), child->getGameObject());
				if (_it != objectsToDestroy.end())
				{
					destroyed = true;
					break;
				}
			}
			//

			if (!destroyed)
			{
				int j = 0;
				for (auto it = child->getChildren().begin(); it != child->getChildren().end(); ++it, ++j)
				{
					Transform* ch = *it;
					nstack.insert(nstack.begin() + j, ch);
				}
			}
		}
	}

	void APIManager::executeForNode(GameObject* node, std::string methodName, std::string className, void** params, std::string stringParams)
	{
		if (Scene::getLoadedScene().empty())
			return;

		if (node == nullptr)
			return;

		if (!node->getActive())
			return;

		if (className.empty())
		{
			std::vector<MonoScript*> scripts = node->getMonoScripts();
			for (auto it : scripts)
			{
				MonoObject* obj = (MonoObject*)it->managedObject;

				if (!it->enabled)
					continue;

				execute(obj, methodName, params, stringParams);

				auto _it = std::find(objectsToDestroy.begin(), objectsToDestroy.end(), node);
				if (_it != objectsToDestroy.end())
					break;
			}
		}
		else
		{
			std::vector<MonoScript*> scripts = node->getMonoScripts();
			auto it = std::find_if(scripts.begin(), scripts.end(), [className](MonoScript* script) -> bool { return mono_class_get_name(script->managedClass) == CP_UNI(className); });

			if (it != scripts.end())
			{
				MonoObject* obj = (MonoObject*)(*it)->managedObject;

				if (!(*it)->enabled)
					return;

				execute(obj, methodName, params, stringParams);
			}
		}
	}

	void APIManager::execute(MonoObject* object, std::string methodName, void** params, std::string stringParams)
	{
		if (object == nullptr)
			return;

		//Build a method description object
		MonoClass* mclass = mono_object_get_class(object);
		std::string class_name = mono_class_get_name(mclass);
		class_name = CP_SYS(class_name);

		std::string methodDescStr = std::string(class_name + ":" + methodName + "(" + stringParams + ")").c_str();
		MonoMethodDesc*  methodDesc = mono_method_desc_new(methodDescStr.c_str(), false);

		MonoMethod* method = nullptr;

		if (methodDesc != nullptr)
		{
			//Search the method in the image
			method = mono_method_desc_search_in_class(methodDesc, mclass);
			mono_method_desc_free(methodDesc);
		}

		while (method == nullptr)
		{
			mclass = mono_class_get_parent(mclass);

			if (mclass == nullptr)
				break;

			class_name = mono_class_get_name(mclass);
			class_name = CP_SYS(class_name);

			methodDescStr = std::string(class_name + ":" + methodName + "(" + stringParams + ")").c_str();
			methodDesc = mono_method_desc_new(methodDescStr.c_str(), false);

			if (methodDesc)
			{
				//Search the method in the image
				method = mono_method_desc_search_in_class(methodDesc, mclass);
				mono_method_desc_free(methodDesc);
			}
		}

		if (method != nullptr)
		{
			MonoObject* except = nullptr;
			mono_runtime_invoke(method, object, params, (MonoObject**)&except);

			if (except != nullptr)
				reportException(except);
		}
	}

	void APIManager::executeStatic(MonoClass* klass, std::string methodName, void** params)
	{
		MonoMethodDesc* methodDesc = mono_method_desc_new(methodName.c_str(), false);
		MonoMethod* method = nullptr;

		if (methodDesc != nullptr)
		{
			//Search the method in the image
			method = mono_method_desc_search_in_class(methodDesc, klass);
			mono_method_desc_free(methodDesc);
		}

		if (method != nullptr)
		{
			MonoObject* except = nullptr;
			mono_runtime_invoke(method, nullptr, params, (MonoObject**)&except);

			if (except != nullptr)
				reportException(except);
		}
	}

	void APIManager::destroyNodes()
	{
		for (auto it = objectsToDestroy.begin(); it != objectsToDestroy.end(); ++it)
		{
			GameObject* node = *it;
			Engine::getSingleton()->destroyGameObject(node);
		}

		objectsToDestroy.clear();
	}

	void APIManager::createObjects()
	{
		std::vector<GameObject*> objects = Engine::getSingleton()->getGameObjects();

		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			GameObject* obj = *it;
			std::vector<MonoScript*> scripts = obj->getMonoScripts();
			for (auto ct = scripts.begin(); ct != scripts.end(); ++ct)
				(*ct)->createManagedObject();
		}
	}

	void APIManager::addDestroyObject(GameObject* node)
	{
		if (std::find(objectsToDestroy.begin(), objectsToDestroy.end(), node) == objectsToDestroy.end())
			objectsToDestroy.push_back(node);
	}

	void APIManager::checkSceneToLoad()
	{
		if (!sceneToLoad.empty())
		{
			std::string _scene = sceneToLoad;
			sceneToLoad = "";

			Scene::load(Engine::getSingleton()->getAssetsPath(), _scene);
		}
	}

	void APIManager::reportException(MonoObject* except)
	{
		if (except == nullptr)
			return;

		MonoClass* exceptionClass;
		MonoType* exceptionType;
		std::string _typeName = "", _message = "", _source = "", _stackTrace = "";

		exceptionClass = mono_object_get_class(except);
		exceptionType = mono_class_get_type(exceptionClass);
		_typeName = mono_type_get_name(exceptionType);
		_message = getStringProperty("Message", exceptionClass, except);
		_source = getStringProperty("Source", exceptionClass, except);
		_stackTrace = getStringProperty("StackTrace", exceptionClass, except);

		std::string error = "Exception: " + _typeName + "\n";
		if (_message != "") error += _message + "\n";
		if (_source != "") error += _source + "\n";
		if (_stackTrace != "") error += _stackTrace + "\n";
		error += "\n";

		std::cout << error;

		Debug::logError(error, false);
	}
}