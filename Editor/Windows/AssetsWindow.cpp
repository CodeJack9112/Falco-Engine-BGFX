#include "AssetsWindow.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>
#include <thread>

#include "MainWindow.h"
#include "InspectorWindow.h"
#include "ConsoleWindow.h"
#include "AnimationEditorWindow.h"

#include "../PropertyEditors/MaterialEditor.h"
#include "../PropertyEditors/CubemapEditor.h"
#include "../PropertyEditors/TextureEditor.h"
#include "../PropertyEditors/Model3DEditor.h"
#include "../PropertyEditors/AudioClipEditor.h"
#include "../PropertyEditors/VideoClipEditor.h"
#include "../Dialogs/DialogAnimationCrop.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/APIManager.h"
#include "../Engine/Core/Debug.h"
#include "../Engine/Renderer/BatchedGeometry.h"
#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"
#include "../Engine/Assets/Scene.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/Shader.h"
#include "../Engine/Assets/Cubemap.h"
#include "../Engine/Assets/Model3DLoader.h"
#include "../Engine/Assets/AnimationClip.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Assets/Mesh.h"
#include "../Engine/Assets/AudioClip.h"
#include "../Engine/Assets/VideoClip.h"
#include "../Engine/Assets/Prefab.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Classes/GUIDGenerator.h"
#include "../Classes/SolutionWorker.h"

#ifndef FREEIMAGE_LIB
#define FREEIMAGE_LIB
#include "../FreeImage/include/FreeImage.h"
#undef FREEIMAGE_LIB
#endif

#ifdef _WIN32
#include <shellapi.h>
#include <ShlObj_core.h>
#else
#include <boost/process.hpp>
#include <sys/types.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <limits.h>
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 255 ) )
#define NOTIFY_FLAGS IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO | IN_MOVED_FROM | IN_CLOSE_WRITE
#endif

namespace GX
{
	AssetsWindow* AssetsWindow::singleton = nullptr;
	bool AssetsWindow::scriptsChanged = false;
	std::vector<std::string> AssetsWindow::changedFiles;

	AssetsWindow::AssetsWindow()
	{
		singleton = this;

		treeView = new TreeView();
		treeView->setTag("Assets");
		//treeView->setAllowMoveNodes(true);
		treeView->setProcessDragDropInTarget(true);
		treeView->setSelectCallback(onNodesSelected);

		treeView->getRootNode()->setSupportedFormats({ "*.", "::GameObject" });
		treeView->getRootNode()->setOnDropCallback([=](TreeNode* node, TreeNode* from) { onDropNode(node, from); });
	}

	AssetsWindow::~AssetsWindow()
	{
		fswatcher->ExitInstance();
		
		delete fswatcher;
		delete treeView;
	}

	void AssetsWindow::init()
	{
		addIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/add.png", false, Texture::CompressionMethod::None, true);
		addFolderIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/add_folder.png", false, Texture::CompressionMethod::None, true);
		closeIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/close.png", false, Texture::CompressionMethod::None, true);

		dialogAnimationCrop = new DialogAnimationCrop();

		folderIcon = MainWindow::loadEditorIcon("Assets/folder.png");
		meshIcon = MainWindow::loadEditorIcon("Assets/mesh.png");
		textureIcon = MainWindow::loadEditorIcon("Assets/texture.png");
		materialIcon = MainWindow::loadEditorIcon("Assets/material.png");
		sceneIcon = MainWindow::loadEditorIcon("Assets/scene.png");
		shaderIcon = MainWindow::loadEditorIcon("Assets/shader.png");
		cubemapIcon = MainWindow::loadEditorIcon("Assets/cubemap.png");
		csIcon = MainWindow::loadEditorIcon("Assets/cs.png");
		audioClipIcon = MainWindow::loadEditorIcon("Assets/audio_clip.png");
		fontIcon = MainWindow::loadEditorIcon("Assets/font.png");
		prefabIcon = MainWindow::loadEditorIcon("Assets/prefab.png");
		textIcon = MainWindow::loadEditorIcon("Assets/text.png");
		animationClipIcon = MainWindow::loadEditorIcon("Assets/animation_clip.png");
		terrainIcon = MainWindow::loadEditorIcon("Inspector/terrain.png");
		packageIcon = MainWindow::loadEditorIcon("Other/package.png");
		navmeshIcon = MainWindow::loadEditorIcon("Toolbar/navmesh.png");
		videoClipIcon = MainWindow::loadEditorIcon("Assets/video_clip.png");

		renameIcon = MainWindow::loadEditorIcon("Menu/rename.png");
		deleteIcon = MainWindow::loadEditorIcon("Menu/delete_object.png");
		showInExplorerIcon = MainWindow::loadEditorIcon("Menu/explorer.png");
		reloadIcon = MainWindow::loadEditorIcon("Menu/reload.png");
		extractAnimsIcon = MainWindow::loadEditorIcon("Menu/extract_anim.png");
		extractAudioIcon = MainWindow::loadEditorIcon("Menu/extract_audio.png");
		cropAnimIcon = MainWindow::loadEditorIcon("Menu/crop_anim.png");

		reloadFiles();

		fswatcher = new FileSystemWatcher();
		fswatcher->InitInstance();
	}

	void AssetsWindow::update()
	{
		bool rootOpened = true;

		if (opened)
		{
			if (ImGui::Begin("Assets", &opened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				focused = ImGui::IsWindowFocused();

				updateAssetsMenu();

				/* TREEVIEW */

				ImGuiWindow* window = GImGui->CurrentWindow;
				ImGuiID id = window->GetIDNoKeepAlive("##AssetsVS");
				ImGui::BeginChild(id);
				//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 0));
				treeView->update();
				//ImGui::PopStyleVar();
				ImGui::EndChild();

				/* TREEVIEW */

				if (ImGui::IsMouseDown(0) && ImGui::IsMouseDragging(0))
				{
					isWindowDragging = true;
				}

				//if (!isWindowDragging || ImGui::IsMouseReleased(0))
				treeView->updateDragDropTarget();

				if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(0))
				{
					if (!isWindowDragging)
					{
						if (!ImGui::IsAnyItemHovered())
						{
							treeView->selectNodes({  });
						}
					}
				}

				if (ImGui::IsMouseReleased(0))
				{
					isWindowDragging = false;
				}
			}

			ImGui::End();
		}

		dialogAnimationCrop->update();
	}

	void AssetsWindow::focusOnFile(std::string filepath)
	{
		TreeNode* node = treeView->getNodeByPath(filepath, treeView->getRootNode());
		if (node != nullptr)
		{
			treeView->selectNodes({ node->name }, false);
			treeView->focusOnNode(node);
		}
	}

	Texture* AssetsWindow::getIconByExtension(std::string ext)
	{
		Texture * icon = Texture::load(Helper::ExePath(), "Editor/Icons/Assets/file.png", false, Texture::CompressionMethod::None, true);

		std::vector<std::string> imgs = Engine::getImageFileFormats();
		std::vector<std::string> models = Engine::getModel3dFileFormats();
		std::vector<std::string> sounds = Engine::getAudioFileFormats();
		std::vector<std::string> videos = Engine::getVideoFileFormats();

		if (ext.empty())
			icon = folderIcon;
		if (std::find(models.begin(), models.end(), ext) != models.end())
			icon = meshIcon;
		if (std::find(imgs.begin(), imgs.end(), ext) != imgs.end())
			icon = textureIcon;
		if (std::find(sounds.begin(), sounds.end(), ext) != sounds.end())
			icon = audioClipIcon;
		if (std::find(videos.begin(), videos.end(), ext) != videos.end())
			icon = videoClipIcon;
		if (ext == "material")
			icon = materialIcon;
		if (ext == "scene")
			icon = sceneIcon;
		if (ext == "shader")
			icon = shaderIcon;
		if (ext == "cubemap")
			icon = cubemapIcon;
		if (ext == "cs")
			icon = csIcon;
		if (ext == "ttf" || ext == "otf")
			icon = fontIcon;
		if (ext == "prefab")
			icon = prefabIcon;
		if (ext == "txt")
			icon = textIcon;
		if (ext == "animation")
			icon = animationClipIcon;
		if (ext == "terrain")
			icon = terrainIcon;
		if (ext == "package")
			icon = packageIcon;
		if (ext == "navmesh")
			icon = navmeshIcon;

		return icon;
	}

