using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public abstract class Component
    {
        protected Component()
        {

        }

        private IntPtr this_ptr = (IntPtr)0;

        /*----------- PUBLIC ------------*/

        /// <summary>
        /// Enable or disable this component
        /// </summary>
        public bool enabled { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Get game object at which this component is attached
        /// </summary>
        public GameObject gameObject { [MethodImpl(MethodImplOptions.InternalCall)] get; }

        /// <summary>
        /// Returns if this component is derived from UIElement (or this is a canvas) or not
        /// </summary>
        public bool isUiComponent { [MethodImpl(MethodImplOptions.InternalCall)] get; }
    }
}
