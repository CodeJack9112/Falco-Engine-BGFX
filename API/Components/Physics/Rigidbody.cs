using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Rigidbody : Component
    {
        internal Rigidbody()
        {

        }

        /*----------- PUBLIC ------------*/

        /// <summary>
        /// The position of the rigidbody
        /// </summary>
        public Vector3 position
        {
            get
            {
                INTERNAL_get_position(out Vector3 value);
                return value;
            }
            set
            {
                INTERNAL_set_position(ref value);
            }
        }

        /// <summary>
        /// The rotation of the rigidbody
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

        /// <summary>
        /// The velocity vector of the rigidbody
        /// It represents the rate of change of rigidbody position
        /// </summary>
        public Vector3 linearVelocity
        {
            get
            {
                INTERNAL_get_linear_velocity(out Vector3 value);
                return value;
            }
            set
            {
                INTERNAL_set_linear_velocity(ref value);
            }
        }

        /// <summary>
        /// The angular velocity vector of the rigidbody measured in radians per second
        /// </summary>
        public Vector3 angularVelocity
        {
            get
            {
                INTERNAL_get_angular_velocity(out Vector3 value);
                return value;
            }
            set
            {
                INTERNAL_set_angular_velocity(ref value);
            }
        }

        /// <summary>
        /// The friction of an object
        /// Usually a value from 0 to 1. A value of zero feels like ice, a value of 1 will make it very hard to get the object moving
        /// </summary>
        public float friction { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// How bouncy is the surface?
        /// A value of 0 will not bounce
        /// A value of 1 will bounce without any loss of energy, certain approximations are to be expected though that might add small amounts of energy to the simulation
        /// </summary>
        public float bounciness { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// The drag of the object
        /// </summary>
        public float linearDamping { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// The angular drag of the object
        /// </summary>
        public float angularDamping { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Controls whether physics affects the rigidbody
        /// </summary>
        public bool isKinematic
        {
            get
            {
                return INTERNAL_get_is_kinematic();
            }
            set
            {
                INTERNAL_set_is_kinematic(value);
            }
        }

        /// <summary>
        /// Static objects behave like a wall or a floor which can only affect the other objects and not moving
        /// </summary>
        public bool isStatic { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Is the collider a trigger?
        /// A trigger doesn't register a collision with an incoming rigidbody
        /// Instead, it sends OnTriggerEnter, OnTriggerExit and OnTriggerStay message when a rigidbody enters or exits the trigger volume.
        /// </summary>
        public bool isTrigger { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// The mass of the rigidbody
        /// </summary>
        public float mass
        {
            get
            {
                return INTERNAL_get_mass();
            }
            set
            {
                INTERNAL_set_mass(value);
            }
        }

        /// <summary>
        /// Adds a force to the rigidbody
        /// </summary>
        /// <param name="force"></param>
        public void AddForce(Vector3 force)
        {
            AddForce(force, Vector3.zero);
        }

        /// <summary>
        /// Adds a force to the rigidbody relative to its coordinate system
        /// </summary>
        /// <param name="force"></param>
        /// <param name="rel_pos"></param>
        public void AddForce(Vector3 force, Vector3 rel_pos)
        {
            INTERNAL_add_force(ref force, ref rel_pos);
        }

        /// <summary>
        /// Adds a torque to the rigidbody
        /// </summary>
        /// <param name="torque"></param>
        public void AddTorque(Vector3 torque)
        {
            INTERNAL_add_torque(ref torque);
        }

        /// <summary>
        /// Controls whether physics will change the position of the object
        /// </summary>
        /// <param name="freezeX"></param>
        /// <param name="freezeY"></param>
        /// <param name="freezeZ"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void SetFreezePosition(bool freezeX, bool freezeY, bool freezeZ);

        /// <summary>
        /// Controls whether physics will change the rotation of the object
        /// </summary>
        /// <param name="freezeX"></param>
        /// <param name="freezeY"></param>
        /// <param name="freezeZ"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void SetFreezeRotation(bool freezeX, bool freezeY, bool freezeZ);

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_position(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_position(ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_rotation(out Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_rotation(ref Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_linear_velocity(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_linear_velocity(ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_angular_velocity(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_angular_velocity(ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_add_force(ref Vector3 force, ref Vector3 rel_pos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_add_torque(ref Vector3 torque);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern bool INTERNAL_get_is_kinematic();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_is_kinematic(bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern float INTERNAL_get_mass();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_mass(float value);
    }
}