	void AssetsWindow::sortTree()
	{
		MainWindow::addOnEndUpdateCallback([=]() {
			sortTree(treeView->getRootNode());
		});
	}

	void AssetsWindow::sortTree(TreeNode* root)
	{
		std::vector<TreeNode*> vec1;
		std::vector<TreeNode*> vec2;

		for (auto it = root->children.begin(); it != root->children.end(); ++it)
		{
			std::string _a = Engine::getSingleton()->getAssetsPath() + CP_SYS((*it)->getPath());

			if (IO::isDir(_a))
			{
				vec1.push_back(*it);
			}
			else
			{
				vec2.push_back(*it);
			}
		}

		std::sort(vec1.begin(), vec1.end(), [=](TreeNode*& a, TreeNode*& b) -> bool {
			return boost::algorithm::to_lower_copy(a->alias) < boost::algorithm::to_lower_copy(b->alias);
		});

		std::sort(vec2.begin(), vec2.end(), [=](TreeNode*& a, TreeNode*& b) -> bool {
			return boost::algorithm::to_lower_copy(a->alias) < boost::algorithm::to_lower_copy(b->alias);
		});

		vec1.insert(vec1.end(), vec2.begin(), vec2.end());

		root->children.clear();
		root->children = vec1;

		vec1.clear();
		vec2.clear();

		for (auto it = root->children.begin(); it != root->children.end(); ++it)
			sortTree(*it);
	}

	void AssetsWindow::reloadFiles()
	{
		std::vector<std::string> allFilesNames;
		std::vector<std::string> allFilesNames1;
		std::vector<std::string> addedFilesNames;
		std::vector<std::string> removeFilesNames;

		//Store all items
		rememberAllItems(treeView->getRootNode(), allFilesNames);

		removeFilesNames = addNewFiles(allFilesNames);
		addedFilesNames = removeFilesNames;

		allFilesNames1 = addedFilesNames;

		addedFilesNames.erase(remove_if(begin(addedFilesNames), end(addedFilesNames), [&](auto x)
		{
			return find(begin(allFilesNames), end(allFilesNames), x) != end(allFilesNames);
		}), end(addedFilesNames));

		allFilesNames.erase(remove_if(begin(allFilesNames), end(allFilesNames), [&](auto x)
		{
			return find(begin(removeFilesNames), end(removeFilesNames), x) != end(removeFilesNames);
		}), end(allFilesNames));

		removeFilesNames = allFilesNames;
		allFilesNames.clear();

		if (removeFilesNames.size() > 0)
		{
			MainWindow::getSingleton()->getInspectorWindow()->setEditor(nullptr);

			auto& models = Engine::getModel3dFileFormats();
			auto& images = Engine::getImageFileFormats();

			std::string location = Engine::getSingleton()->getAssetsPath();

			bool rebuildGeom = false;

			for (auto it = removeFilesNames.begin(); it < removeFilesNames.end(); ++it)
			{
				std::string path = *it;
				std::string ext = IO::GetFileExtension(path);

				if (std::find(models.begin(), models.end(), ext) != models.end())
				{
					for (auto& asset : Asset::getLoadedInstances())
					{
						if (asset.second->getAssetType() == Mesh::ASSET_TYPE)
						{
							Mesh* mesh = (Mesh*)asset.second;
							if (mesh->getSourceFile() == path)
							{
								if (IO::FileExists(mesh->getOrigin()))
									IO::FileDelete(mesh->getOrigin());

								mesh->unload();

								rebuildGeom = true;

								break;
							}
						}
					}
				}
				else if (std::find(images.begin(), images.end(), ext) != images.end())
				{
					for (auto& asset : Asset::getLoadedInstances())
					{
						if (asset.second->getAssetType() == Texture::ASSET_TYPE)
						{
							Texture* texture = (Texture*)asset.second;
							if (texture->getName() == path)
							{
								std::string texPath = texture->getCachedFileName();

								if (IO::FileExists(texPath))
									IO::FileDelete(texPath);

								texture->unload();

								break;
							}
						}
					}
				}
				else if (ext == "animation")
				{
					Asset* inst = Asset::getLoadedInstance(location, path);
					if (inst != nullptr)
					{
						AnimationEditorWindow* animEditor = MainWindow::getAnimationEditorWindow();

						if (inst == animEditor->getOpenedAnimation())
						{
							animEditor->closeAnimation();
						}

						if (inst->isLoaded())
							inst->unload();
					}
				}
				else
				{
					Asset* asset = Asset::getLoadedInstance(location, path);
					if (asset != nullptr && asset->isLoaded())
						asset->unload();
				}

				TreeNode* del = treeView->getNodeByPath(CP_UNI(path), treeView->getRootNode());
				if (del != NULL)
				{
					treeView->deleteNode(del);
				}
			}

			if (rebuildGeom)
			{
				MainWindow::getSingleton()->addOnEndUpdateCallback([]()
					{
						BatchedGeometry::getSingleton()->rebuild();
					}
				);
			}
		}

		//Scripts
		if (addedFilesNames.size() > 0 || removeFilesNames.size() > 0)
		{
			addScripts(allFilesNames1);
			reloadShaderList();
		}

		allFilesNames1.clear();
		addedFilesNames.clear();
		removeFilesNames.clear();
	}

	void AssetsWindow::reloadShaderList()
	{
		shaderList.clear();

		std::string assetsPath = Engine::getSingleton()->getAssetsPath();
		std::string builtinPath = Engine::getSingleton()->getBuiltinResourcesPath();

		IO::listFiles(builtinPath, true, nullptr, [=](std::string dir, std::string file) -> bool
			{
				std::string ext = IO::GetFileExtension(file);

				if (ext == "shader")
				{
					std::string fullPath = dir + file;
					std::string filename = IO::RemovePart(fullPath, builtinPath);
					Shader* sh = Shader::load(builtinPath, filename);
					if (sh != nullptr && !sh->getAlias().empty())
						shaderList.push_back(sh);
				}

				return true;
			}
		);

		IO::listFiles(assetsPath, true, nullptr, [=](std::string dir, std::string file) -> bool
			{
				std::string ext = IO::GetFileExtension(file);

				if (ext == "shader")
				{
					std::string fullPath = dir + file;
					std::string filename = IO::RemovePart(fullPath, assetsPath);
					Shader* sh = Shader::load(assetsPath, filename);
					if (sh != nullptr && !sh->getAlias().empty())
						shaderList.push_back(sh);
				}

				return true;
			}
		);
	}

	void AssetsWindow::reloadChangedFiles()
	{
		if (scriptsChanged)
		{
			if (!Engine::getSingleton()->getIsRuntimeMode())
			{
				if (!MainWindow::getSingleton()->isScriptsCompiling())
					APIManager::getSingleton()->compile(APIManager::CompileConfiguration::Debug);
			}

			scriptsChanged = false;
		}

		bool rebuildGeom = false;

		auto& models = Engine::getModel3dFileFormats();
		for (auto it : changedFiles)
		{
			reloadChangedFile(it);

			if (!rebuildGeom)
			{
				std::string ext = IO::GetFileExtension(it);
				if (std::find(models.begin(), models.end(), ext) != models.end())
					rebuildGeom = true;
			}
		}

		if (rebuildGeom)
		{
			MainWindow::getSingleton()->addOnEndUpdateCallback([]()
				{
					BatchedGeometry::getSingleton()->rebuild();
				}
			);
		}

		changedFiles.clear();
	}

