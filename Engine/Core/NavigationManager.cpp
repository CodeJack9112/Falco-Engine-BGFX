#include "NavigationManager.h"

#include <math.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32
#include <process.h>
#endif

#include <glm/gtc/type_ptr.hpp>

#include "../Core/Engine.h"
#include "../Core/Debug.h"
#include "../Core/Time.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/CSGGeometry.h"
#include "../Navigation/InputGeom.h"
#include "../Navigation/Detour/Include/DetourNavMeshBuilder.h"
#include "../Navigation/Detour/Include/DetourNavMeshQuery.h"
#include "../Navigation/DebugUtils/Include/RecastDump.h"
#include "../Navigation/DetourCrowd/Include/DetourCrowd.h"
#include "../Navigation/Detour/Include/DetourCommon.h"
#include "../Navigation/DetourTileCache/Include/DetourTileCache.h"
#include "../Navigation/DetourTileCache/Include/DetourTileCacheBuilder.h"
#include "../Classes/fastlz.h"

#include "../Components/NavMeshAgent.h"
#include "../Components/NavMeshObstacle.h"
#include "../Components/Transform.h"
#include "../Components/MeshRenderer.h"
#include "../Components/Terrain.h"
#include "../Assets/Mesh.h"
#include "../Core/GameObject.h"
#include "../Classes/ZipHelper.h"

#include "../Classes/IO.h"

