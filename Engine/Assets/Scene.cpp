#include "Scene.h"

#include <fstream>
#include <iostream>
#include <unordered_map>

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Core/NavigationManager.h"

#include "../Renderer/Renderer.h"
#include "../Renderer/Color.h"
#include "../Renderer/BatchedGeometry.h"
#include "../Renderer/CSGGeometry.h"

#include "../Classes/IO.h"
#include "../Classes/Hash.h"

#include "../Assets/Model3DLoader.h"
#include "../Assets/Mesh.h"
#include "../Assets/AnimationClip.h"
#include "../Assets/Texture.h"
#include "../Assets/Font.h"
#include "../Assets/Cubemap.h"
#include "../Assets/Prefab.h"
#include "../Assets/AudioClip.h"
#include "../Assets/VideoClip.h"
#include "../Assets/Material.h"

#include "../Core/GameObject.h"
#include "../Core/Debug.h"
#include "../Core/Time.h"
#include "../Components/Transform.h"
#include "../Components/MeshRenderer.h"
#include "../Components/Light.h"
#include "../Components/Camera.h"
#include "../Components/Animation.h"
#include "../Components/RigidBody.h"
#include "../Components/BoxCollider.h"
#include "../Components/CapsuleCollider.h"
#include "../Components/MeshCollider.h"
#include "../Components/SphereCollider.h"
#include "../Components/TerrainCollider.h"
#include "../Components/Vehicle.h"
#include "../Components/FixedJoint.h"
#include "../Components/FreeJoint.h"
#include "../Components/HingeJoint.h"
#include "../Components/ConeTwistJoint.h"
#include "../Components/AudioListener.h"
#include "../Components/AudioSource.h"
#include "../Components/NavMeshAgent.h"
#include "../Components/NavMeshObstacle.h"
#include "../Components/Canvas.h"
#include "../Components/Image.h"
#include "../Components/Text.h"
#include "../Components/Button.h"
#include "../Components/Mask.h"
#include "../Components/TextInput.h"
#include "../Components/MonoScript.h"
#include "../Components/Terrain.h"
#include "../Components/ParticleSystem.h"
#include "../Components/Water.h"
#include "../Components/Spline.h"
#include "../Components/VideoPlayer.h"
#include "../Components/CSGModel.h"
#include "../Components/CSGBrush.h"
#include "../Components/DecalRenderer.h"

#include "../Serialization/Scene/SScene.h"
#include "../Serialization/Scene/SGameObject.h"

#include "../Classes/ZipHelper.h"
#include "../Classes/md5.h"

namespace GX
{
	std::string Scene::ASSET_TYPE = "Scene";

	std::string Scene::loadedScene = "";

	void Scene::save(std::string location, std::string name)
	{
		SScene scene;

		scene.ambientColor = Renderer::getSingleton()->getAmbientColor();
		scene.skyModel = static_cast<int>(Renderer::getSingleton()->getSkyModel());
		Material* skyMaterial = Renderer::getSingleton()->getSkyMaterial();
		if (skyMaterial != nullptr && skyMaterial->isLoaded())
			scene.skyboxMaterial = skyMaterial->getName();

		scene.giEnabled = Renderer::getSingleton()->getGIEnabled();
		scene.giIntensity = Renderer::getSingleton()->getGIIntensity();

		scene.fogEnabled = Renderer::getSingleton()->getFogEnabled();
		scene.fogStartDistance = Renderer::getSingleton()->getFogStartDistance();
		scene.fogEndDistance = Renderer::getSingleton()->getFogEndDistance();
		scene.fogDensity = Renderer::getSingleton()->getFogDensity();
		scene.fogColor = Renderer::getSingleton()->getFogColor();
		scene.fogIncludeSkybox = Renderer::getSingleton()->getFogIncludeSkybox();
		scene.fogType = Renderer::getSingleton()->getFogType();

		scene.navMeshSettings.cellHeight = NavigationManager::getSingleton()->getCellHeight();
		scene.navMeshSettings.cellSize = NavigationManager::getSingleton()->getCellSize();
		scene.navMeshSettings.maxEdgeLen = NavigationManager::getSingleton()->getMaxEdgeLen();
		scene.navMeshSettings.maxSimplificationError = NavigationManager::getSingleton()->getMaxSimplificationError();
		scene.navMeshSettings.mergeRegionArea = NavigationManager::getSingleton()->getMergeRegionArea();
		scene.navMeshSettings.minRegionArea = NavigationManager::getSingleton()->getMinRegionArea();
		scene.navMeshSettings.walkableClimb = NavigationManager::getSingleton()->getWalkableClimb();
		scene.navMeshSettings.walkableHeight = NavigationManager::getSingleton()->getWalkableHeight();
		scene.navMeshSettings.walkableRadius = NavigationManager::getSingleton()->getWalkableRadius();
		scene.navMeshSettings.walkableSlopeAngle = NavigationManager::getSingleton()->getWalkableSlopeAngle();

		Engine::getSingleton()->stopAllAnimations();

		std::vector<GameObject*> gameObjects = Engine::getSingleton()->getGameObjects();

		for (auto it = gameObjects.begin(); it != gameObjects.end(); ++it)
		{
			GameObject* obj = *it;

			saveObject(obj, &scene);
		}

		//Serialize
		std::string fullPath = location + name;
		std::ofstream ofs(fullPath, std::ios::binary);
		BinarySerializer s;
		s.serialize(&ofs, &scene, Scene::ASSET_TYPE);
		ofs.close();

		if (BatchedGeometry::getSingleton()->needRebuild())
			BatchedGeometry::getSingleton()->rebuild(true);

		auto& csgModels = CSGGeometry::getSingleton()->getModels();

		for (auto model : csgModels)
		{
			if (model->_needRebuild)
				CSGGeometry::getSingleton()->rebuild(model->component, true);
		}

		std::string geomName = IO::GetFilePath(name) + IO::GetFileName(name) + "/Static Geometry/" + md5(IO::GetFileName(name)) + ".mesh";
		
		std::string libPath = Engine::getSingleton()->getLibraryPath();

		if (BatchedGeometry::getSingleton()->getBatches().size() > 0)
		{
			std::string dir = IO::GetFilePath(libPath + geomName);
			if (!IO::DirExists(dir))
				IO::CreateDir(dir, true);

			BatchedGeometry::getSingleton()->saveToFile(libPath, geomName);
		}
		else
		{
			if (IO::FileExists(libPath + geomName))
				IO::FileDelete(libPath + geomName);
		}

		std::string _csgName = IO::GetFilePath(name) + IO::GetFileName(name) + "/CSG Geometry/";
		IO::DirDeleteRecursive(libPath + _csgName);

		if (csgModels.size() > 0)
		{
			for (auto& model : csgModels)
			{
				std::string hash = model->component->getGameObject()->getGuid();
				std::string csgName = IO::GetFilePath(name) + IO::GetFileName(name) + "/CSG Geometry/" + md5(IO::GetFileName(name) + "_" + hash + "_csg") + ".mesh";

				std::string dir = IO::GetFilePath(libPath + csgName);
				if (!IO::DirExists(dir))
					IO::CreateDir(dir, true);

				CSGGeometry::getSingleton()->saveToFile(libPath, csgName, model->component);
			}
		}

		loadedScene = name;
	}

