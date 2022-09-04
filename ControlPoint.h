#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H

#include <QVector2D>

class ControlPoint
{
public:
    ControlPoint();

    ControlPoint *deepCopy() const;

public:
    QVector2D mPosition;
    bool mSelected;
};

#endif // CONTROLPOINT_H
