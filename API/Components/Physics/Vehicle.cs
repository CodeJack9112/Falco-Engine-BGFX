using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;

namespace FalcoEngine
{
    public class Vehicle : Component
    {
        public struct Wheel
        {
            public float radius { get; set; }
            public float width { get; set; }
            public float suspensionStiffness { get; set; }
            public float suspensionDamping { get; set; }
            public float suspensionCompression { get; set; }
            public float suspensionRestLength { get; set; }
            public float friction { get; set; }
            public float rollInfluence { get; set; }
            public Vector3 direction { get; set; }
            public Vector3 axle { get; set; }
            public Vector3 connectionPoint { get; set; }
            public bool isFrontWheel { get; set; }
            public GameObject connectedObject { get; set; }
        }

        internal Vehicle()
        {

        }

        /// <summary>
        /// Gets the number of wheels attached to this vehicle
        /// </summary>
        public int numWheels { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Gets the current vehicle speed in kilometers per hour
        /// </summary>
        public float speedKMH { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Set the wheel steering angle in degrees
        /// </summary>
        /// <param name="angle"></param>
        /// <param name="wheelIndex"></param>
        public void SetSteering(float angle, int wheelIndex)
        {
            INTERNAL_setSteering(angle, wheelIndex);
        }

        /// <summary>
        /// Get the wheel steering angle in degrees
        /// </summary>
        /// <param name="wheelIndex"></param>
        /// <returns></returns>
        public float GetSteering(int wheelIndex)
        {
            return INTERNAL_getSteering(wheelIndex);
        }

        /// <summary>
        /// Set wheel's braking ratio
        /// </summary>
        /// <param name="value"></param>
        /// <param name="wheelIndex"></param>
        public void SetBreak(float value, int wheelIndex)
        {
            INTERNAL_setBreak(value, wheelIndex);
        }

        /// <summary>
        /// Apply engine force to the wheel
        /// </summary>
        /// <param name="value"></param>
        /// <param name="wheelIndex"></param>
        public void ApplyEngineForce(float value, int wheelIndex)
        {
            INTERNAL_applyEngineForce(value, wheelIndex);
        }

        //-------------- INTERNAL CALLS ----------------//

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setSteering(float angle, int wheel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern float INTERNAL_getSteering(int wheel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setBreak(float value, int wheel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_applyEngineForce(float value, int wheel);
    }
}
