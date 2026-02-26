using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public static class Screen
    {
        /// <summary>
        /// The current width of the screen window in pixels
        /// </summary>
        public static int width { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// The current height of the screen window in pixels
        /// </summary>
        public static int height { [MethodImpl(MethodImplOptions.InternalCall)] get; }
    }
}
