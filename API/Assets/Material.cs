using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public class Material : Asset
    {
        internal Material() { }

        /// <summary>
        /// Create a new material and set its name
        /// </summary>
        /// <param name="name">Name for the new material</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern Material(string name);

        /// <summary>
        /// Load existing material
        /// </summary>
        /// <param name="name">Name of the existing material</param>
        /// <returns>Loaded material</returns>
        public static Material Load(string name)
        {
            return INTERNAL_load(name);
        }

        /// <summary>
        /// Get or set a shader for this material
        /// </summary>
        public Shader shader { [MethodImpl(MethodImplOptions.InternalCall)] get; [MethodImpl(MethodImplOptions.InternalCall)] set; }

        //Get parameters

        /// <summary>
        /// Get value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="matrix">Matrix3 value</param>
        public void GetParameter(string name, out Matrix3 matrix)
        {
            INTERNAL_getParameterMat3(name, out matrix);
        }

        /// <summary>
        /// Get value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="matrix">Matrix4 value</param>
        public void GetParameter(string name, out Matrix4 matrix)
        {
            INTERNAL_getParameterMat4(name, out matrix);
        }

        /// <summary>
        /// Get value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="vector2">Vector2 value</param>
        public void GetParameter(string name, out Vector2 vector2)
        {
            INTERNAL_getParameterVec2(name, out vector2);
        }

        /// <summary>
        /// Get value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="vector3">Vector3 value</param>
        public void GetParameter(string name, out Vector3 vector3)
        {
            INTERNAL_getParameterVec3(name, out vector3);
        }

        /// <summary>
        /// Get value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="vector4">Vector4 value</param>
        public void GetParameter(string name, out Vector4 vector4)
        {
            INTERNAL_getParameterVec4(name, out vector4);
        }

        /// <summary>
        /// Get value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="flt">Float value</param>
        public void GetParameter(string name, out float flt)
        {
            flt = INTERNAL_getParameterFloat(name);
        }

        /// <summary>
        /// Get value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="val">Int value</param>
        public void GetParameter(string name, out int val)
        {
            val = INTERNAL_getParameterInt(name);
        }

        /// <summary>
        /// Get value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="val">Boolean value</param>
        public void GetParameter(string name, out bool val)
        {
            val = INTERNAL_getParameterBool(name);
        }

        /// <summary>
        /// Get value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="color">Color value</param>
        public void GetParameter(string name, out Color color)
        {
            INTERNAL_getParameterColor(name, out color);
        }

        /// <summary>
        /// Get value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="texture">Texture value</param>
        public void GetParameter(string name, out Texture texture)
        {
            Texture tex = INTERNAL_getParameterTexture(name);
            texture = tex;

        }

        /// <summary>
        /// Get value of the parameter
        /// </summary>
        /// <typeparam name="T">Parameter type</typeparam>
        /// <param name="name">Parameter name</param>
        /// <returns></returns>
        public T GetParameter<T>(string name)
        {
            if (typeof(T) == typeof(Texture))
            {
                Texture value = INTERNAL_getParameterTexture(name);
                return (T)Convert.ChangeType(value, typeof(T));
            }

            if (typeof(T) == typeof(Matrix3))
            {
                INTERNAL_getParameterMat3(name, out Matrix3 value);
                return (T)Convert.ChangeType(value, typeof(T));
            }

            if (typeof(T) == typeof(Matrix4))
            {
                INTERNAL_getParameterMat4(name, out Matrix4 value);
                return (T)Convert.ChangeType(value, typeof(T));
            }

            if (typeof(T) == typeof(Vector2))
            {
                INTERNAL_getParameterVec2(name, out Vector2 value);
                return (T)Convert.ChangeType(value, typeof(T));
            }

            if (typeof(T) == typeof(Vector3))
            {
                INTERNAL_getParameterVec3(name, out Vector3 value);
                return (T)Convert.ChangeType(value, typeof(T));
            }

            if (typeof(T) == typeof(Vector4))
            {
                INTERNAL_getParameterVec4(name, out Vector4 value);
                return (T)Convert.ChangeType(value, typeof(T));
            }

            if (typeof(T) == typeof(float))
            {
                float value = INTERNAL_getParameterFloat(name);
                return (T)Convert.ChangeType(value, typeof(T));
            }

            if (typeof(T) == typeof(int))
            {
                int value = INTERNAL_getParameterInt(name);
                return (T)Convert.ChangeType(value, typeof(T));
            }

            if (typeof(T) == typeof(bool))
            {
                bool value = INTERNAL_getParameterBool(name);
                return (T)Convert.ChangeType(value, typeof(T));
            }

            if (typeof(T) == typeof(Color))
            {
                INTERNAL_getParameterColor(name, out Color value);
                return (T)Convert.ChangeType(value, typeof(T));
            }

            return default(T);
        }

        //Set parameters

        /// <summary>
        /// Set value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="matrix">Matrix3 value</param>
        public void SetParameter(string name, Matrix3 matrix)
        {
            INTERNAL_setParameterMat3(name, ref matrix);
        }

        /// <summary>
        /// Set value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="matrix">Matrix4 value</param>
        public void SetParameter(string name, Matrix4 matrix)
        {
            INTERNAL_setParameterMat4(name, ref matrix);
        }

        /// <summary>
        /// Set value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="vector2">Vector2 value</param>
        public void SetParameter(string name, Vector2 vector2)
        {
            INTERNAL_setParameterVec2(name, ref vector2);
        }

        /// <summary>
        /// Set value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="vector3">Vector3 value</param>
        public void SetParameter(string name, Vector3 vector3)
        {
            INTERNAL_setParameterVec3(name, ref vector3);
        }

        /// <summary>
        /// Set value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="vector4">Vector4 value</param>
        public void SetParameter(string name, Vector4 vector4)
        {
            INTERNAL_setParameterVec4(name, ref vector4);
        }

        /// <summary>
        /// Set value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="flt">Float value</param>
        public void SetParameter(string name, float flt)
        {
            INTERNAL_setParameterFloat(name, flt);
        }

        /// <summary>
        /// Set value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="val">Int value</param>
        public void SetParameter(string name, int val)
        {
            INTERNAL_setParameterInt(name, val);
        }

        /// <summary>
        /// Set value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="val">Boolean value</param>
        public void SetParameter(string name, bool val)
        {
            INTERNAL_setParameterBool(name, val);
        }

        /// <summary>
        /// Set value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="color">Color value</param>
        public void SetParameter(string name, Color color)
        {
            INTERNAL_setParameterColor(name, ref color);
        }

        /// <summary>
        /// Set value of the parameter
        /// </summary>
        /// <param name="name">Parameter name</param>
        /// <param name="texture">Texture value</param>
        public void SetParameter(string name, Texture texture)
        {
            INTERNAL_setParameterTexture(name, texture);
        }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Material INTERNAL_load(string name);

        //Get
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getParameterMat3(string name, out Matrix3 matrix);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getParameterMat4(string name, out Matrix4 matrix);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getParameterVec2(string name, out Vector2 vector2);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getParameterVec3(string name, out Vector3 vector3);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getParameterVec4(string name, out Vector4 vector4);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern float INTERNAL_getParameterFloat(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern int INTERNAL_getParameterInt(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern bool INTERNAL_getParameterBool(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_getParameterColor(string name, out Color color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern Texture INTERNAL_getParameterTexture(string name);

        //Set
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setParameterMat3(string name, ref Matrix3 matrix);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setParameterMat4(string name, ref Matrix4 matrix);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setParameterVec2(string name, ref Vector2 vector2);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setParameterVec3(string name, ref Vector3 vector3);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setParameterVec4(string name, ref Vector4 vector4);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setParameterFloat(string name, float flt);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setParameterInt(string name, int val);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setParameterBool(string name, bool val);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setParameterColor(string name, ref Color color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void INTERNAL_setParameterTexture(string name, Texture texture);
    }
}
