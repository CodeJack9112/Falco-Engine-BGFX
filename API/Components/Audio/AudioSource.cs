using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class AudioSource : Component
    {
        internal AudioSource()
        {

        }

        /*----------- PUBLIC ------------*/

        /// <summary>
        /// Returns if this audio source is playing
        /// </summary>
        public bool isPlaying { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Returns if this audio source is paused
        /// </summary>
        public bool isPaused { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Return or set whether the audio clip replays after it finishes or not
        /// </summary>
        public bool loop { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns or sets if audio source should start playing after scene is loaded
        /// </summary>
        public bool playOnStart { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// The pitch of the audio source
        /// </summary>
        public float pitch { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// The volume of the audio source (0.0 to 1.0)
        /// </summary>
        public float volume { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Within the min distance the audio source will cease to grow louder in volume
        /// </summary>
        public float minDistance { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Max distance is the distance where the sound is completely inaudible
        /// </summary>
        public float maxDistance { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Is this audio source 2D or 3D?
        /// </summary>
        public bool is2D { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Get or set the audio clip for this audio source
        /// </summary>
        public AudioClip audioClip { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Play current audio clip
        /// </summary>
        public void Play()
        {
            INTERNAL_play();
        }

        /// <summary>
        /// Pause playing
        /// </summary>
        public void Pause()
        {
            INTERNAL_pause();
        }

        /// <summary>
        /// Resume playing
        /// </summary>
        public void Resume()
        {
            INTERNAL_resume();
        }

        /// <summary>
        /// Stop playing
        /// </summary>
        public void Stop()
        {
            INTERNAL_stop();
        }

        /// <summary>
        /// Get total length in seconds
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern int GetTotalLength();

        /// <summary>
        /// Get current playback position in seconds
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern int GetPlaybackPosition();

        /// <summary>
        /// Set current playback position in seconds
        /// </summary>
        /// <param name="seconds"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void SetPlaybackPosition(int seconds);

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_play();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_pause();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_resume();


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_stop();
    }
}
