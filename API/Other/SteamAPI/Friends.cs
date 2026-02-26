using System;
using System.Runtime.CompilerServices;

namespace FalcoEngine
{
    public static partial class SteamAPI
    {
        public static class Friends
        {
            public delegate void AvatarImageLoaded(User.SteamID steamID, int image, int width, int height);
            public static event AvatarImageLoaded OnAvatarImageLoaded;

            public delegate void GameOverlayActivated(bool active);
            public static event GameOverlayActivated OnGameOverlayActivated;

            public enum PersonaState
            {
                Offline = 0, //Offline
                Online = 1, //Online
                Busy = 2, //Online, busy
                Away = 3, //Online, away from PC
                Snooze = 4, //Online, away from PC for too long
                LookingToTrade = 5, //Online, trading
                LookingToPlay = 6  //Online, want to play
            }

            public enum OverlayDialogType
            {
                Friends,
                Community,
                Players,
                Settings,
                OfficialGameGroup,
                Stats,
                Achievements
            }

            public enum OverlayUserDialogType
            {
                SteamID,
                Chat,
                JoinTrade,
                Stats,
                Achievements,
                FriendAdd,
                FriendRemove,
                FriendRequestAccept,
                FriendRequestIgnore
            }

            public enum ActivateGameOverlayToWebPageMode
            {
                Default,
                Modal
            }

            public enum FriendFlags
            {
                None = 0x00,
                Blocked = 0x01,
                FriendshipRequested = 0x02,
                Immediate = 0x04,
                ClanMember = 0x08,
                OnGameServer = 0x10,
                RequestingFriendship = 0x80,
                RequestingInfo = 0x100,
                Ignored = 0x200,
                IgnoredFriend = 0x400,
                ChatMember = 0x1000,
                All = 0xFFFF
            }

            public enum FriendRelationship
            {
                None = 0,
                Blocked = 1,
                RequestRecipient = 2,
                Friend = 3,
                RequestInitiator = 4,
                Ignored = 5,
                IgnoredFriend = 6
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            public static extern string GetPersonaName();

            [MethodImpl(MethodImplOptions.InternalCall)]
            public static extern bool RequestUserInformation(User.SteamID steamIDUser, bool requireNameOnly);

            public static PersonaState GetPersonaState()
            {
                return (PersonaState)INTERNAL_GetPersonaState();
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern int INTERNAL_GetPersonaState();

            public static int GetSmallFriendAvatar(User.SteamID steamID)
            {
                return INTERNAL_GetSmallFriendAvatar(ref steamID);
            }

            public static int GetMediumFriendAvatar(User.SteamID steamID)
            {
                return INTERNAL_GetMediumFriendAvatar(ref steamID);
            }

            public static int GetLargeFriendAvatar(User.SteamID steamID)
            {
                return INTERNAL_GetLargeFriendAvatar(ref steamID);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern int INTERNAL_GetSmallFriendAvatar(ref User.SteamID steamID);

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern int INTERNAL_GetMediumFriendAvatar(ref User.SteamID steamID);

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern int INTERNAL_GetLargeFriendAvatar(ref User.SteamID steamID);

            public static void ActivateGameOverlay(OverlayDialogType dialogType)
            {
                INTERNAL_ActivateGameOverlay((int)dialogType);
            }

            public static void ActivateGameOverlayToUser(OverlayUserDialogType dialogType, User.SteamID steamID)
            {
                INTERNAL_ActivateGameOverlayToUser((int)dialogType, ref steamID);
            }

            public static void ActivateGameOverlayToWebPage(string url, ActivateGameOverlayToWebPageMode mode)
            {
                INTERNAL_ActivateGameOverlayToWebPage(url, (int)mode);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern void INTERNAL_ActivateGameOverlay(int dialogType);

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern void INTERNAL_ActivateGameOverlayToUser(int dialogType, ref User.SteamID steamID);

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern void INTERNAL_ActivateGameOverlayToWebPage(string url, int mode);

            [MethodImpl(MethodImplOptions.InternalCall)]
            public static extern void ClearRichPresence();

            public static void CloseClanChatWindowInSteam(User.SteamID id)
            {
                INTERNAL_CloseClanChatWindowInSteam(ref id);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern void INTERNAL_CloseClanChatWindowInSteam(ref User.SteamID id);

            public static int GetFriendCount(FriendFlags flags)
            {
                return INTERNAL_GetFriendCount((int)flags);
            }

            public static User.SteamID GetFriendByIndex(FriendFlags flags, int index)
            {
                INTERNAL_GetFriendByIndex((int)flags, index, out User.SteamID steamID);

                return steamID;
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern int INTERNAL_GetFriendCount(int flags);

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern void INTERNAL_GetFriendByIndex(int flags, int index, out User.SteamID steamID);

            public static string GetFriendPersonaName(User.SteamID steamID)
            {
                return INTERNAL_GetFriendPersonaName(ref steamID);
            }

            public static PersonaState GetFriendPersonaState(User.SteamID steamID)
            {
                return (PersonaState)INTERNAL_GetFriendPersonaState(ref steamID);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern string INTERNAL_GetFriendPersonaName(ref User.SteamID steamID);

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern int INTERNAL_GetFriendPersonaState(ref User.SteamID steamID);

            public static UInt32 GetFriendCoplayGame(User.SteamID steamID)
            {
                return INTERNAL_GetFriendCoplayGame(ref steamID);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern UInt32 INTERNAL_GetFriendCoplayGame(ref User.SteamID steamID);

            public static FriendRelationship GetFriendRelationship(User.SteamID steamID)
            {
                return (FriendRelationship)INTERNAL_GetFriendRelationship(ref steamID);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern int INTERNAL_GetFriendRelationship(ref User.SteamID steamID);

            public static int GetFriendRichPresenceKeyCount(User.SteamID steamID)
            {
                return INTERNAL_GetFriendRichPresenceKeyCount(ref steamID);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern int INTERNAL_GetFriendRichPresenceKeyCount(ref User.SteamID steamID);

            public static string GetFriendRichPresenceKeyByIndex(User.SteamID steamID, int index)
            {
                return INTERNAL_GetFriendRichPresenceKeyByIndex(ref steamID, index);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern string INTERNAL_GetFriendRichPresenceKeyByIndex(ref User.SteamID steamID, int index);

            public static string GetFriendRichPresence(User.SteamID steamID, string key)
            {
                return INTERNAL_GetFriendRichPresence(ref steamID, key);
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern string INTERNAL_GetFriendRichPresence(ref User.SteamID steamID, string key);

            private static void RunAvatarImageLoadedCallbacks(User.SteamID steamID, int image, int width, int height)
            {
                OnAvatarImageLoaded?.Invoke(steamID, image, width, height);
            }

            private static void RunGameOverlayActivatedCallbacks(bool active)
            {
                OnGameOverlayActivated?.Invoke(active);
            }
        }
    }
}
