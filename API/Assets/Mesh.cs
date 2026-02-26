using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Mesh : Asset, IDisposable
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern Mesh();

        /// <summary>
        /// Get sub mesh count
        /// </summary>
        public int subMeshCount { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Get sub mesh by index
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern SubMesh GetSubMesh(int index);

        /// <summary>
        /// Add sub mesh
        /// </summary>
        /// <param name="subMesh"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void AddSubMesh(SubMesh subMesh);

        /// <summary>
        /// Remove sub mesh by pointer
        /// </summary>
        /// <param name="subMesh"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void RemoveSubMesh(SubMesh subMesh);

        /// <summary>
        /// Remove sub mesh by index
        /// </summary>
        /// <param name="index"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void RemoveSubMesh(int index);

        /// <summary>
        /// Recalculate mesh bounds
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void RecalculateBounds();

        /// <summary>
        /// Apply changes made in runtime for all sub meshes
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void Commit();

        /// <summary>
        /// Free internal unmanaged resources. Use it for procedurally generated meshes at runtime
        /// </summary>
        public void Dispose()
        {
            Destroy();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void Destroy();
    }
}
