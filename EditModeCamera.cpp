#include "EditModeCamera.h"

#include <QMatrix4x4>

EditModeCamera::EditModeCamera(QObject *parent)
    : QObject{parent}
    , mZNear(-1)
    , mZFar(1)
    , mZoom(1.0f)
    , mLeft(0)
    , mTop(0)
    , mWidth(1600)
    , mHeight(900)
    , mPixelRatio(1.0f)

{}

void EditModeCamera::onMousePressed(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        mMouse.x = event->position().x();
        mMouse.y = event->position().y();
        mMouse.pressed = true;
    }
}

void EditModeCamera::onMouseReleased(QMouseEvent *event)
{
    mMouse.pressed = false;
}

void EditModeCamera::onMouseMoved(QMouseEvent *event)
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

void EditModeCamera::onWheelMoved(QWheelEvent *event)
{
    QVector2D cursorWorldPosition = toOpenGL(event->position());

    if (event->angleDelta().y() < 0)
        mZoom = 1.1f * mZoom;
    else
        mZoom = mZoom / 1.1f;

    mZoom = qMax(0.001f, qMin(10.0f, mZoom));

    QVector2D newWorldPosition = toOpenGL(event->position());
    QVector2D delta = cursorWorldPosition - newWorldPosition;
    mLeft += delta.x();
    mTop += delta.y();
}

void EditModeCamera::resize(int width, int height)
{
    mWidth = width;
    mHeight = height;
}

void EditModeCamera::update(float ifps)
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

QMatrix4x4 EditModeCamera::projection() const
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

QVector2D EditModeCamera::toOpenGL(const QPointF &position) const
{
    return QVector2D(mLeft + mZoom * position.x(), mTop + mZoom * position.y());
}

QPointF EditModeCamera::toGUI(const QPointF &position) const
{
    float x = position.x() - mLeft;
    float y = position.y() - mTop;

    return QPointF(x * mPixelRatio / mZoom, y * mPixelRatio / mZoom);
}

QPointF EditModeCamera::toGUI(const QVector2D &position) const
{
    return toGUI(position.toPointF());
}

QRectF EditModeCamera::toGUI(const QRectF &rect) const
{
    float w = rect.width() * mPixelRatio / mZoom;
    float h = rect.height() * mPixelRatio / mZoom;

    QPointF center = toGUI(QPointF(rect.center().x(), rect.center().y()));

    return QRectF(center.x() - 0.5 * w, center.y() - 0.5 * h, w, h);
}

float EditModeCamera::pixelRatio() const
{
    return mPixelRatio;
}

void EditModeCamera::setPixelRatio(float newPixelRatio)
{
    mPixelRatio = newPixelRatio;
}

float EditModeCamera::left() const
{
    return mLeft;
}

void EditModeCamera::setLeft(float newLeft)
{
    mLeft = newLeft;
}

float EditModeCamera::top() const
{
    return mTop;
}

void EditModeCamera::setTop(float newTop)
{
    mTop = newTop;
}

float EditModeCamera::width() const
{
    return mWidth;
}

float EditModeCamera::height() const
{
    return mHeight;
}

float EditModeCamera::zoom() const
{
    return mZoom;
}

void EditModeCamera::setZoom(float newZoom)
{
    mZoom = newZoom;
}

EditModeCamera *EditModeCamera::instance()
{
    static EditModeCamera instance;

    return &instance;
}
