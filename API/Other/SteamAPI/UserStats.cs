using System;
using System.Runtime.CompilerServices;

namespace FalcoEngine
{
    public static partial class SteamAPI
    {
        public static class UserStats
        {
            public enum AchievementDisplayAttribute
            {
                Name,
                Description,
                Hidden
            }

            public delegate void UserStatsReceived(long gameID, Result result, User.SteamID steamIDUser);
            public static event UserStatsReceived OnUserStatsReceived;

            public delegate void UserStatsStored(long gameID, Result result);
            public static event UserStatsStored OnUserStatsStored;

            public delegate void UserAchievementStored(long gameID, string achievementName, int curProgress, int maxProgress);
            public static event UserAchievementStored OnUserAchievementStored;

            public delegate void UserAchievementIconFetched(long gameID, string achievementName, bool achieved, int iconHandle);
            public static event UserAchievementIconFetched OnUserAchievementIconFetched;

            [MethodImpl(MethodImplOptions.InternalCall)]
            public static extern int GetNumAchievements();

            [MethodImpl(MethodImplOptions.InternalCall)]
            public static extern string GetAchievementName(int index);

            [MethodImpl(MethodImplOptions.InternalCall)]
            public static extern bool RequestCurrentStats();

            public static bool RequestUserStats(User.SteamID steamID)
            {
                return INTERNAL_RequestUserStats(ref steamID);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            public static extern bool StoreStats();

            public static bool GetUserAchievement(User.SteamID steamID, string name)
            {
                return INTERNAL_GetUserAchievement(ref steamID, name);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            public static extern bool GetAchievement(string name);

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern bool INTERNAL_RequestUserStats(ref User.SteamID steamID);

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern bool INTERNAL_GetUserAchievement(ref User.SteamID steamID, string name);

            [MethodImpl(MethodImplOptions.InternalCall)]
            public static extern int GetAchievementIcon(string name);

            [MethodImpl(MethodImplOptions.InternalCall)]
            public static extern bool SetAchievement(string name);
            
            [MethodImpl(MethodImplOptions.InternalCall)]
            public static extern bool ClearAchievement(string name);

            public static string GetAchievementDisplayAttribute(string name, AchievementDisplayAttribute displayAttribute)
            {
                return INTERNAL_GetAchievementDisplayAttribute(name, (int)displayAttribute);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern string INTERNAL_GetAchievementDisplayAttribute(string name, int displayAttribute);

            private static void RunUserStatsReceivedCallbacks(long gameID, int result, User.SteamID steamIDUser)
            {
                OnUserStatsReceived?.Invoke(gameID, (Result)result, steamIDUser);
            }

            private static void RunUserStatsStoredCallbacks(long gameID, int result)
            {
                OnUserStatsStored?.Invoke(gameID, (Result)result);
            }

            private static void RunUserAchievementStoredCallbacks(long gameID, string achievementName, int curProgress, int maxProgress)
            {
                OnUserAchievementStored?.Invoke(gameID, achievementName, curProgress, maxProgress);
            }

            private static void RunUserAchievementIconFetchedCallbacks(long gameID, string achievementName, bool achieved, int iconHandle)
            {
                OnUserAchievementIconFetched?.Invoke(gameID, achievementName, achieved, iconHandle);
            }
        }
    }
}