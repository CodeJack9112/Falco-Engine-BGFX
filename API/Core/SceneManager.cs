using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public static class SceneManager
    {
        /// <summary>
        /// Gets the currently loaded scene name
        /// </summary>
        public static string loadedScene { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Loads the scene by its name
        /// </summary>
        /// <param name="fileName"></param>
        public static void LoadScene(string fileName)
        {
            INTERNAL_load_scene(fileName);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_load_scene(string fileName);
    }
}
