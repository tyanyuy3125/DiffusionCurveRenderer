#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H

#include <QVector2D>

class ControlPoint
{
public:
    ControlPoint();

public:
    QVector2D mPosition;
    bool mSelected;
};

#endif // CONTROLPOINT_H