	void AssetsWindow::reloadChangedFile(std::string path)
	{
		auto& models = Engine::getModel3dFileFormats();
		std::string ext = IO::GetFileExtension(path);

		//If not a 3d model
		if (std::find(models.begin(), models.end(), ext) == models.end())
		{
			Asset* asset = Asset::getLoadedInstance(Engine::getSingleton()->getAssetsPath(), path);
			if (asset != nullptr)
			{
				asset->reload();

				if (asset->getAssetType() == Texture::ASSET_TYPE)
					updateTextureIcon(path, true);
			}
		}
		else
		{
			for (auto& asset : Asset::getLoadedInstances())
			{
				if (asset.second->getAssetType() == Mesh::ASSET_TYPE)
				{
					Mesh* mesh = (Mesh*)asset.second;
					if (mesh->getSourceFile() == path)
					{
						if (IO::FileExists(mesh->getOrigin()))
							IO::FileDelete(mesh->getOrigin());

						mesh->unload();
					}
				}
			}

			Model3DLoader::cache3DModel(Engine::getSingleton()->getAssetsPath(), path);
		}
	}

	void AssetsWindow::addChangedFile(std::string value)
	{
		if (std::find(changedFiles.begin(), changedFiles.end(), value) == changedFiles.end())
			changedFiles.push_back(value);
	}

	std::vector<std::string> AssetsWindow::addNewFiles(std::vector<std::string> files)
	{
		std::vector<std::string> filesNames;
		std::vector<std::string> filesNames1;

		struct LambdaFunc
		{
			std::vector<std::string>* filesNames;

			bool operator() (std::string dir)
			{
				std::string str = IO::RemovePart(dir, Engine::getSingleton()->getAssetsPath());
				filesNames->push_back(str);

				return true;
			}

			bool operator() (std::string dir, std::string filename)
			{
				std::string str = IO::RemovePart(dir + filename, Engine::getSingleton()->getAssetsPath());
				filesNames->push_back(str);

				return true;
			}
		};

		LambdaFunc func;
		func.filesNames = &filesNames;

		IO::listFiles(Engine::getSingleton()->getAssetsPath(), true, func, func);

		filesNames1 = filesNames;

		for (auto it = files.begin(); it != files.end(); ++it)
		{
			auto _it = std::find(filesNames.begin(), filesNames.end(), *it);
			if (_it != filesNames.end())
				filesNames.erase(_it);
		}

		for (auto it = filesNames.begin(); it != filesNames.end(); ++it)
		{
			std::string path = *it;
			if (treeView->getNodeByPath(CP_UNI(path), treeView->getRootNode()) == nullptr)
			{
				std::vector<std::string> results;
				boost::split(results, path, [](char c) { return c == '/' || c == '\\'; });

				TreeNode* item = treeView->getRootNode();

				for (auto it2 = results.begin(); it2 != results.end(); ++it2)
				{
					TreeNode* newItem = treeView->getNodeByAlias(CP_UNI(*it2), item, false);

					if (newItem == nullptr)
					{
						std::string ext = IO::GetFileExtension(*it2);
						
						Texture* icon = getIconByExtension(ext);

						std::vector<std::string>& imgs = Engine::getSingleton()->getImageFileFormats();
						if (std::find(imgs.begin(), imgs.end(), ext) != imgs.end())
						{
							Texture* ic = updateTextureIcon(path);
							if (ic != nullptr)
								icon = ic;
						}

						bool isDir = IO::isDir(Engine::getSingleton()->getAssetsPath() + path);

						if (isDir)
							icon = Texture::load(Helper::ExePath(), "Editor/Icons/Assets/folder.png", false, Texture::CompressionMethod::None, true);

						std::vector<std::string> popup = { "Rename", "Delete", "Show in Explorer", "Reload" };
						std::vector<Texture*> icons = { renameIcon, deleteIcon, showInExplorerIcon, reloadIcon };
						
						auto& extsModels = Engine::getModel3dFileFormats();
						auto& extsVideo = Engine::getVideoFileFormats();

						if (std::find(extsModels.begin(), extsModels.end(), ext) != extsModels.end())
						{
							popup.push_back("-");
							popup.push_back("Extract animations");

							icons.push_back(extractAnimsIcon);
						}
						else if (ext == "animation")
						{
							popup.push_back("-");
							popup.push_back("Crop animation");

							icons.push_back(cropAnimIcon);
						}
						else if (std::find(extsVideo.begin(), extsVideo.end(), ext) != extsVideo.end())
						{
							popup.push_back("-");
							popup.push_back("Extract audio");

							icons.push_back(extractAudioIcon);
						}

						TreeNode* newNode = new TreeNode(treeView);
						newNode->alias = CP_UNI(*it2);
						newNode->name = GUIDGenerator::genGuid();
						newNode->icon = icon;
						newNode->enableDrag = true;
						newNode->format = ext;
						newNode->setOnEndEditCallback([=](TreeNode* node, std::string prevAlias) -> bool { return onNodeRenamed(node, prevAlias); });
						newNode->setPopupMenu(popup, [=](TreeNode* node, int val) { onNodePopupMenu(node, val); });
						newNode->setPopupMenuIcons(icons);
						newNode->setOnDoubleClickCallback([=](TreeNode* node) { onNodeDoubleClick(node); });
						newNode->setCustomPopupMenuProcedure([=](TreeNode* node) { updateAssetsPopup(true, node); });
						newNode->setCustomPopupMenuPosition(TreeNode::CustomPopupMenuPosition::CPMP_BEGIN);

						if (isDir)
						{
							newNode->setSupportedFormats({ "*.", "::GameObject" });
							newNode->setOnDropCallback([=](TreeNode* node, TreeNode* from) { onDropNode(node, from); });
						}

						item->addChild(newNode);
						item = newNode;
					}
					else
					{
						item = newItem;
					}
				}

				results.clear();
			}
		}

		sortTree();

		return filesNames1;
	}

	void AssetsWindow::addScripts(std::vector<std::string> files)
	{
		std::vector<std::string> scripts = files;

		if (files.size() == 0)
			rememberAllItems(treeView->getRootNode(), scripts);

		scripts.erase(remove_if(begin(scripts), end(scripts), [&](auto x)
		{
			return x.find(".cs") == std::string::npos;
		}), end(scripts));

		std::vector<std::string> _scripts;
		for (std::vector<std::string>::iterator it = scripts.begin(); it != scripts.end(); ++it)
		{
			std::string _str = "Assets\\" + IO::ReplaceFrontSlashes(*it);
			_scripts.push_back(_str);
		}

		files.clear();
		scripts.clear();

		SolutionWorker solutionWorker;
		solutionWorker.CreateSolution(Engine::getSingleton()->getRootPath(), Engine::getSingleton()->getAppName(), _scripts);
	}

	void AssetsWindow::rememberExpandedItems(TreeNode* root)
	{
		if (root->expanded)
			expandedItems.push_back(treeView->getNodePath(root));

		for (auto it = root->children.begin(); it != root->children.end(); ++it)
			rememberExpandedItems(*it);
	}

	void AssetsWindow::rememberAllItems(TreeNode* root, std::vector<std::string>& list)
	{
		std::string fName = CP_SYS(treeView->getNodePath(root));

		if (fName != "")
			list.push_back(fName);

		for (auto it = root->children.begin(); it != root->children.end(); ++it)
			rememberAllItems(*it, list);
	}

