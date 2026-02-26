using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public static class Time
    {
        /// <summary>
        /// The current frames per second value (FPS)
        /// </summary>
        public static int frameRate { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// The interval in seconds from the last frame to the current one
        /// </summary>
        public static float deltaTime { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// The scale at which time passes
        /// </summary>
        public static float timeScale { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// The real time in seconds since the game started
        /// </summary>
        public static float timeSinceGameStart { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// The time in seconds scaled by timeScale since the game started
        /// </summary>
        public static float timeSinceGameStartScaled { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// The time in seconds since the last scene has finished loading
        /// </summary>
        public static float timeSinceLevelStart { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// The time in seconds scaled by timeScale since the last scene has finished loading
        /// </summary>
        public static float timeSinceLevelStartScaled { [MethodImpl(MethodImplOptions.InternalCall)] get; }
    }
}
