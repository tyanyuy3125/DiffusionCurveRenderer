#ifndef CURVE_H
#define CURVE_H

#include <QVector2D>
#include <QVector>

class Curve
{
public:
    Curve();
    virtual ~Curve();

    virtual QVector2D valueAt(float t) const = 0;
    virtual QVector2D tangentAt(float t) const = 0;
    virtual QVector2D normalAt(float t) const = 0;
    virtual float parameterAt(const QVector2D &point, int intervals = 1000) const;

    virtual float length(int intervals = 100) const;

    virtual QRectF boundingBox(int intervals = 100) const;
    virtual float distanceToPoint(const QVector2D &point, int intervals = 1000) const;

private:
    static QVector2D findMeanCenter(const QVector<QVector2D> &points);
    static QVector<QVector2D> translate(const QVector<QVector2D> &points, const QVector2D &translation);
    static void findLineOfBestFit(const QVector<QVector2D> &points, QVector2D &startingPoint, QVector2D &direction, int segments = 1000);
    static float averageDistanceToLine(const QVector<QVector2D> &points, const QVector2D &startingPoint, const QVector2D &direction);
    static float perpendicularAt(const QVector2D &startingPoint, const QVector2D &direction, const QVector2D &subject);
    static float distanceToLineSegment(const QVector2D &startingPoint, const QVector2D &endPoint, const QVector2D &subject, int intervals = 100);
};

#endif // CURVE_H
