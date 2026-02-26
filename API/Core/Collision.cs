using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FalcoEngine
{
    public struct Collision
    {
        /// <summary>
        /// The other rigidbody that this object collided with
        /// </summary>
        public Rigidbody other;

        /// <summary>
        /// Collision point in world space
        /// </summary>
        public Vector3 point;

        /// <summary>
        /// Collision normal in world space
        /// </summary>
        public Vector3 normal;
    }
}
