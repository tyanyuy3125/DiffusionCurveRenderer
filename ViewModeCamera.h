#ifndef VIEWMODECAMERA_H
#define VIEWMODECAMERA_H

#include <QMatrix4x4>
#include <QMouseEvent>
#include <QObject>

class ViewModeCamera : public QObject
{
    Q_OBJECT

    explicit ViewModeCamera(QObject *parent = nullptr);

public:
    static ViewModeCamera *instance();

    void onMousePressed(QMouseEvent *event);
    void onMouseReleased(QMouseEvent *event);
    void onMouseMoved(QMouseEvent *event);
    void onWheelMoved(QWheelEvent *event);
    void resize(int width, int height);
    void update(float ifps);

    QMatrix4x4 projection() const;

    float pixelRatio() const;
    void setPixelRatio(float newPixelRatio);

    float zoom() const;
    void setZoom(float newZoom);

    float left() const;
    void setLeft(float newLeft);

    float top() const;
    void setTop(float newTop);

    float width() const;
    float height() const;

private:
    float mZNear;
    float mZFar;
    float mZoom;
    float mLeft;
    float mTop;
    float mWidth;
    float mHeight;
    float mPixelRatio;

    bool mUpdatePosition;

    struct Mouse {
        bool pressed = false;
        float x = 0;
        float y = 0;
        float dx = 0;
        float dy = 0;
    } mMouse;
};

#endif // VIEWMODECAMERA_H
