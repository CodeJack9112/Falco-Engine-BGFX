#pragma once

#include <array>

#include "../glm/mat4x4.hpp"
#include "../glm/vec3.hpp"

//#include "Mathf.h"

/** \addtogroup Core
*  @{
*/
/** \addtogroup Math
*  @{
*/

namespace GX
{
    /** A 3D box aligned with the x/y/z axes.
    @remarks
    This class represents a simple box which is aligned with the
    axes. Internally it only stores 2 points as the extremeties of
    the box, one which is the minima of all 3 axes, and the other
    which is the maxima of all 3 axes. This class is typically used
    for an axis-aligned bounding box (AABB) for collision and
    visibility determination.
    */
    class AxisAlignedBox
    {
    public:
        enum class Extent
        {
            EXTENT_NULL,
            EXTENT_FINITE,
            EXTENT_INFINITE
        };
    protected:

        glm::vec3 mMinimum;
        glm::vec3 mMaximum;
        Extent mExtent;

    public:
        /*
        1-------2
        /|      /|
        / |     / |
        5-------4  |
        |  0----|--3
        | /     | /
        |/      |/
        6-------7
        */
        enum class CornerEnum
        {
            FAR_LEFT_BOTTOM = 0,
            FAR_LEFT_TOP = 1,
            FAR_RIGHT_TOP = 2,
            FAR_RIGHT_BOTTOM = 3,
            NEAR_RIGHT_BOTTOM = 7,
            NEAR_LEFT_BOTTOM = 6,
            NEAR_LEFT_TOP = 5,
            NEAR_RIGHT_TOP = 4
        };
        typedef std::array<glm::vec3, 8> Corners;

        inline AxisAlignedBox() : mMinimum(glm::vec3(0, 0, 0)), mMaximum(glm::vec3(1, 1,1))
        {
            // Default to a null box 
            setMinimum( -0.5, -0.5, -0.5 );
            setMaximum( 0.5, 0.5, 0.5 );
            mExtent = Extent::EXTENT_NULL;
        }
        inline AxisAlignedBox(Extent e) : mMinimum(glm::vec3(0, 0, 0)), mMaximum(glm::vec3(1, 1, 1))
        {
            setMinimum( -0.5, -0.5, -0.5 );
            setMaximum( 0.5, 0.5, 0.5 );
            mExtent = e;
        }

        inline AxisAlignedBox(const AxisAlignedBox & rkBox) : mMinimum(glm::vec3(0, 0, 0)), mMaximum(glm::vec3(1, 1, 1))

        {
            if (rkBox.isNull())
                setNull();
            else if (rkBox.isInfinite())
                setInfinite();
            else
                setExtents( rkBox.mMinimum, rkBox.mMaximum );
        }

        inline AxisAlignedBox( const glm::vec3& min, const glm::vec3& max ) : mMinimum(glm::vec3(0, 0, 0)), mMaximum(glm::vec3(1, 1, 1))
        {
            setExtents( min, max );
        }

        inline AxisAlignedBox(
            float mx, float my, float mz,
            float Mx, float My, float Mz ) : mMinimum(glm::vec3(0, 0, 0)), mMaximum(glm::vec3(1, 1, 1))
        {
            setExtents( mx, my, mz, Mx, My, Mz );
        }

        AxisAlignedBox& operator=(const AxisAlignedBox& rhs)
        {
            // Specifically override to avoid copying mCorners
            if (rhs.isNull())
                setNull();
            else if (rhs.isInfinite())
                setInfinite();
            else
                setExtents(rhs.mMinimum, rhs.mMaximum);

            return *this;
        }

        /** Gets the minimum corner of the box.
        */
        inline glm::vec3& getMinimum(void)
        { 
            return mMinimum; 
        }

        /** Gets the maximum corner of the box.
        */
        inline glm::vec3& getMaximum(void)
        { 
            return mMaximum;
        }


        /** Sets the minimum corner of the box.
        */
        inline void setMinimum( const glm::vec3& vec )
        {
            mExtent = Extent::EXTENT_FINITE;
            mMinimum = vec;
        }

