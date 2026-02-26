#pragma once

#include <string>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#endif

#define MAX_DIRS 1
#define MAX_BUFFER 4096

namespace GX
{
	class TreeView;
	class TreeNode;
	class Texture;
	class Shader;
	class DialogAnimationCrop;

	#ifdef _WIN32
	class FileSystemWatcher
	{
	private:
		typedef struct _DIRECTORY_INFO {
			HANDLE hDir;
			wchar_t lpszDirName[260];
			wchar_t lpBuffer[MAX_BUFFER];
			DWORD dwBufLength;
			OVERLAPPED Overlapped;
		} DIRECTORY_INFO, * PDIRECTORY_INFO, * LPDIRECTORY_INFO;

	private:
		void WatchDirectory(const wchar_t* path);

		DIRECTORY_INFO DirInfo[MAX_DIRS];
		DWORD numDirs;
		
		bool work = true;

	public:
		virtual bool InitInstance();
		virtual int ExitInstance()
		{
			work = false;
			return 0;
		}
	};
	#else
	class FileSystemWatcher
	{
	private:
	#ifdef _WIN32
		void WatchDirectory(const wchar_t* path);
	#else
		int fd = 0;
		int wd = 0;

		std::vector<std::pair<std::string, int>> watchDirs;

		void WatchDirectory(const char* path);
	#endif
		bool work = true;
		
	public:
		virtual bool InitInstance();
		virtual int ExitInstance()
		{
			work = false;
			return 0;
		}
	};
	#endif

	class AssetsWindow
	{
	private:
		static AssetsWindow* singleton;

		DialogAnimationCrop* dialogAnimationCrop = nullptr;
		
		FileSystemWatcher* fswatcher = nullptr;

		std::vector<Shader*> shaderList;

		bool opened = true;
		bool isWindowDragging = false;

		TreeView* treeView = nullptr;

		Texture * addIcon = nullptr;
		Texture * addFolderIcon = nullptr;
		Texture * closeIcon = nullptr;

		Texture* folderIcon = nullptr;
		Texture* meshIcon = nullptr;
		Texture* textureIcon = nullptr;
		Texture* materialIcon = nullptr;
		Texture* sceneIcon = nullptr;
		Texture* shaderIcon = nullptr;
		Texture* cubemapIcon = nullptr;
		Texture* csIcon = nullptr;
		Texture* audioClipIcon = nullptr;
		Texture* fontIcon = nullptr;
		Texture* prefabIcon = nullptr;
		Texture* textIcon = nullptr;
		Texture* animationClipIcon = nullptr;
		Texture* terrainIcon = nullptr;
		Texture* packageIcon = nullptr;
		Texture* navmeshIcon = nullptr;
		Texture* videoClipIcon = nullptr;

		Texture* renameIcon = nullptr;
		Texture* deleteIcon = nullptr;
		Texture* showInExplorerIcon = nullptr;
		Texture* reloadIcon = nullptr;
		Texture* extractAnimsIcon = nullptr;
		Texture* extractAudioIcon = nullptr;
		Texture* cropAnimIcon = nullptr;

		std::vector<std::string> expandedItems;
		void sortTree();
		void sortTree(TreeNode* root);

		static void onNodesSelected(std::vector<std::string> names, void* userData);
		void onDropNode(TreeNode* node, TreeNode* from);
		bool onNodeRenamed(TreeNode* item, std::string oldName);
		void onNodePopupMenu(TreeNode* node, int val);
		void onNodeDoubleClick(TreeNode* node);

		void updateAssetsMenu();
		void updateAssetsPopup(bool itemsOnly = false, TreeNode* selNode = nullptr);
		std::vector<std::string> addNewFiles(std::vector<std::string> files = std::vector<std::string>());
		void addScripts(std::vector<std::string> files = std::vector<std::string>());
		void rememberExpandedItems(TreeNode* root);
		void rememberAllItems(TreeNode* root, std::vector<std::string>& list);
		void createFolder(TreeNode* selNode = nullptr);

		Texture* updateTextureIcon(std::string path, bool recreate = false);

		static bool scriptsChanged;
		static std::vector<std::string> changedFiles;

		bool focused = false;

	public:
		AssetsWindow();
		~AssetsWindow();

		void init();
		void update();
		void show(bool show) { opened = show; }
		bool getVisible() { return opened; }
		void focusOnFile(std::string filepath);
		void reloadFiles();
		void reloadShaderList();
		void reloadChangedFiles();
		void reloadChangedFile(std::string path);
		static void setScriptsChanged(bool value) { scriptsChanged = value; }
		static void addChangedFile(std::string value);

		static AssetsWindow* getSingleton() { return singleton; }

		TreeView* getTreeView() { return treeView; }
		Texture* getIconByExtension(std::string ext);

		std::vector<Shader*>& getShaderList() { return shaderList; }

		bool isFocused() { return focused; }
	};
}