	void Scene::saveObject(GameObject* gameObject, SScene* scene)
	{
		if (!gameObject->isSerializable())
			return;

		Transform* transform = gameObject->getTransform();
		GameObject* parent = nullptr;

		if (transform->getParent() != nullptr)
			parent = transform->getParent()->getGameObject();

		SGameObject sObj;
		sObj.enabled = gameObject->getEnabled();
		sObj.name = gameObject->getName();
		sObj.guid = gameObject->getGuid();
		sObj.tag = gameObject->getTag();
		sObj.layer = gameObject->getLayer();
		sObj.navigationStatic = gameObject->getNavigationStatic();
		sObj.lightingStatic = gameObject->getLightingStatic();
		sObj.batchingStatic = gameObject->getBatchingStatic();
		sObj.occlusionStatic = gameObject->getOcclusionStatic();
		if (parent != nullptr) sObj.parentGuid = parent->getGuid();
		sObj.position = SVector3(transform->getPosition());
		sObj.scale = SVector3(transform->getScale());
		sObj.rotation = SQuaternion(transform->getRotation());

		std::vector<Component*>& components = gameObject->getComponents();

		int idx = 0;
		for (auto ci = components.begin(); ci != components.end(); ++ci, ++idx)
		{
			Component* component = *ci;

			// MeshRenderer
			if (component->getComponentType() == MeshRenderer::COMPONENT_TYPE)
			{
				SMeshRenderer sComponent;
				MeshRenderer* component1 = (MeshRenderer*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.castShadows = component1->getCastShadows();
				sComponent.lightmapSize = component1->getLightmapSize();
				sComponent.lodMaxDistance = component1->getLodMaxDistance();
				sComponent.cullOverMaxDistance = component1->getCullOverMaxDistance();

				Mesh* mesh = component1->getMesh();
				if (mesh != nullptr)
				{
					sComponent.mesh = mesh->getName();
					sComponent.meshSourceFile = mesh->getSourceFile();
					sComponent.rootObjectGuid = component1->getRootObjectGuid();

					for (int i = 0; i < component1->getSharedMaterialsCount(); ++i)
					{
						Material* material = component1->getSharedMaterial(i);
						sComponent.materials.push_back(material->getName());
					}
				}

				sObj.meshRenderers.push_back(sComponent);
			}

			// DecalRenderer
			if (component->getComponentType() == DecalRenderer::COMPONENT_TYPE)
			{
				SDecalRenderer sComponent;
				DecalRenderer* component1 = (DecalRenderer*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				Material* material = component1->getMaterial();
				if (material != nullptr)
					sComponent.material = material->getName();

				sObj.decals.push_back(sComponent);
			}

			// Light
			if (component->getComponentType() == Light::COMPONENT_TYPE)
			{
				SLight sComponent;
				Light* component1 = (Light*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.color = SColor(component1->getColor());
				sComponent.intensity = component1->getIntensity();
				sComponent.lightType = static_cast<int>(component1->getLightType());
				sComponent.lightRenderMode = static_cast<int>(component1->getLightRenderMode());
				sComponent.radius = component1->getRadius();
				sComponent.innerRadius = component1->getInnerRadius();
				sComponent.outerRadius = component1->getOuterRadius();
				sComponent.bias = component1->getBias();
				sComponent.castShadows = component1->getCastShadows();

				sObj.lights.push_back(sComponent);
			}

			// Camera
			if (component->getComponentType() == Camera::COMPONENT_TYPE)
			{
				SCamera sComponent;
				Camera* component1 = (Camera*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.clearColor = component1->getClearColor();
				sComponent.depth = component1->getDepth();
				sComponent.fovy = component1->getFOVy();
				sComponent.znear = component1->getNear();
				sComponent.zfar = component1->getFar();
				sComponent.viewport_l = component1->getViewportLeft();
				sComponent.viewport_t = component1->getViewportTop();
				sComponent.viewport_w = component1->getViewportWidth();
				sComponent.viewport_h = component1->getViewportHeight();
				sComponent.clearFlags = static_cast<int>(component1->getClearFlags());
				sComponent.cullingMask.resize(LayerMask::MAX_LAYERS);
				for (int i = 0; i < LayerMask::MAX_LAYERS; ++i)
					sComponent.cullingMask[i] = component1->getCullingMask().getLayer(i);
				sComponent.projectionType = static_cast<int>(component1->getProjectionType());
				sComponent.orthographicSize = component1->getOrthographicSize();
				sComponent.occlusionCulling = component1->getOcclusionCulling();

				sObj.cameras.push_back(sComponent);
			}

			// Animation
			if (component->getComponentType() == Animation::COMPONENT_TYPE)
			{
				SAnimation sComponent;
				Animation* component1 = (Animation*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				for (int i = 0; i < component1->getNumAnimationClips(); ++i)
				{
					AnimationClipInfo* inf = component1->getAnimationClip(i);
					SAnimationClipInfo ainf;
					if (inf->clip != nullptr)
						ainf.clipPath = inf->clip->getName();
					ainf.name = inf->name;
					ainf.speed = inf->speed;
					ainf.startFrame = inf->startFrame;
					ainf.endFrame = inf->endFrame;
					ainf.loop = inf->loop;

					sComponent.animationClips.push_back(ainf);
				}

				sObj.animations.push_back(sComponent);
			}

			// BoxCollider
			if (component->getComponentType() == BoxCollider::COMPONENT_TYPE)
			{
				SBoxCollider sComponent;
				BoxCollider* component1 = (BoxCollider*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.boxSize = component1->getBoxSize();
				sComponent.isTrigger = component1->getIsTrigger();
				sComponent.offset = component1->getOffset();
				sComponent.rotation = component1->getRotation();

				sObj.boxColliders.push_back(sComponent);
			}

			// CapsuleCollider
			if (component->getComponentType() == CapsuleCollider::COMPONENT_TYPE)
			{
				SCapsuleCollider sComponent;
				CapsuleCollider* component1 = (CapsuleCollider*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.height = component1->getHeight();
				sComponent.isTrigger = component1->getIsTrigger();
				sComponent.offset = component1->getOffset();
				sComponent.rotation = component1->getRotation();
				sComponent.radius = component1->getRadius();

				sObj.capsuleColliders.push_back(sComponent);
			}

			// SphereCollider
			if (component->getComponentType() == SphereCollider::COMPONENT_TYPE)
			{
				SSphereCollider sComponent;
				SphereCollider* component1 = (SphereCollider*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.isTrigger = component1->getIsTrigger();
				sComponent.offset = component1->getOffset();
				sComponent.radius = component1->getRadius();

				sObj.sphereColliders.push_back(sComponent);
			}

			// MeshCollider
			if (component->getComponentType() == MeshCollider::COMPONENT_TYPE)
			{
				SMeshCollider sComponent;
				MeshCollider* component1 = (MeshCollider*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.isTrigger = component1->getIsTrigger();
				sComponent.convex = component1->getConvex();

				sObj.meshColliders.push_back(sComponent);
			}

			// TerrainCollider
			if (component->getComponentType() == TerrainCollider::COMPONENT_TYPE)
			{
				STerrainCollider sComponent;
				TerrainCollider* component1 = (TerrainCollider*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.isTrigger = component1->getIsTrigger();

				sObj.terrainColliders.push_back(sComponent);
			}

			// Vehicle
			if (component->getComponentType() == Vehicle::COMPONENT_TYPE)
			{
				SVehicle sComponent;
				Vehicle* component1 = (Vehicle*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.invertForward = component1->getInvertForward();
				sComponent.vehicleAxis = component1->getAxis();

				std::vector<Vehicle::WheelInfo>& wheels = component1->getWheels();

				for (auto _it = wheels.begin(); _it != wheels.end(); ++_it)
				{
					SWheelInfo inf;
					inf.m_connectedObjectGuid = _it->m_connectedObjectGuid;
					inf.m_axle = _it->m_axle;
					inf.m_connectionPoint = _it->m_connectionPoint;
					inf.m_direction = _it->m_direction;
					inf.m_friction = _it->m_friction;
					inf.m_isFrontWheel = _it->m_isFrontWheel;
					inf.m_radius = _it->m_radius;
					inf.m_rollInfluence = _it->m_rollInfluence;
					inf.m_suspensionCompression = _it->m_suspensionCompression;
					inf.m_suspensionDamping = _it->m_suspensionDamping;
					inf.m_suspensionRestLength = _it->m_suspensionRestLength;
					inf.m_suspensionStiffness = _it->m_suspensionStiffness;
					inf.m_width = _it->m_width;

					sComponent.wheels.push_back(inf);
				}

				sObj.vehicles.push_back(sComponent);
			}

			// RigidBody
			if (component->getComponentType() == RigidBody::COMPONENT_TYPE)
			{
				SRigidBody sComponent;
				RigidBody* component1 = (RigidBody*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.mass = component1->getMass();
				sComponent.freezePositionX = component1->getFreezePositionX();
				sComponent.freezePositionY = component1->getFreezePositionY();
				sComponent.freezePositionZ = component1->getFreezePositionZ();
				sComponent.freezeRotationX = component1->getFreezeRotationX();
				sComponent.freezeRotationY = component1->getFreezeRotationY();
				sComponent.freezeRotationZ = component1->getFreezeRotationZ();
				sComponent.isKinematic = component1->getIsKinematic();
				sComponent.isStatic = component1->getIsStatic();
				sComponent.useOwnGravity = component1->getUseOwnGravity();
				sComponent.gravity = component1->getGravity();
				sComponent.friction = component1->getFriction();
				sComponent.bounciness = component1->getBounciness();
				sComponent.linearDamping = component1->getLinearDamping();
				sComponent.angularDamping = component1->getAngularDamping();

				sObj.rigidBodies.push_back(sComponent);
			}

			// FixedJoint
			if (component->getComponentType() == FixedJoint::COMPONENT_TYPE)
			{
				SFixedJoint sComponent;
				FixedJoint* component1 = (FixedJoint*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.connectedObjectGuid = component1->getConnectedObjectGuid();
				sComponent.anchor = component1->getAnchor();
				sComponent.connectedAnchor = component1->getConnectedAnchor();
				sComponent.linkedBodiesCollision = component1->getLinkedBodiesCollision();

				sObj.fixedJoints.push_back(sComponent);
			}

			// FreeJoint
			if (component->getComponentType() == FreeJoint::COMPONENT_TYPE)
			{
				SFreeJoint sComponent;
				FreeJoint* component1 = (FreeJoint*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.connectedObjectGuid = component1->getConnectedObjectGuid();
				sComponent.anchor = component1->getAnchor();
				sComponent.connectedAnchor = component1->getConnectedAnchor();
				sComponent.linkedBodiesCollision = component1->getLinkedBodiesCollision();
				sComponent.limitMin = component1->getLimitMin();
				sComponent.limitMax = component1->getLimitMax();

				sObj.freeJoints.push_back(sComponent);
			}

			// HingeJoint
			if (component->getComponentType() == HingeJoint::COMPONENT_TYPE)
			{
				SHingeJoint sComponent;
				HingeJoint* component1 = (HingeJoint*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.connectedObjectGuid = component1->getConnectedObjectGuid();
				sComponent.anchor = component1->getAnchor();
				sComponent.connectedAnchor = component1->getConnectedAnchor();
				sComponent.linkedBodiesCollision = component1->getLinkedBodiesCollision();
				sComponent.limitMin = component1->getLimitMin();
				sComponent.limitMax = component1->getLimitMax();
				sComponent.axis = component1->getAxis();

				sObj.hingeJoints.push_back(sComponent);
			}

			// ConeTwistJoint
			if (component->getComponentType() == ConeTwistJoint::COMPONENT_TYPE)
			{
				SConeTwistJoint sComponent;
				ConeTwistJoint* component1 = (ConeTwistJoint*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.connectedObjectGuid = component1->getConnectedObjectGuid();
				sComponent.anchor = component1->getAnchor();
				sComponent.connectedAnchor = component1->getConnectedAnchor();
				sComponent.linkedBodiesCollision = component1->getLinkedBodiesCollision();
				sComponent.limits = component1->getLimits();

				sObj.coneTwistJoints.push_back(sComponent);
			}

			// AudioListener
			if (component->getComponentType() == AudioListener::COMPONENT_TYPE)
			{
				SAudioListener sComponent;
				AudioListener* component1 = (AudioListener*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.volume = component1->getVolume();

				sObj.audioListeners.push_back(sComponent);
			}

			// AudioSource
			if (component->getComponentType() == AudioSource::COMPONENT_TYPE)
			{
				SAudioSource sComponent;
				AudioSource* component1 = (AudioSource*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				AudioClip* audioClip = component1->getAudioClip();
				if (audioClip != nullptr && audioClip->isLoaded())
					sComponent.fileName = audioClip->getName();

				sComponent.volume = component1->getVolume();
				sComponent.is2D = component1->getIs2D();
				sComponent.loop = component1->getLooped();
				sComponent.minDistance = component1->getMinDistance();
				sComponent.maxDistance = component1->getMaxDistance();
				sComponent.playOnStart = component1->getPlayOnStart();
				sComponent.pitch = component1->getPitch();

				sObj.audioSources.push_back(sComponent);
			}

			// VideoPlayer
			if (component->getComponentType() == VideoPlayer::COMPONENT_TYPE)
			{
				SVideoPlayer sComponent;
				VideoPlayer* component1 = (VideoPlayer*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				VideoClip* videoClip = component1->getVideoClip();
				if (videoClip != nullptr && videoClip->isLoaded())
					sComponent.videoClip = videoClip->getName();

				sComponent.playOnStart = component1->getPlayOnStart();
				sComponent.loop = component1->getLoop();

				sObj.videoPlayers.push_back(sComponent);
			}

			// NavMeshAgent
			if (component->getComponentType() == NavMeshAgent::COMPONENT_TYPE)
			{
				SNavMeshAgent sComponent;
				NavMeshAgent* component1 = (NavMeshAgent*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.acceleration = component1->getAcceleration();
				sComponent.height = component1->getHeight();
				sComponent.radius = component1->getRadius();
				sComponent.rotationSpeed = component1->getRotationSpeed();
				sComponent.speed = component1->getSpeed();

				sObj.navMeshAgents.push_back(sComponent);
			}

			// NavMeshObstacle
			if (component->getComponentType() == NavMeshObstacle::COMPONENT_TYPE)
			{
				SNavMeshObstacle sComponent;
				NavMeshObstacle* component1 = (NavMeshObstacle*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.size = component1->getSize();
				sComponent.offset = component1->getOffset();

				sObj.navMeshObstacles.push_back(sComponent);
			}

			// Canvas
			if (component->getComponentType() == Canvas::COMPONENT_TYPE)
			{
				SCanvas sComponent;
				Canvas* component1 = (Canvas*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.refScreenWidth = component1->getRefScreenWidth();
				sComponent.refScreenHeight = component1->getRefScreenHeight();
				sComponent.screenMatchSide = component1->getScreenMatchSide();
				sComponent.mode = static_cast<int>(component1->getMode());

				sObj.canvases.push_back(sComponent);
			}

			// Image
			if (component->getComponentType() == Image::COMPONENT_TYPE)
			{
				SImage sComponent;
				Image* component1 = (Image*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.anchor = component1->getAnchor();
				sComponent.color = component1->getColor();
				sComponent.size = component1->getSize();
				sComponent.horizontalCanvasAlignment = static_cast<int>(component1->getHorizontalAlignment());
				sComponent.verticalCanvasAlignment = static_cast<int>(component1->getVerticalAlignment());
				sComponent.raycastTarget = component1->getRaycastTarget();
				if (component1->getTexture() != nullptr)
					sComponent.texturePath = component1->getTexture()->getName();
				sComponent.imageType = static_cast<int>(component1->getImageType());

				sObj.images.push_back(sComponent);
			}

			// Text
			if (component->getComponentType() == Text::COMPONENT_TYPE)
			{
				SText sComponent;
				Text* component1 = (Text*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.anchor = component1->getAnchor();
				sComponent.color = component1->getColor();
				sComponent.size = component1->getSize();
				sComponent.lineSpacing = component1->getLineSpacing();
				sComponent.horizontalCanvasAlignment = static_cast<int>(component1->getHorizontalAlignment());
				sComponent.verticalCanvasAlignment = static_cast<int>(component1->getVerticalAlignment());
				sComponent.raycastTarget = component1->getRaycastTarget();
				if (component1->getFont() != nullptr)
					sComponent.fontPath = component1->getFont()->getName();
				sComponent.fontSize = component1->getFontSize();
				sComponent.fontResolution = component1->getFontResolution();
				sComponent.text = component1->getText();
				sComponent.horizontalTextAlignment = static_cast<int>(component1->getHorizontalTextAlignment());
				sComponent.verticalTextAlignment = static_cast<int>(component1->getVerticalTextAlignment());
				sComponent.wordWrap = static_cast<int>(component1->getWordWrap());

				sObj.texts.push_back(sComponent);
			}

			// Button
			if (component->getComponentType() == Button::COMPONENT_TYPE)
			{
				SButton sComponent;
				Button* component1 = (Button*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.anchor = component1->getAnchor();
				sComponent.color = component1->getColor();
				sComponent.size = component1->getSize();
				sComponent.horizontalCanvasAlignment = static_cast<int>(component1->getHorizontalAlignment());
				sComponent.verticalCanvasAlignment = static_cast<int>(component1->getVerticalAlignment());
				sComponent.raycastTarget = component1->getRaycastTarget();
				if (component1->getTextureNormal() != nullptr)
					sComponent.texNormalPath = component1->getTextureNormal()->getName();
				if (component1->getTextureHover() != nullptr)
					sComponent.texHoverPath = component1->getTextureHover()->getName();
				if (component1->getTextureActive() != nullptr)
					sComponent.texActivePath = component1->getTextureActive()->getName();
				if (component1->getTextureDisabled() != nullptr)
					sComponent.texDisabledPath = component1->getTextureDisabled()->getName();
				sComponent.colHover = component1->getColorHover();
				sComponent.colActive = component1->getColorActive();
				sComponent.colDisabled = component1->getColorDisabled();
				sComponent.imageType = static_cast<int>(component1->getImageType());
				sComponent.interactable = component1->getInteractable();

				sObj.buttons.push_back(sComponent);
			}

			// Mask
			if (component->getComponentType() == Mask::COMPONENT_TYPE)
			{
				SMask sComponent;
				Mask* component1 = (Mask*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.anchor = component1->getAnchor();
				sComponent.color = component1->getColor();
				sComponent.size = component1->getSize();
				sComponent.horizontalCanvasAlignment = static_cast<int>(component1->getHorizontalAlignment());
				sComponent.verticalCanvasAlignment = static_cast<int>(component1->getVerticalAlignment());
				sComponent.raycastTarget = component1->getRaycastTarget();
				
				sObj.masks.push_back(sComponent);
			}

			// TextInput
			if (component->getComponentType() == TextInput::COMPONENT_TYPE)
			{
				STextInput sComponent;
				TextInput* component1 = (TextInput*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.anchor = component1->getAnchor();
				sComponent.color = component1->getColor();
				sComponent.size = component1->getSize();
				sComponent.horizontalCanvasAlignment = static_cast<int>(component1->getHorizontalAlignment());
				sComponent.verticalCanvasAlignment = static_cast<int>(component1->getVerticalAlignment());
				sComponent.raycastTarget = component1->getRaycastTarget();
				if (component1->getTextureNormal() != nullptr)
					sComponent.texNormalPath = component1->getTextureNormal()->getName();
				if (component1->getTextureHover() != nullptr)
					sComponent.texHoverPath = component1->getTextureHover()->getName();
				if (component1->getTextureActive() != nullptr)
					sComponent.texActivePath = component1->getTextureActive()->getName();
				if (component1->getTextureDisabled() != nullptr)
					sComponent.texDisabledPath = component1->getTextureDisabled()->getName();
				sComponent.colHover = component1->getColorHover();
				sComponent.colActive = component1->getColorActive();
				sComponent.colDisabled = component1->getColorDisabled();
				sComponent.colText = component1->getColorText();
				sComponent.colPlaceholder = component1->getColorPlaceholder();
				sComponent.imageType = static_cast<int>(component1->getImageType());
				sComponent.text = component1->getText();
				sComponent.placeholder = component1->getPlaceholder();
				sComponent.multiline = component1->getMultiline();
				sComponent.fontSize = component1->getFontSize();
				sComponent.fontResolution = component1->getFontResolution();
				if (component1->getFont() != nullptr)
					sComponent.fontPath = component1->getFont()->getName();
				sComponent.lineSpacing = component1->getLineSpacing();
				sComponent.padding = component1->getPadding();
				sComponent.horizontalTextAlignment = static_cast<int>(component1->getHorizontalTextAlignment());
				sComponent.verticalTextAlignment = static_cast<int>(component1->getVerticalTextAlignment());
				sComponent.wordWrap = static_cast<int>(component1->getWordWrap());
				sComponent.interactable = component1->getInteractable();

				sObj.textInputs.push_back(sComponent);
			}

			// MonoScript
			if (component->getComponentType() == MonoScript::COMPONENT_TYPE)
			{
				SMonoScript sComponent;
				MonoScript* component1 = (MonoScript*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.className = component1->getClassName();

				MonoScript::FieldList lst = component1->getFields();
				for (auto it = lst.begin(); it != lst.end(); ++it)
				{
					SMonoFieldInfo inf;
					inf.fieldName = it->fieldName;
					inf.fieldType = it->fieldType;
					inf.boolVal = it->fieldValue.boolVal;
					inf.floatVal = it->fieldValue.floatVal;
					inf.intVal = it->fieldValue.intVal;
					inf.stringVal = it->fieldValue.stringVal;
					inf.stringArrVal = it->fieldValue.stringArrVal;
					inf.objectVal = it->fieldValue.objectVal;
					inf.vec2Val = it->fieldValue.vec2Val;
					inf.vec3Val = it->fieldValue.vec3Val;
					inf.vec4Val = it->fieldValue.vec4Val;

					sComponent.fieldList.push_back(inf);
				}

				sObj.monoScripts.push_back(sComponent);
			}

			// Terrain
			if (component->getComponentType() == Terrain::COMPONENT_TYPE)
			{
				STerrain sComponent;
				Terrain* component1 = (Terrain*)component;
				component1->save();

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.filePath = component1->getFilePath();

				sObj.terrains.push_back(sComponent);
			}

			// ParticleSystem
			if (component->getComponentType() == ParticleSystem::COMPONENT_TYPE)
			{
				SParticleSystem sComponent;
				ParticleSystem* component1 = (ParticleSystem*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.castShadows = component1->getCastShadows();
				std::vector<ParticleEmitter*>& emitters = component1->getEmitters();
				for (auto em = emitters.begin(); em != emitters.end(); ++em)
				{
					ParticleEmitter* emitter = *em;
					SParticleEmitter sEmitter;
					Material* mat = emitter->getMaterial();
					if (mat != nullptr && mat->isLoaded())
						sEmitter.material = mat->getName();
					sEmitter.simulationSpace = static_cast<int>(emitter->getSimulationSpace());
					sEmitter.renderType = static_cast<int>(emitter->getRenderType());
					sEmitter.origin = static_cast<int>(emitter->getOrigin());
					sEmitter.shape = static_cast<int>(emitter->getShape());
					sEmitter.radius = emitter->getRadius();
					sEmitter.size = emitter->getSize();
					sEmitter.timeScale = emitter->getTimeScale();
					sEmitter.duration = emitter->getDuration();
					sEmitter.maxParticles = emitter->getMaxParticles();
					sEmitter.emissionCount = emitter->getEmissionCount();
					sEmitter.emissionRate = emitter->getEmissionRate();
					sEmitter.lifeTimeMin = emitter->getLifeTimeMin();
					sEmitter.lifeTimeMax = emitter->getLifeTimeMax();
					sEmitter.startSizeMin = emitter->getStartSizeMin();
					sEmitter.startSizeMax = emitter->getStartSizeMax();
					sEmitter.startDirection = emitter->getStartDirection();
					sEmitter.startSpeed = emitter->getStartSpeed();
					sEmitter.enablePhysics = emitter->getEnablePhysics();
					sEmitter.friction = emitter->getFriction();
					sEmitter.bounciness = emitter->getBounciness();
					sEmitter.animated = emitter->getAnimated();
					sEmitter.animationGridSize = emitter->getAnimationGridSize();
					sEmitter.animationTimeMode = static_cast<int>(emitter->getAnimationTimeMode());
					sEmitter.animationFps = emitter->getAnimationFps();
					sEmitter.loop = emitter->getLoop();

					std::vector<ParticleModifier*>& modifiers = emitter->getModifiers();
					for (auto mod = modifiers.begin(); mod != modifiers.end(); ++mod)
					{
						ParticleModifier* modifier = *mod;
						if (modifier->getType() == ParticleGravityModifier::MODIFIER_TYPE)
						{
							SParticleGravityModifier sModifier;
							ParticleGravityModifier* modifier1 = (ParticleGravityModifier*)modifier;
							sModifier.startTime = modifier->getStartTime();
							sModifier.gravity = modifier1->getGravity();
							sModifier.damping = modifier1->getDamping();

							sEmitter.gravityModifiers.push_back(sModifier);
						}

						if (modifier->getType() == ParticleColorModifier::MODIFIER_TYPE)
						{
							SParticleColorModifier sModifier;
							ParticleColorModifier* modifier1 = (ParticleColorModifier*)modifier;
							sModifier.startTime = modifier->getStartTime();
							auto& colors = modifier1->getColors();
							for (auto c = colors.begin(); c != colors.end(); ++c)
							{
								sModifier.times.push_back(c->first);
								sModifier.colors.push_back(c->second);
							}

							sEmitter.colorModifiers.push_back(sModifier);
						}

						if (modifier->getType() == ParticleSizeModifier::MODIFIER_TYPE)
						{
							SParticleSizeModifier sModifier;
							ParticleSizeModifier* modifier1 = (ParticleSizeModifier*)modifier;
							sModifier.startTime = modifier->getStartTime();
							auto& sizes = modifier1->getSizes();
							for (auto c = sizes.begin(); c != sizes.end(); ++c)
							{
								sModifier.times.push_back(c->first);
								sModifier.sizes.push_back(c->second);
							}

							sEmitter.sizeModifiers.push_back(sModifier);
						}

						if (modifier->getType() == ParticleDirectionModifier::MODIFIER_TYPE)
						{
							SParticleDirectionModifier sModifier;
							ParticleDirectionModifier* modifier1 = (ParticleDirectionModifier*)modifier;
							sModifier.startTime = modifier->getStartTime();
							sModifier.directionType = static_cast<int>(modifier1->getDirectionType());
							sModifier.changeInterval = modifier1->getChangeInterval();
							sModifier.constantDirection = modifier1->getConstantDirection();
							sModifier.randomDirectionMin = modifier1->getRandomDirectionMin();
							sModifier.randomDirectionMax = modifier1->getRandomDirectionMax();

							sEmitter.directionModifiers.push_back(sModifier);
						}

						if (modifier->getType() == ParticleRotationModifier::MODIFIER_TYPE)
						{
							SParticleRotationModifier sModifier;
							ParticleRotationModifier* modifier1 = (ParticleRotationModifier*)modifier;
							sModifier.startTime = modifier->getStartTime();
							sModifier.rotationType = static_cast<int>(modifier1->getRotationType());
							sModifier.changeInterval = modifier1->getChangeInterval();
							sModifier.constantRotation = modifier1->getConstantRotation();
							sModifier.randomRotationMin = modifier1->getRandomRotationMin();
							sModifier.randomRotationMax = modifier1->getRandomRotationMax();
							sModifier.offset = modifier1->getOffset();

							sEmitter.rotationModifiers.push_back(sModifier);
						}

						if (modifier->getType() == ParticleSpeedModifier::MODIFIER_TYPE)
						{
							SParticleSpeedModifier sModifier;
							ParticleSpeedModifier* modifier1 = (ParticleSpeedModifier*)modifier;
							sModifier.startTime = modifier->getStartTime();
							sModifier.speedType = static_cast<int>(modifier1->getSpeedType());
							sModifier.changeInterval = modifier1->getChangeInterval();
							sModifier.constantSpeed = modifier1->getConstantSpeed();
							sModifier.randomSpeedMin = modifier1->getRandomSpeedMin();
							sModifier.randomSpeedMax = modifier1->getRandomSpeedMax();

							sEmitter.speedModifiers.push_back(sModifier);
						}
					}

					sComponent.emitters.push_back(sEmitter);
				}

				sObj.particleSystems.push_back(sComponent);
			}

			// Water
			if (component->getComponentType() == Water::COMPONENT_TYPE)
			{
				SWater sComponent;
				Water* component1 = (Water*)component;
				
				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				Material* mat = component1->getMaterial();
				SMaterial* mt = mat->saveToMemory();

				sComponent.size = component1->getSize();
				sComponent.reflections = component1->getReflections();
				sComponent.reflectSkybox = component1->getReflectSkybox();
				sComponent.reflectObjects = component1->getReflectObjects();
				sComponent.reflectionsDistance = component1->getReflectionsDistance();
				sComponent.reflectionsQuality = component1->getReflectionsQuality();
				sComponent.material = *mt;

				sObj.waters.push_back(sComponent);
			}

			// Spline
			if (component->getComponentType() == Spline::COMPONENT_TYPE)
			{
				SSpline sComponent;
				Spline* component1 = (Spline*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				for (auto& pnt : component1->getPoints())
					sComponent.points.push_back(pnt);

				sComponent.closed = component1->getClosed();

				sObj.splines.push_back(sComponent);
			}

			// CSGBrush
			if (component->getComponentType() == CSGBrush::COMPONENT_TYPE)
			{
				SCSGBrush sComponent;
				CSGBrush* component1 = (CSGBrush*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sComponent.brushType = static_cast<int>(component1->getBrushType());
				sComponent.brushOperation = static_cast<int>(component1->getBrushOperation());
				sComponent.castShadows = component1->getCastShadows();

				auto& verts = component1->getVertices();

				for (auto& v : verts)
					sComponent.vertices.push_back(SVector3(v.x, v.y, v.z));

				auto& faces = component1->getFaces();

				for (int m = 0; m < faces.size(); ++m)
				{
					std::string mName = "";

					if (faces[m].material != nullptr && faces[m].material->isLoaded())
						mName = faces[m].material->getName();

					SFaceInfo face;
					face.material = mName;
					face.indices = faces[m].indices;

					for (auto& t : faces[m].texCoords)
						face.texCoords.push_back(SVector2(t.x, t.y));

					face.texCoordsScale = SVector2(faces[m].texCoordsScale);
					face.texCoordsOffset = SVector2(faces[m].texCoordsOffset);
					face.texCoordsRotation = faces[m].texCoordsRotation;
					face.smoothNormals = faces[m].smoothNormals;

					sComponent.faces.push_back(face);
				}

				sObj.csgBrushes.push_back(sComponent);
			}

			// CSGModel
			if (component->getComponentType() == CSGModel::COMPONENT_TYPE)
			{
				SCSGModel sComponent;
				CSGModel* component1 = (CSGModel*)component;

				sComponent.enabled = component1->getEnabled();
				sComponent.index = idx;

				sObj.csgModels.push_back(sComponent);
			}
		}

		scene->gameObjects.push_back(sObj);
	}

	void Scene::loadObject(SGameObject* sObj, GameObject* obj, std::function<void(float progress, std::string status)> progressCb)
	{
		std::string location = Engine::getSingleton()->getAssetsPath();
		std::string libraryPath = Engine::getSingleton()->getLibraryPath();
		std::string builtinPath = Engine::getSingleton()->getBuiltinResourcesPath();

		obj->setName(sObj->name);
		obj->setTag(sObj->tag);
		obj->setLayer(sObj->layer);
		obj->setNavigationStatic(sObj->navigationStatic);
		obj->setLightingStatic(sObj->lightingStatic);
		obj->setBatchingStatic(sObj->batchingStatic);
		obj->setOcclusionStatic(sObj->occlusionStatic);
		obj->getTransform()->setPosition(sObj->position.getValue());
		obj->getTransform()->setScale(sObj->scale.getValue());
		obj->getTransform()->setRotation(sObj->rotation.getValue());

		std::map<Component*, int> componentCache;

		// MeshRenderer
		for (auto ci = sObj->meshRenderers.begin(); ci != sObj->meshRenderers.end(); ++ci)
		{
			SMeshRenderer& sComponent = *ci;

			MeshRenderer* component = new MeshRenderer();
			component->setEnabled(sComponent.enabled);

			if (IO::isDir(libraryPath))
			{
				if (!IO::FileExists(libraryPath + sComponent.mesh))
				{
					if (IO::FileExists(location + sComponent.meshSourceFile))
						Model3DLoader::cache3DModel(location, sComponent.meshSourceFile, progressCb);
					else if (IO::FileExists(builtinPath + sComponent.meshSourceFile))
						Model3DLoader::cache3DModel(builtinPath, sComponent.meshSourceFile, progressCb);
				}
				else
				{
					if (!IO::FileExists(location + sComponent.meshSourceFile) &&
						!IO::FileExists(builtinPath + sComponent.meshSourceFile))
					{
						IO::FileDelete(libraryPath + sComponent.mesh);
					}
				}
			}

			Mesh* mesh = Mesh::load(libraryPath, sComponent.mesh);

			component->setMesh(mesh, false);
			component->setRootObjectGuid(sComponent.rootObjectGuid);
			component->setCastShadows(sComponent.castShadows);
			component->setLightmapSize(sComponent.lightmapSize);
			component->setLodMaxDistance(sComponent.lodMaxDistance);
			component->setCullOverMaxDistance(sComponent.cullOverMaxDistance);

			for (auto mi = sComponent.materials.begin(); mi != sComponent.materials.end(); ++mi)
			{
				Material* material = Material::load(Engine::getSingleton()->getBuiltinResourcesPath(), *mi);
				if (material == nullptr)
				{
					material = Material::load(location, *mi);
					if (material == nullptr)
					{
						if (!IO::FileExists(builtinPath + sComponent.meshSourceFile))
						{
							material = Material::create(location, *mi);
							material->load();
							material->save();
						}
						else
						{
							material = Renderer::getDefaultStandardMaterial();
						}
					}
				}
				component->addSharedMaterial(material);
			}

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		//DecalRenderer
		for (auto ci = sObj->decals.begin(); ci != sObj->decals.end(); ++ci)
		{
			SDecalRenderer& sComponent = *ci;

			DecalRenderer* component = new DecalRenderer();
			component->setEnabled(sComponent.enabled);

			Material* material = Material::load(location, sComponent.material);
			component->setMaterial(material);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// Light
		for (auto ci = sObj->lights.begin(); ci != sObj->lights.end(); ++ci)
		{
			SLight& sComponent = *ci;

			Light* component = new Light();
			component->setEnabled(sComponent.enabled);
			component->setColor(sComponent.color.getValue());
			component->setIntensity(sComponent.intensity);
			component->setLightType(static_cast<LightType>(sComponent.lightType));
			component->setLightRenderMode(static_cast<LightRenderMode>(sComponent.lightRenderMode));
			component->setRadius(sComponent.radius);
			component->setInnerRadius(sComponent.innerRadius);
			component->setOuterRadius(sComponent.outerRadius);
			component->setBias(sComponent.bias);
			component->setCastShadows(sComponent.castShadows);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// Camera
		for (auto ci = sObj->cameras.begin(); ci != sObj->cameras.end(); ++ci)
		{
			SCamera& sComponent = *ci;

			Camera* component = new Camera();
			component->setEnabled(sComponent.enabled);
			component->setClearColor(sComponent.clearColor.getValue());
			component->setDepth(sComponent.depth);
			component->setFOVy(sComponent.fovy);
			component->setNear(sComponent.znear);
			component->setFar(sComponent.zfar);
			component->setViewportLeft(sComponent.viewport_l);
			component->setViewportTop(sComponent.viewport_t);
			component->setViewportWidth(sComponent.viewport_w);
			component->setViewportHeight(sComponent.viewport_h);
			component->setClearFlags(static_cast<Camera::ClearFlags>(sComponent.clearFlags));
			for (int i = 0; i < LayerMask::MAX_LAYERS; ++i)
			{
				if (i < sComponent.cullingMask.size())
					component->getCullingMask().setLayer(i, sComponent.cullingMask[i]);
			}
			component->setProjectionType(static_cast<ProjectionType>(sComponent.projectionType));
			component->setOrthographicSize(sComponent.orthographicSize);
			component->setOcclusionCulling(sComponent.occlusionCulling);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// Animation
		for (auto ci = sObj->animations.begin(); ci != sObj->animations.end(); ++ci)
		{
			SAnimation& sComponent = *ci;

			Animation* component = new Animation();
			component->setEnabled(sComponent.enabled);

			for (auto at = sComponent.animationClips.begin(); at != sComponent.animationClips.end(); ++at)
			{
				SAnimationClipInfo& inf = *at;
				AnimationClipInfo ainf;

				if (!inf.clipPath.empty())
					ainf.clip = AnimationClip::load(location, inf.clipPath);
				ainf.name = inf.name;
				ainf.speed = inf.speed;
				ainf.startFrame = inf.startFrame;
				ainf.endFrame = inf.endFrame;
				ainf.loop = inf.loop;

				component->addAnimationClip(ainf);
			}

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// BoxCollider
		for (auto ci = sObj->boxColliders.begin(); ci != sObj->boxColliders.end(); ++ci)
		{
			SBoxCollider& sComponent = *ci;

			BoxCollider* component = new BoxCollider();
			component->setEnabled(sComponent.enabled);
			component->setBoxSize(sComponent.boxSize.getValue());
			component->setOffset(sComponent.offset.getValue());
			component->setRotation(sComponent.rotation.getValue());
			component->setIsTrigger(sComponent.isTrigger);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// CapsuleCollider
		for (auto ci = sObj->capsuleColliders.begin(); ci != sObj->capsuleColliders.end(); ++ci)
		{
			SCapsuleCollider& sComponent = *ci;

			CapsuleCollider* component = new CapsuleCollider();
			component->setEnabled(sComponent.enabled);
			component->setRadius(sComponent.radius);
			component->setHeight(sComponent.height);
			component->setOffset(sComponent.offset.getValue());
			component->setRotation(sComponent.rotation.getValue());
			component->setIsTrigger(sComponent.isTrigger);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// SphereCollider
		for (auto ci = sObj->sphereColliders.begin(); ci != sObj->sphereColliders.end(); ++ci)
		{
			SSphereCollider& sComponent = *ci;

			SphereCollider* component = new SphereCollider();
			component->setEnabled(sComponent.enabled);
			component->setRadius(sComponent.radius);
			component->setOffset(sComponent.offset.getValue());
			component->setIsTrigger(sComponent.isTrigger);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// MeshCollider
		for (auto ci = sObj->meshColliders.begin(); ci != sObj->meshColliders.end(); ++ci)
		{
			SMeshCollider& sComponent = *ci;

			MeshCollider* component = new MeshCollider();
			component->setEnabled(sComponent.enabled);
			component->setConvex(sComponent.convex);
			component->setIsTrigger(sComponent.isTrigger);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// TerrainCollider
		for (auto ci = sObj->terrainColliders.begin(); ci != sObj->terrainColliders.end(); ++ci)
		{
			STerrainCollider& sComponent = *ci;

			TerrainCollider* component = new TerrainCollider();
			component->setEnabled(sComponent.enabled);
			component->setIsTrigger(sComponent.isTrigger);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// Vehicle
		for (auto ci = sObj->vehicles.begin(); ci != sObj->vehicles.end(); ++ci)
		{
			SVehicle& sComponent = *ci;

			Vehicle* component = new Vehicle();
			component->setEnabled(sComponent.enabled);
			component->setAxis(sComponent.vehicleAxis.getValue());
			component->setInvertForward(sComponent.invertForward);

			std::vector<Vehicle::WheelInfo>& wheels = component->getWheels();

			for (auto _it = sComponent.wheels.begin(); _it != sComponent.wheels.end(); ++_it)
			{
				Vehicle::WheelInfo inf;
				inf.m_connectedObjectGuid = _it->m_connectedObjectGuid;
				inf.m_axle = _it->m_axle.getValue();
				inf.m_connectionPoint = _it->m_connectionPoint.getValue();
				inf.m_direction = _it->m_direction.getValue();
				inf.m_friction = _it->m_friction;
				inf.m_isFrontWheel = _it->m_isFrontWheel;
				inf.m_radius = _it->m_radius;
				inf.m_rollInfluence = _it->m_rollInfluence;
				inf.m_suspensionCompression = _it->m_suspensionCompression;
				inf.m_suspensionDamping = _it->m_suspensionDamping;
				inf.m_suspensionRestLength = _it->m_suspensionRestLength;
				inf.m_suspensionStiffness = _it->m_suspensionStiffness;
				inf.m_width = _it->m_width;

				wheels.push_back(inf);
			}

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// RigidBody
		for (auto ci = sObj->rigidBodies.begin(); ci != sObj->rigidBodies.end(); ++ci)
		{
			SRigidBody& sComponent = *ci;

			RigidBody* component = new RigidBody();
			component->setEnabled(sComponent.enabled);
			component->setMass(sComponent.mass);
			component->setFreezePositionX(sComponent.freezePositionX);
			component->setFreezePositionY(sComponent.freezePositionY);
			component->setFreezePositionZ(sComponent.freezePositionZ);
			component->setFreezeRotationX(sComponent.freezeRotationX);
			component->setFreezeRotationY(sComponent.freezeRotationY);
			component->setFreezeRotationZ(sComponent.freezeRotationZ);
			component->setIsKinematic(sComponent.isKinematic);
			component->setIsStatic(sComponent.isStatic);
			component->setUseOwnGravity(sComponent.useOwnGravity);
			component->setGravity(sComponent.gravity.getValue());
			component->setFriction(sComponent.friction);
			component->setBounciness(sComponent.bounciness);
			component->setLinearDamping(sComponent.linearDamping);
			component->setAngularDamping(sComponent.angularDamping);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// FixedJoint
		for (auto ci = sObj->fixedJoints.begin(); ci != sObj->fixedJoints.end(); ++ci)
		{
			SFixedJoint& sComponent = *ci;

			FixedJoint* component = new FixedJoint();
			component->setEnabled(sComponent.enabled);
			component->setConnectedObjectGuid(sComponent.connectedObjectGuid);
			component->setAnchor(sComponent.anchor.getValue());
			component->setConnectedAnchor(sComponent.connectedAnchor.getValue());
			component->setLinkedBodiesCollision(sComponent.linkedBodiesCollision);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// FreeJoint
		for (auto ci = sObj->freeJoints.begin(); ci != sObj->freeJoints.end(); ++ci)
		{
			SFreeJoint& sComponent = *ci;

			FreeJoint* component = new FreeJoint();
			component->setEnabled(sComponent.enabled);
			component->setConnectedObjectGuid(sComponent.connectedObjectGuid);
			component->setAnchor(sComponent.anchor.getValue());
			component->setConnectedAnchor(sComponent.connectedAnchor.getValue());
			component->setLinkedBodiesCollision(sComponent.linkedBodiesCollision);
			component->setLimitMin(sComponent.limitMin.getValue());
			component->setLimitMax(sComponent.limitMax.getValue());

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// HingeJoint
		for (auto ci = sObj->hingeJoints.begin(); ci != sObj->hingeJoints.end(); ++ci)
		{
			SHingeJoint& sComponent = *ci;

			HingeJoint* component = new HingeJoint();
			component->setEnabled(sComponent.enabled);
			component->setConnectedObjectGuid(sComponent.connectedObjectGuid);
			component->setAnchor(sComponent.anchor.getValue());
			component->setConnectedAnchor(sComponent.connectedAnchor.getValue());
			component->setLinkedBodiesCollision(sComponent.linkedBodiesCollision);
			component->setLimitMin(sComponent.limitMin);
			component->setLimitMax(sComponent.limitMax);
			component->setAxis(sComponent.axis.getValue());

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// ConeTwistJoint
		for (auto ci = sObj->coneTwistJoints.begin(); ci != sObj->coneTwistJoints.end(); ++ci)
		{
			SConeTwistJoint& sComponent = *ci;

			ConeTwistJoint* component = new ConeTwistJoint();
			component->setEnabled(sComponent.enabled);
			component->setConnectedObjectGuid(sComponent.connectedObjectGuid);
			component->setAnchor(sComponent.anchor.getValue());
			component->setConnectedAnchor(sComponent.connectedAnchor.getValue());
			component->setLinkedBodiesCollision(sComponent.linkedBodiesCollision);
			component->setLimits(sComponent.limits.getValue());

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// AudioListener
		for (auto ci = sObj->audioListeners.begin(); ci != sObj->audioListeners.end(); ++ci)
		{
			SAudioListener& sComponent = *ci;

			AudioListener* component = new AudioListener();
			component->setEnabled(sComponent.enabled);
			component->setVolume(sComponent.volume);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// AudioSource
		for (auto ci = sObj->audioSources.begin(); ci != sObj->audioSources.end(); ++ci)
		{
			SAudioSource& sComponent = *ci;

			AudioClip* audioClip = AudioClip::load(Engine::getSingleton()->getAssetsPath(), sComponent.fileName, false);

			AudioSource* component = new AudioSource();
			component->setEnabled(sComponent.enabled);
			component->setAudioClip(audioClip);
			component->setVolume(sComponent.volume);
			component->setIs2D(sComponent.is2D);
			component->setLoop(sComponent.loop);
			component->setMinDistance(sComponent.minDistance);
			component->setMaxDistance(sComponent.maxDistance);
			component->setPlayOnStart(sComponent.playOnStart);
			component->setPitch(sComponent.pitch);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// VideoPlayer
		for (auto ci = sObj->videoPlayers.begin(); ci != sObj->videoPlayers.end(); ++ci)
		{
			SVideoPlayer& sComponent = *ci;

			VideoClip* videoClip = VideoClip::load(Engine::getSingleton()->getAssetsPath(), sComponent.videoClip, false);

			VideoPlayer* component = new VideoPlayer();
			component->setEnabled(sComponent.enabled);
			component->setVideoClip(videoClip);
			component->setPlayOnStart(sComponent.playOnStart);
			component->setLoop(sComponent.loop);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// NavMeshAgent
		for (auto ci = sObj->navMeshAgents.begin(); ci != sObj->navMeshAgents.end(); ++ci)
		{
			SNavMeshAgent& sComponent = *ci;

			NavMeshAgent* component = new NavMeshAgent();
			component->setEnabled(sComponent.enabled);
			component->setAcceleration(sComponent.acceleration);
			component->setHeight(sComponent.height);
			component->setRadius(sComponent.radius);
			component->setRotationSpeed(sComponent.rotationSpeed);
			component->setSpeed(sComponent.speed);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// NavMeshObstacle
		for (auto ci = sObj->navMeshObstacles.begin(); ci != sObj->navMeshObstacles.end(); ++ci)
		{
			SNavMeshObstacle& sComponent = *ci;

			NavMeshObstacle* component = new NavMeshObstacle();
			component->setEnabled(sComponent.enabled);
			component->setSize(sComponent.size.getValue());
			component->setOffset(sComponent.offset.getValue());

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// Canvas
		for (auto ci = sObj->canvases.begin(); ci != sObj->canvases.end(); ++ci)
		{
			SCanvas& sComponent = *ci;

			Canvas* component = new Canvas();
			component->setEnabled(sComponent.enabled);
			component->setRefScreenSize(glm::vec2(sComponent.refScreenWidth, sComponent.refScreenHeight));
			component->setScreenMatchSide(sComponent.screenMatchSide);
			component->setMode(static_cast<CanvasMode>(sComponent.mode));

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// Image
		for (auto ci = sObj->images.begin(); ci != sObj->images.end(); ++ci)
		{
			SImage& sComponent = *ci;

			Image* component = new Image();
			component->setEnabled(sComponent.enabled);
			component->setAnchor(sComponent.anchor.getValue());
			component->setColor(sComponent.color.getValue());
			component->setSize(sComponent.size.getValue());
			component->setHorizontalAlignment(static_cast<CanvasHorizontalAlignment>(sComponent.horizontalCanvasAlignment));
			component->setVerticalAlignment(static_cast<CanvasVerticalAlignment>(sComponent.verticalCanvasAlignment));
			component->setRaycastTarget(sComponent.raycastTarget);
			if (!sComponent.texturePath.empty())
				component->setTexture(Texture::load(location, sComponent.texturePath, false, Texture::CompressionMethod::None));
			component->setImageType(static_cast<Image::ImageType>(sComponent.imageType));

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// Text
		for (auto ci = sObj->texts.begin(); ci != sObj->texts.end(); ++ci)
		{
			SText& sComponent = *ci;

			Text* component = new Text();
			component->setEnabled(sComponent.enabled);
			component->setAnchor(sComponent.anchor.getValue());
			component->setColor(sComponent.color.getValue());
			component->setSize(sComponent.size.getValue());
			component->setLineSpacing(sComponent.lineSpacing);
			component->setHorizontalAlignment(static_cast<CanvasHorizontalAlignment>(sComponent.horizontalCanvasAlignment));
			component->setVerticalAlignment(static_cast<CanvasVerticalAlignment>(sComponent.verticalCanvasAlignment));
			component->setRaycastTarget(sComponent.raycastTarget);
			if (!sComponent.fontPath.empty())
				component->setFont(Font::load(location, sComponent.fontPath));
			component->setFontSize(sComponent.fontSize);
			component->setFontResolution(sComponent.fontResolution);
			component->setText(sComponent.text);
			component->setHorizontalTextAlignment(static_cast<Text::TextHorizontalAlignment>(sComponent.horizontalTextAlignment));
			component->setVerticalTextAlignment(static_cast<Text::TextVerticalAlignment>(sComponent.verticalTextAlignment));
			component->setWordWrap(static_cast<Text::TextWordWrap>(sComponent.wordWrap));

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// Button
		for (auto ci = sObj->buttons.begin(); ci != sObj->buttons.end(); ++ci)
		{
			SButton& sComponent = *ci;

			Button* component = new Button();
			component->setEnabled(sComponent.enabled);
			component->setAnchor(sComponent.anchor.getValue());
			component->setColor(sComponent.color.getValue());
			component->setSize(sComponent.size.getValue());
			component->setHorizontalAlignment(static_cast<CanvasHorizontalAlignment>(sComponent.horizontalCanvasAlignment));
			component->setVerticalAlignment(static_cast<CanvasVerticalAlignment>(sComponent.verticalCanvasAlignment));
			component->setRaycastTarget(sComponent.raycastTarget);
			if (!sComponent.texNormalPath.empty())
				component->setTextureNormal(Texture::load(location, sComponent.texNormalPath, false, Texture::CompressionMethod::None));
			if (!sComponent.texHoverPath.empty())
				component->setTextureHover(Texture::load(location, sComponent.texHoverPath, false, Texture::CompressionMethod::None));
			if (!sComponent.texActivePath.empty())
				component->setTextureActive(Texture::load(location, sComponent.texActivePath, false, Texture::CompressionMethod::None));
			if (!sComponent.texDisabledPath.empty())
				component->setTextureDisabled(Texture::load(location, sComponent.texDisabledPath, false, Texture::CompressionMethod::None));
			component->setColorHover(sComponent.colHover.getValue());
			component->setColorActive(sComponent.colActive.getValue());
			component->setColorDisabled(sComponent.colDisabled.getValue());
			component->setImageType(static_cast<Button::ImageType>(sComponent.imageType));
			component->setInteractable(sComponent.interactable);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// Mask
		for (auto ci = sObj->masks.begin(); ci != sObj->masks.end(); ++ci)
		{
			SMask& sComponent = *ci;

			Mask* component = new Mask();
			component->setEnabled(sComponent.enabled);
			component->setAnchor(sComponent.anchor.getValue());
			component->setColor(sComponent.color.getValue());
			component->setSize(sComponent.size.getValue());
			component->setHorizontalAlignment(static_cast<CanvasHorizontalAlignment>(sComponent.horizontalCanvasAlignment));
			component->setVerticalAlignment(static_cast<CanvasVerticalAlignment>(sComponent.verticalCanvasAlignment));
			component->setRaycastTarget(sComponent.raycastTarget);
			
			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// TextInput
		for (auto ci = sObj->textInputs.begin(); ci != sObj->textInputs.end(); ++ci)
		{
			STextInput& sComponent = *ci;

			TextInput* component = new TextInput();
			component->setEnabled(sComponent.enabled);
			component->setAnchor(sComponent.anchor.getValue());
			component->setColor(sComponent.color.getValue());
			component->setSize(sComponent.size.getValue());
			component->setHorizontalAlignment(static_cast<CanvasHorizontalAlignment>(sComponent.horizontalCanvasAlignment));
			component->setVerticalAlignment(static_cast<CanvasVerticalAlignment>(sComponent.verticalCanvasAlignment));
			component->setRaycastTarget(sComponent.raycastTarget);
			if (!sComponent.texNormalPath.empty())
				component->setTextureNormal(Texture::load(location, sComponent.texNormalPath, false, Texture::CompressionMethod::None));
			if (!sComponent.texHoverPath.empty())
				component->setTextureHover(Texture::load(location, sComponent.texHoverPath, false, Texture::CompressionMethod::None));
			if (!sComponent.texActivePath.empty())
				component->setTextureActive(Texture::load(location, sComponent.texActivePath, false, Texture::CompressionMethod::None));
			if (!sComponent.texDisabledPath.empty())
				component->setTextureDisabled(Texture::load(location, sComponent.texDisabledPath, false, Texture::CompressionMethod::None));
			component->setColorHover(sComponent.colHover.getValue());
			component->setColorActive(sComponent.colActive.getValue());
			component->setColorDisabled(sComponent.colDisabled.getValue());
			component->setColorText(sComponent.colText.getValue());
			component->setColorPlaceholder(sComponent.colPlaceholder.getValue());
			component->setImageType(static_cast<TextInput::ImageType>(sComponent.imageType));
			component->setText(sComponent.text);
			component->setPlaceholder(sComponent.placeholder);
			component->setMultiline(sComponent.multiline);
			component->setFontSize(sComponent.fontSize);
			component->setFontResolution(sComponent.fontResolution);
			if (!sComponent.fontPath.empty())
				component->setFont(Font::load(location, sComponent.fontPath));
			component->setLineSpacing(sComponent.lineSpacing);
			component->setPadding(sComponent.padding.getValue());
			component->setHorizontalTextAlignment(static_cast<TextInput::TextHorizontalAlignment>(sComponent.horizontalTextAlignment));
			component->setVerticalTextAlignment(static_cast<TextInput::TextVerticalAlignment>(sComponent.verticalTextAlignment));
			component->setWordWrap(static_cast<TextInput::TextWordWrap>(sComponent.wordWrap));
			component->setInteractable(sComponent.interactable);

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// MonoScript
		for (auto ci = sObj->monoScripts.begin(); ci != sObj->monoScripts.end(); ++ci)
		{
			SMonoScript& sComponent = *ci;

			MonoScript* component = new MonoScript();
			component->setEnabled(sComponent.enabled);
			component->setClassFromName(sComponent.className);

			for (auto cf = sComponent.fieldList.begin(); cf != sComponent.fieldList.end(); ++cf)
			{
				MonoScript::MonoFieldInfo mfinf;
				mfinf.fieldName = cf->fieldName;
				mfinf.fieldType = cf->fieldType;
				MonoScript::ValueVariant mfval;
				mfval.boolVal = cf->boolVal;
				mfval.floatVal = cf->floatVal;
				mfval.intVal = cf->intVal;
				mfval.stringVal = cf->stringVal;
				mfval.stringArrVal = cf->stringArrVal;
				mfval.objectVal = cf->objectVal;
				mfval.vec2Val = cf->vec2Val.getValue();
				mfval.vec3Val = cf->vec3Val.getValue();
				mfval.vec4Val = cf->vec4Val.getValue();
				mfinf.fieldValue = mfval;

				component->addSerializedField(mfinf);
			}

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// Terrain
		for (auto ci = sObj->terrains.begin(); ci != sObj->terrains.end(); ++ci)
		{
			STerrain& sComponent = *ci;

			Terrain* component = new Terrain();
			component->setEnabled(sComponent.enabled);

			obj->addComponent(component);
			component->load(sComponent.filePath);

			componentCache[component] = sComponent.index;
		}

		// ParticleSystem
		for (auto ci = sObj->particleSystems.begin(); ci != sObj->particleSystems.end(); ++ci)
		{
			SParticleSystem& sComponent = *ci;

			ParticleSystem* component = new ParticleSystem();
			component->setEnabled(sComponent.enabled);

			component->setCastShadows(sComponent.castShadows);
			component->setPlayAtStart(sComponent.playAtStart);
			std::vector<SParticleEmitter>& emitters = sComponent.emitters;
			for (auto em = emitters.begin(); em != emitters.end(); ++em)
			{
				SParticleEmitter& sEmitter = *em;
				ParticleEmitter* emitter = component->addEmitter();
				Material* mat = Material::load(Engine::getSingleton()->getAssetsPath(), sEmitter.material);
				emitter->setMaterial(mat);
				emitter->setSimulationSpace(static_cast<ParticleEmitter::SimulationSpace>(sEmitter.simulationSpace));
				emitter->setRenderType(static_cast<ParticleEmitter::RenderType>(sEmitter.renderType));
				emitter->setOrigin(static_cast<ParticleEmitter::Origin>(sEmitter.origin));
				emitter->setShape(static_cast<ParticleEmitterShape>(sEmitter.shape));
				emitter->setRadius(sEmitter.radius);
				emitter->setSize(sEmitter.size.getValue());
				emitter->setTimeScale(sEmitter.timeScale);
				emitter->setDuration(sEmitter.duration);
				emitter->setMaxParticles(sEmitter.maxParticles);
				emitter->setEmissionCount(sEmitter.emissionCount);
				emitter->setEmissionRate(sEmitter.emissionRate);
				emitter->setLifeTimeMin(sEmitter.lifeTimeMin);
				emitter->setLifeTimeMax(sEmitter.lifeTimeMax);
				emitter->setStartSizeMin(sEmitter.startSizeMin);
				emitter->setStartSizeMax(sEmitter.startSizeMax);
				emitter->setStartDirection(sEmitter.startDirection.getValue());
				emitter->setStartSpeed(sEmitter.startSpeed);
				emitter->setEnablePhysics(sEmitter.enablePhysics);
				emitter->setFriction(sEmitter.friction);
				emitter->setBounciness(sEmitter.bounciness);
				emitter->setAnimated(sEmitter.animated);
				emitter->setAnimationGridSize(sEmitter.animationGridSize.getValue());
				emitter->setAnimationTimeMode(static_cast<ParticleEmitter::AnimationTimeMode>(sEmitter.animationTimeMode));
				emitter->setAnimationFps(sEmitter.animationFps);
				emitter->setLoop(sEmitter.loop);

				for (auto mod = sEmitter.gravityModifiers.begin(); mod != sEmitter.gravityModifiers.end(); ++mod)
				{
					SParticleGravityModifier& sModifier = *mod;
					ParticleGravityModifier* modifier = emitter->addModifier<ParticleGravityModifier*>();
					modifier->setStartTime(sModifier.startTime);
					modifier->setGravity(sModifier.gravity.getValue());
					modifier->setDamping(sModifier.damping);
				}

				for (auto mod = sEmitter.colorModifiers.begin(); mod != sEmitter.colorModifiers.end(); ++mod)
				{
					SParticleColorModifier& sModifier = *mod;
					ParticleColorModifier* modifier = emitter->addModifier<ParticleColorModifier*>();
					modifier->setStartTime(sModifier.startTime);
					for (int c = 0; c != sModifier.times.size(); ++c)
					{
						modifier->getColors().push_back(std::make_pair(sModifier.times[c], sModifier.colors[c].getValue()));
					}
				}

				for (auto mod = sEmitter.sizeModifiers.begin(); mod != sEmitter.sizeModifiers.end(); ++mod)
				{
					SParticleSizeModifier& sModifier = *mod;
					ParticleSizeModifier* modifier = emitter->addModifier<ParticleSizeModifier*>();
					modifier->setStartTime(sModifier.startTime);
					for (int c = 0; c != sModifier.times.size(); ++c)
					{
						modifier->getSizes().push_back(std::make_pair(sModifier.times[c], sModifier.sizes[c]));
					}
				}

				for (auto mod = sEmitter.directionModifiers.begin(); mod != sEmitter.directionModifiers.end(); ++mod)
				{
					SParticleDirectionModifier& sModifier = *mod;
					ParticleDirectionModifier* modifier = emitter->addModifier<ParticleDirectionModifier*>();
					modifier->setStartTime(sModifier.startTime);
					modifier->setDirectionType(static_cast<ParticleDirectionModifier::DirectionType>(sModifier.directionType));
					modifier->setChangeInterval(sModifier.changeInterval);
					modifier->setConstantDirection(sModifier.constantDirection.getValue());
					modifier->setRandomDirectionMin(sModifier.randomDirectionMin.getValue());
					modifier->setRandomDirectionMax(sModifier.randomDirectionMax.getValue());
				}

				for (auto mod = sEmitter.rotationModifiers.begin(); mod != sEmitter.rotationModifiers.end(); ++mod)
				{
					SParticleRotationModifier& sModifier = *mod;
					ParticleRotationModifier* modifier = emitter->addModifier<ParticleRotationModifier*>();
					modifier->setStartTime(sModifier.startTime);
					modifier->setRotationType(static_cast<ParticleRotationModifier::RotationType>(sModifier.rotationType));
					modifier->setChangeInterval(sModifier.changeInterval);
					modifier->setConstantRotation(sModifier.constantRotation.getValue());
					modifier->setRandomRotationMin(sModifier.randomRotationMin.getValue());
					modifier->setRandomRotationMax(sModifier.randomRotationMax.getValue());
					modifier->setOffset(sModifier.offset.getValue());
				}

				for (auto mod = sEmitter.speedModifiers.begin(); mod != sEmitter.speedModifiers.end(); ++mod)
				{
					SParticleSpeedModifier& sModifier = *mod;
					ParticleSpeedModifier* modifier = emitter->addModifier<ParticleSpeedModifier*>();
					modifier->setStartTime(sModifier.startTime);
					modifier->setSpeedType(static_cast<ParticleSpeedModifier::SpeedType>(sModifier.speedType));
					modifier->setChangeInterval(sModifier.changeInterval);
					modifier->setConstantSpeed(sModifier.constantSpeed);
					modifier->setRandomSpeedMin(sModifier.randomSpeedMin);
					modifier->setRandomSpeedMax(sModifier.randomSpeedMax);
				}
			}

			obj->addComponent(component);
			componentCache[component] = sComponent.index;
		}

		// Water
		for (auto ci = sObj->waters.begin(); ci != sObj->waters.end(); ++ci)
		{
			SWater& sComponent = *ci;

			Water* component = new Water();
			component->setEnabled(sComponent.enabled);
			
			Material* mat = component->getMaterial();
			mat->load(&sComponent.material);

			obj->addComponent(component);
			component->setSize(sComponent.size);
			component->setReflections(sComponent.reflections);
			component->setReflectSkybox(sComponent.reflectSkybox);
			component->setReflectObjects(sComponent.reflectObjects);
			component->setReflectionsDistance(sComponent.reflectionsDistance);
			component->setReflectionsQuality(sComponent.reflectionsQuality);

			componentCache[component] = sComponent.index;
		}

		// Spline
		for (auto ci = sObj->splines.begin(); ci != sObj->splines.end(); ++ci)
		{
			SSpline& sComponent = *ci;

			Spline* component = new Spline();
			component->setEnabled(sComponent.enabled);

			for (auto& pnt : sComponent.points)
				component->getPoints().push_back(pnt.getValue());

			component->setClosed(sComponent.closed);

			obj->addComponent(component);

			componentCache[component] = sComponent.index;
		}

		// CSGBrush
		for (auto ci = sObj->csgBrushes.begin(); ci != sObj->csgBrushes.end(); ++ci)
		{
			SCSGBrush& sComponent = *ci;

			CSGBrush* component = new CSGBrush();
			component->setEnabled(sComponent.enabled);

			component->setBrushType(static_cast<CSGBrush::BrushType>(sComponent.brushType));
			component->setBrushOperation(static_cast<CSGBrush::BrushOperation>(sComponent.brushOperation));
			component->setCastShadows(sComponent.castShadows);

			auto& verts = component->getVertices();
			auto& faces = component->getFaces();

			verts.clear();
			faces.clear();

			for (auto& v : sComponent.vertices)
				verts.push_back(glm::vec3(v.x, v.y, v.z));

			for (auto& sface : sComponent.faces)
			{
				CSGBrush::FaceInfo faceInfo;

				if (!sface.material.empty())
				{
					Material* mt = Material::load(Engine::getSingleton()->getAssetsPath(), sface.material);
					faceInfo.material = mt;
				}

				faceInfo.indices = sface.indices;

				for (auto& t : sface.texCoords)
					faceInfo.texCoords.push_back(t.getValue());

				faceInfo.texCoordsScale = sface.texCoordsScale.getValue();
				faceInfo.texCoordsOffset = sface.texCoordsOffset.getValue();
				faceInfo.texCoordsRotation = sface.texCoordsRotation;
				faceInfo.smoothNormals = sface.smoothNormals;

				faces.push_back(faceInfo);
			}

			obj->addComponent(component);

			componentCache[component] = sComponent.index;
		}

		// CSGModel
		for (auto ci = sObj->csgModels.begin(); ci != sObj->csgModels.end(); ++ci)
		{
			SCSGModel& sComponent = *ci;

			CSGModel* component = new CSGModel();
			component->setEnabled(sComponent.enabled);

			obj->addComponent(component);

			componentCache[component] = sComponent.index;
		}

		//Sort components
		std::vector<Component*>& components = obj->getComponents();
		std::sort(components.begin(), components.end(), [=](Component*& comp1, Component*& comp2) -> bool
			{
				std::map<Component*, int> m = *const_cast<std::map<Component*, int>*>(&componentCache);
				int idx1 = m[comp1];
				int idx2 = m[comp2];
				return idx1 < idx2;
			}
		);

		componentCache.clear();
	}

	void Scene::load(std::string location, std::string name, SScene* scene, std::function<void(float progress, std::string objectName)> progressCb)
	{
		NavigationManager::getSingleton()->setLoadedScene(name);

		Renderer::getSingleton()->setAmbientColor(scene->ambientColor.getValue());
		Renderer::getSingleton()->setSkyModel(static_cast<SkyModel>(scene->skyModel));
		if (!scene->skyboxMaterial.empty())
		{
			Material* skyboxMaterial = Material::load(Engine::getSingleton()->getAssetsPath(), scene->skyboxMaterial);
			Renderer::getSingleton()->setSkyMaterial(skyboxMaterial);
		}

		Renderer::getSingleton()->setGIEnabled(scene->giEnabled);
		Renderer::getSingleton()->setGIIntensity(scene->giIntensity);

		Renderer::getSingleton()->setFogEnabled(scene->fogEnabled);
		Renderer::getSingleton()->setFogStartDistance(scene->fogStartDistance);
		Renderer::getSingleton()->setFogEndDistance(scene->fogEndDistance);
		Renderer::getSingleton()->setFogDensity(scene->fogDensity);
		Renderer::getSingleton()->setFogColor(scene->fogColor.getValue());
		Renderer::getSingleton()->setFogIncludeSkybox(scene->fogIncludeSkybox);
		Renderer::getSingleton()->setFogType(scene->fogType);

		NavigationManager::getSingleton()->setCellHeight(scene->navMeshSettings.cellHeight);
		NavigationManager::getSingleton()->setCellSize(scene->navMeshSettings.cellSize);
		NavigationManager::getSingleton()->setMaxEdgeLen(scene->navMeshSettings.maxEdgeLen);
		NavigationManager::getSingleton()->setMaxSimplificationError(scene->navMeshSettings.maxSimplificationError);
		NavigationManager::getSingleton()->setMergeRegionArea(scene->navMeshSettings.mergeRegionArea);
		NavigationManager::getSingleton()->setMinRegionArea(scene->navMeshSettings.minRegionArea);
		NavigationManager::getSingleton()->setWalkableClimb(scene->navMeshSettings.walkableClimb);
		NavigationManager::getSingleton()->setWalkableHeight(scene->navMeshSettings.walkableHeight);
		NavigationManager::getSingleton()->setWalkableRadius(scene->navMeshSettings.walkableRadius);
		NavigationManager::getSingleton()->setWalkableSlopeAngle(scene->navMeshSettings.walkableSlopeAngle);

		std::vector<std::pair<GameObject*, SGameObject>> objectCache;
		std::vector<GameObject*> objects;

		int iter = 0;
		for (auto it = scene->gameObjects.begin(); it != scene->gameObjects.end(); ++it, ++iter)
		{
			SGameObject& sObj = *it;
			GameObject* obj = Engine::getSingleton()->createGameObject(sObj.guid);

			objectCache.push_back(std::make_pair(obj, sObj));

			if (progressCb != nullptr)
			{
				int progressEvery = (float)scene->gameObjects.size() * (5.0f / 100.0f);
				if (progressEvery > 0)
				{
					int progress = 100.0f / (float)scene->gameObjects.size() * (float)iter;
					if (iter % progressEvery == 0)
						progressCb((float)progress / 100.0f, "Processing object: " + it->name);
				}
			}

			loadObject(&sObj, obj, progressCb);

			objects.push_back(obj);
		}

		for (auto it = objectCache.begin(); it != objectCache.end(); ++it)
		{
			if (it->second.parentGuid.empty())
				continue;

			GameObject* parent = Engine::getSingleton()->getGameObject(it->second.parentGuid);
			it->first->getTransform()->setParent(parent->getTransform(), false);
		}

		for (auto it = objectCache.begin(); it != objectCache.end(); ++it)
			it->first->setEnabled(it->second.enabled);

		objectCache.clear();

		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			GameObject* obj = *it;
			std::vector<Component*>& components = obj->getComponents();

			for (auto ct = components.begin(); ct != components.end(); ++ct)
			{
				Component* comp = *ct;
				comp->onSceneLoaded();
			}
		}

		objects.clear();
	}

	void Scene::load(std::string location, std::string name, std::function<void(float progress, std::string objectName)> progressCb)
	{
		clear();

		SScene scene;

		if (IO::isDir(location))
		{
			std::string fullPath = location + name;
			if (!IO::FileExists(fullPath))
			{
				Debug::log("[" + name + "] Error loading scene: file does not exists", Debug::DbgColorRed);
				return;
			}

			try
			{
				std::ifstream ofs(fullPath, std::ios::binary);
				BinarySerializer s;
				s.deserialize(&ofs, &scene, Scene::ASSET_TYPE);
				ofs.close();
			}
			catch(const std::exception& e)
			{
				Debug::log("[" + name + "] Error loading scene: " + e.what(), Debug::DbgColorRed);
				std::cerr << "[" + name + "] Error loading scene: " << e.what() << '\n';
			}
		}
		else
		{
			zip_t* arch = Engine::getSingleton()->getZipArchive(location);
			if (!ZipHelper::isFileInZip(arch, name))
			{
				Debug::log("[" + name + "] Error loading scene: file does not exists", Debug::DbgColorRed);
				return;
			}

			int sz = 0;
			char* buffer = ZipHelper::readFileFromZip(arch, name, sz);

			try
			{
				boost::iostreams::stream<boost::iostreams::array_source> is(buffer, sz);
				BinarySerializer s;
				s.deserialize(&is, &scene, Scene::ASSET_TYPE);
				is.close();
			}
			catch (const std::exception& e)
			{
				Debug::log("[" + name + "] Error loading scene: " + e.what(), Debug::DbgColorRed);
				std::cerr << "[" + name + "] Error loading scene: " << e.what() << '\n';
			}

			delete[] buffer;
		}

		loadedScene = name;

		load(location, name, &scene, progressCb);

		Time::resetTimeSinceLevelStart();

		std::string geomName = IO::GetFilePath(name) + IO::GetFileName(name) + "/Static Geometry/" + md5(IO::GetFileName(name)) + ".mesh";
		
		std::string libPath = Engine::getSingleton()->getLibraryPath();
		
		if (!BatchedGeometry::getSingleton()->loadFromFile(libPath, geomName))
			BatchedGeometry::getSingleton()->rebuild(true);

		auto& objects = Engine::getSingleton()->getGameObjects();

		for (auto obj : objects)
		{
			CSGBrush* brush = (CSGBrush*)obj->getComponent(CSGBrush::COMPONENT_TYPE);
			if (brush != nullptr)
				brush->rebuild();
		}

		for (auto& object : objects)
		{
			CSGModel* model = (CSGModel*)object->getComponent(CSGModel::COMPONENT_TYPE);
			if (model == nullptr)
				continue;

			std::string hash = object->getGuid();
			std::string csgName = IO::GetFilePath(name) + IO::GetFileName(name) + "/CSG Geometry/" + md5(IO::GetFileName(name) + "_" + hash + "_csg") + ".mesh";

			if (!CSGGeometry::getSingleton()->loadFromFile(libPath, csgName, model))
				CSGGeometry::getSingleton()->rebuild(model, true);
		}

		APIManager::getSingleton()->start();

		if (Engine::getSingleton()->getIsRuntimeMode())
			APIManager::getSingleton()->execute("OnSceneLoaded");
	}

	void Scene::savePrefab(std::string location, std::string name, GameObject* gameObject)
	{
		Engine::getSingleton()->stopObjectAnimations(gameObject);

		SScene scene;

		std::vector<GameObject*> nstack;
		nstack.push_back(gameObject);

		while (nstack.size() > 0)
		{
			GameObject* child = *nstack.begin();
			nstack.erase(nstack.begin());

			saveObject(child, &scene);

			std::vector<Transform*>& children = child->getTransform()->getChildren();

			int j = 0;
			for (auto it = children.begin(); it != children.end(); ++it, ++j)
			{
				Transform* ch = *it;
				nstack.insert(nstack.begin() + j, ch->getGameObject());
			}
		}

		//Serialize
		std::string fullPath = location + name;
		std::ofstream ofs(fullPath, std::ios::binary);
		BinarySerializer s;
		s.serialize(&ofs, &scene, Prefab::ASSET_TYPE);
		ofs.close();
	}

	GameObject* Scene::loadPrefab(Prefab* prefab, glm::vec3 position, glm::quat rotation)
	{
		if (prefab == nullptr || !prefab->isLoaded())
			return nullptr;

		SScene& scene = prefab->getScene();

		std::vector<std::pair<GameObject*, SGameObject>> objectCache;
		std::vector<GameObject*> objects;
		std::unordered_map<std::string, GameObject*> remapList; //old, new

		GameObject* root = nullptr;

		int iter = 0;
		for (auto it = scene.gameObjects.begin(); it != scene.gameObjects.end(); ++it, ++iter)
		{
			SGameObject& sObj = *it;
			GameObject* obj = Engine::getSingleton()->createGameObject();
			remapList[sObj.guid] = obj;

			objectCache.push_back(std::make_pair(obj, sObj));

			loadObject(&sObj, obj, nullptr);

			objects.push_back(obj);
		}

		for (auto it = objectCache.begin(); it != objectCache.end(); ++it)
		{
			GameObject* parent = remapList[it->second.parentGuid];
			if (parent == nullptr)
			{
				if (root == nullptr)
					root = it->first;

				continue;
			}

			it->first->getTransform()->setParent(parent->getTransform(), false);
		}

		for (auto it = objectCache.begin(); it != objectCache.end(); ++it)
			it->first->setEnabled(it->second.enabled);

		objectCache.clear();

		for (auto it = remapList.begin(); it != remapList.end(); ++it)
		{
			std::string oldObj = it->first;
			GameObject* newObj = it->second;

			if (newObj == nullptr)
				continue;

			std::vector<Component*>& components = newObj->getComponents();
			for (auto c = components.begin(); c != components.end(); ++c)
			{
				for (auto r = remapList.begin(); r != remapList.end(); ++r)
				{
					if (r->second == nullptr)
						continue;

					(*c)->onRebindObject(r->first, r->second->getGuid());
				}
			}
		}

		remapList.clear();

		Transform* rootTransform = root->getTransform();
		rootTransform->setPosition(position);
		rootTransform->setRotation(rotation);

		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			GameObject* obj = *it;
			std::vector<Component*>& components = obj->getComponents();

			for (auto ct = components.begin(); ct != components.end(); ++ct)
			{
				Component* comp = *ct;
				comp->onSceneLoaded();
			}
		}

		objects.clear();

		if (Engine::getSingleton()->getIsRuntimeMode())
			APIManager::getSingleton()->execute(root, "Start");

		return root;
	}

	void Scene::clear()
	{
		Engine::getSingleton()->clear();
		BatchedGeometry::getSingleton()->clear();
		CSGGeometry::getSingleton()->clear();

		Mesh::unloadAll();
		Material::unloadAll();
		Texture::unloadAll();
		Font::unloadAll();
		Cubemap::unloadAll();
		Prefab::unloadAll();
		AudioClip::unloadAll();
		AnimationClip::unloadAll();

		Time::resetTimeSinceLevelStart();

		//Reset scene settings to default
		Renderer::getSingleton()->setSkyMaterial(nullptr);
		Renderer::getSingleton()->setSkyModel(SkyModel::Box);
		Renderer::getSingleton()->setAmbientColor(Color(0.4f, 0.4f, 0.4f, 1.0f));
		Renderer::getSingleton()->setGIEnabled(true);
		Renderer::getSingleton()->setGIIntensity(1.0f);
		Renderer::getSingleton()->updateEnvironmentMap();
		NavigationManager::getSingleton()->resetToDefault();
		NavigationManager::getSingleton()->setLoadedScene("");

		loadedScene = "";
	}
}