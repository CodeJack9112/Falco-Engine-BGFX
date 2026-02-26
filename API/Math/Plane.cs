using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FalcoEngine
{
    public struct Plane
    {
        internal const int size = 16;

        private Vector3 m_Normal;

        private float m_Distance;

        /// <summary>
        ///   <para>Normal vector of the plane.</para>
        /// </summary>
        public Vector3 normal
        {
            get
            {
                return m_Normal;
            }
            set
            {
                m_Normal = value;
            }
        }

        /// <summary>
        ///   <para>Distance from the origin to the plane.</para>
        /// </summary>
        public float distance
        {
            get
            {
                return m_Distance;
            }
            set
            {
                m_Distance = value;
            }
        }

        /// <summary>
        ///   <para>Returns a copy of the plane that faces in the opposite direction.</para>
        /// </summary>
        public Plane flipped => new Plane(-m_Normal, 0f - m_Distance);

        /// <summary>
        ///   <para>Creates a plane.</para>
        /// </summary>
        /// <param name="inNormal"></param>
        /// <param name="inPoint"></param>
        public Plane(Vector3 inNormal, Vector3 inPoint)
        {
            m_Normal = Vector3.Normalize(inNormal);
            m_Distance = 0f - Vector3.Dot(m_Normal, inPoint);
        }

        /// <summary>
        ///   <para>Creates a plane.</para>
        /// </summary>
        /// <param name="inNormal"></param>
        /// <param name="d"></param>
        public Plane(Vector3 inNormal, float d)
        {
            m_Normal = Vector3.Normalize(inNormal);
            m_Distance = d;
        }

        /// <summary>
        ///   <para>Creates a plane.</para>
        /// </summary>
        /// <param name="a"></param>
        /// <param name="b"></param>
        /// <param name="c"></param>
        public Plane(Vector3 a, Vector3 b, Vector3 c)
        {
            m_Normal = Vector3.Normalize(Vector3.Cross(b - a, c - a));
            m_Distance = 0f - Vector3.Dot(m_Normal, a);
        }

        /// <summary>
        ///   <para>Sets a plane using a point that lies within it along with a normal to orient it.</para>
        /// </summary>
        /// <param name="inNormal">The plane's normal vector.</param>
        /// <param name="inPoint">A point that lies on the plane.</param>
        public void SetNormalAndPosition(Vector3 inNormal, Vector3 inPoint)
        {
            m_Normal = Vector3.Normalize(inNormal);
            m_Distance = 0f - Vector3.Dot(inNormal, inPoint);
        }

        /// <summary>
        ///   <para>Sets a plane using three points that lie within it.  The points go around clockwise as you look down on the top surface of the plane.</para>
        /// </summary>
        /// <param name="a">First point in clockwise order.</param>
        /// <param name="b">Second point in clockwise order.</param>
        /// <param name="c">Third point in clockwise order.</param>
        public void Set3Points(Vector3 a, Vector3 b, Vector3 c)
        {
            m_Normal = Vector3.Normalize(Vector3.Cross(b - a, c - a));
            m_Distance = 0f - Vector3.Dot(m_Normal, a);
        }

        /// <summary>
        ///   <para>Makes the plane face in the opposite direction.</para>
        /// </summary>
        public void Flip()
        {
            m_Normal = -m_Normal;
            m_Distance = 0f - m_Distance;
        }

        /// <summary>
        ///   <para>Moves the plane in space by the translation vector.</para>
        /// </summary>
        /// <param name="translation">The offset in space to move the plane with.</param>
        public void Translate(Vector3 translation)
        {
            m_Distance += Vector3.Dot(m_Normal, translation);
        }

        /// <summary>
        ///   <para>Returns a copy of the given plane that is moved in space by the given translation.</para>
        /// </summary>
        /// <param name="plane">The plane to move in space.</param>
        /// <param name="translation">The offset in space to move the plane with.</param>
        /// <returns>
        ///   <para>The translated plane.</para>
        /// </returns>
        public static Plane Translate(Plane plane, Vector3 translation)
        {
            return new Plane(plane.m_Normal, plane.m_Distance += Vector3.Dot(plane.m_Normal, translation));
        }

        /// <summary>
        ///   <para>For a given point returns the closest point on the plane.</para>
        /// </summary>
        /// <param name="point">The point to project onto the plane.</param>
        /// <returns>
        ///   <para>A point on the plane that is closest to point.</para>
        /// </returns>
        public Vector3 ClosestPointOnPlane(Vector3 point)
        {
            float d = Vector3.Dot(m_Normal, point) + m_Distance;
            return point - m_Normal * d;
        }

        /// <summary>
        ///   <para>Returns a signed distance from plane to point.</para>
        /// </summary>
        /// <param name="point"></param>
        public float GetDistanceToPoint(Vector3 point)
        {
            return Vector3.Dot(m_Normal, point) + m_Distance;
        }

        /// <summary>
        ///   <para>Is a point on the positive side of the plane?</para>
        /// </summary>
        /// <param name="point"></param>
        public bool GetSide(Vector3 point)
        {
            return Vector3.Dot(m_Normal, point) + m_Distance > 0f;
        }

        /// <summary>
        ///   <para>Are two points on the same side of the plane?</para>
        /// </summary>
        /// <param name="inPt0"></param>
        /// <param name="inPt1"></param>
        public bool SameSide(Vector3 inPt0, Vector3 inPt1)
        {
            float distanceToPoint = GetDistanceToPoint(inPt0);
            float distanceToPoint2 = GetDistanceToPoint(inPt1);
            return (distanceToPoint > 0f && distanceToPoint2 > 0f) || (distanceToPoint <= 0f && distanceToPoint2 <= 0f);
        }

        public bool Raycast(Ray ray, out float enter)
        {
            float num = Vector3.Dot(ray.direction, m_Normal);
            float num2 = 0f - Vector3.Dot(ray.origin, m_Normal) - m_Distance;
            if (Mathf.Approximately(num, 0f))
            {
                enter = 0f;
                return false;
            }
            enter = num2 / num;
            return enter > 0f;
        }

        public override string ToString()
        {
            return String.Format("(normal:({0:F1}, {1:F1}, {2:F1}), distance:{3:F1})", m_Normal.x, m_Normal.y, m_Normal.z, m_Distance);
        }

        public string ToString(string format)
        {
            return String.Format("(normal:({0}, {1}, {2}), distance:{3})", m_Normal.x.ToString(format), m_Normal.y.ToString(format), m_Normal.z.ToString(format), m_Distance.ToString(format));
        }
    }
}
