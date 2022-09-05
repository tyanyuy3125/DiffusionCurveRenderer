#ifndef CURVEMANAGER_H
#define CURVEMANAGER_H

#include "Bezier.h"
#include "EditModeCamera.h"
#include "Manager.h"

#include <QFuture>
#include <QTimer>

class CurveManager : public Manager
{
    explicit CurveManager(QObject *parent = nullptr);

public:
    static CurveManager *instance();
    bool init() override;

    // Actions
    void select(RenderMode renderMode, const QVector2D &position, float radius = 5.0f);
    void addControlPoint(const QVector2D &position, bool select = true);
    void addColorPoint(const QVector2D &position, bool select = true);
    void addBlurPoint(const QVector2D &position, bool select = true);

    void removeSelectedCurve();
    void removeSelectedControlPoint();
    void removeSelectedColorPoint();
    void removeSelectedBlurPoint();

    void addCurve(Bezier *curve);
    void addCurves(const QList<Bezier *> curves);

    void removeCurve(int index);
    void removeCurve(Curve *curve);

    void setGlobalContourThickness(float thickness);
    void setGlobalContourColor(const QVector4D &color);
    void setGlobalDiffusionWidth(float width);
    void setGlobalDiffusionGap(float gap);
    void setGlobalBlurStrength(float strength);

    void deselectAllCurves();

    ControlPoint *getClosestControlPointOnSelectedCurve(const QVector2D &nearbyPoint, float radius = 20.0f) const;
    ColorPoint *getClosestColorPointOnSelectedCurve(const QVector2D &nearbyPoint, float radius = 5.0f) const;
    BlurPoint *getClosestBlurPointOnSelectedCurve(const QVector2D &nearbyPoint, float radius = 5.0f) const;

    const QList<Bezier *> &curves() const;

    Bezier *selectedCurve() const;
    void setSelectedCurve(Bezier *newSelectedCurve);

    ControlPoint *selectedControlPoint() const;
    void setSelectedControlPoint(ControlPoint *newSelectedControlPoint);

    ColorPoint *selectedColorPoint() const;
    void setSelectedColorPoint(ColorPoint *newSelectedColorPoint);

    BlurPoint *selectedBlurPoint() const;
    void setSelectedBlurPoint(BlurPoint *newSelectedBlurPoint);

    void sortCurves();
    void clear();

    void makeVoid(float threshold);

private:
    void selectCurve(QVector2D position, float radius = 20.0f);

private:
    EditModeCamera *mCamera;
    QList<Bezier *> mCurves;
    Bezier *mSelectedCurve;
    ControlPoint *mSelectedControlPoint;
    ColorPoint *mSelectedColorPoint;
    BlurPoint *mSelectedBlurPoint;
};

#endif // CURVEMANAGER_H
