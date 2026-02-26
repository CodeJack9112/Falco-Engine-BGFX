using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FalcoEngine
{
    public struct RaycastHit
    {
        /// <summary>
        /// Is the ray has any intersection with geometry or not
        /// </summary>
        public bool hasHit;

        /// <summary>
        /// The rigidbody that was hit
        /// </summary>
        public Rigidbody rigidbody;

        /// <summary>
        /// The impact point in world space where the ray hit the collider
        /// </summary>
        public Vector3 hitPoint;

        /// <summary>
        /// The normal of the surface the ray hit
        /// </summary>
        public Vector3 worldNormal;
    }
}
