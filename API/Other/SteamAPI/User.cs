using System;
using System.Runtime.CompilerServices;

namespace FalcoEngine
{
    public static partial class SteamAPI
    {
        public static class User
        {
            public enum Universe
            {
                Invalid = 0,
                Public = 1,
                Beta = 2,
                Internal = 3,
                Dev = 4,
            }

            public enum AccountType
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
            }

            public struct SteamID
            {
                public UInt32 accountID;
                public uint accountInstance;
                public AccountType accountType;
                public Universe universe;
                public UInt64 steamID;
            }

            public static SteamID GetSteamID()
            {
                INTERNAL_GetSteamID(out SteamID value);
                return value;
            }

            [MethodImpl(MethodImplOptions.InternalCall)]
            private static extern void INTERNAL_GetSteamID(out SteamID value);
        }
    }
}
