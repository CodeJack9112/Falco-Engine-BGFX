using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;

namespace FalcoEngine
{
    public class Spline : Component
    {
        internal Spline()
        {

        }

        /*----------- PUBLIC ------------*/

        /// <summary>
        /// Get number of points in the spline
        /// </summary>
        public int numPoints { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Add point to the spline
        /// </summary>
        /// <param name="value"></param>
        public void AddPoint(Vector3 value)
        {
            INTERNAL_addPoint(ref value);
        }

        /// <summary>
        /// Remove point from the spline by index
        /// </summary>
        /// <param name="index"></param>
        public void RemovePoint(int index)
        {
            INTERNAL_removePoint(index);
        }

        /// <summary>
        /// Get point by index
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        public Vector3 GetPoint(int index)
        {
            INTERNAL_getPoint(index, out Vector3 value);
            return value;
        }

        /// <summary>
        /// Set point by index
        /// </summary>
        /// <param name="index"></param>
        /// <param name="value"></param>
        public void SetPoint(int index, Vector3 value)
        {
            INTERNAL_setPoint(index, ref value);
        }
        
        /// <summary>
        /// Get interpolated point on the spline at described position (0...1)
        /// </summary>
        /// <param name="t"></param>
        /// <returns></returns>
        public Vector3 GetSplinePoint(float t)
        {
            INTERNAL_getSplinePoint(t, out Vector3 value);
            return value;
        }

        /// <summary>
        /// Get direction on the spline at described position (0...1)
        /// </summary>
        /// <param name="t"></param>
        /// <returns></returns>
        public Vector3 GetSplineDirection(float t)
        {
            INTERNAL_getSplineDirection(t, out Vector3 value);
            return value;
        }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_addPoint(ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_removePoint(int index);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getPoint(int index, out Vector3 value);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setPoint(int index, ref Vector3 value);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getSplinePoint(float t, out Vector3 value);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getSplineDirection(float t, out Vector3 value);
    }
}
