using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Canvas : Component
    {
        internal Canvas()
        {

        }

        /// <summary>
        /// Gets scale of the canvas
        /// </summary>
        public float scale { [MethodImpl(MethodImplOptions.InternalCall)] get; }
    }
}
