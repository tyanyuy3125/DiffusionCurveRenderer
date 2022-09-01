#include "CurveManager.h"
#include "Common.h"

CurveManager::CurveManager(QObject *parent)
    : Manager(parent)
    , mSelectedCurve(nullptr)
    , mSelectedControlPoint(nullptr)
    , mSelectedColorPoint(nullptr)
{}

bool CurveManager::init()
{
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

        if (controlPoint && colorPoint)
        {
            float distanceToControlPoint = position.distanceToPoint(controlPoint->mPosition);
            float distanceToColorPoint = position.distanceToPoint(colorPoint->getPosition2D());

            setSelectedControlPoint(distanceToColorPoint > distanceToControlPoint ? controlPoint : nullptr);
            setSelectedColorPoint(distanceToColorPoint < distanceToControlPoint ? colorPoint : nullptr);
            return;
        } else if (controlPoint)
        {
            setSelectedControlPoint(controlPoint);
            setSelectedColorPoint(nullptr);
            return;
        } else if (colorPoint)
        {
            setSelectedControlPoint(nullptr);
            setSelectedColorPoint(colorPoint);
            return;
        } else
        {
            setSelectedControlPoint(nullptr);
            setSelectedColorPoint(nullptr);
        }
    }

    setSelectedCurve(selectCurve(position, radius));
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
        colorPoint->mParent = mSelectedCurve;
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

Bezier *CurveManager::selectCurve(const QVector2D &position, float radius)
{
    float minDistance = std::numeric_limits<float>::infinity();

    Bezier *selectedCurve = nullptr;

    for (int i = 0; i < mCurves.size(); ++i)
    {
        float distance = mCurves[i]->distanceToPoint(position);
        if (distance < minDistance)
        {
            minDistance = distance;
            selectedCurve = mCurves[i];
        }
    }

    if (minDistance < radius)
        return selectedCurve;

    return nullptr;
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
        if (colorPoint->getPosition2D().distanceToPoint(nearbyPoint) > radius)
            colorPoint = nullptr;

    return colorPoint;
}

CurveManager *CurveManager::instance()
{
    static CurveManager instance;
    return &instance;
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