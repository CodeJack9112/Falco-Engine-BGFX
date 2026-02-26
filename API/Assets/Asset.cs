using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public abstract class Asset
    {
        internal Asset() { }

        private IntPtr asset_ptr = (IntPtr)0;

        /// <summary>
        /// Get asset name
        /// </summary>
        public string name { [MethodImpl(MethodImplOptions.InternalCall)] get; }
    }
}
