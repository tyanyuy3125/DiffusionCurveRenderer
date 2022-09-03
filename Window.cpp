#include "Window.h"
#include "Controller.h"

#include <QDateTime>
#include <QKeyEvent>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QPen>

#include <QDebug>

Window::Window(QWindow *parent)
    : QOpenGLWindow(QOpenGLWindow::UpdateBehavior::NoPartialUpdate, parent)
    , mController(nullptr)

{
    connect(this, &QOpenGLWindow::frameSwapped, this, [=]() { update(); });
}

Window::~Window()
{
    if (mController)
        mController->deleteLater();
}

void Window::initializeGL()
{
    mCurrentTime = QDateTime::currentMSecsSinceEpoch();
    mPreviousTime = mCurrentTime;

    initializeOpenGLFunctions();

    QtImGui::initialize(this);
    mController = new Controller;
    mController->setWindow(this);
    mController->init();
}

void Window::resizeGL(int w, int h)
{
    glViewport(0, 0, width(), height());

    mController->resize(w, h);
}

void Window::paintGL()
{
    mCurrentTime = QDateTime::currentMSecsSinceEpoch();
    float ifps = (mCurrentTime - mPreviousTime) * 0.001f;
    mPreviousTime = mCurrentTime;

    mController->render(ifps);
}

void Window::keyPressEvent(QKeyEvent *event)
{
    mController->onKeyPressed(event);
}

void Window::keyReleaseEvent(QKeyEvent *event)
{
    mController->onKeyReleased(event);
}

void Window::mousePressEvent(QMouseEvent *event)
{
    mController->onMousePressed(event);
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    mController->onMouseReleased(event);
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    mController->onMouseMoved(event);
}

void Window::wheelEvent(QWheelEvent *event)
{
    mController->onWheelMoved(event);
}
