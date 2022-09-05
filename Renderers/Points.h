#ifndef POINTS_H
#define POINTS_H

#include <QOpenGLExtraFunctions>

class Points : protected QOpenGLExtraFunctions
{
public:
    Points();
    ~Points();

    void bind();
    void release();
    float delta() const;
    int size() const;

private:
    unsigned int mVAO;
    unsigned int mVBO;

    QVector<float> mPoints;

    float mStart;
    float mEnd;
    int mSize;
    float mDelta;
};

#endif // POINTS_H
