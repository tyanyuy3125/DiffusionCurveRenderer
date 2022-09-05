#ifndef BEZIER_H
#define BEZIER_H

#include "BlurPoint.h"
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

    void addControlPoint(ControlPoint *controlPoint);
    void addColorPoint(ColorPoint *colorPoint);
    void addBlurPoint(BlurPoint *blurPoint);

    void removeControlPoint(int index);
    void removeControlPoint(ControlPoint *controlPoint);
    void removeLeftColorPoint(int index);
    void removeRightColorPoint(int index);
    void removeColorPoint(ColorPoint *controlPoint);
    void removeBlurPoint(BlurPoint *blurPoint);
    void removeBlurPoint(int index);

    QVector<ColorPoint *> getAllColorPoints() const;

    QVector4D leftColorAt(float t) const;
    QVector4D rightColorAt(float t) const;

    void orderLeftColorPoints();
    void orderRightColorPoints();
    void orderBlurPoints();

    QVector<QVector4D> getLeftColors() const;
    QVector<QVector4D> getRightColors() const;
    QVector<float> getBlurPointStrengths() const;

    QVector<QVector2D> getControlPointPositions() const;
    QVector<float> getLeftColorPositions() const;
    QVector<float> getRightColorPositions() const;
    QVector<float> getBlurPointPositions();

    ControlPoint *getClosestControlPoint(const QVector2D &nearbyPoint) const;
    ColorPoint *getClosestColorPoint(const QVector2D &nearbyPoint) const;
    BlurPoint *getClosestBlurPoint(const QVector2D &nearbyPoint) const;

    int order() const;
    int degree() const;
    int size() const;

    QVector2D valueAt(float t) const override;
    QVector2D tangentAt(float t) const override;
    QVector2D normalAt(float t) const override;
    void scale(float scaleFactor);

    Bezier *deepCopy();

    const QList<ControlPoint *> &controlPoints() const;
    const QList<ColorPoint *> &leftColorPoints() const;
    const QList<ColorPoint *> &rightColorPoints() const;
    const QList<BlurPoint *> &blurPoints() const;

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
    bool mVoid;

private:
    QList<ControlPoint *> mControlPoints;
    QList<ColorPoint *> mLeftColorPoints;
    QList<ColorPoint *> mRightColorPoints;
    QList<BlurPoint *> mBlurPoints;
};

#endif // BEZIER_H
