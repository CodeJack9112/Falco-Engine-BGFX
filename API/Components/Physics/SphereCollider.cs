using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class SphereCollider : Collider
    {
        internal SphereCollider()
        {

        }

        /*----------- PUBLIC ------------*/

        /// <summary>
        /// Get or set the radius of the sphere
        /// </summary>
        public float radius { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }
    }
}
