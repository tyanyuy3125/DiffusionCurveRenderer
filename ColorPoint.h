#ifndef COLORPOINT_H
#define COLORPOINT_H

#include "Common.h"
#include <QVector4D>

class Bezier;

class ColorPoint
{
public:
    ColorPoint();

    void setParent(Bezier *newParent);
    QVector2D getPosition2D(float gap = COLOR_POINT_VISUAL_GAP) const;

    enum class Direction { //
        Left,
        Right
    };

private:
    Bezier *mParent;

public:
    QVector4D mColor;
    float mPosition;
    bool mSelected;
    Direction mDirection;
};

#endif // COLORPOINT_H
