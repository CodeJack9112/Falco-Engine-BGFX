using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Button : UIElement
    {
        internal Button()
        {

        }

        /// <summary>
        /// Returns the texture in the normal state
        /// </summary>
        public Texture normalStateTexture { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns the texture in the hover state (when the mouse cursor is above this button)
        /// </summary>
        public Texture hoverStateTexture { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns the texture in the active state (when the mouse is clicked and held down on this button)
        /// </summary>
        public Texture activeStateTexture { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns the texture in the disabled state
        /// </summary>
        public Texture disabledStateTexture { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Returns the button color in the hover state (when the mouse cursor is above this button)
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
        /// Returns the button color in the active state (when the mouse is clicked and held down on this button)
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
        /// Returns the button color in the disabled state
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
        /// Gets or sets the button interactability (i.e. disable or enable this button or get its state)
        /// </summary>
        public bool interactable { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

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
    }
}
