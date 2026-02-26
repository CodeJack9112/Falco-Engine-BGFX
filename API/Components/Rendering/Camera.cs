using System.Runtime.CompilerServices;

namespace FalcoEngine
{
    public class Camera : Component
    {
        /// <summary>
        /// GBufferTexture enum
        /// </summary>
        public enum GBufferTexture
        {
            Diffuse,
            Normals,
            MetallicRoughnessAOSpecular,
            Lightmap,
            Depth
        }

        internal Camera()
        {

        }

        /// <summary>
        /// Get or set the render texture as a render target
        /// </summary>
        public RenderTexture renderTarget { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Convert world to screen point
        /// </summary>
        /// <param name="world"></param>
        /// <returns></returns>
        public Vector3 WorldToScreenPoint(Vector3 world)
        {
            INTERNAL_worldToScreenPoint(ref world, out Vector3 screen);

            return screen;
        }

        /// <summary>
        /// Convert screen to world point
        /// </summary>
        /// <param name="screen"></param>
        /// <returns></returns>
        public Vector3 ScreenToWorldPoint(Vector3 screen)
        {
            INTERNAL_screenToWorldPoint(ref screen, out Vector3 world);

            return world;
        }

        /// <summary>
        /// Returns a view matrix of this camera
        /// </summary>
        /// <param name="ownFrustumOnly"></param>
        /// <returns></returns>
        public Matrix4 GetViewMatrix(bool ownFrustumOnly = false)
        {
            INTERNAL_getViewMatrix(ownFrustumOnly, out Matrix4 matrix);
            return matrix;
        }

        /// <summary>
        /// Returns a projection matrix of this camera
        /// </summary>
        /// <returns></returns>
        public Matrix4 GetProjectionMatrix()
        {
            INTERNAL_getProjectionMatrix(out Matrix4 matrix);
            return matrix;
        }

        /// <summary>
        /// Checks if the game object is in field of view of this camera
        /// </summary>
        /// <param name="object"></param>
        /// <returns></returns>
        public bool IsObjectVisible(GameObject @object)
        {
            return INTERNAL_isObjectVisible(@object);
        }

        /// <summary>
        /// The distance of the near clipping plane from the camera, in world units
        /// </summary>
        public float nearClipDistance { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// The distance of the far clipping plane from the camera, in world units
        /// </summary>
        public float farClipDistance { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Camera FOV angle (in degrees)
        /// </summary>
        public float fov { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Get GBuffer texture from channel pointed by textureType
        /// </summary>
        /// <param name="textureType"></param>
        /// <returns></returns>
        public Texture GetGBufferTexture(GBufferTexture textureType)
        {
            return INTERNAL_GetGBufferTexture((int)textureType);
        }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getViewMatrix(bool ownFrustumOnly, out Matrix4 matrix);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getProjectionMatrix(out Matrix4 matrix);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_worldToScreenPoint(ref Vector3 world, out Vector3 screen);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_screenToWorldPoint(ref Vector3 screen, out Vector3 world);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern bool INTERNAL_isObjectVisible(GameObject @object);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern Texture INTERNAL_GetGBufferTexture(int textureType);
    }
}
