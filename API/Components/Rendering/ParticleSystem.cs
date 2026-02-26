using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace FalcoEngine
{
    /// <summary>
    /// Particle emitter
    /// </summary>
    public class ParticleEmitter
    {
        private IntPtr this_ptr = (IntPtr)0;

        /// <summary>
        /// Start emitting
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void Play();

        /// <summary>
        /// Stop emitting
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void Stop();
        
        /// <summary>
        /// Is emitting enabled
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool GetIsPlaying();

        /// <summary>
        /// Get emitter playback time
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern float GetPlaybackTime();
    }

    /// <summary>
    /// Particle system
    /// </summary>
    public class ParticleSystem : Component
    {
        internal ParticleSystem() { }

        /// <summary>
        /// Returns number of emitters
        /// </summary>
        public int emittersCount { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Get emitter by index
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern ParticleEmitter GetEmitter(int index);
    }
}
