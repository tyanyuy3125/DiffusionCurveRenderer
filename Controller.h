#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Camera.h"
#include "Common.h"
#include "CurveManager.h"
#include "CustomVariant.h"
#include "RendererManager.h"
#include "ShaderManager.h"

#include <QObject>

#include <imgui.h>
#include <QFileDialog>
#include <QPen>
#include <QtImGui.h>

class Window;

class Controller : public QObject, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    virtual ~Controller();

    void onAction(Action action, CustomVariant value = CustomVariant());

    void init();
    void onWheelMoved(QWheelEvent *event);
    void onMousePressed(QMouseEvent *event);
    void onMouseReleased(QMouseEvent *event);
    void onMouseMoved(QMouseEvent *event);
    void onKeyPressed(QKeyEvent *event);
    void onKeyReleased(QKeyEvent *event);
    void resize(int w, int h);
    void render(float ifps);
    void drawGUI();
    void drawPainter();

    void setWindow(Window *newWindow);

private:
    RendererManager *mRendererManager;
    CurveManager *mCurveManager;
    ShaderManager *mShaderManager;

    QVector<Manager *> mManagers;

    Window *mWindow;
    Camera *mCamera;

    float mIfps;
    bool mImGuiWantsMouseCapture;
    bool mImGuiWantsKeyboardCapture;

    Mode mMode;
    RenderMode mRenderMode;
    float mWidth;
    float mHeight;
    float mPixelRatio;

    float mGlobalContourThickness;
    float mGlobalDiffusionWidth;
    QVector4D mGlobalContourColor;
    float mGlobalBlurStrength;

    // GUI
    QPen mDashedPen;
    QPen mSolidPen;
    QPen mDenseDashedPen;
    QPointF mPreviousMousePosition;
    Qt::MouseButton mPressedButton;
    QFileDialog *mFileDialog;
    Action mLastFileAction;

    // Aux
    Bezier *mSelectedCurve;
    ControlPoint *mSelectedControlPoint;
    ColorPoint *mSelectedColorPoint;
    BlurPoint *mSelectedBlurPoint;

    QList<ControlPoint *> mControlPoints;
    QList<ColorPoint *> mColorPoints;
    QList<BlurPoint *> mBlurPoints;

    int mSmoothIterations;
};

#endif // CONTROLLER_H
