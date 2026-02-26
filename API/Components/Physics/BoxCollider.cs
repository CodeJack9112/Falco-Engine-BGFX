using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class BoxCollider : Collider
    {
        internal BoxCollider()
        {

        }

        /*----------- PUBLIC ------------*/

        /// <summary>
        /// Get or set the size of the box
        /// </summary>
        public Vector3 size
        {
            get
            {
                INTERNAL_get_size(out Vector3 value);
                return value;
            }
            set
            {
                INTERNAL_set_size(ref value);
            }
        }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_size(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_size(ref Vector3 value);
    }
}
