#include "CurveManager.h"
#include "Common.h"

#include <QtConcurrent>

CurveManager::CurveManager(QObject *parent)
    : Manager(parent)
    , mSelectedCurve(nullptr)
    , mSelectedControlPoint(nullptr)
    , mSelectedColorPoint(nullptr)
    , mSelectedBlurPoint(nullptr)
{}

bool CurveManager::init()
{
    mCamera = EditModeCamera::instance(); // For fetching zoom, used for color point and blur point selection

    return true;
}

void CurveManager::addCurve(Bezier *curve)
{
    mCurves << curve;
    sortCurves();
}

void CurveManager::addCurves(const QList<Bezier *> curves)
{
    mCurves << curves;
    sortCurves();
}

void CurveManager::removeCurve(int index)
{
    if (0 <= index && index < mCurves.size())
    {
        Curve *curve = mCurves[index];
        mCurves.removeAt(index);
        delete curve;
        sortCurves();
    }
}

void CurveManager::removeCurve(Curve *curve)
{
    for (int i = 0; i < mCurves.size(); ++i)
    {
        if (mCurves[i] == curve)
        {
            removeCurve(i);
            sortCurves();
            return;
        }
    }
}

void CurveManager::setGlobalContourThickness(float thickness)
{
    for (auto &curve : mCurves)
        curve->mContourThickness = thickness;
}

void CurveManager::setGlobalContourColor(const QVector4D &color)
{
    for (auto &curve : mCurves)
        curve->mContourColor = color;
}

void CurveManager::setGlobalDiffusionWidth(float width)
{
    for (auto &curve : mCurves)
        curve->mDiffusionWidth = width;
}

void CurveManager::setGlobalBlurStrength(float strength)
{
    for (auto &curve : mCurves)
    {
        auto blurPoints = curve->blurPoints();

        for (auto &blurPoint : blurPoints)
        {
            blurPoint->mStrength = strength;
        }
    }
}

void CurveManager::deselectAllCurves()
{
    for (int i = 0; i < mCurves.size(); ++i)
        mCurves[i]->mSelected = false;

    mSelectedCurve = nullptr;
}

void CurveManager::select(const QVector2D &position, float radius)
{
    if (mSelectedCurve)
    {
        ControlPoint *controlPoint = getClosestControlPointOnSelectedCurve(position, radius);
        ColorPoint *colorPoint = getClosestColorPointOnSelectedCurve(position, radius);
        BlurPoint *blurPoint = getClosestBlurPointOnSelectedCurve(position, radius);

        if (controlPoint && colorPoint && blurPoint)
        {
            float distanceToControlPoint = position.distanceToPoint(controlPoint->mPosition);
            float distanceToColorPoint = position.distanceToPoint(colorPoint->getPosition2D(mCamera->zoom() * COLOR_POINT_VISUAL_GAP));
            float distanceToBlurPoint = position.distanceToPoint(blurPoint->getPosition2D(mCamera->zoom() * BLUR_POINT_VISUAL_GAP));

            float min = qMin(distanceToControlPoint, qMin(distanceToColorPoint, distanceToBlurPoint));

            if (qFuzzyCompare(min, distanceToControlPoint))
            {
                setSelectedControlPoint(controlPoint);
                setSelectedColorPoint(nullptr);
                setSelectedBlurPoint(nullptr);
            }

            else if (qFuzzyCompare(min, distanceToColorPoint))
            {
                setSelectedColorPoint(colorPoint);
                setSelectedControlPoint(nullptr);
                setSelectedBlurPoint(nullptr);
            }

            else if (qFuzzyCompare(min, distanceToBlurPoint))
            {
                setSelectedBlurPoint(blurPoint);
                setSelectedControlPoint(nullptr);
                setSelectedColorPoint(nullptr);
            }

            return;
        } else if (controlPoint)
        {
            setSelectedControlPoint(controlPoint);
            setSelectedColorPoint(nullptr);
            setSelectedBlurPoint(nullptr);
            return;
        } else if (colorPoint)
        {
            setSelectedColorPoint(colorPoint);
            setSelectedControlPoint(nullptr);
            setSelectedBlurPoint(nullptr);
            return;
        } else if (blurPoint)
        {
            setSelectedBlurPoint(blurPoint);
            setSelectedControlPoint(nullptr);
            setSelectedColorPoint(nullptr);

            return;
        } else
        {
            setSelectedControlPoint(nullptr);
            setSelectedColorPoint(nullptr);
            setSelectedBlurPoint(nullptr);
        }
    }

    selectCurve(position, radius);
}

