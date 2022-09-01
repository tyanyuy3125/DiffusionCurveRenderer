#ifndef CURVEMANAGER_H
#define CURVEMANAGER_H

#include "Bezier.h"
#include "Manager.h"

class CurveManager : public Manager
{
    explicit CurveManager(QObject *parent = nullptr);

public:
    bool init() override;

    // Actions
    void select(const QVector2D &position, float radius = 5.0f);
    void addControlPoint(const QVector2D &position, bool select = true);
    void addColorPoint(const QVector2D &position, bool select = true);
    void removeSelectedCurve();
    void removeSelectedControlPoint();
    void removeSelectedColorPoint();

    void addCurve(Bezier *curve);
    void addCurves(const QList<Bezier *> curves);

    void removeCurve(int index);
    void removeCurve(Curve *curve);

    void setGlobalContourThickness(float thickness);
    void setGlobalContourColor(const QVector4D &color);
    void setGlobalDiffusionWidth(float width);

    void deselectAllCurves();

    Bezier *selectCurve(const QVector2D &position, float radius = 20.0f);

    ControlPoint *getClosestControlPointOnSelectedCurve(const QVector2D &nearbyPoint, float radius = 20.0f) const;
    ColorPoint *getClosestColorPointOnSelectedCurve(const QVector2D &nearbyPoint, float radius = 5.0f) const;

    const QList<Bezier *> &curves() const;

    Bezier *selectedCurve() const;
    void setSelectedCurve(Bezier *newSelectedCurve);

    ControlPoint *selectedControlPoint() const;
    void setSelectedControlPoint(ControlPoint *newSelectedControlPoint);

    void sortCurves();
    void clear();

    static CurveManager *instance();

    ColorPoint *selectedColorPoint() const;
    void setSelectedColorPoint(ColorPoint *newSelectedColorPoint);

private:
    QList<Bezier *> mCurves;
    Bezier *mSelectedCurve;
    ControlPoint *mSelectedControlPoint;
    ColorPoint *mSelectedColorPoint;
};

#endif // CURVEMANAGER_H
