using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class MeshRenderer : Component
    {
        internal MeshRenderer() { }

        /// <summary>
        /// Returns number of the unique materials used by the mesh
        /// </summary>
        public int materialsCount { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Returns number of the shared materials used by the mesh
        /// </summary>
        public int sharedMaterialsCount { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Get or set the mesh for rendering
        /// </summary>
        public Mesh mesh { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Get unique material by index
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern Material GetMaterial(int index);

        /// <summary>
        /// Get shared material by index
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern Material GetSharedMaterial(int index);

        /// <summary>
        /// Set shared material at certain index
        /// </summary>
        /// <param name="index"></param>
        /// <param name="material"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void SetSharedMaterial(int index, Material material);
    }
}
