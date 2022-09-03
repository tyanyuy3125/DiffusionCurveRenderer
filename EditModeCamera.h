#ifndef EDITMODECAMERA_H
#define EDITMODECAMERA_H

#include <QKeyEvent>
#include <QMatrix4x4>
#include <QObject>

class EditModeCamera : public QObject
{
    Q_OBJECT

    explicit EditModeCamera(QObject *parent = nullptr);

public:
    static EditModeCamera *instance();

    void onMousePressed(QMouseEvent *event);
    void onMouseReleased(QMouseEvent *event);
    void onMouseMoved(QMouseEvent *event);
    void onWheelMoved(QWheelEvent *event);
    void resize(int width, int height);
    void update(float ifps);

    QMatrix4x4 projection() const;

    QVector2D toOpenGL(const QPointF &position) const;
    QPointF toGUI(const QPointF &position) const;
    QPointF toGUI(const QVector2D &position) const;
    QRectF toGUI(const QRectF &rect) const;

    void setPixelRatio(float newPixelRatio);

    float zoom() const;
    void setZoom(float newZoom);

    float left() const;
    void setLeft(float newLeft);

    float top() const;
    void setTop(float newTop);

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

#endif // EDITMODECAMERA_H