	void AssetsWindow::createFolder(TreeNode* selNode)
	{
		if (treeView->getSelectedNodes().size() <= 1)
		{
			TreeNode* root = treeView->getRootNode();

			if (selNode != nullptr)
				root = selNode;
			else
			{
				auto nodes = treeView->getSelectedNodes();

				if (nodes.size() == 1)
					root = nodes[0];
			}

			root->expanded = true;

			if (!IO::isDir(Engine::getSingleton()->getAssetsPath() + CP_SYS(root->getPath())))
			{
				root = root->parent;
			}

			TreeNode * addedAssetNode = new TreeNode(treeView);
			addedAssetNode->name = "##" + GUIDGenerator::genGuid();
			addedAssetNode->icon = getIconByExtension("");
			addedAssetNode->setEditMode(true);
			addedAssetNode->setOnEndEditCallback([=](TreeNode* node, std::string prevAlias) -> bool {
				std::string name = CP_SYS(node->alias);
				std::string _path = CP_SYS(node->getPath());

				if (name.find("/") != std::string::npos || name.find("\\") != std::string::npos)
					name = "";

				if (!name.empty() && !IO::DirExists(Engine::getSingleton()->getAssetsPath() + _path))
				{
					IO::CreateDir(Engine::getSingleton()->getAssetsPath() + _path);

					MainWindow::addOnEndUpdateCallback([=]()
						{
							treeView->deleteNode(addedAssetNode);
							reloadFiles();
						}
					);
				}
				else
				{
					MainWindow::addOnEndUpdateCallback([=]()
						{
							treeView->deleteNode(addedAssetNode);
						}
					);
				}

				return true;
				}
			);

			root->addChild(addedAssetNode);

			treeView->focusOnNode(addedAssetNode);
		}
	}

	Texture* AssetsWindow::updateTextureIcon(std::string path, bool recreate)
	{
		Texture* icon = nullptr;

		std::string iconCacheDir = Engine::getSingleton()->getRootPath() + "Temp/IconCache/";
		std::string fullPath = Engine::getSingleton()->getAssetsPath() + path;

		if (IO::FileExists(fullPath))
		{
			std::string iconName = IO::GetFileName(path) + ".png";
			std::string outDir = iconCacheDir + IO::GetFilePath(path);
			std::string outPath = outDir + iconName;

			if (!IO::DirExists(IO::GetFilePath(outDir)))
				IO::CreateDir(outDir, true);

			if (recreate)
			{
				if (IO::FileExists(outPath))
					IO::FileDelete(outPath);
			}

			if (!IO::FileExists(outPath))
			{
				FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(fullPath.c_str(), 0);
				FIBITMAP* imagen = FreeImage_Load(formato, fullPath.c_str());
				if (FreeImage_GetBPP(imagen) != 32)
				{
					FIBITMAP* convert = FreeImage_ConvertTo32Bits(imagen);
					FreeImage_Unload(imagen);
					imagen = convert;
				}

				FIBITMAP* scaled = FreeImage_Rescale(imagen, 16, 16, FREE_IMAGE_FILTER::FILTER_BOX);
				FreeImage_Unload(imagen);
				imagen = scaled;

				FreeImage_Save(FREE_IMAGE_FORMAT::FIF_PNG, imagen, CP_UNI(outPath).c_str());
				FreeImage_Unload(imagen);
			}

			if (IO::FileExists(outPath))
			{
				if (recreate)
				{
					Asset* asset = Asset::getLoadedInstance(outDir, iconName);
					if (asset != nullptr)
					{
						asset->setPersistent(false);
						asset->unload();
					}
				}

				icon = Texture::load(outDir, iconName, false, Texture::CompressionMethod::None, true, false);
			}
		}

		return icon;
	}

	void AssetsWindow::onNodesSelected(std::vector<std::string> names, void* userData)
	{
		if (names.size() > 0)
		{
			TreeNode* node = singleton->treeView->getNodeByName(names[0]);
			if (node == nullptr)
				return;

			std::string fileName = node->getPath();
			std::string ext = IO::GetFileExtension(fileName);
			auto& imgs = Engine::getImageFileFormats();
			auto& models = Engine::getModel3dFileFormats();
			auto& audios = Engine::getAudioFileFormats();
			auto& videos = Engine::getVideoFileFormats();

			if (ext == "material")
			{
				Material* mat = Material::load(Engine::getSingleton()->getAssetsPath(), fileName);

				if (mat != nullptr)
				{
					MaterialEditor* ed = new MaterialEditor();
					ed->init(mat, true);

					MainWindow::getInspectorWindow()->setEditor(ed);
				}
			}

			if (ext == "cubemap")
			{
				Cubemap* cubemap = Cubemap::load(Engine::getSingleton()->getAssetsPath(), fileName);

				if (cubemap != nullptr)
				{
					CubemapEditor* ed = new CubemapEditor();
					ed->init(cubemap);

					MainWindow::getInspectorWindow()->setEditor(ed);
				}
			}

			if (std::find(imgs.begin(), imgs.end(), ext) != imgs.end())
			{
				std::vector<Texture*> textures;
				for (auto& it : names)
				{
					TreeNode* n = singleton->treeView->getNodeByName(it);
					if (n != nullptr)
					{
						Texture* texture = Texture::load(Engine::getSingleton()->getAssetsPath(), n->getPath(), true, Texture::CompressionMethod::Default);
						if (texture != nullptr && texture->isLoaded())
							textures.push_back(texture);
					}
				}

				TextureEditor* ed = new TextureEditor();
				ed->init(textures);
				textures.clear();

				MainWindow::getInspectorWindow()->setEditor(ed);
			}

			if (std::find(models.begin(), models.end(), ext) != models.end() || ext == "prefab")
			{
				std::vector<std::string> paths;
				for (auto& it : names)
				{
					TreeNode* n = singleton->treeView->getNodeByName(it);
					if (n != nullptr)
						paths.push_back(n->getPath());
				}

				Model3DEditor* ed = new Model3DEditor();
				ed->init(paths);

				MainWindow::getInspectorWindow()->setEditor(ed);

				singleton->reloadFiles();
			}

			if (std::find(audios.begin(), audios.end(), ext) != audios.end())
			{
				AudioClip* audio = AudioClip::load(Engine::getSingleton()->getAssetsPath(), fileName);

				if (audio != nullptr)
				{
					AudioClipEditor* ed = new AudioClipEditor();
					ed->init(audio);

					MainWindow::getInspectorWindow()->setEditor(ed);
				}
			}

			if (std::find(videos.begin(), videos.end(), ext) != videos.end())
			{
				VideoClip* video = VideoClip::load(Engine::getSingleton()->getAssetsPath(), fileName);

				if (video != nullptr)
				{
					VideoClipEditor* ed = new VideoClipEditor();
					ed->init(video);

					MainWindow::getInspectorWindow()->setEditor(ed);
				}
			}
		}
	}

	void AssetsWindow::onDropNode(TreeNode* node, TreeNode* from)
	{
		if (from->treeView == treeView)
		{
			auto items = treeView->getSelectedNodes();
			std::vector<std::string> delItems;
			std::vector<std::string> moveItems;

			for (auto& item : items)
			{
				std::string path = Engine::getSingleton()->getAssetsPath() + item->getPath();
				if (IO::isDir(path))
				{
					for (auto& item2 : items)
					{
						std::string p = item2->getPath();
						std::string path2 = Engine::getSingleton()->getAssetsPath() + p;
						if (path != path2)
						{
							if (path2.find(path) != std::string::npos)
								delItems.push_back(p);
						}
					}
				}
			}

			for (auto& item : items)
			{
				std::string path = item->getPath();
				auto it = std::find(delItems.begin(), delItems.end(), path);
				if (it == delItems.end())
					moveItems.push_back(path);
			}

			delItems.clear();

			for (auto& item : moveItems)
			{
				std::string path = Engine::getSingleton()->getAssetsPath() + item;
				std::string name = IO::GetFileNameWithExt(item);
				std::string nodePath = node->getPath();

				if (!nodePath.empty())
					nodePath += "/";

				std::string newPath = Engine::getSingleton()->getAssetsPath() + nodePath + name;

				std::string srchOldPath = path;
				if (IO::GetFileExtension(srchOldPath).empty())
					srchOldPath += "/";

				std::string srchNewPath = newPath;
				if (IO::GetFileExtension(srchNewPath).empty())
					srchNewPath += "/";

				if (srchNewPath.find(srchOldPath) != std::string::npos)
					continue;

				if (newPath != path)
				{
					if (IO::isDir(path))
					{
						if (!IO::DirExists(newPath))
						{
							IO::DirCopy(path, newPath);
							IO::DirDeleteRecursive(path);
						}
					}
					else
					{
						if (!IO::FileExists(newPath))
						{
							IO::FileCopy(path, newPath);
							IO::FileDelete(path);
						}
					}
				}
			}

			treeView->selectNodes({}, false);

			reloadFiles();
		}
		else
		{
			if (from->treeView->getTag() == "Hierarchy")
			{
				std::string nodePath = "";
				if (node != nullptr)
					nodePath = CP_SYS(node->getPath());

				if (!nodePath.empty())
					nodePath += "/";

				std::string fpath = Engine::getSingleton()->getAssetsPath() + nodePath;

				if (IO::DirExists(fpath))
				{
					GameObject* prefabNode = Engine::getSingleton()->getGameObject(from->name);

					std::string prefabLocation = Engine::getSingleton()->getAssetsPath();
					std::string prefabName = nodePath + from->alias + ".prefab";

					if (IO::FileExists(prefabLocation + prefabName))
					{
						Prefab* loadedPrefab = Prefab::load(prefabLocation, prefabName);
						if (loadedPrefab != nullptr && loadedPrefab->isLoaded())
							loadedPrefab->unload();
					}

					Scene::savePrefab(prefabLocation, prefabName, prefabNode);

					addNewFiles();
				}
			}
		}
	}

