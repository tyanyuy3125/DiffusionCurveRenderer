// This code taken from https://github.com/zhuethanca/DiffusionCurves
#include "Point.h"

#include <cmath>
#include <iostream>

Point::Point()
    : Point(0, 0)
{}

Point::Point(double x, double y)
{
    this->x = x;
    this->y = y;
}

double Point::dist(const Point o) const
{
    return sqrt((x - o.x) * (x - o.x) + (y - o.y) * (y - o.y));
}

void Point::normalize()
{
    double norm = this->norm();
    if (norm != 0)
    {
        x /= norm;
        y /= norm;
    }
}

double Point::norm() const
{
    return sqrt(x * x + y * y);
}

bool Point::isNeighbour(const Point other)
{
    // True if the two points are diagonally connected; false if they are
    // separated by more than 1 pixels on any axis.
    return this->sqdist(other) < 3.0;
}

double Point::sqdist(Point o) const
{
    return (x - o.x) * (x - o.x) + (y - o.y) * (y - o.y);
}

Eigen::Vector2f Point::toVector() const
{
    Eigen::Vector2f converted;
    converted << this->x, this->y;

    return converted;
}

ControlPoint *Point::toControlPoint() const
{
    ControlPoint *controlPoint = new ControlPoint;
    controlPoint->mSelected = false;
    controlPoint->mPosition = QVector2D(this->x, this->y);

    return controlPoint;
}

bool Point::operator==(const Point &rhs) const
{
    return x == rhs.x && y == rhs.y;
}

bool Point::operator!=(const Point &rhs) const
{
    return !(rhs == *this);
}

std::ostream &std::operator<<(std::ostream &strm, const Point &p)
{
    return strm << "(" << p.x << ", " << p.y << ")";
}
