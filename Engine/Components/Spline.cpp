#include "Spline.h"

#include "../Core/APIManager.h"
#include "../Core/GameObject.h"
#include "../Components/Transform.h"
#include "../Math/Mathf.h"

#ifndef _WIN32
#include <cstring>
#endif

namespace GX
{
    std::string Spline::COMPONENT_TYPE = "Spline";

    Spline::Spline() : Component(APIManager::getSingleton()->spline_class)
    {
    }

    Spline::~Spline()
    {
        points.clear();
    }

    Component* Spline::onClone()
    {
        Spline* newComponent = new Spline();
        newComponent->points = points;
        newComponent->closed = closed;

        return newComponent;
    }

    glm::vec3 Spline::getSplinePoint(float t, bool bLooped, glm::vec3* pts, int size)
    {
        int p0, p1, p2, p3;
        if (!bLooped)
        {
            p1 = (int)t + 1;
            p2 = p1 + 1;
            p3 = p2 + 1;
            p0 = p1 - 1;
        }
        else
        {
            p1 = (int)t;
            p2 = (p1 + 1) % size;
            p3 = (p2 + 1) % size;
            p0 = p1 >= 1 ? p1 - 1 : size - 1;
        }

        t = t - (int)t;

        float tt = t * t;
        float ttt = tt * t;

        float q1 = -ttt + 2.0f * tt - t;
        float q2 = 3.0f * ttt - 5.0f * tt + 2.0f;
        float q3 = -3.0f * ttt + 4.0f * tt + t;
        float q4 = ttt - tt;

        glm::vec3 point = 0.5f * (pts[p0] * q1 + pts[p1] * q2 + pts[p2] * q3 + pts[p3] * q4);

        return point;
    }

    glm::vec3 Spline::getSplineDirection(float t, bool bLooped, glm::vec3* pts, int size)
    {
        int p0, p1, p2, p3;
        if (!bLooped)
        {
            p1 = (int)t + 1;
            p2 = p1 + 1;
            p3 = p2 + 1;
            p0 = p1 - 1;
        }
        else
        {
            p1 = (int)t;
            p2 = (p1 + 1) % size;
            p3 = (p2 + 1) % size;
            p0 = p1 >= 1 ? p1 - 1 : size - 1;
        }

        t = t - (int)t;

        float tt = t * t;
        float ttt = tt * t;

        float q1 = -3.0f * tt + 4.0f * t - 1;
        float q2 = 9.0f * tt - 10.0f * t;
        float q3 = -9.0f * tt + 8.0f * t + 1.0f;
        float q4 = 3.0f * tt - 2.0f * t;

        glm::vec3 point = 0.5f * (pts[p0] * q1 + pts[p1] * q2 + pts[p2] * q3 + pts[p3] * q4);

        return glm::normalize(point);
    }

    glm::vec3 Spline::getSplinePoint(float t)
    {
        Transform* trans = gameObject->getTransform();
        glm::vec4 pp1 = glm::vec4(0.0f);

        if (points.size() > 1)
        {
            const int sz = points.size();
            int sz1 = closed ? sz : sz + 2;

            float offset = 3.0f;
            glm::vec3* pts = nullptr;
            if (!closed)
            {
                pts = new glm::vec3[sz1];
                pts[0] = points[0];
                pts[sz + 1] = points[sz - 1];
                memcpy(&pts[1], &points[0], sz * sizeof(glm::vec3));
            }
            else
            {
                pts = &points[0];
                offset = 0.0f;
            }

            float tt = t * (sz1 - offset);

            pp1 = glm::vec4(getSplinePoint(tt, closed, pts, sz1), 1.0f);
            pp1 = trans->getTransformMatrix() * pp1;

            if (!closed)
                delete[] pts;
        }

        return glm::vec3(pp1);
    }

    glm::vec3 Spline::getSplineDirection(float t)
    {
        Transform* trans = gameObject->getTransform();
        glm::vec4 pp1 = glm::vec4(0.0f);

        if (points.size() > 1)
        {
            const int sz = points.size();
            int sz1 = closed ? sz : sz + 2;

            float offset = 3.0f;
            glm::vec3* pts = nullptr;
            if (!closed)
            {
                pts = new glm::vec3[sz1];
                pts[0] = points[0];
                pts[sz + 1] = points[sz - 1];
                memcpy(&pts[1], &points[0], sz * sizeof(glm::vec3));
            }
            else
            {
                pts = &points[0];
                offset = 0.0f;
            }

            float tt = t * (sz1 - offset);

            pp1 = glm::vec4(getSplineDirection(tt, closed, pts, sz1), 1.0f);

            if (!closed)
                delete[] pts;
        }

        return glm::vec3(pp1);
    }
}