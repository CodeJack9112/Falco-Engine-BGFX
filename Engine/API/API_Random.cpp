#include "API_Random.h"

#include <cstdlib>
#include "../Math/Mathf.h"

namespace GX
{
    float API_Random::rangeFloat(float _min, float _max)
    {
        float random = ((float)rand()) / (float)RAND_MAX;
        float diff = _max - _min;
        float r = random * diff;
        return _min + r;
    }

    int API_Random::rangeInt(int _min, int _max)
    {
        int randNum = rand() % (_max - _min) + _min;
        return randNum;
    }
}