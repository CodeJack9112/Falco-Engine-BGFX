using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public static class Application
    {
        /// <summary>
        /// Quit the application
        /// </summary>
        public static void Quit()
        {
            INTERNAL_quit();
        }

        /// <summary>
        /// Returns path to the assets location
        /// </summary>
        public static string assetsPath { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_quit();
    }
}
