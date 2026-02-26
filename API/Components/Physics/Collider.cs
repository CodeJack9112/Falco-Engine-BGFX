using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Collider : Component
    {
        internal Collider()
        {

        }

        /*----------- PUBLIC ------------*/

        /// <summary>
        /// Get or set the offset position from center of the game object
        /// </summary>
        public Vector3 offset
        {
            get
            {
                INTERNAL_get_offset(out Vector3 value);
                return value;
            }
            set
            {
                INTERNAL_set_offset(ref value);
            }
        }

        /// <summary>
        /// Get or set the offset rotation
        /// </summary>
        public Quaternion rotation
        {
            get
            {
                INTERNAL_get_rotation(out Quaternion value);
                return value;
            }
            set
            {
                INTERNAL_set_rotation(ref value);
            }
        }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_offset(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_offset(ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_rotation(out Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_rotation(ref Quaternion value);
    }
}