#include <boost/iostreams/stream.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace ba = boost::asio;

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace GX
{
	NavigationManager NavigationManager::singleton;

	static const int MAX_LAYERS = 32;
	static const int EXPECTED_LAYERS_PER_TILE = 4;

	struct FastLZCompressor : public dtTileCacheCompressor
	{
		virtual int maxCompressedSize(const int bufferSize)
		{
			return (int)(bufferSize * 1.05f);
		}

		virtual dtStatus compress(const unsigned char* buffer, const int bufferSize,
			unsigned char* compressed, const int /*maxCompressedSize*/, int* compressedSize)
		{
			*compressedSize = fastlz_compress((const void* const)buffer, bufferSize, compressed);
			return DT_SUCCESS;
		}

		virtual dtStatus decompress(const unsigned char* compressed, const int compressedSize,
			unsigned char* buffer, const int maxBufferSize, int* bufferSize)
		{
			*bufferSize = fastlz_decompress(compressed, compressedSize, buffer, maxBufferSize);
			return *bufferSize < 0 ? DT_FAILURE : DT_SUCCESS;
		}
	};

	struct LinearAllocator : public dtTileCacheAlloc
	{
		unsigned char* buffer;
		size_t capacity;
		size_t top;
		size_t high;

		LinearAllocator(const size_t cap) : buffer(0), capacity(0), top(0), high(0)
		{
			resize(cap);
		}

		~LinearAllocator()
		{
			dtFree(buffer);
		}

		void resize(const size_t cap)
		{
			if (buffer) dtFree(buffer);
			buffer = (unsigned char*)dtAlloc(cap, DT_ALLOC_PERM);
			capacity = cap;
		}

		virtual void reset()
		{
			high = dtMax(high, top);
			top = 0;
		}

		virtual void* alloc(const size_t size)
		{
			if (!buffer)
				return 0;
			if (top + size > capacity)
				return 0;
			unsigned char* mem = &buffer[top];
			top += size;
			return mem;
		}

		virtual void free(void* /*ptr*/)
		{
			// Empty
		}
	};

	struct MeshProcess : public dtTileCacheMeshProcess
	{
		InputGeom* m_geom;

		inline MeshProcess() : m_geom(0)
		{
		}

		inline void init(InputGeom* geom)
		{
			m_geom = geom;
		}

		virtual void process(struct dtNavMeshCreateParams* params,
			unsigned char* polyAreas, unsigned short* polyFlags)
		{
			// Update poly flags from areas.
			for (int i = 0; i < params->polyCount; ++i)
			{
				if (polyAreas[i] == DT_TILECACHE_WALKABLE_AREA)
					polyAreas[i] = SAMPLE_POLYAREA_GROUND;

				if (polyAreas[i] == SAMPLE_POLYAREA_GROUND ||
					polyAreas[i] == SAMPLE_POLYAREA_GRASS ||
					polyAreas[i] == SAMPLE_POLYAREA_ROAD)
				{
					polyFlags[i] = SAMPLE_POLYFLAGS_WALK;
				}
				else if (polyAreas[i] == SAMPLE_POLYAREA_WATER)
				{
					polyFlags[i] = SAMPLE_POLYFLAGS_SWIM;
				}
				else if (polyAreas[i] == SAMPLE_POLYAREA_DOOR)
				{
					polyFlags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
				}
			}

			// Pass in off-mesh connections.
			if (m_geom)
			{
				params->offMeshConVerts = m_geom->getOffMeshConnectionVerts();
				params->offMeshConRad = m_geom->getOffMeshConnectionRads();
				params->offMeshConDir = m_geom->getOffMeshConnectionDirs();
				params->offMeshConAreas = m_geom->getOffMeshConnectionAreas();
				params->offMeshConFlags = m_geom->getOffMeshConnectionFlags();
				params->offMeshConUserID = m_geom->getOffMeshConnectionId();
				params->offMeshConCount = m_geom->getOffMeshConnectionCount();
			}
		}
	};

	struct TileCacheData
	{
		unsigned char* data;
		int dataSize;
	};

	struct RasterizationContext
	{
		RasterizationContext() :
			solid(0),
			triareas(0),
			lset(0),
			chf(0),
			ntiles(0)
		{
			memset(tiles, 0, sizeof(TileCacheData) * MAX_LAYERS);
		}

		~RasterizationContext()
		{
			rcFreeHeightField(solid);
			delete[] triareas;
			rcFreeHeightfieldLayerSet(lset);
			rcFreeCompactHeightfield(chf);
			for (int i = 0; i < MAX_LAYERS; ++i)
			{
				dtFree(tiles[i].data);
				tiles[i].data = 0;
			}
		}

		rcHeightfield* solid;
		unsigned char* triareas;
		rcHeightfieldLayerSet* lset;
		rcCompactHeightfield* chf;
		TileCacheData tiles[MAX_LAYERS];
		int ntiles;
	};

	struct TileCacheBuildContext
	{
		inline TileCacheBuildContext(struct dtTileCacheAlloc* a) : layer(0), lcset(0), lmesh(0), alloc(a) {}
		inline ~TileCacheBuildContext() { purge(); }
		void purge()
		{
			dtFreeTileCacheLayer(alloc, layer);
			layer = 0;
			dtFreeTileCacheContourSet(alloc, lcset);
			lcset = 0;
			dtFreeTileCachePolyMesh(alloc, lmesh);
			lmesh = 0;
		}
		struct dtTileCacheLayer* layer;
		struct dtTileCacheContourSet* lcset;
		struct dtTileCachePolyMesh* lmesh;
		struct dtTileCacheAlloc* alloc;
	};

	bool m_filterLowHangingObstacles = false;
	bool m_filterLedgeSpans = false;
	bool m_filterWalkableLowHeightSpans = false;

	int NavigationManager::rasterizeTileLayers(const int tx, const int ty, const rcConfig& cfg, TileCacheData* tiles, const int maxTiles, const float* verts, const int nverts, const int* tris, const int ntris)
	{
		FastLZCompressor comp;
		RasterizationContext rc;

		rcChunkyTriMesh* chunkyMesh = new rcChunkyTriMesh();
		rcCreateChunkyTriMesh(verts, tris, ntris, 2, chunkyMesh);

		// Tile bounds.
		const float tcs = cfg.tileSize * cfg.cs;

		rcConfig tcfg;
		memcpy(&tcfg, &cfg, sizeof(tcfg));

		tcfg.bmin[0] = cfg.bmin[0] + tx * tcs;
		tcfg.bmin[1] = cfg.bmin[1];
		tcfg.bmin[2] = cfg.bmin[2] + ty * tcs;
		tcfg.bmax[0] = cfg.bmin[0] + (tx + 1) * tcs;
		tcfg.bmax[1] = cfg.bmax[1];
		tcfg.bmax[2] = cfg.bmin[2] + (ty + 1) * tcs;
		tcfg.bmin[0] -= tcfg.borderSize * tcfg.cs;
		tcfg.bmin[2] -= tcfg.borderSize * tcfg.cs;
		tcfg.bmax[0] += tcfg.borderSize * tcfg.cs;
		tcfg.bmax[2] += tcfg.borderSize * tcfg.cs;

		// Allocate voxel heightfield where we rasterize our input data to.
		rc.solid = rcAllocHeightfield();
		if (!rc.solid)
		{
			//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
			delete chunkyMesh;
			return 0;
		}
		if (!rcCreateHeightfield(m_ctx, *rc.solid, tcfg.width, tcfg.height, tcfg.bmin, tcfg.bmax, tcfg.cs, tcfg.ch))
		{
			//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
			delete chunkyMesh;
			return 0;
		}

		// Allocate array that can hold triangle flags.
		// If you have multiple meshes you need to process, allocate
		// and array which can hold the max number of triangles you need to process.
		rc.triareas = new unsigned char[chunkyMesh->maxTrisPerChunk];
		if (!rc.triareas)
		{
			//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", chunkyMesh->maxTrisPerChunk);
			delete chunkyMesh;
			return 0;
		}

		float tbmin[2], tbmax[2];
		tbmin[0] = tcfg.bmin[0];
		tbmin[1] = tcfg.bmin[2];
		tbmax[0] = tcfg.bmax[0];
		tbmax[1] = tcfg.bmax[2];
		int cid[512];// TODO: Make grow when returning too many items.
		const int ncid = rcGetChunksOverlappingRect(chunkyMesh, tbmin, tbmax, cid, 512);
		if (!ncid)
		{
			delete chunkyMesh;
			return 0; // empty
		}

		for (int i = 0; i < ncid; ++i)
		{
			const rcChunkyTriMeshNode& node = chunkyMesh->nodes[cid[i]];
			const int* tris = &chunkyMesh->tris[node.i * 3];
			const int ntris = node.n;

			memset(rc.triareas, 0, ntris * sizeof(unsigned char));
			rcMarkWalkableTriangles(m_ctx, tcfg.walkableSlopeAngle,
				verts, nverts, tris, ntris, rc.triareas);

			if (!rcRasterizeTriangles(m_ctx, verts, nverts, tris, rc.triareas, ntris, *rc.solid, tcfg.walkableClimb))
			{
				delete chunkyMesh;
				return 0;
			}
		}

		// Once all geometry is rasterized, we do initial pass of filtering to
		// remove unwanted overhangs caused by the conservative rasterization
		// as well as filter spans where the character cannot possibly stand.
		if (m_filterLowHangingObstacles)
			rcFilterLowHangingWalkableObstacles(m_ctx, tcfg.walkableClimb, *rc.solid);
		if (m_filterLedgeSpans)
			rcFilterLedgeSpans(m_ctx, tcfg.walkableHeight, tcfg.walkableClimb, *rc.solid);
		if (m_filterWalkableLowHeightSpans)
			rcFilterWalkableLowHeightSpans(m_ctx, tcfg.walkableHeight, *rc.solid);


		rc.chf = rcAllocCompactHeightfield();
		if (!rc.chf)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
			delete chunkyMesh;
			return 0;
		}
		if (!rcBuildCompactHeightfield(m_ctx, tcfg.walkableHeight, tcfg.walkableClimb, *rc.solid, *rc.chf))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
			delete chunkyMesh;
			return 0;
		}

		// Erode the walkable area by agent radius.
		if (!rcErodeWalkableArea(m_ctx, tcfg.walkableRadius, *rc.chf))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
			delete chunkyMesh;
			return 0;
		}

		// (Optional) Mark areas.
		/*const ConvexVolume* vols = m_geom->getConvexVolumes();
		for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
		{
			rcMarkConvexPolyArea(m_ctx, vols[i].verts, vols[i].nverts,
				vols[i].hmin, vols[i].hmax,
				(unsigned char)vols[i].area, *rc.chf);
		}*/

		rc.lset = rcAllocHeightfieldLayerSet();
		if (!rc.lset)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'lset'.");
			delete chunkyMesh;
			return 0;
		}
		if (!rcBuildHeightfieldLayers(m_ctx, *rc.chf, tcfg.borderSize, tcfg.walkableHeight, *rc.lset))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build heighfield layers.");
			delete chunkyMesh;
			return 0;
		}

		rc.ntiles = 0;
		for (int i = 0; i < rcMin(rc.lset->nlayers, MAX_LAYERS); ++i)
		{
			TileCacheData* tile = &rc.tiles[rc.ntiles++];
			const rcHeightfieldLayer* layer = &rc.lset->layers[i];

			// Store header
			dtTileCacheLayerHeader header;
			header.magic = DT_TILECACHE_MAGIC;
			header.version = DT_TILECACHE_VERSION;

			// Tile layer location in the navmesh.
			header.tx = tx;
			header.ty = ty;
			header.tlayer = i;
			dtVcopy(header.bmin, layer->bmin);
			dtVcopy(header.bmax, layer->bmax);

			// Tile info.
			header.width = (unsigned char)layer->width;
			header.height = (unsigned char)layer->height;
			header.minx = (unsigned char)layer->minx;
			header.maxx = (unsigned char)layer->maxx;
			header.miny = (unsigned char)layer->miny;
			header.maxy = (unsigned char)layer->maxy;
			header.hmin = (unsigned short)layer->hmin;
			header.hmax = (unsigned short)layer->hmax;

			dtStatus status = dtBuildTileCacheLayer(&comp, &header, layer->heights, layer->areas, layer->cons,
				&tile->data, &tile->dataSize);
			if (dtStatusFailed(status))
			{
				delete chunkyMesh;
				return 0;
			}
		}

		// Transfer ownsership of tile data from build context to the caller.
		int n = 0;
		for (int i = 0; i < rcMin(rc.ntiles, maxTiles); ++i)
		{
			tiles[n++] = rc.tiles[i];
			rc.tiles[i].data = 0;
			rc.tiles[i].dataSize = 0;
		}

		delete chunkyMesh;

		return n;
	}

	static int calcLayerBufferSize(const int gridWidth, const int gridHeight)
	{
		const int headerSize = dtAlign4(sizeof(dtTileCacheLayerHeader));
		const int gridSize = gridWidth * gridHeight;
		return headerSize + gridSize * 4;
	}

	BuildContext::BuildContext()
	{
		resetTimers();
	}

	void BuildContext::doResetTimers()
	{
		for (int i = 0; i < RC_MAX_TIMERS; ++i)
			m_accTime[i] = -1;
	}

	void BuildContext::doStartTimer(const rcTimerLabel label)
	{
		m_startTime[label] = getPerfTime();
	}

	void BuildContext::doStopTimer(const rcTimerLabel label)
	{
		const TimeVal endTime = getPerfTime();
		const TimeVal deltaTime = endTime - m_startTime[label];
		if (m_accTime[label] == -1)
			m_accTime[label] = deltaTime;
		else
			m_accTime[label] += deltaTime;
	}

	int BuildContext::doGetAccumulatedTime(const rcTimerLabel label) const
	{
		return getPerfTimeUsec(m_accTime[label]);
	}

	NavigationManager::NavigationManager()
	{
		m_navQuery = new dtNavMeshQuery();
		m_ctx = new BuildContext();

		m_talloc = new LinearAllocator(32000);
		m_tcomp = new FastLZCompressor;
		m_tmproc = new MeshProcess;
	}

	NavigationManager::~NavigationManager()
	{
		delete m_navQuery;
		agentList.clear();

		dtFreeTileCache(m_tileCache);
	}

	struct ArraySlice
	{
	public:
		NavigationManager* mgr = nullptr;
		int start = 0;
		int end = 0;
		float y = 0;
		rcConfig cfg;
		float* verts = nullptr;
		int nverts = 0;
		int* tris = nullptr;
		int ntris = 0;
		dtStatus status;
		dtTileCache* m_tileCache = nullptr;
		dtTileCacheParams tcparams;
		bool finished = false;
	};

	void rasterizeThread(void* param)
	{
		ArraySlice* slice = (ArraySlice*)param;

		for (int x = slice->start; x < slice->end; ++x)
		{
			TileCacheData tiles[MAX_LAYERS];
			memset(tiles, 0, sizeof(tiles));
			int ntiles = slice->mgr->rasterizeTileLayers(x, slice->y, slice->cfg, tiles, MAX_LAYERS, slice->verts, slice->nverts, slice->tris, slice->ntris);

			for (int i = 0; i < ntiles; ++i)
			{
				TileCacheData* tile = &tiles[i];
				slice->status = slice->m_tileCache->addTile(tile->data, tile->dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
				if (dtStatusFailed(slice->status))
				{
					dtFree(tile->data);
					tile->data = 0;
					continue;
				}
			}
		}

		slice->finished = true;

	#ifdef _WIN32
		ExitThread(0);
	#else
		pthread_exit(0);
	#endif
	}

	void NavigationManager::buildNavMesh(std::function<void(int progress, int totalIter, int currentIter)> buildProgressCallback)
	{
		cleanup();

		std::vector<GameObject*> staticNodes = Engine::getSingleton()->getGameObjects();
		
		int _nverts = 0;
		int _ntris = 0;
		int _nindexes = 0;

		std::vector<float> allVerts;
		std::vector<int> allIndices;

		AxisAlignedBox bounds = AxisAlignedBox::BOX_NULL;

		for (auto it = staticNodes.begin(); it != staticNodes.end(); ++it)
		{
			GameObject* obj = *it;
			Transform* t = obj->getTransform();

			if (!obj->getActive())
				continue;

			if (!obj->getNavigationStatic())
				continue;

			MeshRenderer* rend = (MeshRenderer*)obj->getComponent(MeshRenderer::COMPONENT_TYPE);
			if (rend != nullptr && rend->getEnabled())
			{
				Mesh* mesh = rend->getMesh();

				if (mesh != nullptr)
				{
					bounds.merge(rend->getBounds());

					for (int sm = 0; sm < mesh->getSubMeshCount(); ++sm)
					{
						SubMesh* subMesh = mesh->getSubMesh(sm);

						std::vector<VertexBuffer>& vbuf = subMesh->getVertexBuffer();
						std::vector<uint32_t>& ibuf = subMesh->getIndexBuffer();

						_nverts += vbuf.size();
						_ntris += ibuf.size() / 3;

						for (int i = 0; i < vbuf.size(); ++i)
						{
							glm::vec3 pos = t->getTransformMatrix() * glm::vec4(vbuf[i].position, 1.0);
							allVerts.push_back(pos.x);
							allVerts.push_back(pos.y);
							allVerts.push_back(pos.z);
						}

						for (int i = 0; i < ibuf.size(); ++i)
							allIndices.push_back(_nindexes + ibuf[i]);

						_nindexes += vbuf.size();
					}
				}
			}

			Terrain* terrain = (Terrain*)obj->getComponent(Terrain::COMPONENT_TYPE);
			if (terrain != nullptr && terrain->getEnabled())
			{
				bounds.merge(terrain->getBounds());

				Terrain::VertexBuffer* vbuf = terrain->getVertexBuffer();
				uint32_t* ibuf = terrain->getIndexBuffer();

				int terrainVertsCount = terrain->getVertexCount();
				int terrainIndexCount = terrain->getIndexCount();

				_nverts += terrainVertsCount;
				_ntris += terrainIndexCount / 3;

				for (int i = 0; i < terrainVertsCount; ++i)
				{
					glm::vec3 pos = t->getTransformMatrix() * glm::vec4(vbuf[i].position, 1.0);
					allVerts.push_back(pos.x);
					allVerts.push_back(pos.y);
					allVerts.push_back(pos.z);
				}

				for (int i = 0; i < terrainIndexCount; ++i)
					allIndices.push_back(_nindexes + ibuf[i]);

				_nindexes += terrainVertsCount;
			}
		}

		//CSG
		auto& csgModels = CSGGeometry::getSingleton()->getModels();
		for (auto csgModel : csgModels)
		{
			for (auto subMesh : csgModel->subMeshes)
			{
				bounds.merge(subMesh->getBounds());

				std::vector<VertexBuffer>& vbuf = subMesh->getVertexBuffer();
				std::vector<uint32_t>& ibuf = subMesh->getIndexBuffer();

				_nverts += vbuf.size();
				_ntris += ibuf.size() / 3;

				for (int i = 0; i < vbuf.size(); ++i)
				{
					glm::vec3 pos = vbuf[i].position;
					allVerts.push_back(pos.x);
					allVerts.push_back(pos.y);
					allVerts.push_back(pos.z);
				}

				for (int i = 0; i < ibuf.size(); ++i)
					allIndices.push_back(_nindexes + ibuf[i]);

				_nindexes += vbuf.size();
			}
		}

		if (allVerts.size() == 0 || allIndices.size() == 0)
		{
			allVerts.clear();
			allIndices.clear();
			Debug::logWarning("[NavMesh] input data is empty. No static geometry in the scene");

			return;
		}

		const float* bmin = glm::value_ptr(bounds.getMinimum());
		const float* bmax = glm::value_ptr(bounds.getMaximum());

		const float* verts = &allVerts[0];
		const int nverts = _nverts;
		const int* tris = &allIndices[0];
		const int ntris = _ntris;

		dtStatus status;

		int tileSize = 48;
		int vertsPerPoly = 6;
		int sampleDistance = 6;
		int sampleMaxError = 1;

		m_tmproc->init(nullptr);

		// Init cache
		int gw = 0, gh = 0;
		rcCalcGridSize(bmin, bmax, cellSize, &gw, &gh);
		const int ts = (int)tileSize;
		const int tw = (gw + ts - 1) / ts;
		const int th = (gh + ts - 1) / ts;

		// Generation params.
		rcConfig cfg;
		memset(&cfg, 0, sizeof(cfg));
		cfg.cs = cellSize;
		cfg.ch = cellHeight;
		cfg.walkableSlopeAngle = walkableSlopeAngle;
		cfg.walkableHeight = (int)ceilf(walkableHeight / cfg.ch);
		cfg.walkableClimb = (int)floorf(walkableClimb / cfg.ch);
		cfg.walkableRadius = (int)ceilf(walkableRadius / cfg.cs);
		cfg.maxEdgeLen = (int)(maxEdgeLen / cellSize);
		cfg.maxSimplificationError = maxSimplificationError;
		cfg.minRegionArea = (int)rcSqr(minRegionArea);		// Note: area = size*size
		cfg.mergeRegionArea = (int)rcSqr(mergeRegionArea);	// Note: area = size*size
		cfg.maxVertsPerPoly = (int)vertsPerPoly;
		cfg.tileSize = (int)tileSize;
		cfg.borderSize = cfg.walkableRadius + 3; // Reserve enough padding.
		cfg.width = cfg.tileSize + cfg.borderSize * 2;
		cfg.height = cfg.tileSize + cfg.borderSize * 2;
		cfg.detailSampleDist = sampleDistance < 0.9f ? 0 : cellSize * sampleDistance;
		cfg.detailSampleMaxError = cellHeight * sampleMaxError;
		rcVcopy(cfg.bmin, bmin);
		rcVcopy(cfg.bmax, bmax);

		// Tile cache params.
		dtTileCacheParams tcparams;
		memset(&tcparams, 0, sizeof(tcparams));
		rcVcopy(tcparams.orig, bmin);
		tcparams.cs = cellSize;
		tcparams.ch = cellHeight;
		tcparams.width = (int)tileSize;
		tcparams.height = (int)tileSize;
		tcparams.walkableHeight = walkableHeight;
		tcparams.walkableRadius = walkableRadius;
		tcparams.walkableClimb = walkableClimb;
		tcparams.maxSimplificationError = maxSimplificationError;
		tcparams.maxTiles = tw * th * EXPECTED_LAYERS_PER_TILE;
		tcparams.maxObstacles = 128;

		dtFreeTileCache(m_tileCache);

		m_tileCache = dtAllocTileCache();
		if (!m_tileCache)
		{

		}

		status = m_tileCache->init(&tcparams, m_talloc, m_tcomp, m_tmproc);
		if (dtStatusFailed(status))
		{

		}

		dtFreeNavMesh(m_navMesh);

		m_navMesh = dtAllocNavMesh();
		if (!m_navMesh)
		{

		}

		int tileBits = rcMin((int)dtIlog2(dtNextPow2(tw * th * EXPECTED_LAYERS_PER_TILE)), 14);
		if (tileBits > 14) tileBits = 14;
		int polyBits = 22 - tileBits;
		int maxTiles = 1 << tileBits;
		int maxPolysPerTile = 1 << polyBits;

		dtNavMeshParams params;
		memset(&params, 0, sizeof(params));
		rcVcopy(params.orig, bmin);
		params.tileWidth = tileSize * cellSize;
		params.tileHeight = tileSize * cellSize;
		params.maxTiles = maxTiles;
		params.maxPolys = maxPolysPerTile;

		status = m_navMesh->init(&params);
		if (dtStatusFailed(status))
		{

		}

		status = m_navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{

		}

		// Preprocess tiles.

		m_ctx->resetTimers();

	#ifdef _WIN32
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		const int numCPU = sysinfo.dwNumberOfProcessors + 1;
	#else
		const int numCPU = sysconf(_SC_NPROCESSORS_ONLN);
	#endif
		//int m_cacheLayerCount = 0;
		//int m_cacheCompressedSize = 0;
		//int m_cacheRawSize = 0;
		int totalIter = th * tw;
		int iterations = 0;

		for (int y = 0; y < th; ++y)
		{
			//Multithreaded
			int arrayLength = tw;
			//size_t maxPerThread = arrayLength / numCPU;
			size_t coresUsed = arrayLength <= numCPU ? arrayLength : numCPU;
			size_t maxPerThread = arrayLength >= numCPU ? arrayLength / numCPU : 1;

			std::vector<ArraySlice*> slices;

			for (size_t i = 0; i < coresUsed; ++i)
			{
				size_t start = i * maxPerThread;
				size_t end = std::min((int)(start + maxPerThread), arrayLength);

				if (i == coresUsed - 1)
				{
					//if (end < arrayLength)
					end = arrayLength;
				}

				ArraySlice* slice = new ArraySlice();
				slice->cfg = cfg;
				slice->start = start;
				slice->end = end;
				slice->mgr = this;
				slice->m_tileCache = m_tileCache;
				slice->ntris = ntris;
				slice->nverts = nverts;
				slice->tcparams = tcparams;
				slice->tris = const_cast<int*>(tris);
				slice->verts = const_cast<float*>(verts);
				slice->y = y;
				slices.push_back(slice);

				std::thread t([=] () { rasterizeThread(slice); });
				t.detach();
			}

			//Wait all threads
			while (slices.size() > 0)
			{
				if (slices[0]->finished)
				{
					delete slices[0];
					slices.erase(slices.begin());
				}
			}

			iterations += tw;

			int progress = (float)iterations / (float)totalIter * 100.0f;
			if (progress > 100) progress = 100;
			if (iterations > totalIter) iterations = totalIter;
			if (buildProgressCallback != nullptr)
				buildProgressCallback(progress, totalIter, iterations);
		}
		
		// Build initial meshes
		m_ctx->startTimer(RC_TIMER_TOTAL);
		for (int y = 0; y < th; ++y)
			for (int x = 0; x < tw; ++x)
				m_tileCache->buildNavMeshTilesAt(x, y, m_navMesh);

		m_ctx->stopTimer(RC_TIMER_TOTAL);

		if (!loadedScene.empty())
		{
			std::string fp = IO::GetFilePath(loadedScene);
			std::string fn = IO::GetFileName(loadedScene);
			IO::CreateDir(Engine::getSingleton()->getAssetsPath() + fp + "/" + fn + "/");
			std::string saveTo = Engine::getSingleton()->getAssetsPath() + fp + "/" + fn + "/" + fn + ".navmesh";

			saveAll(saveTo.c_str());
		}
		//buildVisualizationMesh();
	}

	void NavigationManager::loadNavMesh()
	{
		cleanup();

		if (IO::isDir(Engine::getSingleton()->getAssetsPath()))
		{
			std::string path = Engine::getSingleton()->getAssetsPath() + IO::GetFilePath(loadedScene) + IO::GetFileName(loadedScene) + "/" + IO::GetFileName(loadedScene) + ".navmesh";

			if (!IO::FileExists(path))
				return;

			loadAll(path.c_str());
		}
		else
		{
			std::string path = IO::GetFilePath(loadedScene) + IO::GetFileName(loadedScene) + "/" + IO::GetFileName(loadedScene) + ".navmesh";

			std::string location = Engine::getSingleton()->getAssetsPath();
			zip_t* arch = Engine::getSingleton()->getZipArchive(location);

			if (!ZipHelper::isFileInZip(arch, path))
			{
				return;
			}

			int sz = 0;
			char* buffer = ZipHelper::readFileFromZip(arch, path, sz);
			loadAllFromBuffer(buffer, sz);
		}
	}

	void NavigationManager::init()
	{
		
	}

	void NavigationManager::update()
	{
		if (m_navMesh == nullptr)
			return;

		for (auto it = obstacleList.begin(); it != obstacleList.end(); ++it)
			(*it)->update();

		if (m_tileCache)
			m_tileCache->update(Time::getDeltaTime(), m_navMesh);

		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			for (auto it = agentList.begin(); it != agentList.end(); ++it)
				(*it)->update();
		}
	}

	void NavigationManager::addAgent(NavMeshAgent * agent)
	{
		agentList.push_back(agent);
	}

	void NavigationManager::removeAgent(NavMeshAgent * agent)
	{
		auto it = std::find(agentList.begin(), agentList.end(), agent);
		if (it != agentList.end())
			agentList.erase(it);
	}

	void NavigationManager::addObstacle(NavMeshObstacle* obstacle)
	{
		obstacleList.push_back(obstacle);
	}

	void NavigationManager::removeObstacle(NavMeshObstacle* obstacle)
	{
		auto it = std::find(obstacleList.begin(), obstacleList.end(), obstacle);
		if (it != obstacleList.end())
			obstacleList.erase(it);
	}

	void NavigationManager::setLoadedScene(std::string scene)
	{
		loadedScene = scene;

		if (!loadedScene.empty())
			loadNavMesh();
		else
			cleanup();
	}

	void NavigationManager::cleanup()
	{
		/* Cleanup */
		cachedVerticesCount = 0;
		dtFreeNavMesh(m_navMesh);
		dtFreeTileCache(m_tileCache);
		m_navMesh = nullptr;
		m_tileCache = nullptr;
		/* Cleanup */
	}

	static const int TILECACHESET_MAGIC = 'T' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'TSET';
	static const int TILECACHESET_VERSION = 1;

	struct TileCacheSetHeader
	{
		int magic;
		int version;
		int numTiles;
		dtNavMeshParams meshParams;
		dtTileCacheParams cacheParams;
	};

	struct TileCacheTileHeader
	{
		dtCompressedTileRef tileRef;
		int dataSize;
	};

	void NavigationManager::saveAll(const char* path)
	{
		if (!m_tileCache) return;

		FILE* fp = fopen(path, "wb");
		if (!fp)
			return;

		// Store header.
		TileCacheSetHeader header;
		header.magic = TILECACHESET_MAGIC;
		header.version = TILECACHESET_VERSION;
		header.numTiles = 0;
		for (int i = 0; i < m_tileCache->getTileCount(); ++i)
		{
			const dtCompressedTile* tile = m_tileCache->getTile(i);
			if (!tile || !tile->header || !tile->dataSize) continue;
			header.numTiles++;
		}
		memcpy(&header.cacheParams, m_tileCache->getParams(), sizeof(dtTileCacheParams));
		memcpy(&header.meshParams, m_navMesh->getParams(), sizeof(dtNavMeshParams));
		fwrite(&header, sizeof(TileCacheSetHeader), 1, fp);

		// Store tiles.
		for (int i = 0; i < m_tileCache->getTileCount(); ++i)
		{
			const dtCompressedTile* tile = m_tileCache->getTile(i);
			if (!tile || !tile->header || !tile->dataSize) continue;

			TileCacheTileHeader tileHeader;
			tileHeader.tileRef = m_tileCache->getTileRef(tile);
			tileHeader.dataSize = tile->dataSize;
			fwrite(&tileHeader, sizeof(tileHeader), 1, fp);

			fwrite(tile->data, tile->dataSize, 1, fp);
		}

		fclose(fp);
	}

	void NavigationManager::loadAll(const char* path)
	{
		FILE* fp = fopen(path, "rb");
		if (!fp) return;

		// Read header.
		TileCacheSetHeader header;
		size_t headerReadReturnCode = fread(&header, sizeof(TileCacheSetHeader), 1, fp);
		if (headerReadReturnCode != 1)
		{
			// Error or early EOF
			fclose(fp);
			return;
		}
		if (header.magic != TILECACHESET_MAGIC)
		{
			fclose(fp);
			return;
		}
		if (header.version != TILECACHESET_VERSION)
		{
			fclose(fp);
			return;
		}

		m_navMesh = dtAllocNavMesh();
		if (!m_navMesh)
		{
			fclose(fp);
			return;
		}

		dtStatus status = m_navMesh->init(&header.meshParams);
		if (dtStatusFailed(status))
		{
			fclose(fp);
			return;
		}

		m_tileCache = dtAllocTileCache();
		if (!m_tileCache)
		{
			fclose(fp);
			return;
		}

		m_tmproc->init(nullptr);

		status = m_tileCache->init(&header.cacheParams, m_talloc, m_tcomp, m_tmproc);
		if (dtStatusFailed(status))
		{
			fclose(fp);
			return;
		}

		// Read tiles.
		for (int i = 0; i < header.numTiles; ++i)
		{
			TileCacheTileHeader tileHeader;
			size_t tileHeaderReadReturnCode = fread(&tileHeader, sizeof(tileHeader), 1, fp);
			if (tileHeaderReadReturnCode != 1)
			{
				// Error or early EOF
				fclose(fp);
				return;
			}
			if (!tileHeader.tileRef || !tileHeader.dataSize)
				break;

			unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
			if (!data) break;
			memset(data, 0, tileHeader.dataSize);
			size_t tileDataReadReturnCode = fread(data, tileHeader.dataSize, 1, fp);
			if (tileDataReadReturnCode != 1)
			{
				// Error or early EOF
				dtFree(data);
				fclose(fp);
				return;
			}

			dtCompressedTileRef tile = 0;
			dtStatus addTileStatus = m_tileCache->addTile(data, tileHeader.dataSize, DT_COMPRESSEDTILE_FREE_DATA, &tile);
			if (dtStatusFailed(addTileStatus))
			{
				dtFree(data);
			}

			if (tile)
			{
				m_tileCache->buildNavMeshTile(tile, m_navMesh);
			}
		}

		fclose(fp);

		delete m_navQuery;
		m_navQuery = new dtNavMeshQuery();
		status = m_navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{
			fclose(fp);
			return;
		}
	}

	void NavigationManager::loadAllFromBuffer(char* buf, size_t bufSize)
	{
		boost::iostreams::stream<boost::iostreams::array_source> is(buf, bufSize);

		// Read header.
		TileCacheSetHeader header;
		is.read((char*)&header, sizeof(TileCacheSetHeader));

		if (header.magic != TILECACHESET_MAGIC)
		{
			is.close();
			delete[] buf;
			return;
		}

		if (header.version != TILECACHESET_VERSION)
		{
			is.close();
			delete[] buf;
			return;
		}

		m_navMesh = dtAllocNavMesh();
		if (!m_navMesh)
		{
			is.close();
			delete[] buf;
			return;
		}

		dtStatus status = m_navMesh->init(&header.meshParams);
		if (dtStatusFailed(status))
		{
			is.close();
			delete[] buf;
			return;
		}

		m_tileCache = dtAllocTileCache();
		if (!m_tileCache)
		{
			is.close();
			delete[] buf;
			return;
		}

		m_tmproc->init(nullptr);

		status = m_tileCache->init(&header.cacheParams, m_talloc, m_tcomp, m_tmproc);
		if (dtStatusFailed(status))
		{
			is.close();
			delete[] buf;
			return;
		}

		// Read tiles.
		for (int i = 0; i < header.numTiles; ++i)
		{
			TileCacheTileHeader tileHeader;

			is.read((char*)&tileHeader, sizeof(tileHeader));

			if (!tileHeader.tileRef || !tileHeader.dataSize)
				break;

			unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
			if (!data) break;
			memset(data, 0, tileHeader.dataSize);

			is.read((char*)data, tileHeader.dataSize);

			dtCompressedTileRef tile = 0;
			dtStatus addTileStatus = m_tileCache->addTile(data, tileHeader.dataSize, DT_COMPRESSEDTILE_FREE_DATA, &tile);
			if (dtStatusFailed(addTileStatus))
			{
				dtFree(data);
			}

			if (tile)
			{
				m_tileCache->buildNavMeshTile(tile, m_navMesh);
			}
		}

		delete m_navQuery;
		m_navQuery = new dtNavMeshQuery();
		status = m_navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{
			is.close();
			delete[] buf;
			return;
		}

		is.close();
		delete[] buf;
	}

	void NavigationManager::resetToDefault()
	{
		walkableSlopeAngle = 45;
		walkableHeight = 0.8;
		walkableClimb = 1.0;
		walkableRadius = 0.35;
		maxEdgeLen = 12;
		maxSimplificationError = 1.5;
		minRegionArea = 1.0;
		mergeRegionArea = 10.0;
		cellSize = 0.30;
		cellHeight = 0.10;
	}

	std::vector<glm::vec3>& NavigationManager::getNavMeshVertices()
	{
		//Copy navigation mesh
		if (m_tileCache != nullptr && m_navMesh != nullptr)
		{
			dtTileCacheAlloc* talloc = m_tileCache->getAlloc();
			dtTileCacheCompressor* tcomp = m_tileCache->getCompressor();
			const dtTileCacheParams* params = m_tileCache->getParams();

			int sz = 0;
			for (int i = 0; i < m_navMesh->getMaxTiles(); ++i)
			{
				const dtMeshTile* tile = m_navMesh->getTile(i);
				if (!tile->header) continue;
				dtPolyRef base = m_navMesh->getPolyRefBase(tile);

				for (int j = 0; j < tile->header->polyCount; ++j)
				{
					const dtPoly* p = &tile->polys[j];
					const unsigned int ip = (unsigned int)(p - tile->polys);
					const dtPolyDetail* pd = &tile->detailMeshes[ip];
					sz += pd->triCount * 3;
				}
			}

			if (cachedVerticesCount != sz)
			{
				cachedVerticesCount = sz;
				navMeshVertices.resize(sz);

				int ii = 0;
				for (int i = 0; i < m_navMesh->getMaxTiles(); ++i)
				{
					const dtMeshTile* tile = m_navMesh->getTile(i);
					if (!tile->header) continue;
					dtPolyRef base = m_navMesh->getPolyRefBase(tile);

					for (int j = 0; j < tile->header->polyCount; ++j)
					{
						const dtPoly* p = &tile->polys[j];
						//if ((p->flags & SAMPLE_POLYFLAGS_DISABLED) == 0) continue;

						const unsigned int ip = (unsigned int)(p - tile->polys);
						const dtPolyDetail* pd = &tile->detailMeshes[ip];

						for (int k = 0; k < pd->triCount; ++k)
						{
							const unsigned char* t = &tile->detailTris[(pd->triBase + k) * 4];
							for (int l = 0; l < 3; ++l)
							{
								if (t[l] < p->vertCount)
									navMeshVertices[ii] = glm::make_vec3(&tile->verts[p->verts[t[l]] * 3]);
								else
									navMeshVertices[ii] = glm::make_vec3(&tile->detailVerts[(pd->vertBase + t[l] - p->vertCount) * 3]);

								++ii;
							}
						}
					}
				}
			}
		}
		else
			navMeshVertices.clear();

		return navMeshVertices;
	}
}