using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Image : UIElement
    {
        internal Image()
        {

        }

        /// <summary>
        /// Get or set texture for this image
        /// </summary>
        public Texture texture { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }
    }
}
