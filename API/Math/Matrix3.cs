using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;

namespace FalcoEngine
{
    public struct Matrix3
    {
        public float m00;
        public float m10;
        public float m20;
        public float m01;
        public float m11;
        public float m21;
        public float m02;
        public float m12;
        public float m22;

        /// <summary>
        ///   <para>Returns a matrix with all elements set to zero (Read Only).</para>
        /// </summary>
        private static readonly Matrix3 zeroMatrix = new Matrix3(new Vector3(0f, 0f, 0f), new Vector3(0f, 0f, 0f), new Vector3(0f, 0f, 0f));

        /// <summary>
        ///   <para>Returns the identity matrix (Read Only).</para>
        /// </summary>
        private static readonly Matrix3 identityMatrix = new Matrix3(new Vector3(1f, 0f, 0f), new Vector3(0f, 1f, 0f), new Vector3(0f, 0f, 1f));

        public Matrix3(Vector3 column0, Vector3 column1, Vector3 column2)
        {
            m00 = column0.x;
            m01 = column1.x;
            m02 = column2.x;
            m10 = column0.y;
            m11 = column1.y;
            m12 = column2.y;
            m20 = column0.z;
            m21 = column1.z;
            m22 = column2.z;
        }

        public Matrix3(float d00, float d01, float d02,
                float d10, float d11, float d12,
                float d20, float d21, float d22)
        {
            m00 = d00;
            m01 = d01;
            m02 = d02;
            m10 = d10;
            m11 = d11;
            m12 = d12;
            m20 = d20;
            m21 = d21;
            m22 = d22;
        }

        public float this[int row, int column]
        {
            get
            {
                return this[row + column * 3];
            }
            set
            {
                this[row + column * 3] = value;
            }
        }

        public float this[int index]
        {
            get
            {
                switch (index)
                {
                    case 0:
                        return m00;
                    case 1:
                        return m10;
                    case 2:
                        return m20;
                    case 3:
                        return m01;
                    case 4:
                        return m11;
                    case 5:
                        return m21;
                    case 6:
                        return m02;
                    case 7:
                        return m12;
                    case 8:
                        return m22;
                    default:
                        throw new IndexOutOfRangeException("Invalid matrix index!");
                }
            }
            set
            {
                switch (index)
                {
                    case 0:
                        m00 = value;
                        break;
                    case 1:
                        m10 = value;
                        break;
                    case 2:
                        m20 = value;
                        break;
                    case 3:
                        m01 = value;
                        break;
                    case 4:
                        m11 = value;
                        break;
                    case 5:
                        m21 = value;
                        break;
                    case 6:
                        m02 = value;
                        break;
                    case 7:
                        m12 = value;
                        break;
                    case 8:
                        m22 = value;
                        break;
                    default:
                        throw new IndexOutOfRangeException("Invalid matrix index!");
                }
            }
        }

        public override bool Equals(object other)
        {
            if (!(other is Matrix3))
            {
                return false;
            }
            return Equals((Matrix3)other);
        }

        public bool Equals(Matrix3 other)
        {
            return GetColumn(0).Equals(other.GetColumn(0)) && GetColumn(1).Equals(other.GetColumn(1)) && GetColumn(2).Equals(other.GetColumn(2)) && GetColumn(3).Equals(other.GetColumn(3));
        }

        public static Matrix3 operator *(Matrix3 lhs, Matrix3 rhs)
        {
            Matrix3 result = default(Matrix3);
            result.m00 = lhs.m00 * rhs.m00 + lhs.m01 * rhs.m10 + lhs.m02 * rhs.m20;
            result.m01 = lhs.m00 * rhs.m01 + lhs.m01 * rhs.m11 + lhs.m02 * rhs.m21;
            result.m02 = lhs.m00 * rhs.m02 + lhs.m01 * rhs.m12 + lhs.m02 * rhs.m22;
            result.m10 = lhs.m10 * rhs.m00 + lhs.m11 * rhs.m10 + lhs.m12 * rhs.m20;
            result.m11 = lhs.m10 * rhs.m01 + lhs.m11 * rhs.m11 + lhs.m12 * rhs.m21;
            result.m12 = lhs.m10 * rhs.m02 + lhs.m11 * rhs.m12 + lhs.m12 * rhs.m22;
            result.m20 = lhs.m20 * rhs.m00 + lhs.m21 * rhs.m10 + lhs.m22 * rhs.m20;
            result.m21 = lhs.m20 * rhs.m01 + lhs.m21 * rhs.m11 + lhs.m22 * rhs.m21;
            result.m22 = lhs.m20 * rhs.m02 + lhs.m21 * rhs.m12 + lhs.m22 * rhs.m22;
            return result;
        }

        public static Vector3 operator *(Matrix3 lhs, Vector3 vector)
        {
            Vector3 result = default(Vector3);
            result.x = lhs.m00 * vector.x + lhs.m01 * vector.y + lhs.m02 * vector.z;
            result.y = lhs.m10 * vector.x + lhs.m11 * vector.y + lhs.m12 * vector.z;
            result.z = lhs.m20 * vector.x + lhs.m21 * vector.y + lhs.m22 * vector.z;
            return result;
        }

