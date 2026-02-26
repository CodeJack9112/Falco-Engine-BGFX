using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;

namespace FalcoEngine
{
    public static class QualitySettings
    {
        //Enums

        /// <summary>
        /// Shadows quality level enum
        /// </summary>
        public enum ShadowsQualityLevel
        {
            Level1,
            Level2,
            Level3,
            Level4,
            Level5,
            Level6
        }

        /// <summary>
        /// Shadows cascades count enum
        /// </summary>
        public enum ShadowsCascadesCount
        {
            Two,
            Three,
            Four
        }

        /// <summary>
        /// Shadows softeness level enum
        /// </summary>
        public enum ShadowsSoftenessLevel
        {
            Off,
            Low,
            Normal,
            High
        }

        //Screen

        /// <summary>
        /// Set window size
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetWindowSize(int width, int height);

        /// <summary>
        /// Return window width
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetWindowWidth();

        /// <summary>
        /// Return window height
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetWindowHeight();

        /// <summary>
        /// Set window fullscreen
        /// </summary>
        /// <param name="value"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetFullScreen(bool value);

        /// <summary>
        /// Return true if window is in fullscreen
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetFullScreen();

        /// <summary>
        /// Set window should run in exclusive mode and switch display resolution if needed
        /// </summary>
        /// <param name="value"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetExclusiveMode(bool value);

        /// <summary>
        /// Return true if window is running in exclusive mode
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetExclusiveMode();

        //V-Sync

        /// <summary>
        /// Set V-Sync enabled or not
        /// </summary>
        /// <param name="value"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetVSync(bool value);

        /// <summary>
        /// Return true if V-Sync is enabled
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetVSync();

        //FXAA

        /// <summary>
        /// Set use FXAA anti aliasing
        /// </summary>
        /// <param name="value"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetFXAA(bool value);

        /// <summary>
        /// Return true if FXAA anti aliasing is enabled
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetFXAA();

        //Dynamic resolution
        
        /// <summary>
        /// Set use back buffer dynamic resolution
        /// </summary>
        /// <param name="value"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetDynamicResolution(bool value);

        /// <summary>
        /// Returns if back buffer dynamic resolution is using
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetDynamicResolution();
        
        /// <summary>
        /// Set back buffer resolution ratio (from 0.5 to 1)
        /// </summary>
        /// <param name="value"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetDynamicResolutionValue(float value);

        /// <summary>
        /// Return back buffer resolution ratio
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetDynamicResolutionValue();

        /// <summary>
        /// Set use auto back buffer dynamic resolution
        /// </summary>
        /// <param name="value"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetAutoDynamicResolution(bool value);

        /// <summary>
        /// Returns if auto back buffer dynamic resolution is using
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetAutoDynamicResolution();

        /// <summary>
        /// Set target frames per second for auto dynamic resolution
        /// </summary>
        /// <param name="value"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetAutoDynamicResolutionTargetFPS(int value);

        /// <summary>
        /// Return target frames per second for auto dynamic resolution
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetAutoDynamicResolutionTargetFPS();

        //Shadows

        /// <summary>
        /// Set directional shadows quality
        /// </summary>
        /// <param name="qualityLevel"></param>
        public static void SetDirectionalShadowsQuality(ShadowsQualityLevel qualityLevel)
        {
            INTERNAL_SetDirectionalShadowsQuality((int)qualityLevel);
        }

        /// <summary>
        /// Return directional shadows quality
        /// </summary>
        /// <returns></returns>
        public static ShadowsQualityLevel GetDirectionalShadowsQuality()
        {
            return (ShadowsQualityLevel)INTERNAL_GetDirectionalShadowsQuality();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern void INTERNAL_SetDirectionalShadowsQuality(int qualityLevel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern int INTERNAL_GetDirectionalShadowsQuality();

        //

        /// <summary>
        /// Set point shadows quality
        /// </summary>
        /// <param name="qualityLevel"></param>
        public static void SetPointShadowsQuality(ShadowsQualityLevel qualityLevel)
        {
            INTERNAL_SetPointShadowsQuality((int)qualityLevel);
        }

        /// <summary>
        /// Return point shadows quality
        /// </summary>
        /// <returns></returns>
        public static ShadowsQualityLevel GetPointShadowsQuality()
        {
            return (ShadowsQualityLevel)INTERNAL_GetPointShadowsQuality();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern void INTERNAL_SetPointShadowsQuality(int qualityLevel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern int INTERNAL_GetPointShadowsQuality();

        //

        /// <summary>
        /// Set spot shadows quality
        /// </summary>
        /// <param name="qualityLevel"></param>
        public static void SetSpotShadowsQuality(ShadowsQualityLevel qualityLevel)
        {
            INTERNAL_SetSpotShadowsQuality((int)qualityLevel);
        }

        /// <summary>
        /// Return spot shadows quality
        /// </summary>
        /// <returns></returns>
        public static ShadowsQualityLevel GetSpotShadowsQuality()
        {
            return (ShadowsQualityLevel)INTERNAL_GetSpotShadowsQuality();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern void INTERNAL_SetSpotShadowsQuality(int qualityLevel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern int INTERNAL_GetSpotShadowsQuality();

        //

        /// <summary>
        /// Set shadows cascades count (for directional lights only)
        /// </summary>
        /// <param name="cascadesCount"></param>
        public static void SetShadowsCascadesCount(ShadowsCascadesCount cascadesCount)
        {
            INTERNAL_SetShadowsCascadesCount((int)cascadesCount);
        }

        /// <summary>
        /// Return shadows cascades count
        /// </summary>
        /// <returns></returns>
        public static ShadowsCascadesCount GetShadowsCascadesCount()
        {
            return (ShadowsCascadesCount)INTERNAL_GetShadowsCascadesCount();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern void INTERNAL_SetShadowsCascadesCount(int cascadesCount);

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern int INTERNAL_GetShadowsCascadesCount();

        //

        /// <summary>
        /// Set shadows softeness
        /// </summary>
        /// <param name="shadowsSofteness"></param>
        public static void SetShadowsSofteness(ShadowsSoftenessLevel shadowsSofteness)
        {
            INTERNAL_SetShadowsSofteness((int)shadowsSofteness);
        }

        /// <summary>
        /// Return shadows softeness
        /// </summary>
        /// <returns></returns>
        public static ShadowsSoftenessLevel GetShadowsSofteness()
        {
            return (ShadowsSoftenessLevel)INTERNAL_GetShadowsSofteness();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern void INTERNAL_SetShadowsSofteness(int shadowsSofteness);

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern int INTERNAL_GetShadowsSofteness();

        //

        /// <summary>
        /// Set distance in which shadows will be visible
        /// </summary>
        /// <param name="value"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetShadowsDistance(float value);

        /// <summary>
        /// Return shadows distance
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetShadowsDistance();

        /// <summary>
        /// Enable/disable shadows
        /// </summary>
        /// <param name="value"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetShadowsEnabled(bool value);

        /// <summary>
        /// Returns if shadows are enabled
        /// </summary>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetShadowsEnabled();
    }
}
