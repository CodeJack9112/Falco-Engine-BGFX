#include <vector>
#include <functional>
#include <map>

#include <bgfx/bgfx.h>

#include "../glm/vec3.hpp"
#include "../glm/mat4x4.hpp"

#include "Color.h"

namespace GX
{
	class Texture;
	class Camera;
	class Transform;
	class Mesh;
	class Material;

	class Primitives
	{
		friend class Renderer;

	private:
		struct PosTexCoord0Vertex
		{
			float m_x;
			float m_y;
			float m_z;
			float m_u;
			float m_v;
		};

		struct Sphere
		{
		public:
			struct Mesh
			{
				std::vector<glm::vec3> vertices;
				std::vector<glm::vec3> normals;
				std::vector<glm::vec2> texCoords;
				std::vector<uint32_t> indices;
			};
			
			static std::map<int, Mesh> meshes;

			static void build(int size);
		};

	public:
		static bool screenSpaceQuad();
		static void cube();
		static void billboard(Transform* trans, Texture * texture, Color color, float scale, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera * camera);
		static void mesh(glm::mat4x4 trans, Mesh * _mesh, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera * camera);
		static void mesh(glm::mat4x4 trans, Mesh * _mesh, Material* material, int viewId, uint64_t state, Camera * camera, std::function<void()> cb = nullptr);
		static void mesh(glm::mat4x4 trans, Mesh * _mesh, std::vector<Material*> materials, int viewId, uint64_t state, Camera * camera, int renderMode, std::function<void()> cb = nullptr, int lod = 0);
		static void mesh(glm::mat4x4 trans, std::vector<glm::vec3>& vertices, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera * camera);
		static void box(glm::mat4x4 trans, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
		static void skybox(glm::mat4x4 trans, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
		static void sphere(const glm::mat4x4& trans, int size, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
		static void circle(glm::mat4x4 trans, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
		static void circledSphere(glm::mat4x4 trans, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
		static void circledCapsule(glm::mat4x4 trans, float height, float radius, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
		static void circledCone(glm::mat4x4 trans, float range, float radius, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
		static void circledRays(glm::mat4x4 trans, float range, float radius, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
		static void cameraFrustum(glm::mat4x4 trans, Camera * frustum, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
		static void point(glm::mat4x4 trans, float size, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
		static void line(glm::mat4x4 trans, glm::vec3 p1, glm::vec3 p2, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
		static void plane(glm::mat4x4 trans, float width, float height, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
		static void grid(glm::mat4x4 trans, int cellCount, float cellSize, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera);
	};
}