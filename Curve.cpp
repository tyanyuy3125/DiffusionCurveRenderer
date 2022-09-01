#include "Curve.h"

#include <QLineF>
#include <QRectF>

Curve::Curve() {}

Curve::~Curve() {}

float Curve::distanceToPoint(const QVector2D &point, int intervals) const
{
    float minDistance = std::numeric_limits<float>::infinity();
    float t = 0.0f;
    float dt = 1.0f / intervals;

    for (int i = 0; i < intervals; ++i)
    {
        float distance = valueAt(t).distanceToPoint(point);
        if (distance < minDistance)
            minDistance = distance;

        t += dt;
    }

    return minDistance;
}

float Curve::length(int intervals) const
{
    float length = 0.0f;
    float t = 0.0f;
    const float dt = 1.0f / static_cast<float>(intervals);

    for (int i = 0; i < intervals; ++i)
    {
        QVector2D v1 = valueAt(t);
        QVector2D v2 = valueAt(t + dt);
        length += v1.distanceToPoint(v2);
        t += dt;
    }

    return length;
}

QRectF Curve::boundingBox(int intervals) const
{
    float xMin = std::numeric_limits<float>::infinity();
    float xMax = -std::numeric_limits<float>::infinity();
    float yMin = std::numeric_limits<float>::infinity();
    float yMax = -std::numeric_limits<float>::infinity();

    float t = 0.0f;
    float dt = 1.0f / static_cast<float>(intervals);

    for (int i = 0; i <= intervals; i++)
    {
        QVector2D value = valueAt(t);

        if (value.x() > xMax)
            xMax = value.x();
        if (value.x() < xMin)
            xMin = value.x();
        if (value.y() > yMax)
            yMax = value.y();
        if (value.y() < yMin)
            yMin = value.y();

        t += dt;
    }

    return QRectF(xMin, yMin, xMax - xMin, yMax - yMin);
}

float Curve::parameterAt(const QVector2D &point, int intervals) const
{
    float t = 0;
    float dt = 1.0f / intervals;
    float minimumDistance = std::numeric_limits<float>::infinity();
    float parameter = 0;

    for (int i = 0; i <= intervals; i++)
    {
        float distance = valueAt(t).distanceToPoint(point);
        if (distance < minimumDistance)
        {
            minimumDistance = distance;
            parameter = t;
        }
        t += dt;
    }

    return parameter;
}

QVector2D Curve::findMeanCenter(const QVector<QVector2D> &points)
{
    float x = 0;
    float y = 0;

    for (int i = 0; i < points.size(); ++i)
    {
        x += points[i].x();
        y += points[i].y();
    }
    return QVector2D(x, y) / points.size();
}

QVector<QVector2D> Curve::translate(const QVector<QVector2D> &points, const QVector2D &translation)
{
    QVector<QVector2D> result;
    result.reserve(points.size());

    for (int i = 0; i < points.size(); ++i)
    {
        result << points[i] + translation;
    }

    return result;
}

void Curve::findLineOfBestFit(const QVector<QVector2D> &points, QVector2D &startingPoint, QVector2D &direction, int segments)
{
    // Find mean center of the points
    QVector2D meanCenter = findMeanCenter(points);

    // Move mean center to the origin
    QVector<QVector2D> translatedPoints = translate(points, -meanCenter);

    // Set variables
    QVector2D origin(0, 0);
    float theta = 0;
    float dt = 2 * 3.141565 / segments;
    float minimumAverageDistance = averageDistanceToLine(translatedPoints, origin, QVector2D(1, 0));
    float beta = 0;

    for (int i = 0; i < segments; ++i)
    {
        theta += dt;
        float averageDistance = averageDistanceToLine(translatedPoints, origin, QVector2D(cos(theta), sin(theta)));
        if (averageDistance < minimumAverageDistance)
        {
            minimumAverageDistance = averageDistance;
            beta = theta;
        }
    }

    startingPoint = meanCenter;
    direction = QVector2D(cos(beta), sin(beta));
}

float Curve::averageDistanceToLine(const QVector<QVector2D> &points, const QVector2D &startingPoint, const QVector2D &direction)
{
    float distance = 0;

    for (int i = 0; i < points.size(); ++i)
    {
        distance += points[i].distanceToLine(startingPoint, direction);
    }

    return distance / points.size();
}

float Curve::perpendicularAt(const QVector2D &startingPoint, const QVector2D &direction, const QVector2D &subject)
{
    const float &sx = startingPoint.x();
    const float &sy = startingPoint.y();

    const float &dx = direction.x();
    const float &dy = direction.y();

    const float &cx = subject.x();
    const float &cy = subject.y();

    return (cy * dy + cx * dx - sx * dx - sy * dy) / (dx * dx + dy * dy);
}

float Curve::distanceToLineSegment(const QVector2D &startingPoint, const QVector2D &endPoint, const QVector2D &subject, int intervals)
{
    float t = 0;
    float dt = 1.0f / intervals;

    QVector2D direction = endPoint - startingPoint;

    float minimumDistance = std::numeric_limits<float>::infinity();
    for (int i = 0; i <= intervals; ++i)
    {
        float distance = subject.distanceToPoint(startingPoint + t * direction);
        if (distance < minimumDistance)
            minimumDistance = distance;
        t += dt;
    }

    return minimumDistance;
}