        inline void setMinimum( float x, float y, float z )
        {
            mExtent = Extent::EXTENT_FINITE;
            mMinimum.x = x;
            mMinimum.y = y;
            mMinimum.z = z;
        }

        /** Changes one of the components of the minimum corner of the box
        used to resize only one dimension of the box
        */
        inline void setMinimumX(float x)
        {
            mMinimum.x = x;
        }

        inline void setMinimumY(float y)
        {
            mMinimum.y = y;
        }

        inline void setMinimumZ(float z)
        {
            mMinimum.z = z;
        }

        /** Sets the maximum corner of the box.
        */
        inline void setMaximum( const glm::vec3& vec )
        {
            mExtent = Extent::EXTENT_FINITE;
            mMaximum = vec;
        }

        inline void setMaximum( float x, float y, float z )
        {
            mExtent = Extent::EXTENT_FINITE;
            mMaximum.x = x;
            mMaximum.y = y;
            mMaximum.z = z;
        }

        /** Changes one of the components of the maximum corner of the box
        used to resize only one dimension of the box
        */
        inline void setMaximumX( float x )
        {
            mMaximum.x = x;
        }

        inline void setMaximumY( float y )
        {
            mMaximum.y = y;
        }

        inline void setMaximumZ( float z )
        {
            mMaximum.z = z;
        }

        /** Sets both minimum and maximum extents at once.
        */
        inline void setExtents( const glm::vec3& min, const glm::vec3& max )
        {
            assert( (min.x <= max.x && min.y <= max.y && min.z <= max.z) &&
                "The minimum corner of the box must be less than or equal to maximum corner" );

            mExtent = Extent::EXTENT_FINITE;
            mMinimum = min;
            mMaximum = max;
        }

        inline void setExtents(
            float mx, float my, float mz,
            float Mx, float My, float Mz )
        {
            assert( (mx <= Mx && my <= My && mz <= Mz) &&
                "The minimum corner of the box must be less than or equal to maximum corner" );

            mExtent = Extent::EXTENT_FINITE;

            mMinimum.x = mx;
            mMinimum.y = my;
            mMinimum.z = mz;

            mMaximum.x = Mx;
            mMaximum.y = My;
            mMaximum.z = Mz;

        }

        /** Returns a pointer to an array of 8 corner points, useful for
        collision vs. non-aligned objects.
        @remarks
        If the order of these corners is important, they are as
        follows: The 4 points of the minimum Z face (note that
        because Ogre uses right-handed coordinates, the minimum Z is
        at the 'back' of the box) starting with the minimum point of
        all, then anticlockwise around this face (if you are looking
        onto the face from outside the box). Then the 4 points of the
        maximum Z face, starting with maximum point of all, then
        anticlockwise around this face (looking onto the face from
        outside the box). Like this:
        <pre>
            1-------2
            /|      /|
            / |     / |
        5-------4  |
        |  0----|--3
        | /     | /
        |/      |/
        6-------7
        </pre>
        */
        inline Corners getAllCorners(void) const
        {
            assert( (mExtent == Extent::EXTENT_FINITE) && "Can't get corners of a null or infinite AAB" );

            // The order of these items is, using right-handed co-ordinates:
            // Minimum Z face, starting with Min(all), then anticlockwise
            //   around face (looking onto the face)
            // Maximum Z face, starting with Max(all), then anticlockwise
            //   around face (looking onto the face)
            // Only for optimization/compatibility.
            Corners corners;

            corners[0] = getCorner(CornerEnum::FAR_LEFT_BOTTOM);
            corners[1] = getCorner(CornerEnum::FAR_LEFT_TOP);
            corners[2] = getCorner(CornerEnum::FAR_RIGHT_TOP);
            corners[3] = getCorner(CornerEnum::FAR_RIGHT_BOTTOM);

            corners[4] = getCorner(CornerEnum::NEAR_RIGHT_TOP);
            corners[5] = getCorner(CornerEnum::NEAR_LEFT_TOP);
            corners[6] = getCorner(CornerEnum::NEAR_LEFT_BOTTOM);
            corners[7] = getCorner(CornerEnum::NEAR_RIGHT_BOTTOM);

            return corners;
        }

