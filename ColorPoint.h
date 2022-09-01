#ifndef COLORPOINT_H
#define COLORPOINT_H

#include <QVector4D>

class Bezier;

class ColorPoint
{
public:
    ColorPoint();

    QVector2D getPosition2D(float gap = 5.0f) const;

    enum class Direction { //
        Left,
        Right
    };

public:
    Bezier *mParent;
    QVector4D mColor;
    float mPosition;
    bool mSelected;
    Direction mDirection;
    void setParent(Bezier *newParent);
};

#endif // COLORPOINT_H
