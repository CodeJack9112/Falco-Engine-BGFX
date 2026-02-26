
/*-------------------------------*/
/*-------------------------------*/
/*------Falco Engine C# API------*/
/*-------------------------------*/
/*-------------------------------*/
/*-------Author: tracer0707------*/
/*------------2018-2019----------*/
/*-------------------------------*/
/*-------------------------------*/

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public abstract class MonoBehaviour : Component
    {
        /*------------PUBLIC FIELDS------------*/
        /*-------------------------------------*/
        protected MonoBehaviour()
        {

        }

        /// <summary>
        /// Game object's name
        /// </summary>
        public string name { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Game object's unique name (UID)
        /// </summary>
        public string uniqueName { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Transform component
        /// </summary>
        public Transform transform { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Rigidbody component
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
        /// Find object of type
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public static T FindObjectOfType<T>(bool findInactive = true)
        {
            return INTERNAL_find_object_of_type_t<T>(typeof(T), findInactive);
        }

        /// <summary>
        /// Find objects of type
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public static T[] FindObjectsOfType<T>(bool findInactive = true)
        {
            return INTERNAL_find_objects_of_type_t<T>(typeof(T), findInactive);
        }

        /// <summary>
        /// Destroy game object
        /// </summary>
        /// <param name="gameObject"></param>
        public void Destroy(GameObject gameObject)
        {
            INTERNAL_destroy_game_object(gameObject);
        }

        /// <summary>
        /// Destroy component
        /// </summary>
        /// <param name="component"></param>
        public void Destroy(Component component)
        {
            INTERNAL_destroy_component(component);
        }

        /*-----------INTERNAL CALLS------------*/
        /*-------------------------------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern T INTERNAL_get_component_t<T>(Type type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern T INTERNAL_find_object_of_type_t<T>(Type type, bool findInactive);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern T[] INTERNAL_find_objects_of_type_t<T>(Type type, bool findInactive);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_destroy_game_object(GameObject gameObject);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_destroy_component(Component component);
    }
}
