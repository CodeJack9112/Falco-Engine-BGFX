using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;

namespace FalcoEngine
{
    public static class Random
    {
        //Generate random number in range[min (Inclusive), max (Inclusive)]
        public static float Range(float min, float max)
        {
            return RangeFloat(min, max);
        }

        //Generate random number in range[min (Inclusive), max (Exclusive)]
        public static int Range(int min, int max)
        {
            return RangeInt(min, max);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float RangeFloat(float min, float max);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static int RangeInt(int min, int max);
    }
}
