#ifndef CAMERA_H
#define CAMERA_H

#include <QKeyEvent>
#include <QMatrix4x4>
#include <QObject>

class Camera : public QObject
{
    Q_OBJECT

private:
    explicit Camera(QObject *parent = nullptr);

public:
    void onMousePressed(QMouseEvent *event);
    void onMouseReleased(QMouseEvent *event);
    void onMouseMoved(QMouseEvent *event);
    void onWheelMoved(QWheelEvent *event);
    void resize(int width, int height);
    void update(float ifps);
    void drawGUI();

    QMatrix4x4 projection() const;

    static Camera *instance();

    float zoom() const;

    QVector2D toOpenGL(const QPointF &position) const;
    QPointF toGUI(const QPointF &position) const;
    QPointF toGUI(const QVector2D &position) const;
    QRectF toGUI(const QRectF &rect) const;

    void setPixelRatio(float newPixelRatio);

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

    QMatrix4x4 mTransform;
};

#endif // CAMERA_H
