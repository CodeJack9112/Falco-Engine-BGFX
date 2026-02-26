using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class SubMesh : Object, IDisposable
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern SubMesh();

        /// <summary>
        /// Get parent mesh
        /// </summary>
        public Mesh parent { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Indices array
        /// </summary>
        public int[] indices { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Vertices array
        /// </summary>
        public Vector3[] vertices { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Normals array
        /// </summary>
        public Vector3[] normals { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Tangents array
        /// </summary>
        public Vector3[] tangents { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Bitangents array
        /// </summary>
        public Vector3[] bitangents { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Texture coords array at channel 0
        /// </summary>
        public Vector2[] texcoords0 { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Texture coords array at channel 1. Used for lightmaps
        /// </summary>
        public Vector2[] texcoords1 { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Bones blend weights array. Used for GPU skinning
        /// </summary>
        public Vector4[] blendWeights { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Bones blend indices array. Used for GPU skinning
        /// </summary>
        public Vector4[] blendIndices { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Vertex colors array
        /// </summary>
        public Color[] colors { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Apply changes made in runtime
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
