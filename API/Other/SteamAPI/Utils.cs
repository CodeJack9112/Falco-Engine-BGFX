using System;
using System.Runtime.CompilerServices;

namespace FalcoEngine
{
    public static partial class SteamAPI
    {
        public static class Utils
        {
            public static Vector2 GetImageSize(int imageId)
            {
                Vector2 size = Vector2.zero;
                INTERNAL_GetImageSize(imageId, ref size);
                return size;
            }

            public static sbyte[] GetImageRGBA(int imageId)
            {
                return INTERNAL_GetImageRGBA(imageId);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern void INTERNAL_GetImageSize(int imageId, ref Vector2 size);

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern sbyte[] INTERNAL_GetImageRGBA(int imageId);
        }
    }
}
