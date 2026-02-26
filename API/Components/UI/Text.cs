using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Text : UIElement
    {
        internal Text()
        {

        }

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
        private extern int INTERNAL_get_textHorizontalAlignment();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_textHorizontalAlignment(int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern int INTERNAL_get_textVerticalAlignment();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_set_textVerticalAlignment(int value);
    }
}
