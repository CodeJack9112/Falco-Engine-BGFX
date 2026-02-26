using System;
using System.Collections;
using System.Runtime.CompilerServices;

namespace FalcoEngine
{
    public class LayerMask
    {
        private BitArray bitset = new BitArray(32, true);

        private static readonly LayerMask all = new LayerMask();

        /// <summary>
        /// Return layer mask with all layers selected
        /// </summary>
        public static LayerMask All => all;

        /// <summary>
        /// Return name of the layer
        /// </summary>
        /// <param name="layer"></param>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string LayerToName(int layer);

        /// <summary>
        /// Return layer by it's name
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int NameToLayer(string name);

        /// <summary>
        /// Enable or disable layer
        /// </summary>
        /// <param name="layer"></param>
        /// <param name="value"></param>
        public void Set(int layer, bool value)
        {
            if (layer < bitset.Count)
                bitset.Set(layer, value);
        }

        /// <summary>
        /// Get state of the layer
        /// </summary>
        /// <param name="layer"></param>
        /// <returns></returns>
        public bool Get(int layer)
        {
            if (layer < bitset.Count)
                return bitset.Get(layer);
            else
                return false;
        }

        /// <summary>
        /// Convert this layer mask to ulong
        /// </summary>
        /// <returns></returns>
        public ulong ToULong()
        {
            var array = new byte[32];
            bitset.CopyTo(array, 0);

            return BitConverter.ToUInt64(array, 0);
        }

        /// <summary>
        /// Restore layers from ulong
        /// </summary>
        /// <param name="value"></param>
        public void FromULong(ulong value)
        {
            byte[] bytes = BitConverter.GetBytes(value);
            bitset = new BitArray(bytes);
        }
    }
}
