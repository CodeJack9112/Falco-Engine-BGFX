#pragma once

#include "../Navigation/Detour/Include/DetourNavMesh.h"
#include "../Navigation/Recast/Include/Recast.h"
#include "../Navigation/DetourTileCache/Include/DetourTileCache.h"
#include "../Navigation/PerfTimer.h"

#include <vector>
#include <string>
#include <functional>

#include "../glm/vec3.hpp"

class dtNavMeshQuery;

namespace GX
{
	class NavMeshAgent;
	class NavMeshObstacle;
	struct TileCacheData;
	class Transform;

	/// These are just sample areas to use consistent values across the samples.
	/// The use should specify these base on his needs.
	enum SamplePolyAreas
	{
		SAMPLE_POLYAREA_GROUND,
		SAMPLE_POLYAREA_WATER,
		SAMPLE_POLYAREA_ROAD,
		SAMPLE_POLYAREA_DOOR,
		SAMPLE_POLYAREA_GRASS,
		SAMPLE_POLYAREA_JUMP,
	};
	enum SamplePolyFlags
	{
		SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
		SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
		SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
		SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
		SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
		SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
	};
	enum SamplePartitionType
	{
		SAMPLE_PARTITION_WATERSHED,
		SAMPLE_PARTITION_MONOTONE,
		SAMPLE_PARTITION_LAYERS,
	};

	/// Recast build context.
	class BuildContext : public rcContext
	{
		TimeVal m_startTime[RC_MAX_TIMERS];
		TimeVal m_accTime[RC_MAX_TIMERS];

	public:
		BuildContext();

	protected:
		virtual void doResetTimers();
		virtual void doStartTimer(const rcTimerLabel label);
		virtual void doStopTimer(const rcTimerLabel label);
		virtual int doGetAccumulatedTime(const rcTimerLabel label) const;
	};

	class NavigationManager
	{
	protected:
		class dtNavMesh* m_navMesh = nullptr;
		class dtTileCache* m_tileCache = nullptr;
		dtNavMeshQuery* m_navQuery = nullptr;

		struct LinearAllocator* m_talloc = nullptr;
		struct FastLZCompressor* m_tcomp = nullptr;
		struct MeshProcess* m_tmproc = nullptr;

		std::string loadedScene = "";
		bool isNavMeshDirty = false;

		unsigned char m_navMeshDrawFlags;

		float walkableSlopeAngle = 45.0f;
		float walkableHeight = 0.8f;
		float walkableClimb = 1.0f;
		float walkableRadius = 0.35f;
		float maxEdgeLen = 12.0f;
		float maxSimplificationError = 1.5f;
		float minRegionArea = 1.0f;
		float mergeRegionArea = 10.0f;
		float cellSize = 0.30f;
		float cellHeight = 0.10f;

		int m_partitionType;

		bool m_filterLowHangingObstacles = false;
		bool m_filterLedgeSpans = false;
		bool m_filterWalkableLowHeightSpans = false;

		BuildContext * m_ctx = nullptr;

	private:
		static NavigationManager singleton;

		bool m_keepInterResults = false;
		float m_totalBuildTimeMs = 0.0f;

		std::vector<NavMeshAgent*> agentList;
		std::vector<NavMeshObstacle*> obstacleList;

		int cachedVerticesCount = 0;
		std::vector<glm::vec3> navMeshVertices;

		void saveAll(const char* path);
		void loadAll(const char* path);
		void loadAllFromBuffer(char * buf, size_t bufSize);

	public:
		NavigationManager();
		~NavigationManager();

		static NavigationManager* getSingleton() { return &singleton; }

		void buildNavMesh(std::function<void(int progress, int totalIter, int currentIter)> buildProgressCallback = nullptr);
		void loadNavMesh();
		void cleanup();

		void init();
		void update();
		void addAgent(NavMeshAgent* agent);
		void removeAgent(NavMeshAgent* agent);
		void addObstacle(NavMeshObstacle* obstacle);
		void removeObstacle(NavMeshObstacle* obstacle);
		std::vector<NavMeshAgent*>& getAgentList() { return agentList; }

		void setLoadedScene(std::string scene);
		std::string getLoadedScene() { return loadedScene; }

		void setNavMeshIsDirty() { isNavMeshDirty = true; }

		std::vector<glm::vec3>& getNavMeshVertices();

		int rasterizeTileLayers(const int tx, const int ty, const rcConfig& cfg, TileCacheData* tiles, const int maxTiles, const float* verts, const int nverts, const int* tris, const int ntris);

		//dtCrowd * GetCrowd() { return m_crowd; }
		dtNavMesh * getNavMesh() { return m_navMesh; }
		dtNavMeshQuery * getNavMeshQuery() { return m_navQuery; }
		dtTileCache * getTileCache() { return m_tileCache; }

		void resetToDefault();

		void setWalkableSlopeAngle(float value) { walkableSlopeAngle = value; }
		void setWalkableHeight(float value) { walkableHeight = value; }
		void setWalkableClimb(float value) { walkableClimb = value; }
		void setWalkableRadius(float value) { walkableRadius = value; }
		void setMaxEdgeLen(float value) { maxEdgeLen = value; }
		void setMaxSimplificationError(float value) { maxSimplificationError = value; }
		void setMinRegionArea(float value) { minRegionArea = value; }
		void setMergeRegionArea(float value) { mergeRegionArea = value; }
		void setCellSize(float value) { cellSize = value; }
		void setCellHeight(float value) { cellHeight = value; }

		float& getWalkableSlopeAngle() { return walkableSlopeAngle; }
		float& getWalkableHeight() { return walkableHeight; }
		float& getWalkableClimb() { return walkableClimb; }
		float& getWalkableRadius() { return walkableRadius; }
		float& getMaxEdgeLen() { return maxEdgeLen; }
		float& getMaxSimplificationError() { return maxSimplificationError; }
		float& getMinRegionArea() { return minRegionArea; }
		float& getMergeRegionArea() { return mergeRegionArea; }
		float& getCellSize() { return cellSize; }
		float& getCellHeight() { return cellHeight; }
	};
}