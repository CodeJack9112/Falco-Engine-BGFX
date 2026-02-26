using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class TextInput : UIElement
    {
        internal TextInput()
        {

        }

        /// <summary>
        /// Returns the texture in the normal state
        /// </summary>
        public Texture normalStateTexture { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns the texture in the hover state (when the mouse cursor is above this text input)
        /// </summary>
        public Texture hoverStateTexture { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns the texture in the active state (when the mouse is clicked and held down on this text input)
        /// </summary>
        public Texture activeStateTexture { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns the texture in the disabled state
        /// </summary>
        public Texture disabledStateTexture { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns the color in the hover state (when the mouse cursor is above this text input)
        /// </summary>
        public Color hoverStateColor
        {
            get
            {
                INTERNAL_getHoverStateColor(out Color value);
                return value;
            }
            set
            {
                INTERNAL_setHoverStateColor(ref value);
            }
        }

        /// <summary>
        /// Returns the color in the active state (when the mouse is clicked and held down on this text input)
        /// </summary>
        public Color activeStateColor
        {
            get
            {
                INTERNAL_getActiveStateColor(out Color value);
                return value;
            }
            set
            {
                INTERNAL_setActiveStateColor(ref value);
            }
        }

        /// <summary>
        /// Returns the color in the disabled state
        /// </summary>
        public Color disabledStateColor
        {
            get
            {
                INTERNAL_getDisabledStateColor(out Color value);
                return value;
            }
            set
            {
                INTERNAL_setDisabledStateColor(ref value);
            }
        }

        /// <summary>
        /// Gets or sets the text input interactability (i.e. disable or enable this text input or get its state)
        /// </summary>
        public bool interactable { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// TextHorizontalAlignment enum
        /// </summary>
        public enum TextHorizontalAlignment { Left, Center, Right };

        /// <summary>
        /// TextVerticalAlignment enum
        /// </summary>
        public enum TextVerticalAlignment { Top, Middle, Bottom };

        /// <summary>
        /// Get or set the text
        /// </summary>
        public string text { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Get or set the font
        /// </summary>
        public Font font { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Get or set the font size
        /// </summary>
        public float fontSize { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Get or set text horizontal alignment
        /// </summary>
        public TextHorizontalAlignment textHorizontalAlignment
        {
            get
            {
                return (TextHorizontalAlignment)INTERNAL_get_textHorizontalAlignment();
            }
            set
            {
                INTERNAL_set_textHorizontalAlignment((int)value);
            }
        }

        /// <summary>
        /// Get or set text vertical alignment
        /// </summary>
        public TextVerticalAlignment textVerticalAlignment
        {
            get
            {
                return (TextVerticalAlignment)INTERNAL_get_textVerticalAlignment();
            }
            set
            {
                INTERNAL_set_textVerticalAlignment((int)value);
            }
        }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getHoverStateColor(out Color value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setHoverStateColor(ref Color value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getActiveStateColor(out Color value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setActiveStateColor(ref Color value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getDisabledStateColor(out Color value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setDisabledStateColor(ref Color value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern int INTERNAL_get_textHorizontalAlignment();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_textHorizontalAlignment(int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern int INTERNAL_get_textVerticalAlignment();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_textVerticalAlignment(int value);
    }
}
