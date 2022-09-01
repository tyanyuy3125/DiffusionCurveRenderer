#include "ColorPoint.h"
#include "Bezier.h"

ColorPoint::ColorPoint()
    : mParent(nullptr)
    , mColor(1, 1, 1, 1)
    , mPosition(0)
    , mSelected(false)
{}

QVector2D ColorPoint::getPosition2D(float gap) const
{
    if (mParent)
    {
        QVector2D vector = (mParent->mContourThickness + gap) * mParent->normalAt(mPosition);

        if (mDirection == ColorPoint::Direction::Right)
            vector = -vector;

        return mParent->valueAt(mPosition) + vector;
    }

    return QVector2D();
}

void ColorPoint::setParent(Bezier *newParent)
{
    mParent = newParent;
}
