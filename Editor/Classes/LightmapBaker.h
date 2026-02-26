#pragma once

#include <string>
#include <vector>
#include <functional>

#include "../Engine/glm/vec3.hpp"
#include "../Engine/glm/vec2.hpp"

#include "../Engine/Renderer/BatchedGeometry.h"
#include "../Engine/Renderer/CSGGeometry.h"

namespace GX
{
	class Light;
	class MeshRenderer;
	class SubMesh;
	class Texture;

	enum class GIQuality
	{
		Low,
		Normal,
		High,
		VeryHigh
	};

	class LightmapID;

	class LightmapBaker
	{
	public:
		struct PixelInfo
		{
		public:
			PixelInfo() {}
			PixelInfo(float _r, float _g, float _b, float _a) { r = _r; g = _g, b = _b, a = _a; }

			float r = 0;
			float g = 0;
			float b = 0;
			float a = 0;

			PixelInfo operator+(PixelInfo lhs)
			{
				return PixelInfo(r + lhs.r, g + lhs.g, b + lhs.b, a + lhs.a);
			}

			PixelInfo operator-(PixelInfo lhs)
			{
				return PixelInfo(r - lhs.r, g - lhs.g, b - lhs.b, a - lhs.a);
			}

			PixelInfo& operator+=(const PixelInfo& lhs)
			{
				this->r += lhs.r;
				this->g += lhs.g;
				this->b += lhs.b;
				this->a += lhs.a;

				return *this;
			}

			PixelInfo& operator+=(const float& lhs)
			{
				this->r += lhs;
				this->g += lhs;
				this->b += lhs;
				this->a += lhs;

				return *this;
			}

			PixelInfo operator-=(const PixelInfo& lhs)
			{
				this->r -= lhs.r;
				this->g -= lhs.g;
				this->b -= lhs.b;
				this->a -= lhs.a;

				return *this;
			}

			PixelInfo operator*=(const PixelInfo& lhs)
			{
				this->r *= lhs.r;
				this->g *= lhs.g;
				this->b *= lhs.b;
				this->a *= lhs.a;

				return *this;
			}

			PixelInfo operator/=(const PixelInfo& lhs)
			{
				this->r /= lhs.r;
				this->g /= lhs.g;
				this->b /= lhs.b;
				this->a /= lhs.a;

				return *this;
			}

			PixelInfo operator*=(float lhs)
			{
				this->r *= lhs;
				this->g *= lhs;
				this->b *= lhs;
				this->a *= lhs;

				return *this;
			}

			PixelInfo operator/=(float lhs)
			{
				this->r /= lhs;
				this->g /= lhs;
				this->b /= lhs;
				this->a /= lhs;

				return *this;
			}

			PixelInfo operator*(PixelInfo lhs)
			{
				return PixelInfo(r * lhs.r, g * lhs.g, b * lhs.b, a * lhs.a);
			}

			PixelInfo operator/(PixelInfo lhs)
			{
				return PixelInfo(r / lhs.r, g / lhs.g, b / lhs.b, a / lhs.a);
			}

			PixelInfo operator*(float lhs)
			{
				return PixelInfo(r * lhs, g * lhs, b * lhs, a * lhs);
			}

			PixelInfo operator/(float lhs)
			{
				return PixelInfo(r / lhs, g / lhs, b / lhs, a / lhs);
			}

			bool operator<(const PixelInfo& rhs)
			{
				return this->r < rhs.r &&
					this->g < rhs.g &&
					this->b < rhs.b &&
					this->a < rhs.a;
			}

			bool operator>(const PixelInfo& rhs)
			{
				return this->r > rhs.r &&
					this->g > rhs.g &&
					this->b > rhs.b &&
					this->a > rhs.a;
			}
		};

		LightmapBaker();
		~LightmapBaker();

		static bool giBake;
		static int giBounces;
		static float giIntensity;
		static GIQuality giQuality;

		static void bakeAll(std::function<void(float p, std::string text)> cb);

		static void directTriangle(const glm::vec3& P1, const glm::vec3& P2, const glm::vec3& P3,
			const glm::vec3& N1, const glm::vec3& N2, const glm::vec3& N3,
			const glm::vec2& T1, const glm::vec2& T2, const glm::vec2& T3,
			const glm::vec2& T11, const glm::vec2& T22, const glm::vec2& T33,
			int textureSize, PixelInfo* m_LightMapDirect, PixelInfo* m_LightMapIndirect, std::vector<Light*>& lights,
			Material* material);

		static int getLightmapSize() { return lightmapSize; }
		static void setLightmapSize(int value) { lightmapSize = value; }

		static bool getGIBake() { return giBake; }
		static void setGIBake(bool value) { giBake = value; }

		static int getGIBounces() { return giBounces; }
		static void setGIBounces(int value) { giBounces = value; }

		static float getGIIntensity() { return giIntensity; }
		static void setGIIntensity(float value) { giIntensity = value; }

		static GIQuality getGIQuality() { return giQuality; }
		static void setGIQuality(GIQuality value) { giQuality = value; }

	private:
		static int lightmapSize;

		struct TextureCache
		{
		public:
			Texture* texture = nullptr;
			void* handle = nullptr;
		};

		static std::vector<std::pair<int, int>> m_SearchPattern;
		static std::vector<LightmapID*> m_lightMaps;
		static std::vector<TextureCache> textureCache;

		static LightmapID* getLightmapID(MeshRenderer* renderer, int subMeshIndex);
		static LightmapID* getLightmapID(BatchedGeometry::Batch* batch);
		static LightmapID* getLightmapID(CSGGeometry::SubMesh* csgSubMesh);

		static int getLightmapSize(int value);
		static float getTriangleArea(const glm::vec3& P1, const glm::vec3& P2, const glm::vec3& P3);
		static void fillInvalidPixels(int textureSize, PixelInfo* m_LightMap);
		static void buildSearchPattern();
		static int getPixelCoordinate(float textureCoord, int texSize);
		static float getTextureCoordinate(int iPixelCoord, int texSize);
		static glm::vec2 worldToUV(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p, const glm::vec2& t1, const glm::vec2& t2, const glm::vec2& t3);
		static glm::vec3 getBarycentricCoordinates(const glm::vec2& P1, const glm::vec2& P2, const glm::vec2& P3, const glm::vec2& P);
		static PixelInfo getDirectIntensity(const glm::vec3& Position, const glm::vec3& Normal, std::vector<Light*>& lights, int textureSize, Material* material, PixelInfo pixelColor);
		static void indirectLight(const glm::vec3& Position, const glm::vec3& Normal, const glm::vec3& Direction, const PixelInfo& LightColor, Material* material);
		static void blurLightmap(PixelInfo* lightmap, int texSize, float radius);
		static void clampColor(PixelInfo& pixel);
		static PixelInfo fetchColor(Texture* tex, int tx, int ty);
	};

	class LightmapID
	{
	public:
		LightmapBaker::PixelInfo* directData = nullptr;
		LightmapBaker::PixelInfo* indirectData = nullptr;
		MeshRenderer* renderer = nullptr;
		BatchedGeometry::Batch* batch = nullptr;
		CSGGeometry::SubMesh* csgSubMesh = nullptr;
		int subMeshIndex = 0;
		int texSize = 0;
		std::string guid = "";
	};
}