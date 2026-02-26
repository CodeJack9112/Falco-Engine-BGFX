#pragma once

#include "../Data/SVector.h"
#include "../Data/SQuaternion.h"

#include "../Components/SComponent.h"
#include "../Components/SLight.h"
#include "../Components/SCamera.h"
#include "../Components/SMeshRenderer.h"
#include "../Components/SAnimation.h"
#include "../Components/SRigidBody.h"
#include "../Components/SBoxCollider.h"
#include "../Components/SCapsuleCollider.h"
#include "../Components/SMeshCollider.h"
#include "../Components/SSphereCollider.h"
#include "../Components/STerrainCollider.h"
#include "../Components/SVehicle.h"
#include "../Components/SFixedJoint.h"
#include "../Components/SFreeJoint.h"
#include "../Components/SHingeJoint.h"
#include "../Components/SConeTwistJoint.h"
#include "../Components/SAudioListener.h"
#include "../Components/SAudioSource.h"
#include "../Components/SNavMeshAgent.h"
#include "../Components/SNavMeshObstacle.h"
#include "../Components/SCanvas.h"
#include "../Components/SImage.h"
#include "../Components/SButton.h"
#include "../Components/SText.h"
#include "../Components/STextInput.h"
#include "../Components/SMask.h"
#include "../Components/SMonoScript.h"
#include "../Components/STerrain.h"
#include "../Components/SParticleSystem.h"
#include "../Components/SWater.h"
#include "../Components/SSpline.h"
#include "../Components/SVideoPlayer.h"
#include "../Components/SCSGModel.h"
#include "../Components/SCSGBrush.h"
#include "../Components/SDecalRenderer.h"

namespace GX
{
	class SGameObject : public Archive
	{
	public:
		SGameObject() {}
		~SGameObject() {}

		virtual int getVersion() { return 3; }

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(enabled);
			data(name);
			data(guid);
			data(parentGuid);
			data(position);
			data(rotation);
			data(scale);
			data(navigationStatic);
			data(lightingStatic);
			data(batchingStatic);
			data(tag);
			data(layer);
			data(occlusionStatic);
			data(meshRenderers);
			data(lights);
			data(cameras);
			data(animations);
			data(rigidBodies);
			data(boxColliders);
			data(capsuleColliders);
			data(meshColliders);
			data(sphereColliders);
			data(vehicles);
			data(fixedJoints);
			data(freeJoints);
			data(hingeJoints);
			data(audioListeners);
			data(audioSources);
			data(navMeshAgents);
			data(navMeshObstacles);
			data(monoScripts);
			data(canvases);
			data(images);
			data(texts);
			data(buttons);
			data(terrains);
			data(particleSystems);
			data(terrainColliders);
			data(coneTwistJoints);
			data(waters);
			data(masks);
			data(splines);
			data(videoPlayers);
			data(csgBrushes);
			if (version > 0)
				data(textInputs);
			if (version > 1)
				data(csgModels);
			if (version > 2)
				data(decals);
		}

	public:
		bool enabled = true;

		std::string name = "";
		std::string guid = "";
		std::string parentGuid = "";

		bool navigationStatic = false;
		bool lightingStatic = false;
		bool batchingStatic = false;
		bool occlusionStatic = false;

		int tag = 0;
		int layer = 0;

		SVector3 position = SVector3(0, 0, 0);
		SQuaternion rotation = SQuaternion(0, 0, 0, 1);
		SVector3 scale = SVector3(1, 1, 1);

		std::vector<SMeshRenderer> meshRenderers;
		std::vector<SLight> lights;
		std::vector<SCamera> cameras;
		std::vector<SAnimation> animations;
		std::vector<SRigidBody> rigidBodies;
		std::vector<SBoxCollider> boxColliders;
		std::vector<SCapsuleCollider> capsuleColliders;
		std::vector<SMeshCollider> meshColliders;
		std::vector<SSphereCollider> sphereColliders;
		std::vector<STerrainCollider> terrainColliders;
		std::vector<SVehicle> vehicles;
		std::vector<SFixedJoint> fixedJoints;
		std::vector<SFreeJoint> freeJoints;
		std::vector<SHingeJoint> hingeJoints;
		std::vector<SConeTwistJoint> coneTwistJoints;
		std::vector<SAudioListener> audioListeners;
		std::vector<SAudioSource> audioSources;
		std::vector<SNavMeshAgent> navMeshAgents;
		std::vector<SNavMeshObstacle> navMeshObstacles;
		std::vector<SCanvas> canvases;
		std::vector<SImage> images;
		std::vector<SText> texts;
		std::vector<SButton> buttons;
		std::vector<SMask> masks;
		std::vector<SMonoScript> monoScripts;
		std::vector<STerrain> terrains;
		std::vector<SParticleSystem> particleSystems;
		std::vector<SWater> waters;
		std::vector<SSpline> splines;
		std::vector<SVideoPlayer> videoPlayers;
		std::vector<SCSGModel> csgModels;
		std::vector<SCSGBrush> csgBrushes;
		std::vector<STextInput> textInputs;
		std::vector<SDecalRenderer> decals;
	};
}