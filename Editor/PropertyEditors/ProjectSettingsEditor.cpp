#include "ProjectSettingsEditor.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"

#include "../Windows/MainWindow.h"
#include "../Dialogs/DialogProgress.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/LayerMask.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Components/Light.h"
#include "../Engine/Serialization/Settings/ProjectSettings.h"
#include "../Engine/UI/ImGUIWidgets.h"

#include "PropString.h"
#include "PropBool.h"
#include "PropComboBox.h"
#include "PropFloat.h"
#include "PropInt.h"
#include "PropCustom.h"

namespace GX
{
	ProjectSettingsEditor::ProjectSettingsEditor()
	{
		progressDialog = new DialogProgress();
	}

	ProjectSettingsEditor::~ProjectSettingsEditor()
	{
		delete progressDialog;
	}

	void ProjectSettingsEditor::init()
	{
		ProjectSettings * projectSettings = Engine::getSingleton()->getSettings();

		//Display
		Property* displaySettings = new Property(this, "Display");

		PropBool* fullScreen = new PropBool(this, "Full screen", projectSettings->getFullScreen());
		fullScreen->setOnChangeCallback([=](Property* prop, bool val) { onChangeFullScreen(prop, val); });
		displaySettings->addChild(fullScreen);

		if (projectSettings->getFullScreen())
		{
			PropBool* exclusiveMode = new PropBool(this, "Exclusive mode", projectSettings->getExclusiveMode());
			exclusiveMode->setOnChangeCallback([=](Property* prop, bool val) { onChangeExclusiveMode(prop, val); });
			displaySettings->addChild(exclusiveMode);
		}

		if (!projectSettings->getFullScreen() || (projectSettings->getFullScreen() && projectSettings->getExclusiveMode()))
		{
			PropBool* useNativeResolution = new PropBool(this, "Use native resolution", projectSettings->getUseNativeResolution());
			useNativeResolution->setOnChangeCallback([=](Property* prop, bool val) { onChangeUseNativeResolution(prop, val); });

			displaySettings->addChild(useNativeResolution);

			if (!projectSettings->getUseNativeResolution())
			{
				PropInt* screenWidth = new PropInt(this, "Screen width", projectSettings->getScreenWidth());
				screenWidth->setOnChangeCallback([=](Property* prop, int val) { onChangeScreenWidth(prop, val); });
				displaySettings->addChild(screenWidth);

				PropInt* screenHeight = new PropInt(this, "Screen height", projectSettings->getScreenHeight());
				screenHeight->setOnChangeCallback([=](Property* prop, int val) { onChangeScreenHeight(prop, val); });
				displaySettings->addChild(screenHeight);
			}
		}

		PropBool* useDynamicResolution = new PropBool(this, "Use dynamic resolution", projectSettings->getUseDynamicResolution());
		useDynamicResolution->setOnChangeCallback([=](Property* prop, bool val) { onChangeUseDynamicResolution(prop, val); });
		displaySettings->addChild(useDynamicResolution);

		if (projectSettings->getUseDynamicResolution())
		{
			PropBool* autoDynamicResolution = new PropBool(this, "Auto dynamic resolution", projectSettings->getAutoDynamicResolution());
			autoDynamicResolution->setOnChangeCallback([=](Property* prop, bool val) { onChangeAutoDynamicResolution(prop, val); });
			displaySettings->addChild(autoDynamicResolution);

			if (!projectSettings->getAutoDynamicResolution())
			{
				PropFloat* dynamicResolution = new PropFloat(this, "Dynamic resolution", projectSettings->getDynamicResolution());
				dynamicResolution->setOnChangeCallback([=](Property* prop, float val) { onChangeDynamicResolution(prop, val); });
				dynamicResolution->setMinValue(0.5f);
				dynamicResolution->setMaxValue(1.0f);
				displaySettings->addChild(dynamicResolution);
			}
			else
			{
				PropInt* dynamicResolutionFPS = new PropInt(this, "Target FPS", projectSettings->getAutoDynamicResolutionTargetFPS());
				dynamicResolutionFPS->setOnChangeCallback([=](Property* prop, float val) { onChangeAutoDynamicResolutionTargetFPS(prop, val); });
				dynamicResolutionFPS->setMinValue(30);
				dynamicResolutionFPS->setMaxValue(120);
				displaySettings->addChild(dynamicResolutionFPS);
			}
		}

		addProperty(displaySettings);

		//Graphics
		Property* graphicsSettings = new Property(this, "Graphics");

		PropBool* fxaa = new PropBool(this, "FXAA", projectSettings->getFXAA());
		fxaa->setOnChangeCallback([=](Property* prop, bool val) { onChangeFXAA(prop, val); });
		graphicsSettings->addChild(fxaa);

		PropBool* vsync = new PropBool(this, "V-Sync", projectSettings->getVSync());
		graphicsSettings->addChild(vsync);

		//Shadows
		Property* shadowSettings = new Property(this, "Shadows");
		graphicsSettings->addChild(shadowSettings);

		PropBool* shadowsEnabled = new PropBool(this, "Enable shadows", projectSettings->getShadowsEnabled());

		PropComboBox* dirShadows = new PropComboBox(this, "Directional resolution", { "512", "1024", "2048", "4096" });
		dirShadows->setCurrentItem(std::to_string(projectSettings->getDirectionalShadowResolution()));

		PropComboBox* spotShadows = new PropComboBox(this, "Spot resolution", { "256", "512", "1024", "2048", "4096" });
		spotShadows->setCurrentItem(std::to_string(projectSettings->getSpotShadowResolution()));

		PropComboBox* pointShadows = new PropComboBox(this, "Point resolution", { "512", "1024", "2048", "4096" });
		pointShadows->setCurrentItem(std::to_string(projectSettings->getPointShadowResolution()));

		PropComboBox* shadowCascades = new PropComboBox(this, "Cascades count", { "2", "3", "4" });
		shadowCascades->setCurrentItem(std::to_string(projectSettings->getShadowCascadesCount()));

		PropComboBox* shadowSamples = new PropComboBox(this, "Softeness", { "1", "2", "4", "6" });
		shadowSamples->setCurrentItem(std::to_string(projectSettings->getShadowSamplesCount()));

		PropFloat* shadowDistance = new PropFloat(this, "Shadow distance", projectSettings->getShadowDistance());

		//PropBool* shadowCascadesBlending = new PropBool(this, "Cascades blending", projectSettings->getShadowCascadesBlending());

		shadowSettings->addChild(shadowsEnabled);
		shadowSettings->addChild(dirShadows);
		shadowSettings->addChild(spotShadows);
		shadowSettings->addChild(pointShadows);
		shadowSettings->addChild(shadowCascades);
		shadowSettings->addChild(shadowSamples);
		shadowSettings->addChild(shadowDistance);
		//shadowSettings->addChild(shadowCascadesBlending);

		dirShadows->setOnChangeCallback([=](Property* prop, std::string val) { onChangeDirShadows(prop, val); });
		spotShadows->setOnChangeCallback([=](Property* prop, std::string val) { onChangeSpotShadows(prop, val); });
		pointShadows->setOnChangeCallback([=](Property* prop, std::string val) { onChangePointShadows(prop, val); });
		shadowCascades->setOnChangeCallback([=](Property* prop, std::string val) { onChangeCascades(prop, val); });
		shadowSamples->setOnChangeCallback([=](Property* prop, std::string val) { onChangeSamples(prop, val); });
		shadowsEnabled->setOnChangeCallback([=](Property* prop, bool val) { onChangeShadowsEnabled(prop, val); });
		shadowDistance->setOnChangeCallback([=](Property* prop, float val) { onChangeShadowDistance(prop, val); });
		//shadowCascadesBlending->setOnChangeCallback([=](Property* prop, bool val) { onChangeCascadesBlending(prop, val); });
		vsync->setOnChangeCallback([=](Property* prop, bool val) { onChangeVSync(prop, val); });

		Property* textureSettings = new Property(this, "Textures");
		graphicsSettings->addChild(textureSettings);

		PropComboBox* textureCompression = new PropComboBox(this, "Compression", { "None", "BC7" });
		textureCompression->setCurrentItem(projectSettings->getTextureCompression() - 1);
		textureCompression->setOnChangeCallback([=](Property* prop, int val) { onChangeTextureCompression(prop, val); });

		textureSettings->addChild(textureCompression);

		PropComboBox* textureCompressionQuality = new PropComboBox(this, "Compression quality", { "Low", "Normal", "High", "Very High" });
		textureCompressionQuality->setCurrentItem(projectSettings->getTextureCompressionQuality());
		textureCompressionQuality->setOnChangeCallback([=](Property* prop, int val) { onChangeTextureCompressionQuality(prop, val); });

		textureSettings->addChild(textureCompressionQuality);

		PropComboBox* textureMaxResolution = new PropComboBox(this, "Max resolution", { "8192", "4096", "2048", "1024", "512", "256", "128", "64", "32", "16" });
		textureMaxResolution->setCurrentItem(std::to_string(projectSettings->getTextureMaxResolution()));
		textureMaxResolution->setOnChangeCallback([=](Property* prop, int val) { onChangeTextureMaxResolution(prop, val); });

		textureSettings->addChild(textureMaxResolution);

		addProperty(graphicsSettings);

		//Steam API
		Property* steamAPI = new Property(this, "Steam API");

		PropBool* enableSteamAPI = new PropBool(this, "Enable Steam API", projectSettings->getEnableSteamAPI());
		enableSteamAPI->setOnChangeCallback([=](Property* prop, bool val) { onChangeEnableSteamAPI(prop, val); });
		
		steamAPI->addChild(enableSteamAPI);

		if (projectSettings->getEnableSteamAPI())
		{
			PropInt* steamAppId = new PropInt(this, "App ID", projectSettings->getSteamAppId());
			steamAppId->setIsDraggable(false);
			steamAppId->setOnChangeCallback([=](Property* prop, int val) { onChangeSteamAppID(prop, val); });
			
			steamAPI->addChild(steamAppId);
		}
		
		addProperty(steamAPI);

		Property* tags = new Property(this, "Tags");
		tags->setHasButton(true);
		tags->setButtonText("+");
		tags->setOnButtonClickCallback([=](TreeNode* prop) { onAddTag(prop); });

		int i = 0;
		for (auto it = projectSettings->getTags().begin(); it != projectSettings->getTags().end(); ++it, ++i)
		{
			PropString* tag = new PropString(this, std::to_string(i), *it);
			tag->setUserData(reinterpret_cast<void*>(new int(i)));
			tag->setPopupMenu({ "Remove" }, [=](TreeNode* node, int val) { onTagPopup(node, val); });
			tag->setOnChangeCallback([=](Property* prop, std::string val) { onChangeTag(prop, val); });

			tags->addChild(tag);
		}

		addProperty(tags);

		Property* layers = new Property(this, "Layers");
		layers->setHasButton(true);
		layers->setButtonText("+");
		layers->setOnButtonClickCallback([=](TreeNode* prop) { onAddLayer(prop); });

		i = 0;
		for (auto it = projectSettings->getLayers().begin(); it != projectSettings->getLayers().end(); ++it, ++i)
		{
			PropString* layer = new PropString(this, std::to_string(i), *it);
			layer->setUserData(reinterpret_cast<void*>(new int(i)));
			layer->setPopupMenu({ "Remove" }, [=](TreeNode* node, int val) { onLayerPopup(node, val); });
			layer->setOnChangeCallback([=](Property* prop, std::string val) { onChangeLayer(prop, val); });

			layers->addChild(layer);
		}

		addProperty(layers);

		Property* physics = new Property(this, "Physics");
		addProperty(physics);
		
		Property* layerMatrixGroup = new Property(this, "Layer collision matrix");
		physics->addChild(layerMatrixGroup);

		auto _layers = LayerMask::getAllLayers();

		PropCustom* layerMatrix = new PropCustom(this, "Layer collision matrix");
		layerMatrix->setOnUpdateCallback([=](Property* prop)
			{
				ImVec2 cp = ImGui::GetCursorPos();

				float maxTextHeight = 0;
				for (int i = 0; i < _layers.size(); ++i)
				{
					float th = ImGui::CalcTextSizeVertical(_layers[_layers.size() - 1 - i].c_str());
					maxTextHeight = std::max(maxTextHeight, th);
				}

				float maxTextWidth = 0;
				for (int i = 0; i < _layers.size(); ++i)
				{
					ImGui::SetCursorPos(ImVec2(cp.x + 10, cp.y + 10 + 10 + maxTextHeight + i * 24));
					ImGui::Text(_layers[i].c_str());
					float tw = ImGui::CalcTextSize(_layers[i].c_str()).x;
					maxTextWidth = std::max(maxTextWidth, tw);
				}

				ImVec2 winPos = ImGui::GetWindowPos();
				ImVec2 scrollPos = ImVec2(-ImGui::GetScrollX(), -ImGui::GetScrollY());
				for (int i = 0; i < _layers.size(); ++i)
				{
					ImVec2 p = ImVec2(cp.x + winPos.x + scrollPos.x + 10 + 10 + maxTextWidth + (i * 24), cp.y + winPos.y + scrollPos.y + 10 + maxTextHeight);
					ImGui::AddTextVertical(ImGui::GetWindowDrawList(), _layers[_layers.size() - 1 - i].c_str(), p, 0xffffffff);
				}
				
				for (int i = 0; i < _layers.size(); ++i)
				{
					for (int j = 0; j < _layers.size() - i; ++j)
					{
						bool v = projectSettings->getCollisionMask(_layers.size() - 1 - i, j);
						std::string _name = "##layerMask_" + std::to_string(i) + std::to_string(j);
						ImGui::SetCursorPos(ImVec2(cp.x + 10 + 10 + maxTextWidth + (i * 24), cp.y + 10 + 10 + maxTextHeight + (j * 24)));
						if (ImGui::Checkbox(_name.c_str(), &v))
						{
							onChangeCollisionMask(_layers.size() - 1 - i, j, v);
						}
					}
				}
			}
		);

		layerMatrixGroup->addChild(layerMatrix);
	}