void CurveManager::addControlPoint(const QVector2D &position, bool select)
{
    if (mSelectedCurve)
    {
        if (mSelectedCurve->size() >= MAX_CONTROL_POINT_COUNT)
            return;

        ControlPoint *controlPoint = new ControlPoint;
        controlPoint->mPosition = position;
        controlPoint->mSelected = true;

        mSelectedCurve->addControlPoint(controlPoint);

        if (select)
        {
            setSelectedControlPoint(controlPoint);
            setSelectedColorPoint(nullptr);
        }

    } else
    {
        ControlPoint *controlPoint = new ControlPoint;
        controlPoint->mPosition = position;
        controlPoint->mSelected = true;

        Bezier *curve = new Bezier;
        curve->addControlPoint(controlPoint);
        addCurve(curve);
        setSelectedCurve(curve);

        if (select)
        {
            setSelectedControlPoint(controlPoint);
            setSelectedColorPoint(nullptr);
        }
    }
}

void CurveManager::addColorPoint(const QVector2D &position, bool select)
{
    if (mSelectedCurve && mSelectedCurve->size() >= 2)
    {
        float parameter = mSelectedCurve->parameterAt(position);
        QVector3D positionOnCurve = mSelectedCurve->valueAt(parameter).toVector3D();
        QVector3D tangent = mSelectedCurve->tangentAt(parameter).toVector3D();
        QVector3D direction = (position.toVector3D() - positionOnCurve).normalized();
        QVector3D cross = QVector3D::crossProduct(tangent, direction);

        ColorPoint::Direction type = cross.z() > 0 ? ColorPoint::Direction::Left : ColorPoint::Direction::Right;

        ColorPoint *colorPoint = new ColorPoint;
        colorPoint->setParent(mSelectedCurve);
        colorPoint->mPosition = parameter;
        colorPoint->mDirection = type;
        colorPoint->mColor = QVector4D(1, 1, 1, 1);
        mSelectedCurve->addColorPoint(colorPoint);

        if (select)
        {
            setSelectedControlPoint(nullptr);
            setSelectedColorPoint(colorPoint);
        }
    }
}

void CurveManager::addBlurPoint(const QVector2D &position, bool select)
{
    if (mSelectedCurve && mSelectedCurve->size() >= 2)
    {
        float parameter = mSelectedCurve->parameterAt(position);

        BlurPoint *blurPoint = new BlurPoint;
        blurPoint->setParent(mSelectedCurve);
        blurPoint->mPosition = parameter;
        mSelectedCurve->addBlurPoint(blurPoint);

        if (select)
        {
            setSelectedControlPoint(nullptr);
            setSelectedColorPoint(nullptr);
            setSelectedBlurPoint(blurPoint);
        }
    }
}

void CurveManager::removeSelectedCurve()
{
    if (mSelectedCurve)
    {
        removeCurve(mSelectedCurve);
        setSelectedCurve(nullptr);
        setSelectedControlPoint(nullptr);
        setSelectedColorPoint(nullptr);
    }
}

void CurveManager::removeSelectedControlPoint()
{
    if (mSelectedCurve && mSelectedControlPoint)
    {
        mSelectedCurve->removeControlPoint(mSelectedControlPoint);
        setSelectedControlPoint(nullptr);

        if (mSelectedCurve->size() == 0)
        {
            setSelectedCurve(nullptr);
            removeCurve(mSelectedCurve);
        }
    }
}

void CurveManager::removeSelectedColorPoint()
{
    if (mSelectedCurve && mSelectedColorPoint)
    {
        mSelectedCurve->removeColorPoint(mSelectedColorPoint);
        setSelectedColorPoint(nullptr);
    }
}

void CurveManager::removeSelectedBlurPoint()
{
    if (mSelectedCurve && mSelectedBlurPoint)
    {
        mSelectedCurve->removeBlurPoint(mSelectedBlurPoint);
        setSelectedBlurPoint(nullptr);
    }
}

ControlPoint *CurveManager::getClosestControlPointOnSelectedCurve(const QVector2D &nearbyPoint, float radius) const
{
    if (!mSelectedCurve)
        return nullptr;

    ControlPoint *controlPoint = mSelectedCurve->getClosestControlPoint(nearbyPoint);

    if (controlPoint)
        if (controlPoint->mPosition.distanceToPoint(nearbyPoint) > radius)
            controlPoint = nullptr;

    return controlPoint;
}

ColorPoint *CurveManager::getClosestColorPointOnSelectedCurve(const QVector2D &nearbyPoint, float radius) const
{
    if (!mSelectedCurve)
        return nullptr;

    QVector<ColorPoint *> allColorPoints = mSelectedCurve->getAllColorPoints();

    if (allColorPoints.size() == 0)
        return nullptr;

    ColorPoint *colorPoint = mSelectedCurve->getClosestColorPoint(nearbyPoint);

    if (colorPoint)
        if (colorPoint->getPosition2D(mCamera->zoom() * COLOR_POINT_VISUAL_GAP).distanceToPoint(nearbyPoint) > radius)
            colorPoint = nullptr;

    return colorPoint;
}

