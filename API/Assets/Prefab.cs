using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Prefab : Asset
    {
        internal Prefab() { }

        /// <summary>
        /// Create game object from this prefab
        /// </summary>
        /// <returns>Created game object</returns>
        public GameObject Instantiate()
        {
            Vector3 pos = Vector3.zero;
            Quaternion rot = Quaternion.identity;

            return INTERNAL_instantiate(ref pos, ref rot);
        }

        /// <summary>
        /// Create game object from this prefab
        /// </summary>
        /// <param name="position">Set position</param>
        /// <returns>Created game object</returns>
        public GameObject Instantiate(Vector3 position)
        {
            Quaternion rot = Quaternion.identity;

            return INTERNAL_instantiate(ref position, ref rot);
        }

        /// <summary>
        /// Create game object from this prefab
        /// </summary>
        /// <param name="position">Set position</param>
        /// <param name="rotation">Set rotation</param>
        /// <returns>Created game object</returns>
        public GameObject Instantiate(Vector3 position, Quaternion rotation)
        {
            return INTERNAL_instantiate(ref position, ref rotation);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern GameObject INTERNAL_instantiate(ref Vector3 position, ref Quaternion rotation);
    }
}
