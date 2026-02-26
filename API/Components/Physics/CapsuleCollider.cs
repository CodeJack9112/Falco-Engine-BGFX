using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class CapsuleCollider : Collider
    {
        internal CapsuleCollider()
        {

        }

        /*----------- PUBLIC ------------*/
        
        /// <summary>
        /// Get or set the radiuse of the capsule
        /// </summary>
        public float radius { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Get or set the height of the capsule
        /// </summary>
        public float height { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }
    }
}
