#include "Bezier.h"
#include "Common.h"

Bezier::Bezier()
    : mContourColor(DEFAULT_CONTOUR_COLOR)
    , mContourThickness(DEFAULT_CONTOUR_THICKNESS)
    , mDiffusionWidth(DEFAULT_DIFFUSION_WIDTH)
    , mSelected(false)
    , mDepth(0)
{}

Bezier::~Bezier()
{
    for (int i = 0; i < mControlPoints.size(); ++i)
        if (mControlPoints[i])
            delete mControlPoints[i];
}

ControlPoint *Bezier::getControlPoint(int index) const
{
    if (0 <= index && index < mControlPoints.size())
        return mControlPoints[index];
    else
        return nullptr;
}

QVector<QVector2D> Bezier::getControlPointPositions() const
{
    QVector<QVector2D> positions;

    for (const auto &point : mControlPoints)
        positions << point->mPosition;

    return positions;
}

void Bezier::addControlPoint(ControlPoint *controlPoint)
{
    mControlPoints << controlPoint;
}

void Bezier::removeControlPoint(int index)
{
    if (0 <= index && index < mControlPoints.size())
    {
        delete mControlPoints[index];
        mControlPoints.removeAt(index);
    }
}

void Bezier::removeControlPoint(ControlPoint *controlPoint)
{
    for (int i = 0; i < mControlPoints.size(); ++i)
    {
        ControlPoint *point = mControlPoints[i];

        if (point == controlPoint)
        {
            removeControlPoint(i);
            return;
        }
    }
}

void Bezier::addColorPoint(ColorPoint *colorPoint)
{
    switch (colorPoint->mDirection)
    {
    case ColorPoint::Direction::Left: {
        if (colorPoint)
        {
            mLeftColorPoints << colorPoint;
            colorPoint->mParent = this;
            orderLeftColorPoints();
        }

        break;
    }
    case ColorPoint::Direction::Right: {
        if (colorPoint)
        {
            mRightColorPoints << colorPoint;
            colorPoint->mParent = this;
            orderRightColorPoints();
        }
        break;
    }
    }
}

QVector<ColorPoint *> Bezier::getLeftColorPoints() const
{
    QVector<ColorPoint *> points;

    for (auto &point : mLeftColorPoints)
        points << point;

    return points;
}

QVector<ColorPoint *> Bezier::getRightColorPoints() const
{
    QVector<ColorPoint *> points;

    for (auto &point : mRightColorPoints)
    {
        points << point;
    }

    return points;
}

QVector<ColorPoint *> Bezier::getAllColorPoints() const
{
    QVector<ColorPoint *> all;
    all << mLeftColorPoints;
    all << mRightColorPoints;

    return all;
}

ColorPoint *Bezier::getLeftColorPoint(int index) const
{
    return mLeftColorPoints[index];
}

ColorPoint *Bezier::getRightColorPoint(int index) const
{
    return mRightColorPoints[index];
}

void Bezier::removeLeftColorPoint(int index)
{
    delete mLeftColorPoints[index];
    mLeftColorPoints.removeAt(index);
    orderLeftColorPoints();
}

void Bezier::removeRightColorPoint(int index)
{
    delete mRightColorPoints[index];
    mRightColorPoints.removeAt(index);
    orderRightColorPoints();
}

void Bezier::removeColorPoint(ColorPoint *point)
{
    switch (point->mDirection)
    {
    case ColorPoint::Direction::Left: {
        for (int i = 0; i < mLeftColorPoints.size(); ++i)
        {
            if (point == mLeftColorPoints[i])
            {
                removeLeftColorPoint(i);
                return;
            }
        }
        break;
    }
    case ColorPoint::Direction::Right: {
        for (int i = 0; i < mRightColorPoints.size(); ++i)
        {
            if (point == mRightColorPoints[i])
            {
                removeRightColorPoint(i);
                return;
            }
        }
        break;
    }
    }
}

