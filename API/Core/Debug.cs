using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;


namespace FalcoEngine
{
    public class Debug
    {
        /// <summary>
        /// Print message to the editor's console
        /// </summary>
        /// <param name="message"></param>
        public static void Log(string message)
        {
            INTERNAL_log(message);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_log(string message);
    }
}
