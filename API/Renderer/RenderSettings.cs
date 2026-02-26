using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;

namespace FalcoEngine
{
    public static class RenderSettings
    {

        /// <summary>
        /// Get or set scene ambient color
        /// </summary>
        public static Color ambientColor
        {
            get
            {
                INTERNAL_get_ambientColor(out Color value);
                return value;
            }
            set
            {
                INTERNAL_set_ambientColor(ref value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_get_ambientColor(out Color color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_set_ambientColor(ref Color color);
    }
}