	bool AssetsWindow::onNodeRenamed(TreeNode* item, std::string oldName)
	{
		std::string newName = Engine::getSingleton()->getAssetsPath() + item->getPath();
		std::string path = IO::GetFilePath(newName);
		std::string prevName = path + oldName;

		if (IO::FileExists(newName))
			return false;

		boost::system::error_code err = IO::FileRename(prevName, newName);
		
		if (err.value() != 0)
		{
			std::string errMsg = err.message();
	#ifdef _WIN32
			MessageBoxW(0, StringConvert::s2ws(errMsg).c_str(), L"Error", MB_OK);
	#else
			std::string cmd = "zenity --icon-name='dialog-warning' --no-wrap --info --title='Error' --text='" + errMsg + "'";
			FILE *ff = popen(cmd.c_str(), "r");
			pclose(ff);
	#endif
			return false;
		}

		return true;
	}

	void AssetsWindow::onNodePopupMenu(TreeNode* node, int val)
	{
		if (val == 0)
		{
			if (treeView->getSelectedNodes().size() <= 1)
			{
				node->setEditMode(true);
			}
		}

		if (val == 1)
		{
			MainWindow::getSingleton()->getInspectorWindow()->setEditor(nullptr);

			std::vector<TreeNode*>& nodes = treeView->getSelectedNodes();

			if (nodes.size() == 0)
				nodes = { node };
			else if (find(nodes.begin(), nodes.end(), node) == nodes.end())
				nodes = { node };

			bool recompileScripts = false;

			for (auto it = nodes.begin(); it != nodes.end(); ++it)
			{
				TreeNode* curNode = *it;
				TreeNode* par = curNode->parent;
				if (find(nodes.begin(), nodes.end(), par) != nodes.end())
					continue;

				std::string location = Engine::getSingleton()->getAssetsPath();

				std::string path = CP_SYS(curNode->getPath());
				std::string fileName = location + path;

				if (fileName != location)
				{
					if (IO::isDir(fileName))
					{
						IO::listFiles(fileName + "/", true, nullptr, [&recompileScripts](std::string str1, std::string str2) -> bool
							{
								std::string _fn = str1 + str2;
								if (IO::GetFileExtension(_fn) == "cs")
								{
									recompileScripts = true;

									return false;
								}

								return true;
							}
						);
					}
					else
					{
						if (IO::GetFileExtension(fileName) == "cs")
							recompileScripts = true;
					}
	#ifdef _WIN32
					//Delete file
					std::string str1 = fileName;
					fileName = boost::replace_all_copy(fileName, "/", "\\");
					std::wstring wstr = StringConvert::s2ws(fileName);
					wstr.append(L"00");
					wstr[wstr.size() - 2] = '\0';
					wstr[wstr.size() - 1] = '\0';
					const wchar_t * _strfn = wstr.c_str();

					SHFILEOPSTRUCTW f = { 0 };
					ZeroMemory(&f, sizeof(SHFILEOPSTRUCTW));
					f.hwnd = MainWindow::getSingleton()->getHwnd();
					f.wFunc = FO_DELETE;
					f.fFlags = FOF_ALLOWUNDO | FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
					f.pFrom = _strfn;
					f.pTo = NULL;
					f.fAnyOperationsAborted = false;
					f.hNameMappings = NULL;
					f.lpszProgressTitle = NULL;
					f.hNameMappings = NULL;

					int result = SHFileOperationW(&f);

					if (IO::FileExists(str1))
						MainWindow::getSingleton()->getConsoleWindow()->log("Failed to delete asset(s)", LogMessageType::LMT_ERROR);

					//Delete resource
					//deleteResource(path, true);
					//deleteNodes.push_back(curNode);
	#else
					std::string _path = Engine::getSingleton()->getAssetsPath() + curNode->getPath();
					std::string cmd = "gio trash \"" + _path + "\"";

					boost::process::child c(cmd);
					c.wait();
	#endif
				}
			}

			MainWindow::addOnEndUpdateCallback([=]()
				{
					setScriptsChanged(false);
					reloadFiles();

					if (recompileScripts)
						APIManager::getSingleton()->compile(APIManager::CompileConfiguration::Debug);
				}
			);
			
			nodes.clear();
		}

		if (val == 2)
		{
			auto nodes = treeView->getSelectedNodes();

			if (nodes.size() == 0)
				nodes = { node };
			else if (find(nodes.begin(), nodes.end(), node) == nodes.end())
				nodes = { node };

			for (auto it = nodes.begin(); it != nodes.end(); ++it)
			{
				TreeNode* node = *it;

	#ifdef _WIN32
				std::string _path = boost::replace_all_copy(Engine::getSingleton()->getAssetsPath() + node->getPath(), "/", "\\");

				ITEMIDLIST* pidl = ILCreateFromPathW(StringConvert::s2ws(_path).c_str());
				if (pidl)
				{
					SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
					ILFree(pidl);
				}
	#else
				std::string _path = Engine::getSingleton()->getAssetsPath() + node->getPath();
				std::string cmd = "nautilus \"" + _path + "\"";

				boost::process::child c(cmd);
				c.detach();
	#endif
			}
		}

		if (val == 3)
		{
			MainWindow::addOnEndUpdateCallback([=]()
				{
					reloadChangedFile(node->getPath());

					auto& models = Engine::getModel3dFileFormats();
					std::string ext = IO::GetFileExtension(node->getPath());
					if (std::find(models.begin(), models.end(), ext) != models.end())
					{
						BatchedGeometry::getSingleton()->rebuild();
					}
				}
			);
		}

		if (val == 4)
		{
			auto& modelExts = Engine::getModel3dFileFormats();
			auto& videoExts = Engine::getVideoFileFormats();
			std::string ext = IO::GetFileExtension(node->alias);
			if (std::find(modelExts.begin(), modelExts.end(), ext) != modelExts.end()) // Extract animations
			{
				Model3DLoader::extractAnimations(Engine::getSingleton()->getAssetsPath(), node->getPath());
				MainWindow::addOnEndUpdateCallback([=]()
					{
						reloadFiles();
					}
				);
			}
			else if (IO::GetFileExtension(node->alias) == "animation") // Crop animation
			{
				AnimationClip* animClip = AnimationClip::load(Engine::getSingleton()->getAssetsPath(), node->getPath());
				dialogAnimationCrop->show(animClip, [=]()
					{
						MainWindow::addOnEndUpdateCallback([=]()
							{
								reloadFiles();
							}
						);
					}
				);
			}
			else if (std::find(videoExts.begin(), videoExts.end(), ext) != videoExts.end()) // Extract audio
			{
				VideoClip* clip = VideoClip::load(Engine::getSingleton()->getAssetsPath(), node->getPath());
				if (clip != nullptr && clip->isLoaded())
					clip->extractAudio();

				MainWindow::addOnEndUpdateCallback([=]()
					{
						reloadFiles();
					}
				);
			}
		}
	}

