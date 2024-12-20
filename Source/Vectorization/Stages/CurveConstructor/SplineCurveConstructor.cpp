#include "SplineCurveConstructor.h"

#include <QDebug>

DiffusionCurveRenderer::SplineCurveConstructor::SplineCurveConstructor(QObject* parent)
    : VectorizationStageBase(parent)
{
}

void DiffusionCurveRenderer::SplineCurveConstructor::Run(const QVector<QVector<Point>>& polylines)
{
    for (int i = 0; i < polylines.size(); i++)
    {
        if (CurvePtr curve = ConstructCurve(polylines.at(i)))
        {
            mCurves << curve;
        }

        float progress = float(i) / (polylines.size() - 1);
        emit ProgressChanged(progress);
    }
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::SplineCurveConstructor::ConstructCurve(const QVector<Point>& polyline, double tension)
{
    const int nPoints = polyline.size();

    if (nPoints <= 1)
    {
        qInfo() << "Number of points in the polyline is less than or equal to 1. Number of points is" << nPoints;
        return nullptr;
    }

    SplinePtr spline = std::make_shared<Spline>();

    for (int i = 0; i < nPoints; i++)
    {
        const auto point = polyline.at(i);
        spline->AddControlPoint(QVector2D(point.x, point.y));
    }

    return spline;
}

const QVector<DiffusionCurveRenderer::CurvePtr>& DiffusionCurveRenderer::SplineCurveConstructor::GetCurves() const
{
    return mCurves;
}

void DiffusionCurveRenderer::SplineCurveConstructor::Reset()
{
    mCurves.clear();
}