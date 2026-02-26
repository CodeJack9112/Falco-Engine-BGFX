
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Animation : Component
    {
        internal Animation() { }

        /*----------- PUBLIC ------------*/

        public delegate void AnimationEvent(Animation sender, string animName);
        public event AnimationEvent onEnded;

        /// <summary>
        /// Returns if any of animation clips attached to this component is playing
        /// </summary>
        public bool isPlaying { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Get animation clips count
        /// </summary>
        public int numAnimationClips { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Play animation clip by name
        /// </summary>
        /// <param name="name"></param>
        public void Play(string name)
        {
            INTERNAL_play(name);
        }

        /// <summary>
        /// Play animation clip by name with smooth transition between this and previous clips
        /// </summary>
        /// <param name="name"></param>
        /// <param name="duration"></param>
        public void CrossFade(string name, float duration = 1.0f)
        {
            INTERNAL_crossFade(name, duration);
        }

        /// <summary>
        /// Stop playing
        /// </summary>
        public void Stop()
        {
            INTERNAL_stop();
        }

        /// <summary>
        /// Set animation clip is looping by name
        /// </summary>
        /// <param name="name"></param>
        /// <param name="loop"></param>
        public void SetLoop(string name, bool loop)
        {
            INTERNAL_setLoop(name, loop);
        }

        /// <summary>
        /// Get animation clip is looping by name
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public bool GetLoop(string name)
        {
            return INTERNAL_getLoop(name);
        }

        /// <summary>
        /// Returns if the animation clip is playing
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public bool IsPlaying(string name)
        {
            return INTERNAL_isPlaying(name);
        }

        /// <summary>
        /// Returns animation clip info by its name
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        public AnimationClipInfo GetAnimationClipInfo(int index)
        {
            INTERNAL_getAnimationClipInfo(index, out AnimationClipInfo info);
            return info;
        }

        private void CallOnEnded(Animation sender, string animName)
        {
            onEnded?.Invoke(sender, animName);
        }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_play(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_crossFade(string name, float duration);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_stop();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setLoop(string name, bool loop);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern bool INTERNAL_getLoop(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern bool INTERNAL_isPlaying(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getAnimationClipInfo(int index, out AnimationClipInfo info);
    }
}