	void AssetsWindow::onNodeDoubleClick(TreeNode* node)
	{
		std::string path = node->getPath();
		std::string ext = IO::GetFileExtension(path);

		MainWindow* mainWnd = MainWindow::getSingleton();

		if (ext == "scene")
		{
			if (mainWnd->checkSceneIsDirty())
				mainWnd->openScene(path);
		}

		if (ext == "cs")
		{
	#ifdef _WIN32
			std::string slnDir = Engine::getSingleton()->getRootPath() + Engine::getSingleton()->getAppName();
			std::string cmd = "\"" + slnDir + ".sln\" \"" + slnDir + ".csproj\" \"" + Engine::getSingleton()->getAssetsPath() + CP_SYS(path) + "\"";
			::ShellExecuteW(NULL, L"open", L"devenv", StringConvert::s2ws(cmd).c_str(), NULL, 1);
	#else
			std::string cmd = "gedit \"" + Engine::getSingleton()->getAssetsPath() + path + "\"";

			boost::process::child c(cmd);
			c.detach();
	#endif
		}

		if (ext == "animation")
		{
			AnimationClip* anim = AnimationClip::load(Engine::getSingleton()->getAssetsPath(), node->getPath());

			if (anim != nullptr && anim->isLoaded())
			{
				AnimationEditorWindow* animEditor = MainWindow::getAnimationEditorWindow();
				animEditor->openAnimation(anim);
			}
		}
	}

	void AssetsWindow::updateAssetsPopup(bool itemsOnly, TreeNode* selNode)
	{
		bool contextMenu = false;

		if (!itemsOnly)
			contextMenu = ImGui::BeginPopupContextItem("add_asset_popup", 0);
		else
		{
			MainWindow::MenuItemIcon(addIcon);
			contextMenu = ImGui::BeginMenu("Create");
		}

		if (contextMenu)
		{
			MainWindow::MenuItemIcon(folderIcon);
			if (ImGui::Selectable("Folder", false))
				createFolder(selNode);

			ImGui::Separator();

			MainWindow::MenuItemIcon(csIcon);
			if (ImGui::Selectable("C# Script", false))
			{
				if (treeView->getSelectedNodes().size() <= 1)
				{
					TreeNode* root = treeView->getRootNode();

					if (selNode != nullptr)
						root = selNode;
					else
					{
						auto nodes = treeView->getSelectedNodes();

						if (nodes.size() == 1)
							root = nodes[0];
					}

					root->expanded = true;

					if (!IO::isDir(Engine::getSingleton()->getAssetsPath() + CP_SYS(root->getPath())))
					{
						root = root->parent;
					}

					TreeNode * addedAssetNode = new TreeNode(treeView);
					addedAssetNode->name = "##" + GUIDGenerator::genGuid();
					addedAssetNode->icon = getIconByExtension("cs");
					addedAssetNode->setEditMode(true);
					
					addedAssetNode->setOnEndEditCallback([=](TreeNode* node, std::string prevAlias) -> bool
						{
							std::string name = CP_SYS(node->alias);

							std::string _path = CP_SYS(root->getPath());
							if (!_path.empty()) _path = _path + "/";

							if (!name.empty() && !IO::FileExists(Engine::getSingleton()->getAssetsPath() + _path + name + ".cs"))
							{
								std::string _name = boost::replace_all_copy(name, " ", "");

								std::string content =
									"using FalcoEngine;\n"
									"using System;\n"
									"using System.Collections;\n"
									"using System.Collections.Generic;\n"
									"\n"
									"public class " + _name + " : MonoBehaviour" + "\n"
									"{\n"
									"	void Start()\n"
									"	{\n"
									"		\n"
									"	}\n"
									"	\n"
									"	void Update()\n"
									"	{\n"
									"		\n"
									"	}\n"
									"}\n";

								IO::WriteText(Engine::getSingleton()->getAssetsPath() + _path + name + ".cs", CP_UNI(content));

								MainWindow::addOnEndUpdateCallback([=]()
									{
										treeView->deleteNode(addedAssetNode);
										setScriptsChanged(false);
										reloadFiles();
										APIManager::getSingleton()->compile(APIManager::CompileConfiguration::Debug);
									}
								);
							}
							else
							{
								MainWindow::addOnEndUpdateCallback([=]()
									{
										treeView->deleteNode(addedAssetNode);
									}
								);
							}

							return true;
						}
					);

					root->addChild(addedAssetNode);

					treeView->focusOnNode(addedAssetNode);
				}
			}

			ImGui::Separator();

			MainWindow::MenuItemIcon(materialIcon);
			if (ImGui::Selectable("Material"))
			{
				if (treeView->getSelectedNodes().size() <= 1)
				{
					TreeNode* root = treeView->getRootNode();

					if (selNode != nullptr)
						root = selNode;
					else
					{
						auto nodes = treeView->getSelectedNodes();

						if (nodes.size() == 1)
							root = nodes[0];
					}

					root->expanded = true;

					if (!IO::isDir(Engine::getSingleton()->getAssetsPath() + CP_SYS(root->getPath())))
					{
						root = root->parent;
					}

					TreeNode* addedAssetNode = new TreeNode(treeView);
					addedAssetNode->name = "##" + GUIDGenerator::genGuid();
					addedAssetNode->icon = getIconByExtension("material");
					addedAssetNode->setEditMode(true);

					addedAssetNode->setOnEndEditCallback([=](TreeNode* node, std::string prevAlias) -> bool
						{
							std::string name = CP_SYS(node->alias) + ".material";

							std::string _path = CP_SYS(root->getPath());
							if (!_path.empty()) _path = _path + "/";

							if (!node->alias.empty() && !IO::FileExists(Engine::getSingleton()->getAssetsPath() + _path + name))
							{
								Material* newMaterial = Material::create(Engine::getSingleton()->getAssetsPath(), _path + name);
								newMaterial->load();
								newMaterial->save();

								MainWindow::addOnEndUpdateCallback([=]()
									{
										treeView->deleteNode(addedAssetNode);
										reloadFiles();
									}
								);
							}
							else
							{
								MainWindow::addOnEndUpdateCallback([=]()
									{
										treeView->deleteNode(addedAssetNode);
									}
								);
							}

							return true;
						}
					);

					root->addChild(addedAssetNode);

					treeView->focusOnNode(addedAssetNode);
				}
			}

			MainWindow::MenuItemIcon(cubemapIcon);
			if (ImGui::Selectable("Cubemap", false))
			{
				if (treeView->getSelectedNodes().size() <= 1)
				{
					TreeNode* root = treeView->getRootNode();

					if (selNode != nullptr)
						root = selNode;
					else
					{
						auto nodes = treeView->getSelectedNodes();

						if (nodes.size() == 1)
							root = nodes[0];
					}

					root->expanded = true;

					if (!IO::isDir(Engine::getSingleton()->getAssetsPath() + CP_SYS(root->getPath())))
					{
						root = root->parent;
					}

					TreeNode* addedAssetNode = new TreeNode(treeView);
					addedAssetNode->name = "##" + GUIDGenerator::genGuid();
					addedAssetNode->icon = getIconByExtension("cubemap");
					addedAssetNode->setEditMode(true);

					addedAssetNode->setOnEndEditCallback([=](TreeNode* node, std::string prevAlias) -> bool
						{
							std::string name = CP_SYS(node->alias) + ".cubemap";

							std::string _path = CP_SYS(root->getPath());
							if (!_path.empty()) _path = _path + "/";

							if (!node->alias.empty() && !IO::FileExists(Engine::getSingleton()->getAssetsPath() + _path + name))
							{
								Cubemap* newCubemap = Cubemap::create(Engine::getSingleton()->getAssetsPath(), _path + name);
								newCubemap->save();

								MainWindow::addOnEndUpdateCallback([=]()
									{
										treeView->deleteNode(addedAssetNode);
										reloadFiles();
									}
								);
							}
							else
							{
								MainWindow::addOnEndUpdateCallback([=]()
									{
										treeView->deleteNode(addedAssetNode);
									}
								);
							}

							return true;
						}
					);

					root->addChild(addedAssetNode);

					treeView->focusOnNode(addedAssetNode);
				}
			}

			MainWindow::MenuItemIcon(shaderIcon);
			if (ImGui::Selectable("Shader", false))
			{
				if (treeView->getSelectedNodes().size() <= 1)
				{
					TreeNode* root = treeView->getRootNode();

					if (selNode != nullptr)
						root = selNode;
					else
					{
						auto nodes = treeView->getSelectedNodes();

						if (nodes.size() == 1)
							root = nodes[0];
					}

					root->expanded = true;

					if (!IO::isDir(Engine::getSingleton()->getAssetsPath() + CP_SYS(root->getPath())))
					{
						root = root->parent;
					}

					TreeNode* addedAssetNode = new TreeNode(treeView);
					addedAssetNode->name = "##" + GUIDGenerator::genGuid();
					addedAssetNode->icon = getIconByExtension("shader");
					addedAssetNode->setEditMode(true);

					addedAssetNode->setOnEndEditCallback([=](TreeNode* node, std::string prevAlias) -> bool
						{
							std::string name = CP_SYS(node->alias);

							std::string _path = CP_SYS(root->getPath());
							if (!_path.empty()) _path = _path + "/";

							if (!node->alias.empty() && !IO::FileExists(Engine::getSingleton()->getAssetsPath() + _path + name + ".shader"))
							{
								std::string _name = boost::replace_all_copy(name, " ", "");

								std::string content =
									"name \"Forward/" + _name + "\"\n"
									"render_mode forward\n"
									"\n"
									"params\n"
									"{\n"
									"	\n"
									"}\n"
									"\n"
									"pass\n"
									"{\n"
									"	tags\n"
									"	{\n"
									"		\n"
									"	}\n"
									"	\n"
									"	varying\n"
									"	{\n"
									"		vec3 a_position  : POSITION;\n"
									"		vec2 a_texcoord0 : TEXCOORD0;\n"
									"		\n"
									"		vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);\n"
									"		vec4 v_color0 : COLOR0 = vec4(1.0, 1.0, 1.0, 1.0);\n"
									"	}\n"
									"	\n"
									"	vertex\n"
									"	{\n"
									"		$input a_position, a_texcoord0\n"
									"		$output v_texcoord0\n"
									"		\n"
									"		#include \"common.sh\"\n"
									"		\n"
									"		void main()\n"
									"		{\n"
									"			gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));\n"
									"			v_texcoord0 = a_texcoord0;\n"
									"		}\n"
									"	}\n"
									"	\n"
									"	fragment\n"
									"	{\n"
									"		$input v_texcoord0\n"
									"		\n"
									"		#include \"common.sh\"\n"
									"		\n"
									"		void main()\n"
									"		{\n"
									"			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
									"		}\n"
									"	}\n"
									"}\n";

								IO::WriteText(Engine::getSingleton()->getAssetsPath() + _path + name + ".shader", CP_UNI(content));

								Shader* newShader = Shader::create(Engine::getSingleton()->getAssetsPath() + _path, _path + name + ".shader");
								newShader->loadFromSource(content);

								for (auto& asset : Asset::getLoadedInstances())
								{
									if (asset.second->getAssetType() == Material::ASSET_TYPE)
									{
										Material* mat = (Material*)asset.second;
										Shader* ms = mat->getShader();
										if (ms != nullptr && ms->getName() == _path + name + ".shader")
										{
											mat->setShader(nullptr);
											mat->setShader(newShader);
										}
									}
								}

								MainWindow::addOnEndUpdateCallback([=]()
									{
										treeView->deleteNode(addedAssetNode);
										reloadFiles();
									}
								);
							}
							else
							{
								MainWindow::addOnEndUpdateCallback([=]()
									{
										treeView->deleteNode(addedAssetNode);
									}
								);
							}

							return true;
						}
					);

					root->addChild(addedAssetNode);

					treeView->focusOnNode(addedAssetNode);
				}
			}

			ImGui::Separator();

			MainWindow::MenuItemIcon(animationClipIcon);
			if (ImGui::Selectable("Animation", false))
			{
				if (treeView->getSelectedNodes().size() <= 1)
				{
					TreeNode* root = treeView->getRootNode();

					if (selNode != nullptr)
						root = selNode;
					else
					{
						auto nodes = treeView->getSelectedNodes();

						if (nodes.size() == 1)
							root = nodes[0];
					}

					root->expanded = true;

					if (!IO::isDir(Engine::getSingleton()->getAssetsPath() + CP_SYS(root->getPath())))
					{
						root = root->parent;
					}

					TreeNode* addedAssetNode = new TreeNode(treeView);
					addedAssetNode->name = "##" + GUIDGenerator::genGuid();
					addedAssetNode->icon = getIconByExtension("animation");
					addedAssetNode->setEditMode(true);

					addedAssetNode->setOnEndEditCallback([=](TreeNode* node, std::string prevAlias) -> bool
						{
							std::string name = CP_SYS(node->alias);

							std::string _path = CP_SYS(root->getPath());
							if (!_path.empty()) _path = _path + "/";

							if (!node->alias.empty() && !IO::FileExists(Engine::getSingleton()->getAssetsPath() + _path + name))
							{
								AnimationClip* newAnim = AnimationClip::create(Engine::getSingleton()->getAssetsPath(), _path + name);
								newAnim->save();

								MainWindow::addOnEndUpdateCallback([=]()
									{
										treeView->deleteNode(addedAssetNode);
										reloadFiles();
									}
								);
							}
							else
							{
								MainWindow::addOnEndUpdateCallback([=]()
									{
										treeView->deleteNode(addedAssetNode);
									}
								);
							}

							return true;
						}
					);

					root->addChild(addedAssetNode);

					treeView->focusOnNode(addedAssetNode);
				}
			}

			if (!itemsOnly)
				ImGui::EndPopup();
			else
				ImGui::EndMenu();
		}

		if (itemsOnly)
			ImGui::Separator();
	}

