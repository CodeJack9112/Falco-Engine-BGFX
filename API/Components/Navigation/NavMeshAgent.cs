using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class NavMeshAgent : Component
    {
        internal NavMeshAgent()
        {

        }

        /*----------- PUBLIC ------------*/

        /// <summary>
        /// Gets or attempts to set the destination of the agent in world-space units
        /// </summary>
        public Vector3 targetPosition
        {
            get
            {
                INTERNAL_getTargetPosition(out Vector3 position);
                return position;
            }
            set
            {
                INTERNAL_setTargetPosition(ref value);
            }
        }

        /// <summary>
        /// The avoidance radius for the agent
        /// This is the agent's space within which obstacles and other agents should not pass
        /// </summary>
        public float radius { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// The height of the agent for purposes of passing under obstacles, etc.
        /// </summary>
        public float height { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Maximum movement speed when following a path
        /// </summary>
        public float speed { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// The maximum acceleration of an agent as it follows a path
        /// </summary>
        public float acceleration { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getTargetPosition(out Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setTargetPosition(ref Vector3 position);
    }
}
