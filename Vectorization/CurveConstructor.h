#ifndef CURVECONSTRUCTOR_H
#define CURVECONSTRUCTOR_H

#include "Bezier.h"
#include "Point.h"

class CurveConstructor
{
public:
    CurveConstructor();

    void run(const QVector<QVector<Point>> &polylines);

    const QVector<Bezier *> &curves() const;
    float progress() const;

private:
    Bezier *constructCurve(const QVector<Point> &polyline, double tension = 2.0);

private:
    QVector<Bezier *> mCurves;
    float mProgress;
};

#endif // CURVECONSTRUCTOR_H