	void AssetsWindow::updateAssetsMenu()
	{
		ImGui::ImageButton((void*)addIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0));

		updateAssetsPopup();

		ImGui::SameLine();

		if (ImGui::ImageButton((void*)addFolderIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0)))
		{
			createFolder();
		}

		ImGui::SameLine();

		if (treeView->getFilter().empty())
		{
			ImGui::SetNextItemWidth(-1);
		}
		else
		{
			float w = ImGui::GetContentRegionAvail().x;
			ImGui::SetNextItemWidth(w - 25);
		}

		if (ImGui::InputText("##AssetsFilter", &treeView->getFilter()))
		{
			treeView->setFilter(treeView->getFilter());
		}

		ImVec2 pos = ImGui::GetItemRectMin();
		pos = ImVec2(pos.x - ImGui::GetWindowPos().x, pos.y - ImGui::GetWindowPos().y);

		if (!treeView->getFilter().empty())
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5);
			if (ImGui::ImageButton((void*)closeIcon->getHandle().idx, ImVec2(15, 15), ImVec2(0, 1), ImVec2(1, 0)))
			{
				treeView->setFilter("");
			}
		}
		else
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(pos.x + 5);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY());
			ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 0.5), "Search...");
		}

		ImGui::Separator();
	}

	#ifdef _WIN32
	void FileSystemWatcher::WatchDirectory(const wchar_t* path)
	{

		char buf[2048];
		DWORD nRet;
		BOOL result = TRUE;
		char filename[MAX_PATH];
		DirInfo[0].hDir = CreateFileW(path, GENERIC_READ | FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);

		if (DirInfo[0].hDir == INVALID_HANDLE_VALUE)
		{
			return; //cannot open folder
		}

		lstrcpyW(DirInfo[0].lpszDirName, path);
		OVERLAPPED PollingOverlap;

		FILE_NOTIFY_INFORMATION* pNotify;
		int offset;
		PollingOverlap.OffsetHigh = 0;
		PollingOverlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		while (result)
		{
			result = ReadDirectoryChangesW(
				DirInfo[0].hDir,// handle to the directory to be watched
				&buf,// pointer to the buffer to receive the read results
				sizeof(buf),// length of lpBuffer
				TRUE,// flag for monitoring directory or directory tree
				FILE_NOTIFY_CHANGE_FILE_NAME |
				//FILE_NOTIFY_CHANGE_DIR_NAME |
				//FILE_NOTIFY_CHANGE_SIZE |
				FILE_NOTIFY_CHANGE_LAST_WRITE,
				//FILE_NOTIFY_CHANGE_LAST_ACCESS |
				//FILE_NOTIFY_CHANGE_CREATION,
				&nRet,// number of bytes returned
				&PollingOverlap,// pointer to structure needed for overlapped I/O
				NULL);

			WaitForSingleObject(PollingOverlap.hEvent, INFINITE);
			offset = 0;
			int rename = 0;
			char oldName[260];
			char newName[260];
			do
			{
				pNotify = (FILE_NOTIFY_INFORMATION*)((char*)buf + offset);
				strcpy(filename, "");
				int filenamelen = WideCharToMultiByte(CP_ACP, 0, pNotify->FileName, pNotify->FileNameLength / 2, filename, sizeof(filename), NULL, NULL);
				filename[pNotify->FileNameLength / 2];
				std::string fn = std::string(filename, pNotify->FileNameLength / 2);
				fn = IO::ReplaceBackSlashes(fn);

				std::string ext = IO::GetFileExtension(fn);

				switch (pNotify->Action)
				{
				case FILE_ACTION_ADDED:
					if (ext == "cs" || ext.find("~") != std::string::npos)
					{
						AssetsWindow::setScriptsChanged(true);
					}

					break;

				case FILE_ACTION_REMOVED:
					if (ext == "cs" || ext.find("~") != std::string::npos)
					{
						AssetsWindow::setScriptsChanged(true);
					}

					break;

				case FILE_ACTION_MODIFIED:
					if (ext == "cs" || ext.find("~") != std::string::npos)
					{
						AssetsWindow::setScriptsChanged(true);
					}
					else
					{
						AssetsWindow::addChangedFile(fn);
					}

					break;
				case FILE_ACTION_RENAMED_OLD_NAME:
					break;
				case FILE_ACTION_RENAMED_NEW_NAME:
					if (ext != "cs" && ext.find("~") == std::string::npos)
					{
						AssetsWindow::addChangedFile(fn);
					}
					break;
				default:
					break;
				}

				offset += pNotify->NextEntryOffset;

			} while (pNotify->NextEntryOffset);
		}

		CloseHandle(DirInfo[0].hDir);
	}
	#else
	void FileSystemWatcher::WatchDirectory(const char* path)
	{
		char buffer[BUF_LEN];

		ssize_t length = read(fd, buffer, BUF_LEN);

		if (length < 0)
		{
			Debug::log("FileSystemWatcher error: read", Debug::DbgColorRed);
			return;
		}

		int i = 0;

		while (i < length)
		{
			struct inotify_event* event = (struct inotify_event*)&buffer[i];
			if (event->len)
			{
				char* _n = new char[event->len];
				memcpy(_n, &event->name, event->len);
				std::string name = _n;
				delete[] _n;

				std::string ext = IO::GetFileExtension(name);
				std::string currentDir = "";
				auto kt = std::find_if(watchDirs.begin(), watchDirs.end(), [=] (std::pair<std::string, int>& a) -> bool
				{
					return a.second == event->wd;
				});

				if (kt != watchDirs.end())
					currentDir = kt->first;

				if (event->mask & IN_CREATE)
				{
					if (!(event->mask & IN_ISDIR))
					{
						if (ext == "cs")
						{
							AssetsWindow::setScriptsChanged(true);
						}
					}
					if (event->mask & IN_ISDIR)
					{
						std::string new_dir = currentDir + name + "/";

						auto it = std::find_if(watchDirs.begin(), watchDirs.end(), [=] (std::pair<std::string, int>& a) -> bool
						{
							return a.first == new_dir;
						});
						
						if (it == watchDirs.end())
						{
							int new_wd = inotify_add_watch( fd, new_dir.c_str(), NOTIFY_FLAGS);
							watchDirs.push_back(std::make_pair(new_dir, new_wd));
						}
					}
				}
				else if (event->mask & IN_DELETE)
				{
					if (!(event->mask & IN_ISDIR))
					{
						if (ext == "cs")
						{
							AssetsWindow::setScriptsChanged(true);
						}
					}
					if (event->mask & IN_ISDIR)
					{
						std::string dir = currentDir + name + "/";

						auto it = std::find_if(watchDirs.begin(), watchDirs.end(), [=] (std::pair<std::string, int>& a) -> bool
						{
							return a.first == dir;
						});

						if (it != watchDirs.end())
						{
							inotify_rm_watch(fd, it->second);
							watchDirs.erase(it);
						}
					}
				}
				else if (event->mask & IN_MODIFY)
				{
					if (!(event->mask & IN_ISDIR))
					{
						if (ext == "cs")
						{
							AssetsWindow::setScriptsChanged(true);
						}
						else
						{
							std::string fn = currentDir + name;
							AssetsWindow::addChangedFile(fn);
						}
					}
				}
				else if ((event->mask & IN_MOVED_TO) || (event->mask & IN_MOVED_FROM))
				{
					if (!(event->mask & IN_ISDIR))
					{
						if (ext == "cs")
						{
							AssetsWindow::setScriptsChanged(true);
						}
						else
						{
							std::string fn = currentDir + name;
							AssetsWindow::addChangedFile(fn);
						}
					}
					if (event->mask & IN_ISDIR)
					{
						std::string dir = currentDir + name + "/";

						if (IO::DirExists(dir))
						{
							IO::listFiles(dir, true, nullptr, [=](std::string d, std::string f) -> bool
							{
								std::string fn = d + f;

								if (IO::GetFileExtension(fn) == "cs")
								{
									AssetsWindow::setScriptsChanged(true);
								}
								else
								{
									AssetsWindow::addChangedFile(fn);
								}

								return true;
							});
						}
						else
						{
							AssetsWindow::setScriptsChanged(true);
						}
					}
				}
				else if (event->mask & IN_CLOSE_WRITE)
				{
					if (!(event->mask & IN_ISDIR))
					{
						if (ext == "cs")
						{
							AssetsWindow::setScriptsChanged(true);
						}
						else
						{
							std::string fn = currentDir + name;
							AssetsWindow::addChangedFile(fn);
						}
					}
				}
			}

			i += EVENT_SIZE + event->len;
		}
	}
	#endif

	bool FileSystemWatcher::InitInstance()
	{
	#ifndef _WIN32
		std::string assetsPath = Engine::getSingleton()->getAssetsPath();

		fd = inotify_init();

		if (fd < 0)
		{
			Debug::log("FileSystemWatcher error: inotify_init", Debug::DbgColorRed);
			return false;
		}

		wd = inotify_add_watch(fd, assetsPath.c_str(), NOTIFY_FLAGS);
		watchDirs.push_back(std::make_pair(assetsPath, wd));

		IO::listFiles(assetsPath, true, [=](std::string dir) -> bool
		{
			if (!dir.empty() && IO::DirExists(dir))
			{
				int new_wd = inotify_add_watch(fd, dir.c_str(), NOTIFY_FLAGS);
				watchDirs.push_back(std::make_pair(dir + "/", new_wd));
			}

			return true;
		}, nullptr);
	#endif

		std::thread thr([=]()
			{
				while (work)
				{
	#ifdef _WIN32
					std::wstring assetsPath = StringConvert::s2ws(Engine::getSingleton()->getAssetsPath());
					WatchDirectory(assetsPath.c_str());
	#else
					WatchDirectory(assetsPath.c_str());
	#endif
				}

#ifndef _WIN32
				(void)inotify_rm_watch(fd, wd);
				(void)close(fd);

				watchDirs.clear();
#endif
			}
		);

		thr.detach();

		return true;
	}
}