        /** Gets the position of one of the corners
        */
        glm::vec3 getCorner(CornerEnum cornerToGet) const
        {
            switch(cornerToGet)
            {
            case CornerEnum::FAR_LEFT_BOTTOM:
                return mMinimum;
            case CornerEnum::FAR_LEFT_TOP:
                return glm::vec3(mMinimum.x, mMaximum.y, mMinimum.z);
            case CornerEnum::FAR_RIGHT_TOP:
                return glm::vec3(mMaximum.x, mMaximum.y, mMinimum.z);
            case CornerEnum::FAR_RIGHT_BOTTOM:
                return glm::vec3(mMaximum.x, mMinimum.y, mMinimum.z);
            case CornerEnum::NEAR_RIGHT_BOTTOM:
                return glm::vec3(mMaximum.x, mMinimum.y, mMaximum.z);
            case CornerEnum::NEAR_LEFT_BOTTOM:
                return glm::vec3(mMinimum.x, mMinimum.y, mMaximum.z);
            case CornerEnum::NEAR_LEFT_TOP:
                return glm::vec3(mMinimum.x, mMaximum.y, mMaximum.z);
            case CornerEnum::NEAR_RIGHT_TOP:
                return mMaximum;
            default:
                return glm::vec3();
            }
        }

        /** Merges the passed in box into the current box. The result is the
        box which encompasses both.
        */
        void merge(const AxisAlignedBox& rhs);

        /** Extends the box to encompass the specified point (if needed).
        */
        void merge(const glm::vec3& point);

        /** Transforms the box according to the matrix supplied.
        @remarks
        By calling this method you get the axis-aligned box which
        surrounds the transformed version of this box. Therefore each
        corner of the box is transformed by the matrix, then the
        extents are mapped back onto the axes to produce another
        AABB. Useful when you have a local AABB for an object which
        is then transformed.
        */
        inline void transform( const glm::mat4x4& matrix )
        {
            // Do nothing if current null or infinite
            if (mExtent != Extent::EXTENT_FINITE)
                return;

            glm::vec3 centre = getCenter();
            glm::vec3 halfSize = getHalfSize();

            glm::vec3 newCentre = matrix * glm::vec4(centre, 1.0f);
            glm::vec3 newHalfSize(
                abs(matrix[0].x) * halfSize.x + abs(matrix[1].x) * halfSize.y + abs(matrix[2].x) * halfSize.z,
                abs(matrix[0].y) * halfSize.x + abs(matrix[1].y) * halfSize.y + abs(matrix[2].y) * halfSize.z,
                abs(matrix[0].z) * halfSize.x + abs(matrix[1].z) * halfSize.y + abs(matrix[2].z) * halfSize.z);

            setExtents(newCentre - newHalfSize, newCentre + newHalfSize);
        }

        /** Sets the box to a 'null' value i.e. not a box.
        */
        inline void setNull()
        {
            mExtent = Extent::EXTENT_NULL;
        }

        /** Returns true if the box is null i.e. empty.
        */
        inline bool isNull(void) const
        {
            return (mExtent == Extent::EXTENT_NULL);
        }

        /** Returns true if the box is finite.
        */
        bool isFinite(void) const
        {
            return (mExtent == Extent::EXTENT_FINITE);
        }

        /** Sets the box to 'infinite'
        */
        inline void setInfinite()
        {
            mExtent = Extent::EXTENT_INFINITE;
        }

        /** Returns true if the box is infinite.
        */
        bool isInfinite(void) const
        {
            return (mExtent == Extent::EXTENT_INFINITE);
        }

        /** Returns whether or not this box intersects another. */
        inline bool intersects(const AxisAlignedBox& b2) const
        {
            // Early-fail for nulls
            if (this->isNull() || b2.isNull())
                return false;

            // Early-success for infinites
            if (this->isInfinite() || b2.isInfinite())
                return true;

            // Use up to 6 separating planes
            if (mMaximum.x < b2.mMinimum.x)
                return false;
            if (mMaximum.y < b2.mMinimum.y)
                return false;
            if (mMaximum.z < b2.mMinimum.z)
                return false;

            if (mMinimum.x > b2.mMaximum.x)
                return false;
            if (mMinimum.y > b2.mMaximum.y)
                return false;
            if (mMinimum.z > b2.mMaximum.z)
                return false;

            // otherwise, must be intersecting
            return true;

        }

