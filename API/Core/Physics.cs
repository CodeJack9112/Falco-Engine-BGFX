using System.Runtime.CompilerServices;

namespace FalcoEngine
{
    public static class Physics
    {
        /// <summary>
        /// Check a ray intersection
        /// </summary>
        /// <param name="from"></param>
        /// <param name="to"></param>
        /// <returns></returns>
        public static RaycastHit Raycast(Vector3 from, Vector3 to)
        {
            RaycastHit hit;

            INTERNAL_raycast(ref from, ref to, LayerMask.All.ToULong(), out hit);

            return hit;
        }

        /// <summary>
        /// Check a ray intersection by layer mask
        /// </summary>
        /// <param name="from"></param>
        /// <param name="to"></param>
        /// <param name="layerMask"></param>
        /// <returns></returns>
        public static RaycastHit Raycast(Vector3 from, Vector3 to, LayerMask layerMask)
        {
            RaycastHit hit;

            INTERNAL_raycast(ref from, ref to, layerMask.ToULong(), out hit);

            return hit;
        }

        /// <summary>
        /// Return an array of colliders which are overlapped by a sphere
        /// </summary>
        /// <param name="center"></param>
        /// <param name="radius"></param>
        /// <returns></returns>
        public static Rigidbody[] OverlapSphere(Vector3 center, float radius)
        {
            return INTERNAL_overlapSphere(ref center, radius, LayerMask.All.ToULong());
        }

        /// <summary>
        /// Return an array of colliders which are overlapped by a sphere with layer mask
        /// </summary>
        /// <param name="center"></param>
        /// <param name="radius"></param>
        /// <param name="layerMask"></param>
        /// <returns></returns>
        public static Rigidbody[] OverlapSphere(Vector3 center, float radius, LayerMask layerMask)
        {
            return INTERNAL_overlapSphere(ref center, radius, layerMask.ToULong());
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_raycast(ref Vector3 from, ref Vector3 to, ulong layer, out RaycastHit hit);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Rigidbody[] INTERNAL_overlapSphere(ref Vector3 center, float radius, ulong layer);
    }
}
