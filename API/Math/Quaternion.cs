using System;
using System.Runtime.CompilerServices;

namespace FalcoEngine
{
    public struct Quaternion
    {
        public const float kEpsilon = 0.000001F;

        public float x;
        public float y;
        public float z;
        public float w;

        public static Quaternion identity { get; } = new Quaternion(0, 0, 0, 1);

        public Quaternion(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public static Quaternion operator *(Quaternion lhs, Quaternion rhs)
        {
            return new Quaternion(lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y, lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z, lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x, lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z);
        }

        public static Vector3 operator *(Quaternion rotation, Vector3 point)
        {
            float num = rotation.x * 2f;
            float num2 = rotation.y * 2f;
            float num3 = rotation.z * 2f;
            float num4 = rotation.x * num;
            float num5 = rotation.y * num2;
            float num6 = rotation.z * num3;
            float num7 = rotation.x * num2;
            float num8 = rotation.x * num3;
            float num9 = rotation.y * num3;
            float num10 = rotation.w * num;
            float num11 = rotation.w * num2;
            float num12 = rotation.w * num3;
            Vector3 result = default(Vector3);
            result.x = (1f - (num5 + num6)) * point.x + (num7 - num12) * point.y + (num8 + num11) * point.z;
            result.y = (num7 + num12) * point.x + (1f - (num4 + num6)) * point.y + (num9 - num10) * point.z;
            result.z = (num8 - num11) * point.x + (num9 + num10) * point.y + (1f - (num4 + num5)) * point.z;
            return result;
        }

        public static Quaternion operator +(Quaternion a, Quaternion b)
        {
            return new Quaternion(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        }

        public static Quaternion operator -(Quaternion q)
        {
            return new Quaternion(-q.x, -q.y, -q.z, -q.w);
        }

        // Is the dot product of two quaternions within tolerance for them to be considered equal?
        private static bool IsEqualUsingDot(float dot)
        {
            // Returns false in the presence of NaN values.
            return dot > 1.0f - kEpsilon;
        }

        // Are two quaternions equal to each other?
        public static bool operator ==(Quaternion lhs, Quaternion rhs)
        {
            return IsEqualUsingDot(Dot(lhs, rhs));
        }

        // Are two quaternions different from each other?
        public static bool operator !=(Quaternion lhs, Quaternion rhs)
        {
            // Returns true in the presence of NaN values.
            return !(lhs == rhs);
        }

        // The dot product between two rotations.
        public static float Dot(Quaternion a, Quaternion b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        }

        public static Quaternion Euler(Vector3 angles)
        {
            INTERNAL_Euler(ref angles, out Quaternion value);
            return value;
        }

        public static Quaternion Euler(float x, float y, float z)
        {
            Vector3 vec = new Vector3(x, y, z);
            INTERNAL_Euler(ref vec, out Quaternion value);
            return value;
        }

        public static Quaternion AngleAxis(float angle, Vector3 axis)
        {
            INTERNAL_AngleAxis(angle, ref axis, out Quaternion ret);
            return ret;
        }

        public static Quaternion LookRotation(Vector3 direction, Vector3 axis)
        {
            INTERNAL_LookRotation(ref direction, out Quaternion ret);

            return ret;
        }

        // Set x, y, z and w components of an existing Quaternion.
        public void Set(float newX, float newY, float newZ, float newW)
        {
            x = newX;
            y = newY;
            z = newZ;
            w = newW;
        }

        //////////////

        public void SetLookRotation(Vector3 view)
        {
            Vector3 up = Vector3.up;
            SetLookRotation(view, up);
        }

        // Creates a rotation with the specified /forward/ and /upwards/ directions.
        public void SetLookRotation(Vector3 view, Vector3 up)
        {
            this = LookRotation(view, up);
        }

        // Returns the angle in degrees between two rotations /a/ and /b/.
        public static float Angle(Quaternion a, Quaternion b)
        {
            float dot = Dot(a, b);
            return IsEqualUsingDot(dot) ? 0.0f : Mathf.Acos(Mathf.Min(Mathf.Abs(dot), 1.0F)) * 2.0F * Mathf.Rad2Deg;
        }

        public static Quaternion Slerp(Quaternion q1, Quaternion q2, float t)
        {
            INTERNAL_Slerp(ref q1, ref q2, t, out Quaternion ret);

            return ret;
        }

        public static Quaternion RotateTowards(Quaternion from, Quaternion to, float maxDegreesDelta)
        {
            float angle = Quaternion.Angle(from, to);
            if (angle == 0.0f) return to;
            return Slerp(from, to, Mathf.Min(1.0f, maxDegreesDelta / angle));
        }

        public static Quaternion Normalize(Quaternion q)
        {
            float mag = Mathf.Sqrt(Dot(q, q));

            if (mag < Mathf.Epsilon)
                return Quaternion.identity;

            return new Quaternion(q.x / mag, q.y / mag, q.z / mag, q.w / mag);
        }

        public void Normalize()
        {
            this = Normalize(this);
        }

        public Quaternion normalized
        {
            get { return Normalize(this); }
        }

        // used to allow Quaternions to be used as keys in hash tables
        public override int GetHashCode()
        {
            return x.GetHashCode() ^ (y.GetHashCode() << 2) ^ (z.GetHashCode() >> 2) ^ (w.GetHashCode() >> 1);
        }

        // also required for being able to use Quaternions as keys in hash tables
        public override bool Equals(object other)
        {
            if (!(other is Quaternion)) return false;

            return Equals((Quaternion)other);
        }

        public bool Equals(Quaternion other)
        {
            return x.Equals(other.x) && y.Equals(other.y) && z.Equals(other.z) && w.Equals(other.w);
        }

        public string ToString(string format, IFormatProvider formatProvider)
        {
            if (string.IsNullOrEmpty(format))
                format = "F1";
            return string.Format("({0}, {1}, {2}, {3})", x.ToString(format, formatProvider), y.ToString(format, formatProvider), z.ToString(format, formatProvider), w.ToString(format, formatProvider));
        }

        public Vector3 eulerAngles {
            get
            {
                INTERNAL_EulerAngles(ref this, out Vector3 ret);
                return ret;
            }
            set
            {
                this = Euler(value);
            }
        }

        public Quaternion Conjugate()
        {
            return new Quaternion(-x, -y, -z, w);
        }

        Quaternion Scale(float s)
        {
            return new Quaternion(x * s, y * s, z * s, w * s);
        }

        public Quaternion Inverse()
        {
            float norm = x * x + y * y + z * z + w * w;

            return Conjugate().Scale(1.0f / norm);
        }

        /*----------- INTERNAL CALLS ------------*/

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void INTERNAL_Euler(ref Vector3 euler, out Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void INTERNAL_AngleAxis(float angle, ref Vector3 axis, out Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void INTERNAL_LookRotation(ref Vector3 direction, out Quaternion ret);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void INTERNAL_EulerAngles(ref Quaternion quaternion, out Vector3 ret);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void INTERNAL_Slerp(ref Quaternion q1, ref Quaternion q2, float t, out Quaternion ret);
    }
}
