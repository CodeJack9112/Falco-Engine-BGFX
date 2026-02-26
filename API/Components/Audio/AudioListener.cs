using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;


namespace FalcoEngine
{
    public class AudioListener : Component
    {
        internal AudioListener()
        {

        }

        /// <summary>
        /// Get or set the master volume
        /// </summary>
        public float volume { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Pause or resume the audio system
        /// </summary>
        public bool paused { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }
    }
}
