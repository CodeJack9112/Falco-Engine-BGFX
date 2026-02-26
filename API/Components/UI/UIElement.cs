using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class UIElement : Component
    {
        internal UIElement()
        {

        }

        /// <summary>
        /// CanvasHorizontalAlignment enum
        /// </summary>
        public enum CanvasHorizontalAlignment { Left, Center, Right };

        /// <summary>
        /// CanvasVerticalAlignment enum
        /// </summary>
        public enum CanvasVerticalAlignment { Top, Middle, Bottom };

        public delegate void MouseButtonEvent(UIElement sender, int button, Vector2 cursorPosition);
        public delegate void MouseMoveEvent(UIElement sender, Vector2 cursorPosition);
        public event MouseButtonEvent onMouseDown;
        public event MouseButtonEvent onMouseUp;
        public event MouseMoveEvent onMouseMove;
        public event MouseMoveEvent onMouseEnter;
        public event MouseMoveEvent onMouseExit;

        /// <summary>
        /// Get or set horizontal alignment of this element
        /// </summary>
        public CanvasHorizontalAlignment horizontalAlignment
        {
            get
            {
                return (CanvasHorizontalAlignment)INTERNAL_getHorizontalAlignment();
            }
            set
            {
                INTERNAL_setHorizontalAlignment((int)value);
            }
        }

        /// <summary>
        /// Get or set vertical alignment of this element
        /// </summary>
        public CanvasVerticalAlignment verticalAlignment
        {
            get
            {
                return (CanvasVerticalAlignment)INTERNAL_getVerticalAlignment();
            }
            set
            {
                INTERNAL_setVerticalAlignment((int)value);
            }
        }

        /// <summary>
        /// Get or set the main color of this element
        /// </summary>
        public Color color
        {
            get
            {
                INTERNAL_getColor(out Color value);
                return value;
            }
            set
            {
                INTERNAL_setColor(ref value);
            }
        }

        /// <summary>
        /// Get or set the anchor of this element (pivot point)
        /// </summary>
        public Vector2 anchor
        {
            get
            {
                INTERNAL_getAnchor(out Vector2 value);
                return value;
            }
            set
            {
                INTERNAL_setAnchor(ref value);
            }
        }

        /// <summary>
        /// Get or set the size of this element (width and height)
        /// </summary>
        public Vector2 size
        {
            get
            {
                INTERNAL_getSize(out Vector2 value);
                return value;
            }
            set
            {
                INTERNAL_setSize(ref value);
            }
        }

        /// <summary>
        /// Returns if this element is hovered by mouse cursor
        /// </summary>
        public bool hovered { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Returns a canvas of this element
        /// </summary>
        public Canvas canvas { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        public Vector3 position
        {
            get
            {
                INTERNAL_getPosition(out Vector3 value);
                return value;
            }
            set
            {
                INTERNAL_setPosition(ref value);
            }
        }

        public Rect rect
        {
            get
            {
                INTERNAL_getRect(out Rect value);
                return value;
            }
        }

        /// <summary>
        /// Get or set the user data
        /// </summary>
        public object userData { get; set; }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getColor(out Color value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setColor(ref Color value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getAnchor(out Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setAnchor(ref Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern int INTERNAL_getHorizontalAlignment();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setHorizontalAlignment(int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern int INTERNAL_getVerticalAlignment();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setVerticalAlignment(int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getSize(out Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setSize(ref Vector2 value);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getPosition(out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setPosition(ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getRect(out Rect value);

        private void CallOnMouseDown(UIElement sender, int button, Vector2 cursorPosition)
        {
            onMouseDown?.Invoke(sender, button, cursorPosition);
        }

        private void CallOnMouseUp(UIElement sender, int button, Vector2 cursorPosition)
        {
            onMouseUp?.Invoke(sender, button, cursorPosition);
        }

        private void CallOnMouseMove(UIElement sender, Vector2 cursorPosition)
        {
            onMouseMove?.Invoke(sender, cursorPosition);
        }

        private void CallOnMouseEnter(UIElement sender, Vector2 cursorPosition)
        {
            onMouseEnter?.Invoke(sender, cursorPosition);
        }

        private void CallOnMouseExit(UIElement sender, Vector2 cursorPosition)
        {
            onMouseExit?.Invoke(sender, cursorPosition);
        }
    }
}