QVector4D Bezier::leftColorAt(float t) const
{
    if (mLeftColorPoints.size() == 0 || mLeftColorPoints.size() == 1)
        return QVector4D(0, 0, 0, 0);

    for (int i = 1; i < mLeftColorPoints.size(); ++i)
    {
        if (mLeftColorPoints[i - 1]->mPosition <= t && t <= mLeftColorPoints[i]->mPosition)
        {
            float s = (t - mLeftColorPoints[i - 1]->mPosition) / (mLeftColorPoints[i]->mPosition - mLeftColorPoints[i - 1]->mPosition);
            return mLeftColorPoints[i - 1]->mColor + s * (mLeftColorPoints[i]->mColor - mLeftColorPoints[i - 1]->mColor);
        }
    }

    return QVector4D(0, 0, 0, 0);
}

QVector4D Bezier::rightColorAt(float t) const
{
    if (mRightColorPoints.size() == 0 || mRightColorPoints.size() == 1)
        return QVector4D(0, 0, 0, 0);

    for (int i = 1; i < mRightColorPoints.size(); ++i)
    {
        if (mRightColorPoints[i - 1]->mPosition <= t && t <= mRightColorPoints[i]->mPosition)
        {
            float s = (t - mRightColorPoints[i - 1]->mPosition) / (mRightColorPoints[i]->mPosition - mRightColorPoints[i - 1]->mPosition);
            return mRightColorPoints[i - 1]->mColor + s * (mRightColorPoints[i]->mColor - mRightColorPoints[i - 1]->mColor);
        }
    }

    return QVector4D(0, 0, 0, 0);
}

void Bezier::orderLeftColorPoints()
{
    if (mLeftColorPoints.size() == 0 || mLeftColorPoints.size() == 1)
        return;

    QVector<ColorPoint *> orderedColorPoints;

    orderedColorPoints << mLeftColorPoints[0];

    for (int i = 1; i < mLeftColorPoints.size(); ++i)
    {
        ColorPoint *currentColorPoint = mLeftColorPoints[i];

        if (orderedColorPoints.last()->mPosition <= currentColorPoint->mPosition)
            orderedColorPoints << currentColorPoint;
        else
            for (int j = 0; j < orderedColorPoints.size(); j++)
                if (currentColorPoint->mPosition < orderedColorPoints[j]->mPosition)
                {
                    orderedColorPoints.insert(j, currentColorPoint);
                    break;
                }
    }

    mLeftColorPoints = orderedColorPoints;
}

void Bezier::orderRightColorPoints()
{
    if (mRightColorPoints.size() == 0 || mRightColorPoints.size() == 1)
        return;

    QVector<ColorPoint *> orderedColorPoints;

    orderedColorPoints << mRightColorPoints[0];

    for (int i = 1; i < mRightColorPoints.size(); ++i)
    {
        ColorPoint *currentColorPoint = mRightColorPoints[i];

        if (orderedColorPoints.last()->mPosition <= currentColorPoint->mPosition)
            orderedColorPoints << currentColorPoint;
        else
            for (int j = 0; j < orderedColorPoints.size(); j++)
                if (currentColorPoint->mPosition < orderedColorPoints[j]->mPosition)
                {
                    orderedColorPoints.insert(j, currentColorPoint);
                    break;
                }
    }

    mRightColorPoints = orderedColorPoints;
}

QVector<QVector4D> Bezier::getLeftColors() const
{
    QVector<QVector4D> leftColors;
    for (int i = 0; i < mLeftColorPoints.size(); i++)
        leftColors << mLeftColorPoints[i]->mColor;

    return leftColors;
}

QVector<QVector4D> Bezier::getRightColors() const
{
    QVector<QVector4D> rightColors;
    for (int i = 0; i < mRightColorPoints.size(); i++)
        rightColors << mRightColorPoints[i]->mColor;

    return rightColors;
}

