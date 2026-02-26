using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class VideoPlayer : Component
    {
        internal VideoPlayer() { }

        public delegate void PlaybackEvent(VideoPlayer sender);
        public event PlaybackEvent onEnded;

        /// <summary>
        /// Get or set the video clip for this video player
        /// </summary>
        public VideoClip videoClip { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns if this video player is playing
        /// </summary>
        public bool isPlaying { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Returns if this video player is paused
        /// </summary>
        public bool isPaused { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Return or set whether the video player replays after it finishes or not
        /// </summary>
        public bool loop { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns or sets if video player should start playing after scene is loaded
        /// </summary>
        public bool playOnStart { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns current frame as a texture
        /// </summary>
        public Texture texture { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Play current audio clip
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void Play();

        /// <summary>
        /// Pause playing
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void Pause();

        /// <summary>
        /// Resume playing
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void Resume();

        /// <summary>
        /// Stop playing
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void Stop();

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

        private void CallOnEnded(VideoPlayer sender)
        {
            onEnded?.Invoke(sender);
        }
    }
}
