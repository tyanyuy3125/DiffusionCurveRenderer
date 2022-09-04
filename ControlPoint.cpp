#include "ControlPoint.h"

ControlPoint::ControlPoint()
    : mSelected(false)
{}

ControlPoint *ControlPoint::deepCopy() const
{
    ControlPoint *point = new ControlPoint;
    point->mPosition = mPosition;
    point->mSelected = mSelected;
    return point;
}