        /// Calculate the area of intersection of this box and another
        AxisAlignedBox intersection(AxisAlignedBox& b2);

        /// Calculate the volume of this box
        float volume(void) const;

        /** Scales the AABB by the vector given. */
        inline void scale(const glm::vec3& s)
        {
            // Do nothing if current null or infinite
            if (mExtent != Extent::EXTENT_FINITE)
                return;

            // NB assumes centered on origin
            glm::vec3 min = mMinimum * s;
            glm::vec3 max = mMaximum * s;
            setExtents(min, max);
        }

        ///** Tests whether this box intersects a sphere. */
        //bool intersects(const Sphere& s) const
        //{
        //    return Math::intersects(s, *this); 
        //}
        ///** Tests whether this box intersects a plane. */
        //bool intersects(const Plane& p) const
        //{
        //    return Math::intersects(p, *this);
        //}

        /** Tests whether the vector point is within this box. */
        bool intersects(const glm::vec3& v) const
        {
            switch (mExtent)
            {
            case Extent::EXTENT_NULL:
                return false;

            case Extent::EXTENT_FINITE:
                return(v.x >= mMinimum.x  &&  v.x <= mMaximum.x  && 
                    v.y >= mMinimum.y  &&  v.y <= mMaximum.y  && 
                    v.z >= mMinimum.z  &&  v.z <= mMaximum.z);

            case Extent::EXTENT_INFINITE:
                return true;

            default: // shut up compiler
                assert( false && "Never reached" );
                return false;
            }
        }

