using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Shader : Asset
    {
        internal Shader() { }

        /// <summary>
        /// Load existing shader
        /// </summary>
        /// <param name="name">Name of the existing shader</param>
        /// <returns>Loaded shader</returns>
        public static Shader Load(string name)
        {
            return INTERNAL_load(name);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Shader INTERNAL_load(string name);
    }
}
