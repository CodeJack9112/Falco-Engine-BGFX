#include "Color.h"

#include "../Math/Mathf.h"

namespace GX
{
    Color Color::White = Color(1.0f, 1.0f, 1.0f, 1.0f);
    Color Color::Black = Color(0.0f, 0.0f, 0.0f, 1.0f);
    Color Color::Red = Color(1.0f, 0.0f, 0.0f, 1.0f);
    Color Color::Green = Color(0.0f, 1.0f, 0.0f, 1.0f);
    Color Color::Blue = Color(0.0f, 0.0f, 1.0f, 1.0f);
    Color Color::Yellow = Color(1.0f, 1.0f, 0.0f, 1.0f);

    bool operator==(const Color& c1, const Color& c2)
    {
        return c1.val[0] == c2.val[0] &&
            c1.val[1] == c2.val[1] &&
            c1.val[2] == c2.val[2] &&
            c1.val[3] == c2.val[3];
    }

    bool operator!=(const Color& c1, const Color& c2)
    {
        return !(c1==c2);
    }

    uint32_t Color::packABGR(Color value)
    {
        int iR = value[0] * 255.0f;
        int iG = value[1] * 255.0f;
        int iB = value[2] * 255.0f;
        int iA = value[3] * 255.0f;

        return (iA << 24) | (iB << 16) | (iG << 8) | iR;
    }

    Color Color::unpackABGR(uint32_t value)
    {
        int iA = (value >> 24) & 0xff;
        int iB = (value >> 16) & 0xff;
        int iG = (value >> 8) & 0xff;
        int iR = (value) & 0xff;

        float r = 1.0f / 255.0f * iR;
        float g = 1.0f / 255.0f * iG;
        float b = 1.0f / 255.0f * iB;
        float a = 1.0f / 255.0f * iA;

        return Color(r, g, b, a);
    }

    uint32_t Color::packRGBA(Color value)
    {
        int iR = value[0] * 255.0f;
        int iG = value[1] * 255.0f;
        int iB = value[2] * 255.0f;
        int iA = value[3] * 255.0f;

        return (iR << 24) | (iG << 16) | (iB << 8) | iA;
    }

    Color Color::unpackRGBA(uint32_t value)
    {
        int iR = (value >> 24) & 0xff;
        int iG = (value >> 16) & 0xff;
        int iB = (value >> 8) & 0xff;
        int iA = (value) & 0xff;

        float r = 1.0f / 255.0f * iR;
        float g = 1.0f / 255.0f * iG;
        float b = 1.0f / 255.0f * iB;
        float a = 1.0f / 255.0f * iA;

        return Color(r, g, b, a);
    }

    Color Color::lerp(Color a, Color b, float t)
    {
        t = Mathf::Clamp01(t);
        return Color(a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t, a[2] + (b[2] - a[2]) * t, a[3] + (b[3] - a[3]) * t);
    }
}