using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Texture : Asset
    {
        /// <summary>
        /// Get texture width
        /// </summary>
        public int width { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Get texture height
        /// </summary>
        public int height { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Load existing texture
        /// </summary>
        /// <param name="name">Name of the existing texture</param>
        /// <returns>Loaded texture</returns>
        public static Texture Load(string name)
        {
            return INTERNAL_load(name);
        }

        /// <summary>
        /// Create texture from bytes containing RGBA data
        /// </summary>
        /// <param name="data">Byte array</param>
        /// <param name="width">Texture width</param>
        /// <param name="height">Texture height</param>
        /// <returns></returns>
        public static Texture FromBytesRGBA8(sbyte[] data, int width, int height)
        {
            return INTERNAL_fromBytesRGBA8(data, width, height);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Texture INTERNAL_load(string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Texture INTERNAL_fromBytesRGBA8(sbyte[] data, int width, int height);
    }
}
