using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class MeshCollider : Collider
    {
        internal MeshCollider()
        {

        }

        /// <summary>
        /// Enable or disable convex decomposition of the mesh collider
        /// </summary>
        public bool convex { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }
    }
}