	void ProjectSettingsEditor::update()
	{
		PropertyEditor::update();
	}

	void ProjectSettingsEditor::updateEditor()
	{
		MainWindow::addOnEndUpdateCallback([=]() {
			getTreeView()->clear();
			init();
		});
	}

	void ProjectSettingsEditor::onChangeUseNativeResolution(Property* prop, bool val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setUseNativeResolution(val);
		projectSettings->save();

		updateEditor();
	}

	void ProjectSettingsEditor::onChangeExclusiveMode(Property* prop, bool val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setExclusiveMode(val);
		projectSettings->save();

		updateEditor();
	}

	void ProjectSettingsEditor::onChangeFullScreen(Property* prop, bool val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setFullScreen(val);
		projectSettings->save();

		updateEditor();
	}

	void ProjectSettingsEditor::onChangeScreenWidth(Property* prop, int val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setScreenWidth(val);
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeScreenHeight(Property* prop, int val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setScreenHeight(val);
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeShadowsEnabled(Property* prop, bool val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setShadowsEnabled(val);
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeDirShadows(Property* prop, std::string val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setDirectionalShadowResolution(atoi(val.c_str()));
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeSpotShadows(Property* prop, std::string val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setSpotShadowResolution(atoi(val.c_str()));
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangePointShadows(Property* prop, std::string val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setPointShadowResolution(atoi(val.c_str()));
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeCascades(Property* prop, std::string val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setShadowCascadesCount(atoi(val.c_str()));
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeSamples(Property* prop, std::string val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setShadowSamplesCount(atoi(val.c_str()));
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeCascadesBlending(Property* prop, bool val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setShadowCascadesBlending(val);
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeShadowDistance(Property* prop, float val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setShadowDistance(val);
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeVSync(Property* prop, bool val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setVSync(val);
		projectSettings->save();

		updateEditor();
	}

	void ProjectSettingsEditor::onChangeFXAA(Property* prop, bool val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setFXAA(val);
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeUseDynamicResolution(Property* prop, bool val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setUseDynamicResolution(val);
		projectSettings->save();

		updateEditor();
	}

	void ProjectSettingsEditor::onChangeAutoDynamicResolution(Property* prop, bool val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setAutoDynamicResolution(val);
		projectSettings->save();

		updateEditor();
	}

	void ProjectSettingsEditor::onChangeAutoDynamicResolutionTargetFPS(Property* prop, float val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setAutoDynamicResolutionTargetFPS(val);
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeDynamicResolution(Property* prop, float val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setDynamicResolution(val);
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeTextureCompression(Property* prop, int val)
	{
		MainWindow::addOnEndUpdateCallback([=]()
			{
				progressDialog->show();
				progressDialog->setTitle("Reimporting textures");

				ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
				projectSettings->setTextureCompression(val + 1, [=](std::string status, int progress)
					{
						progressDialog->setStatusText(status);
						progressDialog->setProgress(progress / 100.0f);
					}
				);
				projectSettings->save();

				progressDialog->hide();
			}
		);
	}

	void ProjectSettingsEditor::onChangeTextureCompressionQuality(Property* prop, int val)
	{
		MainWindow::addOnEndUpdateCallback([=]()
			{
				progressDialog->show();
				progressDialog->setTitle("Reimporting textures");

				ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
				projectSettings->setTextureCompressionQuality(val, [=](std::string status, int progress)
					{
						progressDialog->setStatusText(status);
						progressDialog->setProgress(progress / 100.0f);
					}
				);
				projectSettings->save();

				progressDialog->hide();
			}
		);
	}

	void ProjectSettingsEditor::onChangeTextureMaxResolution(Property* prop, int val)
	{
		std::vector<std::string> resolutions = { "8192", "4096", "2048", "1024", "512", "256", "128", "64", "32", "16" };
		int resolution = std::atoi(resolutions[val].c_str());

		MainWindow::addOnEndUpdateCallback([=]()
			{
				progressDialog->show();
				progressDialog->setTitle("Reimporting textures");

				ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
				projectSettings->setTextureMaxResolution(resolution, [=](std::string status, int progress)
					{
						progressDialog->setStatusText(status);
						progressDialog->setProgress(progress / 100.0f);
					}
				);
				projectSettings->save();

				progressDialog->hide();
			}
		);
	}

	void ProjectSettingsEditor::onChangeEnableSteamAPI(Property* prop, bool val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setEnableSteamAPI(val);
		projectSettings->save();

		updateEditor();
	}

	void ProjectSettingsEditor::onChangeSteamAppID(Property* prop, int val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setSteamAppId(val);
		projectSettings->save();
	}

	void ProjectSettingsEditor::onChangeTag(Property* prop, std::string val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		void* data = prop->getUserData();
		int i = *reinterpret_cast<int*>(data);

		auto it = projectSettings->getTags().begin() + i;
		*it = val;

		projectSettings->save();
	}

	void ProjectSettingsEditor::onAddTag(TreeNode* prop)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->getTags().push_back("");
		updateEditor();
	}

	void ProjectSettingsEditor::onTagPopup(TreeNode* node, int val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		if (val == 0)
		{
			void* data = node->getUserData();
			int i = *reinterpret_cast<int*>(data);

			MainWindow::addOnEndUpdateCallback([=]() {
				auto it = projectSettings->getTags().begin() + i;
				projectSettings->getTags().erase(it);
				projectSettings->save();

				updateEditor();
			});
		}
	}

	void ProjectSettingsEditor::onChangeLayer(Property* prop, std::string val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		void* data = prop->getUserData();
		int i = *reinterpret_cast<int*>(data);

		auto it = projectSettings->getLayers().begin() + i;
		*it = val;

		projectSettings->save();

		updateEditor();
	}

	void ProjectSettingsEditor::onAddLayer(TreeNode* prop)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		if (projectSettings->getLayers().size() + 1 < LayerMask::MAX_LAYERS)
		{
			projectSettings->getLayers().push_back("");
			updateEditor();
		}
	}

	void ProjectSettingsEditor::onLayerPopup(TreeNode* node, int val)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		if (val == 0)
		{
			void* data = node->getUserData();
			int i = *reinterpret_cast<int*>(data);

			MainWindow::addOnEndUpdateCallback([=]() {
				auto it = projectSettings->getLayers().begin() + i;
				projectSettings->getLayers().erase(it);
				projectSettings->save();

				updateEditor();
			});
		}
	}

	void ProjectSettingsEditor::onChangeCollisionMask(int i, int j, bool value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setCollisionMask(i, j, value);
		projectSettings->save();
	}
}