        public static bool operator ==(Matrix3 lhs, Matrix3 rhs)
        {
            return lhs.GetColumn(0) == rhs.GetColumn(0) && lhs.GetColumn(1) == rhs.GetColumn(1) && lhs.GetColumn(2) == rhs.GetColumn(2);
        }

        public static bool operator !=(Matrix3 lhs, Matrix3 rhs)
        {
            return !(lhs == rhs);
        }

        /// <summary>
        ///   <para>Get a column of the matrix.</para>
        /// </summary>
        /// <param name="index"></param>
        public Vector3 GetColumn(int index)
        {
            switch (index)
            {
                case 0:
                    return new Vector3(m00, m10, m20);
                case 1:
                    return new Vector3(m01, m11, m21);
                case 2:
                    return new Vector3(m02, m12, m22);
                default:
                    throw new IndexOutOfRangeException("Invalid column index!");
            }
        }

        /// <summary>
        ///   <para>Returns a row of the matrix.</para>
        /// </summary>
        /// <param name="index"></param>
        public Vector3 GetRow(int index)
        {
            switch (index)
            {
                case 0:
                    return new Vector3(m00, m01, m02);
                case 1:
                    return new Vector3(m10, m11, m12);
                case 2:
                    return new Vector3(m20, m21, m22);
                default:
                    throw new IndexOutOfRangeException("Invalid row index!");
            }
        }

        /// <summary>
        ///   <para>Sets a column of the matrix.</para>
        /// </summary>
        /// <param name="index"></param>
        /// <param name="column"></param>
        public void SetColumn(int index, Vector3 column)
        {
            this[0, index] = column.x;
            this[1, index] = column.y;
            this[2, index] = column.z;
        }

        /// <summary>
        ///   <para>Sets a row of the matrix.</para>
        /// </summary>
        /// <param name="index"></param>
        /// <param name="row"></param>
        public void SetRow(int index, Vector3 row)
        {
            this[index, 0] = row.x;
            this[index, 1] = row.y;
            this[index, 2] = row.z;
        }

        /// <summary>
        ///   <para>Transforms a direction by this matrix.</para>
        /// </summary>
        /// <param name="vector"></param>
        public Vector3 MultiplyVector(Vector3 vector)
        {
            Vector3 result = default(Vector3);
            result.x = m00 * vector.x + m01 * vector.y + m02 * vector.z;
            result.y = m10 * vector.x + m11 * vector.y + m12 * vector.z;
            result.z = m20 * vector.x + m21 * vector.y + m22 * vector.z;
            return result;
        }

        /// <summary>
        ///   <para>Creates a scaling matrix.</para>
        /// </summary>
        /// <param name="vector"></param>
        public static Matrix3 Scale(Vector3 vector)
        {
            Matrix3 result = default(Matrix3);
            result.m00 = vector.x;
            result.m01 = 0f;
            result.m02 = 0f;
            result.m10 = 0f;
            result.m11 = vector.y;
            result.m12 = 0f;
            result.m20 = 0f;
            result.m21 = 0f;
            result.m22 = vector.z;
            return result;
        }

        /// <summary>
        ///   <para>Creates a rotation matrix.</para>
        /// </summary>
        /// <param name="q"></param>
        public static Matrix3 Rotate(Quaternion q)
        {
            float num = q.x * 2f;
            float num2 = q.y * 2f;
            float num3 = q.z * 2f;
            float num4 = q.x * num;
            float num5 = q.y * num2;
            float num6 = q.z * num3;
            float num7 = q.x * num2;
            float num8 = q.x * num3;
            float num9 = q.y * num3;
            float num10 = q.w * num;
            float num11 = q.w * num2;
            float num12 = q.w * num3;
            Matrix3 result = default(Matrix3);
            result.m00 = 1f - (num5 + num6);
            result.m10 = num7 + num12;
            result.m20 = num8 - num11;
            result.m01 = num7 - num12;
            result.m11 = 1f - (num4 + num6);
            result.m21 = num9 + num10;
            result.m02 = num8 + num11;
            result.m12 = num9 - num10;
            result.m22 = 1f - (num4 + num5);
            return result;
        }

        /// <summary>
        ///   <para>Returns a nicely formatted string for this matrix.</para>
        /// </summary>
        /// <param name="format"></param>
        public override string ToString()
        {
            return String.Format("{0:F5}\t{1:F5}\t{2:F5}\t{3:F5}\n{4:F5}\t{5:F5}\t{6:F5}\t{7:F5}\n{8:F5}\n", m00, m01, m02, m10, m11, m12, m20, m21, m22);
        }

        /// <summary>
        ///   <para>Returns a nicely formatted string for this matrix.</para>
        /// </summary>
        /// <param name="format"></param>
        public string ToString(string format)
        {
            return String.Format("{0}\t{1}\t{2}\t{3}\n{4}\t{5}\t{6}\t{7}\n{8}\n", ToInvariantString(format, m00), ToInvariantString(format, m01), ToInvariantString(format, m02), ToInvariantString(format, m10), ToInvariantString(format, m11), ToInvariantString(format, m12), ToInvariantString(format, m20), ToInvariantString(format, m21), ToInvariantString(format, m22));
        }

        private string ToInvariantString(string format, float val)
        {
            return val.ToString(format, CultureInfo.InvariantCulture.NumberFormat);
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
    }
}
