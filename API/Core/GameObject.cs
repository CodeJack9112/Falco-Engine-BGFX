using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public class GameObject
    {
        private IntPtr this_ptr = (IntPtr)0;

        /*----------- PUBLIC ------------*/

        /// <summary>
        /// Create new empty game object
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern GameObject();

        /// <summary>
        /// Return game object name
        /// </summary>
        public string name { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Return game object unique name (UID)
        /// </summary>
        public string uniqueName { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Get or set game object enabled state
        /// </summary>
        public bool enabled { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Return true if game object enabled self and all of it's parents are enabled too
        /// </summary>
        public bool active { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Get or set game object tag
        /// </summary>
        public string tag { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Get or set game object layer
        /// </summary>
        public int layer { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Transform component
        /// </summary>
        public Transform transform { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// RigidBody component
        /// </summary>
        public Rigidbody rigidbody { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Animation component
        /// </summary>
        public Animation animation { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// AudioSource component
        /// </summary>
        public AudioSource audioSource { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// NavMeshAgent component
        /// </summary>
        public NavMeshAgent navMeshAgent { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Return an array of all game object's components
        /// </summary>
        public Component[] components { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Return component by type
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public T GetComponent<T>()
        {
            return INTERNAL_get_component_t<T>(typeof(T));
        }

        /// <summary>
        /// Create new component of type T and attach it to this game object
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public T AddComponent<T>()
        {
            return INTERNAL_add_component_t<T>(typeof(T));
        }

        /// <summary>
        /// Create a copy of this game object
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern GameObject Clone();

        /// <summary>
        /// Find game object by name. If scene has multiple game objects with the same name, the first will be returned
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static GameObject Find(string name)
        {
            return INTERNAL_find(name);
        }

        /// <summary>
        /// Return game object by it's unique name (UID)
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static GameObject GetByUniqueName(string name)
        {
            return INTERNAL_getByUniqueName(name);
        }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern GameObject INTERNAL_find(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern GameObject INTERNAL_getByUniqueName(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern T INTERNAL_get_component_t<T>(Type type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern T INTERNAL_add_component_t<T>(Type type);
    }
}