BlurPoint *CurveManager::getClosestBlurPointOnSelectedCurve(const QVector2D &nearbyPoint, float radius) const
{
    if (!mSelectedCurve)
        return nullptr;

    auto blurPoints = mSelectedCurve->blurPoints();

    if (blurPoints.size() == 0)
        return nullptr;

    BlurPoint *blurPoint = mSelectedCurve->getClosestBlurPoint(nearbyPoint);

    if (blurPoint)
        if (blurPoint->getPosition2D(mCamera->zoom() * BLUR_POINT_VISUAL_GAP).distanceToPoint(nearbyPoint) > radius)
            blurPoint = nullptr;

    return blurPoint;
}

CurveManager *CurveManager::instance()
{
    static CurveManager instance;
    return &instance;
}

void CurveManager::selectCurve(QVector2D position, float radius)
{
    Bezier *curve = nullptr;

    float minDistance = std::numeric_limits<float>::infinity();

    for (int i = 0; i < mCurves.size(); ++i)
    {
        if (mCurves[i]->mVoid)
            continue;

        float distance = mCurves[i]->distanceToPoint(position);
        if (distance < minDistance)
        {
            minDistance = distance;
            curve = mCurves[i];
        }
    }

    if (minDistance < radius)
        setSelectedCurve(curve);
    else
        setSelectedCurve(nullptr);
}

BlurPoint *CurveManager::selectedBlurPoint() const
{
    return mSelectedBlurPoint;
}

void CurveManager::setSelectedBlurPoint(BlurPoint *newSelectedBlurPoint)
{
    if (mSelectedBlurPoint == newSelectedBlurPoint)
        return;

    if (mSelectedBlurPoint)
        mSelectedBlurPoint->mSelected = false;

    if (newSelectedBlurPoint)
        newSelectedBlurPoint->mSelected = true;

    mSelectedBlurPoint = newSelectedBlurPoint;
}

ColorPoint *CurveManager::selectedColorPoint() const
{
    return mSelectedColorPoint;
}

void CurveManager::setSelectedColorPoint(ColorPoint *newSelectedColorPoint)
{
    if (mSelectedColorPoint == newSelectedColorPoint)
        return;

    if (mSelectedColorPoint)
        mSelectedColorPoint->mSelected = false;

    if (newSelectedColorPoint)
        newSelectedColorPoint->mSelected = true;

    mSelectedColorPoint = newSelectedColorPoint;
}

const QList<Bezier *> &CurveManager::curves() const
{
    return mCurves;
}

Bezier *CurveManager::selectedCurve() const
{
    return mSelectedCurve;
}

void CurveManager::setSelectedCurve(Bezier *newSelectedCurve)
{
    if (mSelectedCurve == newSelectedCurve)
        return;

    if (mSelectedCurve)
        mSelectedCurve->mSelected = false;

    if (newSelectedCurve)
        newSelectedCurve->mSelected = true;

    mSelectedCurve = newSelectedCurve;
}

ControlPoint *CurveManager::selectedControlPoint() const
{
    return mSelectedControlPoint;
}

void CurveManager::setSelectedControlPoint(ControlPoint *newSelectedControlPoint)
{
    if (mSelectedControlPoint == newSelectedControlPoint)
        return;

    if (mSelectedControlPoint)
        mSelectedControlPoint->mSelected = false;

    if (newSelectedControlPoint)
        newSelectedControlPoint->mSelected = true;

    mSelectedControlPoint = newSelectedControlPoint;
}

void CurveManager::sortCurves()
{
    if (mCurves.size() == 0 || mCurves.size() == 1)
        return;

    QList<Bezier *> sortedCurves;

    sortedCurves << mCurves[0];

    for (int i = 1; i < mCurves.size(); i++)
    {
        Bezier *curve = mCurves[i];
        if (sortedCurves.last()->mDepth <= curve->mDepth)
            sortedCurves << curve;
        else
            for (int j = 0; j < sortedCurves.size(); j++)
                if (sortedCurves[j]->mDepth > curve->mDepth)
                {
                    sortedCurves.insert(j, curve);
                    break;
                }
    }

    mCurves = sortedCurves;
}

void CurveManager::clear()
{
    for (auto &curve : mCurves)
    {
        if (curve)
            delete curve;

        curve = nullptr;
    }

    setSelectedColorPoint(nullptr);
    setSelectedControlPoint(nullptr);
    setSelectedCurve(nullptr);
    mCurves.clear();
}

void CurveManager::makeVoid(float threshold)
{
    for (int i = 0; i < mCurves.size(); i++)
    {
        float length = mCurves[i]->length();

        if (length < threshold)
            mCurves[i]->mVoid = true;
        else
            mCurves[i]->mVoid = false;
    }
}
