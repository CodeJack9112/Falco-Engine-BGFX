using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FalcoEngine
{
    public struct Ray
    {
        private Vector3 m_Origin;

        private Vector3 m_Direction;

        /// <summary>
        ///   <para>The origin point of the ray.</para>
        /// </summary>
        public Vector3 origin
        {
            get
            {
                return m_Origin;
            }
            set
            {
                m_Origin = value;
            }
        }

        /// <summary>
        ///   <para>The direction of the ray.</para>
        /// </summary>
        public Vector3 direction
        {
            get
            {
                return m_Direction;
            }
            set
            {
                m_Direction = value.normalized;
            }
        }

        /// <summary>
        ///   <para>Creates a ray starting at origin along direction.</para>
        /// </summary>
        /// <param name="origin"></param>
        /// <param name="direction"></param>
        public Ray(Vector3 origin, Vector3 direction)
        {
            m_Origin = origin;
            m_Direction = direction.normalized;
        }

        /// <summary>
        ///   <para>Returns a point at distance units along the ray.</para>
        /// </summary>
        /// <param name="distance"></param>
        public Vector3 GetPoint(float distance)
        {
            return m_Origin + m_Direction * distance;
        }

        /// <summary>
        ///   <para>Returns a nicely formatted string for this ray.</para>
        /// </summary>
        /// <param name="format"></param>
        public override string ToString()
        {
            return String.Format("Origin: {0}, Dir: {1}", m_Origin, m_Direction);
        }

        /// <summary>
        ///   <para>Returns a nicely formatted string for this ray.</para>
        /// </summary>
        /// <param name="format"></param>
        public string ToString(string format)
        {
            return String.Format("Origin: {0}, Dir: {1}", m_Origin.ToString(format), m_Direction.ToString(format));
        }
    }

}
