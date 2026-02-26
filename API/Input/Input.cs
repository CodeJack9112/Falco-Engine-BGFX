using System.Runtime.CompilerServices;

namespace FalcoEngine
{
    public class Input
    {
        /// <summary>
        /// Get mouse cursor direction (delta position)
        /// </summary>
        public static Vector2 cursorDirection
        {
            get
            {
                INTERNAL_get_cursor_direction(out Vector2 value);
                return value;
            }
        }

        /// <summary>
        /// Get mouse cursor global position (from top left of the screen)
        /// </summary>
        public static Vector2 cursorPosition
        {
            get
            {
                INTERNAL_get_cursor_position(out Vector2 value);
                return value;
            }
        }

        /// <summary>
        /// Get mouse cursor position relative to window (from top left of the window)
        /// </summary>
        public static Vector2 cursorRelativePosition
        {
            get
            {
                INTERNAL_get_cursor_relative_position(out Vector2 value);
                return value;
            }
        }

        /// <summary>
        /// Returns true while the user holds down the key identified by the key ScanCode enum parameter
        /// </summary>
        /// <param name="key"></param>
        /// <returns></returns>
        public static bool GetKey(ScanCode key)
        {
            return INTERNAL_get_key((int)key);
        }

        /// <summary>
        /// Returns true during the frame the user starts pressing down the key identified by the key ScanCode enum parameter
        /// </summary>
        /// <param name="key"></param>
        /// <returns></returns>
        public static bool GetKeyDown(ScanCode key)
        {
            return INTERNAL_get_key_down((int)key);
        }

        /// <summary>
        /// Returns true during the frame the user releases the key identified by the key ScanCode enum parameter
        /// </summary>
        /// <param name="key"></param>
        /// <returns></returns>
        public static bool GetKeyUp(ScanCode key)
        {
            return INTERNAL_get_key_up((int)key);
        }

        /// <summary>
        /// Returns whether the given mouse button is held down
        /// </summary>
        /// <param name="button"></param>
        /// <returns></returns>
        public static bool GetMouseButton(int button)
        {
            return INTERNAL_get_mouse_button(button);
        }

        /// <summary>
        /// Returns true during the frame the user pressed the given mouse button
        /// </summary>
        /// <param name="button"></param>
        /// <returns></returns>
        public static bool GetMouseButtonDown(int button)
        {
            return INTERNAL_get_mouse_button_down(button);
        }

        /// <summary>
        /// Returns true during the frame the user releases the given mouse button
        /// </summary>
        /// <param name="button"></param>
        /// <returns></returns>
        public static bool GetMouseButtonUp(int button)
        {
            return INTERNAL_get_mouse_button_up(button);
        }

        /// <summary>
        /// Returns direction in which the mouse wheel was scrolled
        /// </summary>
        /// <returns></returns>
        public static Vector2 GetMouseWheel()
        {
            INTERNAL_get_mouse_wheel(out Vector2 value);
            return value;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_get_cursor_direction(out Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_get_cursor_position(out Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_get_cursor_relative_position(out Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool INTERNAL_get_key(int key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool INTERNAL_get_key_down(int key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool INTERNAL_get_key_up(int key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool INTERNAL_get_mouse_button(int button);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool INTERNAL_get_mouse_button_down(int button);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool INTERNAL_get_mouse_button_up(int button);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_get_mouse_wheel(out Vector2 value);
    }
}