QVector<float> Bezier::getLeftColorPositions() const
{
    QVector<float> colorPointPositions;

    for (int i = 0; i < mLeftColorPoints.size(); i++)
        colorPointPositions << mLeftColorPoints[i]->mPosition;

    return colorPointPositions;
}

QVector<float> Bezier::getRightColorPositions() const
{
    QVector<float> colorPointPositions;

    for (int i = 0; i < mRightColorPoints.size(); i++)
        colorPointPositions << mRightColorPoints[i]->mPosition;

    return colorPointPositions;
}

ColorPoint *Bezier::getClosestColorPoint(const QVector2D &point) const
{
    QVector<ColorPoint *> allColorPoints;
    allColorPoints << mLeftColorPoints << mRightColorPoints;

    if (allColorPoints.size() == 0)
        return nullptr;

    float minimumDistance = std::numeric_limits<float>::infinity();
    int index = 0;

    for (int i = 0; i < allColorPoints.size(); ++i)
    {
        float distance = allColorPoints[i]->getPosition2D().distanceToPoint(point);

        if (distance < minimumDistance)
        {
            minimumDistance = distance;
            index = i;
        }
    }

    return allColorPoints[index];
}

ControlPoint *Bezier::getClosestControlPoint(const QVector2D &nearbyPoint) const
{
    if (mControlPoints.size() == 0)
        return nullptr;

    float minimumDistance = std::numeric_limits<float>::infinity();
    int index = 0;

    for (int i = 0; i < mControlPoints.size(); ++i)
    {
        float distance = mControlPoints[i]->mPosition.distanceToPoint(nearbyPoint);

        if (distance < minimumDistance)
        {
            minimumDistance = distance;
            index = i;
        }
    }

    return mControlPoints[index];
}

QVector2D Bezier::valueAt(float t) const
{
    QVector2D value = QVector2D(0, 0);
    int n = mControlPoints.size() - 1;

    for (int i = 0; i <= n; i++)
        value += choose(n, i) * pow(t, i) * pow(1 - t, n - i) * mControlPoints[i]->mPosition;

    return value;
}

QVector2D Bezier::tangentAt(float t) const
{
    QVector2D tangent = QVector2D(0, 0);
    int order = mControlPoints.size() - 1;

    for (int i = 0; i <= order - 1; i++)
    {
        float coefficient = choose(order - 1, i) * pow(t, i) * pow(1 - t, order - 1 - i);
        tangent += coefficient * (mControlPoints.at(i)->mPosition - mControlPoints.at(i + 1)->mPosition);
    }

    tangent.normalize();

    return tangent;
}

QVector2D Bezier::normalAt(float t) const
{
    QVector2D tangent = tangentAt(t);

    return QVector2D(-tangent.y(), tangent.x());
}

QVector<float> Bezier::getCoefficients() const
{
    QVector<float> coefficients;
    int n = mControlPoints.size() - 1;

    for (int i = 0; i <= n; ++i)
        coefficients << choose(n, i);

    return coefficients;
}

QVector<float> Bezier::getDerivativeCoefficients() const
{
    QVector<float> coefficients;
    int n = mControlPoints.size() - 1;

    for (int i = 0; i <= n - 1; ++i)
        coefficients << choose(n - 1, i);

    return coefficients;
}

float Bezier::factorial(int n) const
{
    double result = 1.0;

    for (int i = 1; i <= n; ++i)
        result *= i;

    return result;
}

float Bezier::choose(int n, int k) const
{
    return factorial(n) / (factorial(k) * factorial(n - k));
}

const QList<ControlPoint *> &Bezier::controlPoints() const
{
    return mControlPoints;
}

int Bezier::order() const
{
    return mControlPoints.size();
}

int Bezier::degree() const
{
    return mControlPoints.size() - 1;
}

int Bezier::size() const
{
    return mControlPoints.size();
}
