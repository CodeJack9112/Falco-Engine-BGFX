using System;
using System.Runtime.CompilerServices;

namespace FalcoEngine
{
    public class RenderTexture : Object, IDisposable
    {
        /// <summary>
        /// Render texture type
        /// </summary>
        public enum TextureType { ColorWithDepth, ColorOnly }

        internal RenderTexture() { }

        /// <summary>
        /// Create new render texture
        /// </summary>
        /// <param name="w">Width</param>
        /// <param name="h">Height</param>
        public RenderTexture(int w, int h)
        {
            INTERNAL_constructor(w, h, (int)TextureType.ColorWithDepth);
        }

        /// <summary>
        /// Create new render texture
        /// </summary>
        /// <param name="w">Width</param>
        /// <param name="h">Height</param>
        /// <param name="textureType">Texture type</param>
        public RenderTexture(int w, int h, TextureType textureType)
        {
            INTERNAL_constructor(w, h, (int)textureType);
        }

        /// <summary>
        /// Recreate this render texture with new width and height
        /// </summary>
        /// <param name="w">New width</param>
        /// <param name="h">New height</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void Reset(int w, int h);

        /// <summary>
        /// Get texture containing a color data
        /// </summary>
        public Texture colorTexture { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Get texture containing a depth data
        /// </summary>
        public Texture depthTexture { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Get width of this render texture
        /// </summary>
        int width { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Get height of this render texture
        /// </summary>
        int height { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_constructor(int w, int h, int textureType);

        /// <summary>
        /// Delete unmanaged resources for this render texture used by engine
        /// </summary>
        public void Dispose()
        {
            INTERNAL_dispose();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_dispose();
    }
}