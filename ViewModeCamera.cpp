#include "ViewModeCamera.h"

ViewModeCamera::ViewModeCamera(QObject *parent)
    : QObject{parent}
    , mZNear(-1)
    , mZFar(1)
    , mZoom(4.0f)
    , mLeft(0)
    , mTop(0)
    , mWidth(1600)
    , mHeight(900)
    , mPixelRatio(1.0f)
{}

void ViewModeCamera::onMousePressed(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        mMouse.x = event->position().x();
        mMouse.y = event->position().y();
        mMouse.pressed = true;
    }
}

void ViewModeCamera::onMouseReleased(QMouseEvent *event)
{
    mMouse.pressed = false;
}

void ViewModeCamera::onMouseMoved(QMouseEvent *event)
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

void ViewModeCamera::onWheelMoved(QWheelEvent *event)
{
    if (event->angleDelta().y() < 0)
        mZoom = 1.1f * mZoom;
    else
        mZoom = mZoom / 1.1f;

    mZoom = qMax(0.25f, qMin(10.0f, mZoom));
}

void ViewModeCamera::resize(int width, int height)
{
    mWidth = width;
    mHeight = height;
}

void ViewModeCamera::update(float ifps)
{
    if (mUpdatePosition)
    {
        mLeft -= mMouse.dx * mZoom;
        mTop += mMouse.dy * mZoom;
        mMouse.dx = 0;
        mMouse.dy = 0;
        mUpdatePosition = false;
    }
}

QMatrix4x4 ViewModeCamera::projection() const
{
    QMatrix4x4 projection;

    projection.ortho(-mZoom * mWidth / 2 - mLeft, //
                     mZoom * mWidth / 2 - mLeft,
                     -mZoom * mHeight / 2 - mTop,
                     mZoom * mHeight / 2 - mTop,
                     mZNear,
                     mZFar);

    return projection;
}

void ViewModeCamera::setPixelRatio(float newPixelRatio)
{
    mPixelRatio = newPixelRatio;
}

float ViewModeCamera::left() const
{
    return mLeft;
}

void ViewModeCamera::setLeft(float newLeft)
{
    mLeft = newLeft;
}

float ViewModeCamera::top() const
{
    return mTop;
}

void ViewModeCamera::setTop(float newTop)
{
    mTop = newTop;
}

float ViewModeCamera::zoom() const
{
    return mZoom;
}

void ViewModeCamera::setZoom(float newZoom)
{
    mZoom = newZoom;
}

ViewModeCamera *ViewModeCamera::instance()
{
    static ViewModeCamera instance;

    return &instance;
}
