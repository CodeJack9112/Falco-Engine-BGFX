#pragma once

#include "API.h"

namespace GX
{
    namespace API
    {
        namespace Steam
        {
            enum class Universe
            {
                Invalid = 0,
                Public = 1,
                Beta = 2,
                Internal = 3,
                Dev = 4,
            };

            enum class AccountType
            {
                Invalid = 0,
                Individual = 1,
                Multiseat = 2,
                GameServer = 3,
                AnonGameServer = 4,
                Pending = 5,
                ContentServer = 6,
                Clan = 7,
                Chat = 8,
                ConsoleUser = 9,
                AnonUser = 10
            };

            struct SteamID
            {
            public:
                uint32_t accountID;
                unsigned int accountInstance;
                AccountType accountType;
                Universe universe;
                uint64_t steamID;
            };
        }
    }

    class API_SteamAPI
    {
    public:
        //Register methods
        static void Register()
        {
            //Friends
    #ifdef _WIN32
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::GetPersonaName", (void*)getPersonaName);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetPersonaState", (void*)getPersonaState);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::RequestUserInformation", (void*)requestUserInformation);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetSmallFriendAvatar", (void*)getSmallFriendAvatar);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetMediumFriendAvatar", (void*)getMediumFriendAvatar);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetLargeFriendAvatar", (void*)getLargeFriendAvatar);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_ActivateGameOverlay", (void*)activateGameOverlay);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_ActivateGameOverlayToUser", (void*)activateGameOverlayToUser);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_ActivateGameOverlayToWebPage", (void*)activateGameOverlayToWebPage);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::ClearRichPresence", (void*)clearRichPresence);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_CloseClanChatWindowInSteam", (void*)closeClanChatWindowInSteam);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetFriendCount", (void*)getFriendCount);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetFriendByIndex", (void*)getFriendByIndex);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetFriendPersonaName", (void*)getFriendPersonaName);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetFriendPersonaState", (void*)getFriendPersonaState);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetFriendCoplayGame", (void*)getFriendCoplayGame);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetFriendRelationship", (void*)getFriendRelationship);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetFriendRichPresenceKeyCount", (void*)getFriendRichPresenceKeyCount);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetFriendRichPresenceKeyByIndex", (void*)getFriendRichPresenceKeyByIndex);
            mono_add_internal_call("FalcoEngine.SteamAPI/Friends::INTERNAL_GetFriendRichPresence", (void*)getFriendRichPresence);

            //User
            mono_add_internal_call("FalcoEngine.SteamAPI/User::INTERNAL_GetSteamID", (void*)getSteamID);

            //Utils
            mono_add_internal_call("FalcoEngine.SteamAPI/Utils::INTERNAL_GetImageSize", (void*)getImageSize);
            mono_add_internal_call("FalcoEngine.SteamAPI/Utils::INTERNAL_GetImageRGBA", (void*)getImageRGBA);

            //UserStats
            mono_add_internal_call("FalcoEngine.SteamAPI/UserStats::GetNumAchievements", (void*)getNumAchievements);
            mono_add_internal_call("FalcoEngine.SteamAPI/UserStats::GetAchievementName", (void*)getAchievementName);
            mono_add_internal_call("FalcoEngine.SteamAPI/UserStats::RequestCurrentStats", (void*)requestCurrentStats);
            mono_add_internal_call("FalcoEngine.SteamAPI/UserStats::INTERNAL_RequestUserStats", (void*)requestUserStats);
            mono_add_internal_call("FalcoEngine.SteamAPI/UserStats::StoreStats", (void*)storeStats);
            mono_add_internal_call("FalcoEngine.SteamAPI/UserStats::INTERNAL_GetUserAchievement", (void*)getUserAchievement);
            mono_add_internal_call("FalcoEngine.SteamAPI/UserStats::GetAchievement", (void*)getAchievement);
            mono_add_internal_call("FalcoEngine.SteamAPI/UserStats::GetAchievementIcon", (void*)getAchievementIcon);
            mono_add_internal_call("FalcoEngine.SteamAPI/UserStats::SetAchievement", (void*)setAchievement);
            mono_add_internal_call("FalcoEngine.SteamAPI/UserStats::ClearAchievement", (void*)clearAchievement);
            mono_add_internal_call("FalcoEngine.SteamAPI/UserStats::INTERNAL_GetAchievementDisplayAttribute", (void*)getAchievementDisplayAttribute);
    #endif
        }

    private:

    #ifdef _WIN32
        //Friends
        static MonoString* getPersonaName();
        static int getPersonaState();
        static bool requestUserInformation(API::Steam::SteamID* ref_id, bool requireNameOnly);

        static int getSmallFriendAvatar(API::Steam::SteamID* ref_value);
        static int getMediumFriendAvatar(API::Steam::SteamID* ref_value);
        static int getLargeFriendAvatar(API::Steam::SteamID* ref_value);
        static void activateGameOverlay(int overlayType);
        static void activateGameOverlayToUser(int overlayType, API::Steam::SteamID* ref_steamID);
        static void activateGameOverlayToWebPage(MonoString* url, int mode);
        static void clearRichPresence();
        static void closeClanChatWindowInSteam(API::Steam::SteamID* ref_steamID);
        static int getFriendCount(int flags);
        static void getFriendByIndex(int flags, int index, API::Steam::SteamID* out_steamID);
        static MonoString* getFriendPersonaName(API::Steam::SteamID* ref_steamID);
        static int getFriendPersonaState(API::Steam::SteamID* ref_steamID);
        static unsigned int getFriendCoplayGame(API::Steam::SteamID* ref_steamID);
        static int getFriendRelationship(API::Steam::SteamID* ref_steamID);
        static int getFriendRichPresenceKeyCount(API::Steam::SteamID* ref_steamID);
        static MonoString* getFriendRichPresenceKeyByIndex(API::Steam::SteamID* ref_steamID, int index);
        static MonoString* getFriendRichPresence(API::Steam::SteamID* ref_steamID, MonoString* key);

        //User
        static void getSteamID(API::Steam::SteamID* out_value);

        //Utils
        static void getImageSize(int id, API::Vector2* out_value);
        static MonoArray* getImageRGBA(int id);

        //UserStats
        static int getNumAchievements();
        static MonoString* getAchievementName(int index);
        static bool requestCurrentStats();
        static bool requestUserStats(API::Steam::SteamID* ref_steamID);
        static bool storeStats();
        static bool getUserAchievement(API::Steam::SteamID* ref_steamID, MonoString* name);
        static bool getAchievement(MonoString* name);
        static int getAchievementIcon(MonoString* name);
        static bool setAchievement(MonoString* name);
        static bool clearAchievement(MonoString* name);
        static MonoString* getAchievementDisplayAttribute(MonoString* name, int displayAttribute);
    #endif
    };
}