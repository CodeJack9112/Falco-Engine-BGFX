#include "DialogAssetExporter.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>
#include <boost/algorithm/string.hpp>

#include "../LibZip/include/zip.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Classes/StringConverter.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Classes/GUIDGenerator.h"

#include "../Windows/MainWindow.h"
#include "../Windows/ConsoleWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"
#include "DialogProgress.h"

#include <fstream>
#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#include <combaseapi.h>
#include <shobjidl_core.h>
#endif

namespace GX
{
	DialogProgress* DialogAssetExporter::progressDialog = nullptr;

	DialogAssetExporter::DialogAssetExporter()
	{
		treeView = new TreeView();

		progressDialog = new DialogProgress();
		progressDialog->setTitle("Exporting package");
	}

	DialogAssetExporter::~DialogAssetExporter()
	{
		delete progressDialog;
	}

	void DialogAssetExporter::show(Mode m)
	{
		allSelected = true;
		setMode(m);

		treeView->clear();

		if (mode == Mode::Import)
		{
	#ifdef _WIN32
			IFileDialog* pfd;
			if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
			{
				DWORD dwOptions;
				if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
				{
					pfd->SetOptions(dwOptions);
				}

				COMDLG_FILTERSPEC ComDlgFS[1] = { {L"Package files", L"*.package"} };
				pfd->SetFileTypes(1, ComDlgFS);

				if (SUCCEEDED(pfd->Show(NULL)))
				{
					IShellItem* psi;
					if (SUCCEEDED(pfd->GetResult(&psi)))
					{
						LPWSTR g_path = NULL;
						LPWSTR g_name = NULL;

						if (!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &g_path)))
						{
							MessageBox(NULL, "SIGDN_DESKTOPABSOLUTEPARSING failed", NULL, NULL);
						}
						else
						{
							if (!SUCCEEDED(psi->GetDisplayName(SIGDN_NORMALDISPLAY, &g_name)))
							{
								MessageBox(NULL, "SIGDN_NORMALDISPLAY failed", NULL, NULL);
							}
							else
							{
								std::string fileName = StringConvert::ws2s(g_name);
								std::string filePath = StringConvert::ws2s(g_path);
	#else
			char filename[1024];
			FILE *f = popen("zenity --file-selection --file-filter='Package files | *.package'", "r");
			fgets(filename, 1024, f);
			int r = pclose(f);

			if (r == 0)
			{
				std::string filePath = filename;
				int idx = filePath.find("\n");
				if (idx != std::string::npos)
					filePath = filePath.substr(0, idx);
	#endif

								filePath = IO::ReplaceBackSlashes(filePath);
								
								if (IO::FileExists(filePath))
								{
									visible = true;

									struct zip* za;
									int err = 0;
									char buf[100];
									struct zip_stat sb;

									if ((za = zip_open_z(filePath.c_str(), 0, &err)) == NULL) {
										zip_error_to_str(buf, sizeof(buf), err, errno);
										MainWindow::getConsoleWindow()->log("Failed to open package: " + std::string(buf), LogMessageType::LMT_ERROR);
										return;
									}

									openedPackage = filePath;

									for (int i = 0; i < zip_get_num_entries(za, 0); ++i)
									{
										zip_stat_index(za, i, 0, &sb);
										std::string path = CP_SYS(sb.name);

										if (treeView->getNodeByPath(path, treeView->getRootNode()) == nullptr)
										{
											std::vector<std::string> results;
											boost::split(results, path, [](char c) { return c == '/' || c == '\\'; });

											TreeNode* item = treeView->getRootNode();

											for (std::vector<std::string>::iterator it2 = results.begin(); it2 != results.end(); ++it2)
											{
												TreeNode* newItem = treeView->getNodeByPath(*it2, item);
												if (newItem == nullptr)
												{
													Texture* icon = MainWindow::getAssetsWindow()->getIconByExtension(IO::GetFileExtension(*it2));

													TreeNode* node = new TreeNode(treeView);
													node->alias = CP_UNI(*it2);
													node->name = GUIDGenerator::genGuid();
													node->icon = icon;
													node->enableDrag = false;
													node->setChecked(true);
													node->setCheckboxEnabled(true);
													node->setEditable(false);
													node->setCheckChildren(true);
													item->addChild(node);

													item = node;

													std::string ext = IO::GetFileExtension(*it2);
													std::string _str = CP_SYS(item->getPath());

													std::string assetsPath = Engine::getSingleton()->getAssetsPath();
													if (IO::FileExists(assetsPath + _str) || IO::DirExists(assetsPath + _str))
													{
														item->setChecked(true);
														item->setCheckboxEnabled(false);
													}
												}
												else
												{
													item = newItem;
												}
											}

											results.clear();
										}
									}

									zip_close_z(za);
								}
								else
									visible = false;
	#ifdef _WIN32
							}
						}

						CoTaskMemFree(g_path);
						CoTaskMemFree(g_name);

						psi->Release();
					}
				}
				pfd->Release();
			}
	#else
			}
	#endif
		}
		else
		{
			visible = true;
			copyTreeView(MainWindow::getAssetsWindow()->getTreeView()->getRootNode(), treeView->getRootNode());
		}
	}

	void DialogAssetExporter::update()
	{
		std::string caption = "";
		std::string btnImportExport = "";

		if (mode == Mode::Export)
		{
			caption = "Export Assets";
			btnImportExport = "Export";
		}
		else
		{
			caption = "Import Assets";
			btnImportExport = "Import";
		}

		bool closed = false;
		bool ok = false;

		if (visible)
		{
			ImGui::SetNextWindowSize(ImVec2(300.0f, 500.0f), ImGuiCond_Appearing);
			ImGui::SetNextWindowPos(ImVec2(MainWindow::getSingleton()->getWidth() / 2, MainWindow::getSingleton()->getHeight() / 2), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
			if (ImGui::Begin(caption.c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
			{
				if (ImGui::Checkbox("", &allSelected))
				{
					TreeNode* root = treeView->getRootNode();

					for (auto it = root->children.begin(); it != root->children.end(); ++it)
						checkNodes(*it, allSelected);
				}

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

				ImGuiWindow* window = GImGui->CurrentWindow;
				ImGuiID id = window->GetIDNoKeepAlive("##AssetsExporterVS");
				ImGui::BeginChild(id, ImVec2(0, ImGui::GetWindowSize().y - 95));
				treeView->update();
				ImGui::EndChild();

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

				if (ImGui::Button("Cancel"))
				{
					closed = true;
					ok = false;
				}

				ImGui::SameLine();

				if (ImGui::Button(btnImportExport.c_str()))
				{
					closed = true;
					ok = true;
				}
			}

			ImGui::End();
		}

		if (closed && ok)
		{
			if (mode == Mode::Export)
			{
	#ifdef _WIN32
				IFileDialog* pfd;
				if (SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
				{
					DWORD dwOptions;
					if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
					{
						pfd->SetOptions(dwOptions);
					}

					COMDLG_FILTERSPEC ComDlgFS[1] = { {L"Package files", L"*.package"} };
					pfd->SetFileTypes(1, ComDlgFS);

					if (SUCCEEDED(pfd->Show(NULL)))
					{
						IShellItem* psi;
						if (SUCCEEDED(pfd->GetResult(&psi)))
						{
							LPWSTR g_path = NULL;
							LPWSTR g_name = NULL;

							if (!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &g_path)))
							{
								MessageBox(NULL, "SIGDN_DESKTOPABSOLUTEPARSING failed", NULL, NULL);
							}
							else
							{
								if (!SUCCEEDED(psi->GetDisplayName(SIGDN_NORMALDISPLAY, &g_name)))
								{
									MessageBox(NULL, "SIGDN_NORMALDISPLAY failed", NULL, NULL);
								}
								else
								{
									std::string fileName = StringConvert::ws2s(g_name);
									std::string filePath = StringConvert::ws2s(g_path);

									filePath = IO::ReplaceBackSlashes(filePath);

									if (filePath.find(".package") == std::string::npos)
										filePath += ".package";

									MainWindow::addOnEndUpdateCallback([=]() {
											exportAssets(filePath);
										}
									);
								}
							}

							CoTaskMemFree(g_path);
							CoTaskMemFree(g_name);

							psi->Release();
						}
					}
					pfd->Release();
				}
	#else
				char filename[1024];
				FILE *f = popen("zenity --file-selection --save --file-filter='Package files | *.package'", "r");
				fgets(filename, 1024, f);
				int r = pclose(f);

				if (r == 0)
				{
					std::string filePath = filename;
					int idx = filePath.find("\n");
					if (idx != std::string::npos)
						filePath = filePath.substr(0, idx);

					filePath = IO::ReplaceBackSlashes(filePath);

					if (filePath.find(".package") == std::string::npos)
						filePath += ".package";

					MainWindow::addOnEndUpdateCallback([=]() {
							exportAssets(filePath);
						}
					);
				}
	#endif
			}
			else
			{
				MainWindow::addOnEndUpdateCallback([=]() {
					importAssets();
				});
			}
		}

		if (closed && !ok)
		{
			visible = false;
		}
	}

	void DialogAssetExporter::listFiles(std::vector<std::string>& outList, TreeNode* htStart)
	{
		for (auto it = htStart->children.begin(); it != htStart->children.end(); ++it)
		{
			std::string path = (*it)->getPath();

			if (!IO::GetFileExtension(path).empty() && (*it)->children.size() == 0)
			{
				if ((*it)->getChecked())
					outList.push_back(path);
			}
			else
				listFiles(outList, *it);
		}
	}

	void DialogAssetExporter::copyTreeView(TreeNode* root, TreeNode* dstRoot)
	{
		for (auto it = root->children.begin(); it != root->children.end(); ++it)
		{
			TreeNode* copy = *it;

			TreeNode* node = new TreeNode(dstRoot->treeView);
			node->alias = copy->alias;
			node->name = GUIDGenerator::genGuid();
			node->icon = copy->icon;
			node->enableDrag = false;
			node->setChecked(true);
			node->setCheckboxEnabled(true);
			node->setEditable(false);
			node->setCheckChildren(true);
			dstRoot->addChild(node);

			copyTreeView(copy, node);
		}
	}

	void DialogAssetExporter::exportAssets(std::string filePath)
	{
		visible = false;
		progressDialog->show();

		std::string ext = IO::GetFileExtension(filePath);

		if (ext != "package")
		{
			progressDialog->hide();
			MainWindow::getConsoleWindow()->log("Error exporting package. Can not write to file of unsupported package format (." + ext + ")", LogMessageType::LMT_ERROR);
			return;
		}

		std::vector<std::string> files;
		listFiles(files, treeView->getRootNode());

		if (files.size() == 0)
		{
			progressDialog->hide();
			MainWindow::getConsoleWindow()->log("Error exporting package. No files specified", LogMessageType::LMT_ERROR);
			return;
		}

		progressDialog->setTitle("Exporting package");
		progressDialog->setStatusText("Packing assets...");

		int err = 0;
		char buf[100];
		struct zip_stat sb;

		zip_t* _zip = zip_open_z(filePath.c_str(), ZIP_CREATE, &err);
		
		if (_zip == nullptr) {
			zip_error_to_str(buf, sizeof(buf), err, errno);
			MainWindow::getConsoleWindow()->log("Error exporting package: " + std::string(filePath) + " (" + buf + ")", LogMessageType::LMT_ERROR);
			progressDialog->hide();
			return;
		}

		int totalFiles = files.size();
		int i = 0;
		
		for (auto it = files.begin(); it != files.end(); ++it, ++i)
		{
			float progress = (float)1.0f / (float)totalFiles * (float)i;

			zip_source_t* source = zip_source_file(_zip, (CP_UNI(Engine::getSingleton()->getAssetsPath()) + *it).c_str(), 0, 0);
			if (zip_file_add(_zip, (*it).c_str(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0)
			{
				zip_source_free(source);
				MainWindow::getConsoleWindow()->log("Error exporting package: " + std::string(zip_strerror(_zip)), LogMessageType::LMT_ERROR);
			}
		}

		zip_register_progress_callback(_zip, progressCallback);
		err = zip_close_z(_zip);

		if (err == 0)
		{
			IO::FileRename(filePath, filePath);
			MainWindow::getConsoleWindow()->log("Assets successfully exported!", LogMessageType::LMT_INFO);
		}
		else
		{
			MainWindow::getConsoleWindow()->log("Error exporting package: " + std::string(zip_strerror(_zip)), LogMessageType::LMT_ERROR);
		}

		progressDialog->hide();
		visible = false;
	}

	void DialogAssetExporter::importAssets()
	{
		visible = false;
		progressDialog->show();

		std::vector<std::string> selectedAssets;
		listFiles(selectedAssets, treeView->getRootNode());

		progressDialog->setTitle("Importing package");
		progressDialog->setStatusText("Extracting assets...");

		const char* archive;
		struct zip* za = nullptr;
		struct zip_file* zf = nullptr;
		struct zip_stat sb;
		char buf[100];
		int err = 0;
		int i = 0, len = 0;
		int fd = 0;
		long long sum = 0;

		if ((za = zip_open_z(openedPackage.c_str(), 0, &err)) == NULL)
		{
			zip_error_to_str(buf, sizeof(buf), err, errno);

			progressDialog->hide();
			visible = false;
			MainWindow::getConsoleWindow()->log("Filed to open package: " + std::string(buf), LogMessageType::LMT_ERROR);
			return;
		}

		int totalFiles = 0;
		int j = 0;

		for (auto it = selectedAssets.begin(); it != selectedAssets.end(); ++it)
		{
			if (!IO::FileExists(Engine::getSingleton()->getAssetsPath() + *it))
				totalFiles += 1;
		}

		for (auto it = selectedAssets.begin(); it != selectedAssets.end(); ++it)
		{
			float progress = (float)1.0f / (float)totalFiles * (float)j;

			progressDialog->setStatusText("Decompressing files... (" + std::to_string(j) + "/" + std::to_string(totalFiles) + ")");
			progressDialog->setProgress(progress);

			if (!IO::FileExists(Engine::getSingleton()->getAssetsPath() + *it))
			{
				++j;

				std::string _p = IO::GetFilePath(Engine::getSingleton()->getAssetsPath() + *it);
				IO::CreateDir(_p, true);

				if (zip_stat(za, (*it).c_str(), ZIP_FL_ENC_UTF_8, &sb) == 0)
				{
					zf = zip_fopen(za, (*it).c_str(), ZIP_FL_ENC_UTF_8);

#ifdef _WIN32
					fd = open((Engine::getSingleton()->getAssetsPath() + CP_SYS(sb.name)).c_str(), O_RDWR | O_TRUNC | O_CREAT | O_BINARY, 0644);
#else
					fd = open((Engine::getSingleton()->getAssetsPath() + CP_SYS(sb.name)).c_str(), O_RDWR | O_TRUNC | O_CREAT, 0644);
#endif
					if (fd < 0)
					{
						std::string err = zip_strerror(za);
						MainWindow::getConsoleWindow()->log("Error extracting package: " + err + " (" + (*it) + ")", LogMessageType::LMT_ERROR);
					}

					sum = 0;
					while (sum != sb.size)
					{
						len = zip_fread(zf, buf, 100);
						if (len < 0)
						{
							std::string err = zip_strerror(za);
							MainWindow::getConsoleWindow()->log("Error extracting package: " + err + " (" + (*it) + ")", LogMessageType::LMT_ERROR);
						}
						write(fd, buf, len);
						sum += len;
					}

					close(fd);
					zip_fclose(zf);
				}
				else
				{
					std::string err = zip_strerror(za);
					MainWindow::getConsoleWindow()->log("Error extracting package: " + err + " (" + (*it) + ")", LogMessageType::LMT_ERROR);
				}
			}
		}

		zip_close_z(za);

		progressDialog->hide();
		visible = false;

		MainWindow::getSingleton()->onRestore();
		MainWindow::getConsoleWindow()->log("Assets successfully imported!", LogMessageType::LMT_INFO);
	}

	void DialogAssetExporter::checkNodes(TreeNode* root, bool check)
	{
		root->setChecked(check);

		for (auto it = root->children.begin(); it != root->children.end(); ++it)
			checkNodes(*it, allSelected);
	}

	void DialogAssetExporter::progressCallback(double progress)
	{
		progressDialog->setStatusText("Compressing files...");
		progressDialog->setProgress(progress);
	}
}