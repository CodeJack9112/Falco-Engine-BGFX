using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;

namespace FalcoEngine
{
    public class Light : Component
    {
        internal Light()
        {

        }

        /*----------- PUBLIC ------------*/

        /// <summary>
        /// LightType enum
        /// </summary>
        public enum LightType
        {
            Point,
            Spot,
            Directional
        };

        /// <summary>
        /// Get or set the light type. Point, spot or directional
        /// </summary>
        public LightType type { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// The color of the light
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
        /// The Intensity of a light is multiplied with the Light color
        /// </summary>
        public float intensity { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// The range of the light
        /// </summary>
        public float radius { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// The angle of the light's spotlight inner cone in degrees
        /// </summary>
        public float innerRadius { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// The angle of the light's spotlight cone in degrees.
        /// </summary>
        public float outerRadius { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Shadow mapping constant bias
        /// </summary>
        public float bias { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /// <summary>
        /// Enable or disable casting shadows for this light source
        /// </summary>
        public bool castShadows { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getColor(out Color color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setColor(ref Color color);
    }
}
