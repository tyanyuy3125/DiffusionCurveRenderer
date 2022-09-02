#include "BlurPoint.h"
#include "Bezier.h"
#include "Common.h"

BlurPoint::BlurPoint()
    : mParent(nullptr)
    , mStrength(DEFAULT_BLUR_STRENGTH)
    , mPosition(0.0f)
    , mSelected(false)
{}

void BlurPoint::setParent(Bezier *newParent)
{
    mParent = newParent;
}

QVector2D BlurPoint::getPosition2D(float gap) const
{
    if (mParent)
    {
        QVector2D vector = (mParent->mContourThickness + gap) * mParent->normalAt(mPosition);
        return mParent->valueAt(mPosition) + vector;
    }

    return QVector2D();
}