        std::pair<bool, float> intersects(glm::vec3 start, glm::vec3 dir)
        {
            if (isNull()) return std::pair<bool, float>(false, (float)0);
            if (isInfinite()) return std::pair<bool, float>(true, (float)0);

            float lowt = 0.0f;
            float t;
            bool hit = false;
            glm::vec3 hitpoint;
            glm::vec3 min = getMinimum();
            glm::vec3 max = getMaximum();
            glm::vec3 rayorig = start;
            glm::vec3 raydir = dir;

            // Check origin inside first
            if (intersects(rayorig))
            {
                return std::pair<bool, float>(true, (float)0);
            }

            // Check each face in turn, only check closest 3
            // Min x
            if (rayorig.x <= min.x && raydir.x > 0)
            {
                t = (min.x - rayorig.x) / raydir.x;

                // Substitute t back into ray and check bounds and dist
                hitpoint = rayorig + raydir * t;
                if (hitpoint.y >= min.y && hitpoint.y <= max.y &&
                    hitpoint.z >= min.z && hitpoint.z <= max.z &&
                    (!hit || t < lowt))
                {
                    hit = true;
                    lowt = t;
                }
            }
            // Max x
            if (rayorig.x >= max.x && raydir.x < 0)
            {
                t = (max.x - rayorig.x) / raydir.x;

                // Substitute t back into ray and check bounds and dist
                hitpoint = rayorig + raydir * t;
                if (hitpoint.y >= min.y && hitpoint.y <= max.y &&
                    hitpoint.z >= min.z && hitpoint.z <= max.z &&
                    (!hit || t < lowt))
                {
                    hit = true;
                    lowt = t;
                }
            }
            // Min y
            if (rayorig.y <= min.y && raydir.y > 0)
            {
                t = (min.y - rayorig.y) / raydir.y;

                // Substitute t back into ray and check bounds and dist
                hitpoint = rayorig + raydir * t;
                if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
                    hitpoint.z >= min.z && hitpoint.z <= max.z &&
                    (!hit || t < lowt))
                {
                    hit = true;
                    lowt = t;
                }
            }
            // Max y
            if (rayorig.y >= max.y && raydir.y < 0)
            {
                t = (max.y - rayorig.y) / raydir.y;

                // Substitute t back into ray and check bounds and dist
                hitpoint = rayorig + raydir * t;
                if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
                    hitpoint.z >= min.z && hitpoint.z <= max.z &&
                    (!hit || t < lowt))
                {
                    hit = true;
                    lowt = t;
                }
            }
            // Min z
            if (rayorig.z <= min.z && raydir.z > 0)
            {
                t = (min.z - rayorig.z) / raydir.z;

                // Substitute t back into ray and check bounds and dist
                hitpoint = rayorig + raydir * t;
                if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
                    hitpoint.y >= min.y && hitpoint.y <= max.y &&
                    (!hit || t < lowt))
                {
                    hit = true;
                    lowt = t;
                }
            }
            // Max z
            if (rayorig.z >= max.z && raydir.z < 0)
            {
                t = (max.z - rayorig.z) / raydir.z;

                // Substitute t back into ray and check bounds and dist
                hitpoint = rayorig + raydir * t;
                if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
                    hitpoint.y >= min.y && hitpoint.y <= max.y &&
                    (!hit || t < lowt))
                {
                    hit = true;
                    lowt = t;
                }
            }

            return std::pair<bool, float>(hit, (float)lowt);
        }

        /// Gets the centre of the box
        glm::vec3 getCenter(void) const
        {
            //assert( (mExtent == Extent::EXTENT_INFINITE) && "Can't get center of a null or infinite AAB" );
            if (mExtent != Extent::EXTENT_FINITE)
                return glm::vec3(0.0f);

            return glm::vec3(
                (mMaximum.x + mMinimum.x) * 0.5f,
                (mMaximum.y + mMinimum.y) * 0.5f,
                (mMaximum.z + mMinimum.z) * 0.5f);
        }
        /// Gets the size of the box
        glm::vec3 getSize(void) const;

        /// Gets the half-size of the box
        glm::vec3 getHalfSize(void) const;

        float getRadius()
        {
            glm::vec3 hs = getHalfSize();
            return std::max(std::max(hs.x, hs.y), hs.z);
        }

        float getMinRadius()
        {
            glm::vec3 hs = getHalfSize();
            return std::min(std::min(hs.x, hs.y), hs.z);
        }

        /** Tests whether the given point contained by this box.
        */
        bool contains(const glm::vec3& v) const
        {
            if (isNull())
                return false;
            if (isInfinite())
                return true;

            return mMinimum.x <= v.x && v.x <= mMaximum.x &&
                    mMinimum.y <= v.y && v.y <= mMaximum.y &&
                    mMinimum.z <= v.z && v.z <= mMaximum.z;
        }
        
        /** Returns the squared minimum distance between a given point and any part of the box.
            *  This is faster than distance since avoiding a squareroot, so use if you can. */
        float squaredDistance(const glm::vec3& v) const;
            
        /** Returns the minimum distance between a given point and any part of the box. */
        float distance (const glm::vec3& v) const
        {
            return sqrt(squaredDistance(v));
        }

        /** Tests whether another box contained by this box.
        */
        bool contains(const AxisAlignedBox& other) const
        {
            if (other.isNull() || this->isInfinite())
                return true;

            if (this->isNull() || other.isInfinite())
                return false;

            return this->mMinimum.x <= other.mMinimum.x &&
                    this->mMinimum.y <= other.mMinimum.y &&
                    this->mMinimum.z <= other.mMinimum.z &&
                    other.mMaximum.x <= this->mMaximum.x &&
                    other.mMaximum.y <= this->mMaximum.y &&
                    other.mMaximum.z <= this->mMaximum.z;
        }

        /** Tests 2 boxes for equality.
        */
        bool operator== (const AxisAlignedBox& rhs) const
        {
            if (this->mExtent != rhs.mExtent)
                return false;

            if (!this->isFinite())
                return true;

            return this->mMinimum == rhs.mMinimum &&
                    this->mMaximum == rhs.mMaximum;
        }

        /** Tests 2 boxes for inequality.
        */
        bool operator!= (const AxisAlignedBox& rhs) const
        {
            return !(*this == rhs);
        }

        // special values
        static const AxisAlignedBox BOX_NULL;
        static const AxisAlignedBox BOX_INFINITE;
    };
}