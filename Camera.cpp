#include "Camera.h"

#include <QMatrix4x4>

Camera::Camera(QObject *parent)
    : QObject{parent}
    , mZNear(-1)
    , mZFar(1)
    , mZoom(1)
    , mLeft(0)
    , mTop(0)
    , mWidth(1600)
    , mHeight(900)
    , mPixelRatio(1.0f)

{}

void Camera::onMousePressed(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        mMouse.x = event->position().x();
        mMouse.y = event->position().y();
        mMouse.pressed = true;
    }
}

void Camera::onMouseReleased(QMouseEvent *event)
{
    mMouse.pressed = false;
}

void Camera::onMouseMoved(QMouseEvent *event)
{
    if (mMouse.pressed)
    {
        mMouse.dx += mMouse.x - event->position().x();
        mMouse.dy += mMouse.y - event->position().y();

        mMouse.x = event->position().x();
        mMouse.y = event->position().y();
        mUpdatePosition = true;
    }
}

void Camera::onWheelMoved(QWheelEvent *event)
{
    QVector2D cursorWorldPosition = toOpenGL(event->position());

    if (event->angleDelta().y() < 0)
        mZoom = 1.1f * mZoom;
    else
        mZoom = mZoom / 1.1f;

    mZoom = qMax(0.01f, qMin(1.0f, mZoom));

    QVector2D newWorldPosition = toOpenGL(event->position());
    QVector2D delta = cursorWorldPosition - newWorldPosition;
    mLeft += delta.x();
    mTop += delta.y();
}

void Camera::resize(int width, int height)
{
    mWidth = width;
    mHeight = height;
}

void Camera::update(float ifps)
{
    if (mUpdatePosition)
    {
        mLeft += mZoom * mMouse.dx;
        mTop += mZoom * mMouse.dy;
        mMouse.dx = 0;
        mMouse.dy = 0;
        mUpdatePosition = false;
    }
}

QMatrix4x4 Camera::projection() const
{
    QMatrix4x4 projection;
    projection.ortho(mLeft, //
                     mLeft + mWidth * mZoom,
                     mTop + mHeight * mZoom,
                     mTop,
                     -1,
                     1);

    return projection;
}

QVector2D Camera::toOpenGL(const QPointF &position) const
{
    return QVector2D(mLeft + mZoom * position.x(), mTop + mZoom * position.y());
}

QPointF Camera::toGUI(const QPointF &position) const
{
    float x = position.x() - mLeft;
    float y = position.y() - mTop;

    return QPointF(x / mZoom * mPixelRatio, y / mZoom * mPixelRatio);
}

QPointF Camera::toGUI(const QVector2D &position) const
{
    return toGUI(position.toPointF());
}

QRectF Camera::toGUI(const QRectF &rect) const
{
    float w = rect.width() / mZoom * mPixelRatio;
    float h = rect.height() / mZoom * mPixelRatio;

    QPointF center = toGUI(QPointF(rect.center().x(), rect.center().y()));

    return QRectF(center.x() - 0.5 * w, center.y() - 0.5 * h, w, h);
}

void Camera::setPixelRatio(float newPixelRatio)
{
    mPixelRatio = newPixelRatio;
}

Camera *Camera::instance()
{
    static Camera camera;
    return &camera;
}

float Camera::zoom() const
{
    return mZoom;
}
