using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Transform : Component
    {
        internal Transform()
        {

        }

        private IntPtr native_ptr = (IntPtr)0;

        /*----------- PUBLIC ------------*/

        /// <summary>
        /// The world space position of the transform
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
        /// A Quaternion that stores the rotation of the transform in world space
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
        /// The global scale of the object
        /// </summary>
        public Vector3 scale
        {
            get
            {
                INTERNAL_get_scale(out Vector3 value);
                return value;
            }
            set
            {
                INTERNAL_set_scale(ref value);
            }
        }

        /// <summary>
        /// Position of the transform relative to the parent transform.
        /// </summary>
        public Vector3 localPosition
        {
            get
            {
                INTERNAL_get_local_position(out Vector3 value);
                return value;
            }
            set
            {
                INTERNAL_set_local_position(ref value);
            }
        }

        /// <summary>
        /// The rotation of the transform relative to the transform rotation of the parent
        /// </summary>
        public Quaternion localRotation
        {
            get
            {
                INTERNAL_get_local_rotation(out Quaternion value);
                return value;
            }
            set
            {
                INTERNAL_set_local_rotation(ref value);
            }
        }

        /// <summary>
        /// The scale of the transform relative to the GameObjects parent.
        /// </summary>
        public Vector3 localScale
        {
            get
            {
                INTERNAL_get_local_scale(out Vector3 value);
                return value;
            }
            set
            {
                INTERNAL_set_local_scale(ref value);
            }
        }

        /// <summary>
        /// Returns a normalized vector representing the blue axis of the transform in world space
        /// </summary>
        public Vector3 forward
        {
            get
            {
                INTERNAL_get_forward(out Vector3 value);
                return value;
            }
        }

        /// <summary>
        /// The green axis of the transform in world space
        /// </summary>
        public Vector3 up
        {
            get
            {
                INTERNAL_get_up(out Vector3 value);
                return value;
            }
        }

        /// <summary>
        /// The negative red axis of the transform in world space
        /// </summary>
        public Vector3 left
        {
            get
            {
                INTERNAL_get_left(out Vector3 value);
                return value;
            }
        }

        /// <summary>
        /// The red axis of the transform in world space
        /// </summary>
        public Vector3 right
        {
            get
            {
                INTERNAL_get_right(out Vector3 value);
                return value;
            }
        }

        /// <summary>
        /// The number of children the transform has
        /// </summary>
        public int childCount { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// The parent of the transform
        /// </summary>
        public Transform parent { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns a transform child by index
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        public Transform GetChild(int index)
        {
            return INTERNAL_get_child(index);
        }

        /// <summary>
        /// Finds a child by name and returns it
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public Transform FindChild(string name)
        {
            return INTERNAL_find_child(name);
        }

        /// <summary>
		///   <para>Transforms direction from local space to world space.</para>
		/// </summary>
		/// <param name="direction"></param>
		public Vector3 TransformDirection(Vector3 direction)
        {
            INTERNAL_TransformDirection(ref direction, out Vector3 ret);
            return ret;
        }

        /// <summary>
        ///   <para>Transforms direction x, y, z from local space to world space.</para>
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <param name="z"></param>
        public Vector3 TransformDirection(float x, float y, float z)
        {
            return TransformDirection(new Vector3(x, y, z));
        }

        /// <summary>
        ///   <para>Transforms a direction from world space to local space. The opposite of Transform.TransformDirection.</para>
        /// </summary>
        /// <param name="direction"></param>
        public Vector3 InverseTransformDirection(Vector3 direction)
        {
            INTERNAL_InverseTransformDirection(ref direction, out Vector3 ret);
            return ret;
        }

        /// <summary>
        ///   <para>Transforms the direction x, y, z from world space to local space. The opposite of Transform.TransformDirection.</para>
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <param name="z"></param>
        public Vector3 InverseTransformDirection(float x, float y, float z)
        {
            return InverseTransformDirection(new Vector3(x, y, z));
        }

        /// <summary>
        ///   <para>Transforms position from local space to world space.</para>
        /// </summary>
        /// <param name="position"></param>
        public Vector3 TransformPoint(Vector3 position)
        {
            INTERNAL_TransformPoint(ref position, out Vector3 ret);
            return ret;
        }

        /// <summary>
        ///   <para>Transforms the position x, y, z from local space to world space.</para>
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <param name="z"></param>
        public Vector3 TransformPoint(float x, float y, float z)
        {
            return TransformPoint(new Vector3(x, y, z));
        }

        /// <summary>
        ///   <para>Transforms position from world space to local space.</para>
        /// </summary>
        /// <param name="position"></param>
        public Vector3 InverseTransformPoint(Vector3 position)
        {
            INTERNAL_InverseTransformPoint(ref position, out Vector3 ret);
            return ret;
        }

        /// <summary>
        ///   <para>Transforms the position x, y, z from world space to local space. The opposite of Transform.TransformPoint.</para>
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <param name="z"></param>
        public Vector3 InverseTransformPoint(float x, float y, float z)
        {
            return InverseTransformPoint(new Vector3(x, y, z));
        }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_position(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_position(ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_scale(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_scale(ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_rotation(out Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_rotation(ref Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_local_position(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_local_position(ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_local_rotation(out Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_local_rotation(ref Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_local_scale(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_local_scale(ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern Transform INTERNAL_get_child(int index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern Transform INTERNAL_find_child(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_forward(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_up(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_left(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_get_right(out Vector3 value);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_TransformDirection(ref Vector3 direction, out Vector3 ret);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_InverseTransformDirection(ref Vector3 direction, out Vector3 ret);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_TransformPoint(ref Vector3 position, out Vector3 ret);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_InverseTransformPoint(ref Vector3 position, out Vector3 ret);
    }
}