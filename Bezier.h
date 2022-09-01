#ifndef BEZIER_H
#define BEZIER_H

#include "ColorPoint.h"
#include "ControlPoint.h"
#include "Curve.h"

#include <QObject>
#include <QVector4D>

class Bezier : public Curve
{
public:
    Bezier();
    virtual ~Bezier();

    ControlPoint *getControlPoint(int index) const;
    QVector<QVector2D> getControlPointPositions() const;

    void addControlPoint(ControlPoint *controlPoint);
    void addColorPoint(ColorPoint *colorPoint);

    void removeControlPoint(int index);
    void removeControlPoint(ControlPoint *controlPoint);

    QVector<ColorPoint *> getLeftColorPoints() const;
    QVector<ColorPoint *> getRightColorPoints() const;
    QVector<ColorPoint *> getAllColorPoints() const;

    ColorPoint *getLeftColorPoint(int index) const;
    ColorPoint *getRightColorPoint(int index) const;

    void removeLeftColorPoint(int index);
    void removeRightColorPoint(int index);
    void removeColorPoint(ColorPoint *controlPoint);

    QVector4D leftColorAt(float t) const;
    QVector4D rightColorAt(float t) const;

    void orderLeftColorPoints();
    void orderRightColorPoints();

    QVector<QVector4D> getLeftColors() const;
    QVector<QVector4D> getRightColors() const;

    QVector<float> getLeftColorPositions() const;
    QVector<float> getRightColorPositions() const;

    ColorPoint *getClosestColorPoint(const QVector2D &nearbyPoint) const;
    ControlPoint *getClosestControlPoint(const QVector2D &nearbyPoint) const;

    int order() const;
    int degree() const;
    int size() const;

    QVector2D valueAt(float t) const override;
    QVector2D tangentAt(float t) const override;
    QVector2D normalAt(float t) const override;

    const QList<ControlPoint *> &controlPoints() const;

private:
    QVector<float> getCoefficients() const;
    QVector<float> getDerivativeCoefficients() const;
    float factorial(int n) const;
    float choose(int n, int k) const;

public:
    QVector4D mContourColor;
    float mContourThickness;
    float mDiffusionWidth;
    bool mSelected;
    int mDepth;

private:
    QList<ControlPoint *> mControlPoints;
    QList<ColorPoint *> mLeftColorPoints;
    QList<ColorPoint *> mRightColorPoints;
};

#endif // BEZIER_H
