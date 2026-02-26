using System.Runtime.CompilerServices;

namespace FalcoEngine
{
    public class ConeTwistJoint : Component
    {
        internal ConeTwistJoint() { }

        /// <summary>
        /// Get or set an anchor point relative to this object
        /// </summary>
        public Vector3 anchor
        {
            get
            {
                INTERNAL_get_anchor(out Vector3 value);
                return value;
            }
            set
            {
                INTERNAL_set_anchor(ref value);
            }
        }

        /// <summary>
        /// Get or set an anchor point relative to connected object
        /// </summary>
        public Vector3 connectedAnchor
        {
            get
            {
                INTERNAL_get_connectedAnchor(out Vector3 value);
                return value;
            }
            set
            {
                INTERNAL_set_connectedAnchor(ref value);
            }
        }

        /// <summary>
        /// Get or set the angle limits of this joint
        /// </summary>
        public Vector3 limits
        {
            get
            {
                INTERNAL_get_limits(out Vector3 value);
                return value;
            }
            set
            {
                INTERNAL_set_limits(ref value);
            }
        }

        /// <summary>
        /// Get or set the linked bodies collision flag
        /// </summary>
        public bool linkedBodiesCollision { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Get or set the object to connect with
        /// </summary>
        public Rigidbody connectedObject { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Auto configure connected anchor point depending on an anchor of this object
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void AutoConfigureAnchors();

        /// <summary>
        /// Rebuild this joint
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void Rebuild();

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_anchor(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_anchor(ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_connectedAnchor(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_connectedAnchor(ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_limits(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_limits(ref Vector3 value);
    }
}
