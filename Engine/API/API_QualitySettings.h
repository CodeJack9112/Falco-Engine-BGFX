#pragma once

#include "API.h"

namespace GX
{
    class API_QualitySettings
    {
    public:
        //Register methods
        static void Register()
        {
            //Screen
            mono_add_internal_call("FalcoEngine.QualitySettings::SetWindowSize", (void*)setWindowSize);
            mono_add_internal_call("FalcoEngine.QualitySettings::GetWindowWidth", (void*)getWindowWidth);
            mono_add_internal_call("FalcoEngine.QualitySettings::GetWindowHeight", (void*)getWindowHeight);
            mono_add_internal_call("FalcoEngine.QualitySettings::SetFullScreen", (void*)setFullScreen);
            mono_add_internal_call("FalcoEngine.QualitySettings::GetFullScreen", (void*)getFullScreen);
            mono_add_internal_call("FalcoEngine.QualitySettings::SetExclusiveMode", (void*)setExclusiveMode);
            mono_add_internal_call("FalcoEngine.QualitySettings::GetExclusiveMode", (void*)getExclusiveMode);

            //V-Sync
            mono_add_internal_call("FalcoEngine.QualitySettings::SetVSync", (void*)setVSync);
            mono_add_internal_call("FalcoEngine.QualitySettings::GetVSync", (void*)getVSync);

            //FXAA
            mono_add_internal_call("FalcoEngine.QualitySettings::SetFXAA", (void*)setFXAA);
            mono_add_internal_call("FalcoEngine.QualitySettings::GetFXAA", (void*)getFXAA);

            //Dynamic resolution
            mono_add_internal_call("FalcoEngine.QualitySettings::SetDynamicResolution", (void*)setDynamicResolution);
            mono_add_internal_call("FalcoEngine.QualitySettings::GetDynamicResolution", (void*)getDynamicResolution);
            mono_add_internal_call("FalcoEngine.QualitySettings::SetDynamicResolutionValue", (void*)setDynamicResolutionValue);
            mono_add_internal_call("FalcoEngine.QualitySettings::GetDynamicResolutionValue", (void*)getDynamicResolutionValue);
            mono_add_internal_call("FalcoEngine.QualitySettings::SetAutoDynamicResolution", (void*)setAutoDynamicResolution);
            mono_add_internal_call("FalcoEngine.QualitySettings::GetAutoDynamicResolution", (void*)getAutoDynamicResolution);
            mono_add_internal_call("FalcoEngine.QualitySettings::SetAutoDynamicResolutionTargetFPS", (void*)setAutoDynamicResolutionTargetFPS);
            mono_add_internal_call("FalcoEngine.QualitySettings::GetAutoDynamicResolutionTargetFPS", (void*)getAutoDynamicResolutionTargetFPS);

            //Shadows
            mono_add_internal_call("FalcoEngine.QualitySettings::INTERNAL_SetDirectionalShadowsQuality", (void*)setDirectionalShadowsQuality);
            mono_add_internal_call("FalcoEngine.QualitySettings::INTERNAL_GetDirectionalShadowsQuality", (void*)getDirectionalShadowsQuality);
            mono_add_internal_call("FalcoEngine.QualitySettings::INTERNAL_SetPointShadowsQuality", (void*)setPointShadowsQuality);
            mono_add_internal_call("FalcoEngine.QualitySettings::INTERNAL_GetPointShadowsQuality", (void*)getPointShadowsQuality);
            mono_add_internal_call("FalcoEngine.QualitySettings::INTERNAL_SetSpotShadowsQuality", (void*)setSpotShadowsQuality);
            mono_add_internal_call("FalcoEngine.QualitySettings::INTERNAL_GetSpotShadowsQuality", (void*)getSpotShadowsQuality);
            mono_add_internal_call("FalcoEngine.QualitySettings::INTERNAL_SetShadowsCascadesCount", (void*)setShadowsCascadesCount);
            mono_add_internal_call("FalcoEngine.QualitySettings::INTERNAL_GetShadowsCascadesCount", (void*)getShadowsCascadesCount);
            mono_add_internal_call("FalcoEngine.QualitySettings::INTERNAL_SetShadowsSofteness", (void*)setShadowsSofteness);
            mono_add_internal_call("FalcoEngine.QualitySettings::INTERNAL_GetShadowsSofteness", (void*)getShadowsSofteness);
            mono_add_internal_call("FalcoEngine.QualitySettings::SetShadowsDistance", (void*)setShadowsDistance);
            mono_add_internal_call("FalcoEngine.QualitySettings::GetShadowsDistance", (void*)getShadowsDistance);
            mono_add_internal_call("FalcoEngine.QualitySettings::SetShadowsEnabled", (void*)setShadowsEnabled);
            mono_add_internal_call("FalcoEngine.QualitySettings::GetShadowsEnabled", (void*)getShadowsEnabled);
        }

    private:
        //Screen
        static void setWindowSize(int width, int height);
        static int getWindowWidth();
        static int getWindowHeight();
        static void setFullScreen(bool value);
        static bool getFullScreen();
        static void setExclusiveMode(bool value);
        static bool getExclusiveMode();

        //V-Sync
        static void setVSync(bool value);
        static bool getVSync();

        //FXAA
        static void setFXAA(bool value);
        static bool getFXAA();

        //Dynamic resolution
        static void setDynamicResolution(bool value);
        static bool getDynamicResolution();
        static void setDynamicResolutionValue(float value);
        static float getDynamicResolutionValue();
        static void setAutoDynamicResolution(bool value);
        static bool getAutoDynamicResolution();
        static void setAutoDynamicResolutionTargetFPS(int value);
        static int getAutoDynamicResolutionTargetFPS();

        //Shadows
        static void setDirectionalShadowsQuality(int value);
        static int getDirectionalShadowsQuality();
        static void setPointShadowsQuality(int value);
        static int getPointShadowsQuality();
        static void setSpotShadowsQuality(int value);
        static int getSpotShadowsQuality();
        static void setShadowsCascadesCount(int value);
        static int getShadowsCascadesCount();
        static void setShadowsSofteness(int value);
        static int getShadowsSofteness();
        static void setShadowsDistance(int value);
        static int getShadowsDistance();
        static void setShadowsEnabled(bool value);
        static int getShadowsEnabled();
    };
}