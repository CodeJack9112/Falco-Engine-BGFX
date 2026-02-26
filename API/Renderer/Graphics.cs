using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;

namespace FalcoEngine
{
    public static class Graphics
    {
        /// <summary>
        /// Render quad to the render target using material
        /// </summary>
        /// <param name="material">Material</param>
        /// <param name="target">Render target</param>
        /// <param name="copyToBackBuffer">Render to screen or to render target only</param>
        public static void Blit(Material material, RenderTexture target, bool copyToBackBuffer = false)
        {
            INTERNAL_blit(material, target, copyToBackBuffer);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_blit(Material material, RenderTexture target, bool copyToBackBuffer);
    }
}
