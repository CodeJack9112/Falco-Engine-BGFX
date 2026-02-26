
namespace FalcoEngine
{
    public enum ScanCode
    {
        /// <summary>
        ///   <para>Not assigned (never returned as the result of a keystroke).</para>
        /// </summary>
        None = 0,
        /// <summary>
        ///   <para>The backspace key.</para>
        /// </summary>
        Backspace = 42,
        /// <summary>
        ///   <para>The forward delete key.</para>
        /// </summary>
        Delete = 76,
        /// <summary>
        ///   <para>The tab key.</para>
        /// </summary>
        Tab = 43,
        /// <summary>
        ///   <para>The Clear key.</para>
        /// </summary>
        Clear = 156,
        /// <summary>
        ///   <para>Return key.</para>
        /// </summary>
        Return = 40,
        /// <summary>
        ///   <para>Pause on PC machines.</para>
        /// </summary>
        Pause = 72,
        /// <summary>
        ///   <para>Escape key.</para>
        /// </summary>
        Escape = 41,
        /// <summary>
        ///   <para>Space key.</para>
        /// </summary>
        Space = 44,
        /// <summary>
        ///   <para>Numeric keypad 0.</para>
        /// </summary>
        Keypad0 = 98,
        /// <summary>
        ///   <para>Numeric keypad 1.</para>
        /// </summary>
        Keypad1 = 89,
        /// <summary>
        ///   <para>Numeric keypad 2.</para>
        /// </summary>
        Keypad2 = 90,
        /// <summary>
        ///   <para>Numeric keypad 3.</para>
        /// </summary>
        Keypad3 = 91,
        /// <summary>
        ///   <para>Numeric keypad 4.</para>
        /// </summary>
        Keypad4 = 92,
        /// <summary>
        ///   <para>Numeric keypad 5.</para>
        /// </summary>
        Keypad5 = 93,
        /// <summary>
        ///   <para>Numeric keypad 6.</para>
        /// </summary>
        Keypad6 = 94,
        /// <summary>
        ///   <para>Numeric keypad 7.</para>
        /// </summary>
        Keypad7 = 95,
        /// <summary>
        ///   <para>Numeric keypad 8.</para>
        /// </summary>
        Keypad8 = 96,
        /// <summary>
        ///   <para>Numeric keypad 9.</para>
        /// </summary>
        Keypad9 = 97,
        /// <summary>
        ///   <para>Numeric keypad '.'.</para>
        /// </summary>
        KeypadPeriod = 99,
        /// <summary>
        ///   <para>Numeric keypad '/'.</para>
        /// </summary>
        KeypadDivide = 84,
        /// <summary>
        ///   <para>Numeric keypad '*'.</para>
        /// </summary>
        KeypadMultiply = 85,
        /// <summary>
        ///   <para>Numeric keypad '-'.</para>
        /// </summary>
        KeypadMinus = 86,
        /// <summary>
        ///   <para>Numeric keypad '+'.</para>
        /// </summary>
        KeypadPlus = 87,
        /// <summary>
        ///   <para>Numeric keypad enter.</para>
        /// </summary>
        KeypadEnter = 88,
        /// <summary>
        ///   <para>Numeric keypad '='.</para>
        /// </summary>
        KeypadEquals = 103,
        /// <summary>
        ///   <para>Up arrow key.</para>
        /// </summary>
        UpArrow = 82,
        /// <summary>
        ///   <para>Down arrow key.</para>
        /// </summary>
        DownArrow = 81,
        /// <summary>
        ///   <para>Right arrow key.</para>
        /// </summary>
        RightArrow = 79,
        /// <summary>
        ///   <para>Left arrow key.</para>
        /// </summary>
        LeftArrow = 80,
        /// <summary>
        ///   <para>Insert key key.</para>
        /// </summary>
        Insert = 73,
        /// <summary>
        ///   <para>Home key.</para>
        /// </summary>
        Home = 74,
        /// <summary>
        ///   <para>End key.</para>
        /// </summary>
        End = 77,
        /// <summary>
        ///   <para>Page up.</para>
        /// </summary>
        PageUp = 75,
        /// <summary>
        ///   <para>Page down.</para>
        /// </summary>
        PageDown = 78,
        /// <summary>
        ///   <para>F1 function key.</para>
        /// </summary>
        F1 = 58,
        /// <summary>
        ///   <para>F2 function key.</para>
        /// </summary>
        F2 = 59,
        /// <summary>
        ///   <para>F3 function key.</para>
        /// </summary>
        F3 = 60,
        /// <summary>
        ///   <para>F4 function key.</para>
        /// </summary>
        F4 = 61,
        /// <summary>
        ///   <para>F5 function key.</para>
        /// </summary>
        F5 = 62,
        /// <summary>
        ///   <para>F6 function key.</para>
        /// </summary>
        F6 = 63,
        /// <summary>
        ///   <para>F7 function key.</para>
        /// </summary>
        F7 = 64,
        /// <summary>
        ///   <para>F8 function key.</para>
        /// </summary>
        F8 = 65,
        /// <summary>
        ///   <para>F9 function key.</para>
        /// </summary>
        F9 = 66,
        /// <summary>
        ///   <para>F10 function key.</para>
        /// </summary>
        F10 = 67,
        /// <summary>
        ///   <para>F11 function key.</para>
        /// </summary>
        F11 = 68,
        /// <summary>
        ///   <para>F12 function key.</para>
        /// </summary>
        F12 = 69,
        /// <summary>
        ///   <para>F13 function key.</para>
        /// </summary>
        F13 = 104,
        /// <summary>
        ///   <para>F14 function key.</para>
        /// </summary>
        F14 = 105,
        /// <summary>
        ///   <para>F15 function key.</para>
        /// </summary>
        F15 = 106,
        /// <summary>
        ///   <para>The '0' key on the top of the alphanumeric keyboard.</para>
        /// </summary>
        Alpha0 = 39,
        /// <summary>
        ///   <para>The '1' key on the top of the alphanumeric keyboard.</para>
        /// </summary>
        Alpha1 = 30,
        /// <summary>
        ///   <para>The '2' key on the top of the alphanumeric keyboard.</para>
        /// </summary>
        Alpha2 = 31,
        /// <summary>
        ///   <para>The '3' key on the top of the alphanumeric keyboard.</para>
        /// </summary>
        Alpha3 = 32,
        /// <summary>
        ///   <para>The '4' key on the top of the alphanumeric keyboard.</para>
        /// </summary>
        Alpha4 = 33,
        /// <summary>
        ///   <para>The '5' key on the top of the alphanumeric keyboard.</para>
        /// </summary>
        Alpha5 = 34,
        /// <summary>
        ///   <para>The '6' key on the top of the alphanumeric keyboard.</para>
        /// </summary>
        Alpha6 = 35,
        /// <summary>
        ///   <para>The '7' key on the top of the alphanumeric keyboard.</para>
        /// </summary>
        Alpha7 = 36,
        /// <summary>
        ///   <para>The '8' key on the top of the alphanumeric keyboard.</para>
        /// </summary>
        Alpha8 = 37,
        /// <summary>
        ///   <para>The '9' key on the top of the alphanumeric keyboard.</para>
        /// </summary>
        Alpha9 = 38,
        /// <summary>
        ///   <para>Exclamation mark key '!'.</para>
        /// </summary>
        Exclaim = 207,
        /// <summary>
        ///   <para>Double quote key '"'.</para>
        /// </summary>
        //DoubleQuote = 34,
        /// <summary>
        ///   <para>Hash key '#'.</para>
        /// </summary>
        Hash = 204,
        /// <summary>
        ///   <para>Dollar sign key '$'.</para>
        /// </summary>
        Dollar = 49,
        /// <summary>
        ///   <para>Ampersand key '&amp;'.</para>
        /// </summary>
        Ampersand = 199,
        /// <summary>
        ///   <para>Quote key '.</para>
        /// </summary>
        //Quote = 39,
        /// <summary>
        ///   <para>Left Parenthesis key '('.</para>
        /// </summary>
        LeftParen = 182,
        /// <summary>
        ///   <para>Right Parenthesis key ')'.</para>
        /// </summary>
        RightParen = 183,
        /// <summary>
        ///   <para>Asterisk key '*'.</para>
        /// </summary>
        Asterisk = 49,
        /// <summary>
        ///   <para>Plus key '+'.</para>
        /// </summary>
        Plus = 87,
        /// <summary>
        ///   <para>Comma ',' key.</para>
        /// </summary>
        Comma = 54,
        /// <summary>
        ///   <para>Minus '-' key.</para>
        /// </summary>
        Minus = 86,
        /// <summary>
        ///   <para>Period '.' key.</para>
        /// </summary>
        Period = 55,
        /// <summary>
        ///   <para>Slash '/' key.</para>
        /// </summary>
        Slash = 56,
        /// <summary>
        ///   <para>Colon ':' key.</para>
        /// </summary>
        Colon = 203,
        /// <summary>
        ///   <para>Semicolon ';' key.</para>
        /// </summary>
        Semicolon = 51,
        /// <summary>
        ///   <para>Less than '&lt;' key.</para>
        /// </summary>
        Less = 197,
        /// <summary>
        ///   <para>Equals '=' key.</para>
        /// </summary>
        Equals = 103,
        /// <summary>
        ///   <para>Greater than '&gt;' key.</para>
        /// </summary>
        Greater = 198,
        /// <summary>
        ///   <para>Question mark '?' key.</para>
        /// </summary>
        //Question = 0x3F,
        /// <summary>
        ///   <para>At key '@'.</para>
        /// </summary>
        At = 206,
        /// <summary>
        ///   <para>Left square bracket key '['.</para>
        /// </summary>
        LeftBracket = 47,
        /// <summary>
        ///   <para>Backslash key '\'.</para>
        /// </summary>
        Backslash = 49,
        /// <summary>
        ///   <para>Right square bracket key ']'.</para>
        /// </summary>
        RightBracket = 48,
        /// <summary>
        ///   <para>Caret key '^'.</para>
        /// </summary>
        //Caret = 94,
        /// <summary>
        ///   <para>Underscore '_' key.</para>
        /// </summary>
        //Underscore = 95,
        /// <summary>
        ///   <para>Back quote key '`'.</para>
        /// </summary>
        //BackQuote = 96,
        /// <summary>
        ///   <para>'a' key.</para>
        /// </summary>
        A = 4,
        /// <summary>
        ///   <para>'b' key.</para>
        /// </summary>
        B = 5,
        /// <summary>
        ///   <para>'c' key.</para>
        /// </summary>
        C = 6,
        /// <summary>
        ///   <para>'d' key.</para>
        /// </summary>
        D = 7,
        /// <summary>
        ///   <para>'e' key.</para>
        /// </summary>
        E = 8,
        /// <summary>
        ///   <para>'f' key.</para>
        /// </summary>
        F = 9,
        /// <summary>
        ///   <para>'g' key.</para>
        /// </summary>
        G = 10,
        /// <summary>
        ///   <para>'h' key.</para>
        /// </summary>
        H = 11,
        /// <summary>
        ///   <para>'i' key.</para>
        /// </summary>
        I = 12,
        /// <summary>
        ///   <para>'j' key.</para>
        /// </summary>
        J = 13,
        /// <summary>
        ///   <para>'k' key.</para>
        /// </summary>
        K = 14,
        /// <summary>
        ///   <para>'l' key.</para>
        /// </summary>
        L = 15,
        /// <summary>
        ///   <para>'m' key.</para>
        /// </summary>
        M = 16,
        /// <summary>
        ///   <para>'n' key.</para>
        /// </summary>
        N = 17,
        /// <summary>
        ///   <para>'o' key.</para>
        /// </summary>
        O = 18,
        /// <summary>
        ///   <para>'p' key.</para>
        /// </summary>
        P = 19,
        /// <summary>
        ///   <para>'q' key.</para>
        /// </summary>
        Q = 20,
        /// <summary>
        ///   <para>'r' key.</para>
        /// </summary>
        R = 21,
        /// <summary>
        ///   <para>'s' key.</para>
        /// </summary>
        S = 22,
        /// <summary>
        ///   <para>'t' key.</para>
        /// </summary>
        T = 23,
        /// <summary>
        ///   <para>'u' key.</para>
        /// </summary>
        U = 24,
        /// <summary>
        ///   <para>'v' key.</para>
        /// </summary>
        V = 25,
        /// <summary>
        ///   <para>'w' key.</para>
        /// </summary>
        W = 26,
        /// <summary>
        ///   <para>'x' key.</para>
        /// </summary>
        X = 27,
        /// <summary>
        ///   <para>'y' key.</para>
        /// </summary>
        Y = 28,
        /// <summary>
        ///   <para>'z' key.</para>
        /// </summary>
        Z = 29,
        /// <summary>
        ///   <para>Numlock key.</para>
        /// </summary>
        Numlock = 83,
        /// <summary>
        ///   <para>Capslock key.</para>
        /// </summary>
        CapsLock = 57,
        /// <summary>
        ///   <para>Scroll lock key.</para>
        /// </summary>
        ScrollLock = 71,
        /// <summary>
        ///   <para>Right shift key.</para>
        /// </summary>
        RightShift = 229,
        /// <summary>
        ///   <para>Left shift key.</para>
        /// </summary>
        LeftShift = 225,
        /// <summary>
        ///   <para>Right Control key.</para>
        /// </summary>
        RightControl = 228,
        /// <summary>
        ///   <para>Left Control key.</para>
        /// </summary>
        LeftControl = 224,
        /// <summary>
        ///   <para>Right Alt key.</para>
        /// </summary>
        RightAlt = 230,
        /// <summary>
        ///   <para>Left Alt key.</para>
        /// </summary>
        LeftAlt = 226,
        /// <summary>
        ///   <para>Left Command key.</para>
        /// </summary>
        LeftCommand = 227,
        /// <summary>
        ///   <para>Left Command key.</para>
        /// </summary>
        LeftApple = 227,
        /// <summary>
        ///   <para>Left Windows key.</para>
        /// </summary>
        LeftWindows = 227,
        /// <summary>
        ///   <para>Right Command key.</para>
        /// </summary>
        RightCommand = 231,
        /// <summary>
        ///   <para>Right Command key.</para>
        /// </summary>
        RightApple = 231,
        /// <summary>
        ///   <para>Right Windows key.</para>
        /// </summary>
        RightWindows = 231,
        /// <summary>
        ///   <para>Alt Gr key.</para>
        /// </summary>
        AltGr = 230,
        /// <summary>
        ///   <para>Help key.</para>
        /// </summary>
        Help = 117,
        /// <summary>
        ///   <para>Print key.</para>
        /// </summary>
        Print = 70,
        /// <summary>
        ///   <para>Sys Req key.</para>
        /// </summary>
        SysReq = 154,
        /// <summary>
        ///   <para>Break key.</para>
        /// </summary>
        //Break = 318,
        /// <summary>
        ///   <para>Menu key.</para>
        /// </summary>
        Menu = 118,
    }
}
