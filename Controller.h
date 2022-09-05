#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "BitmapRenderer.h"
#include "Common.h"
#include "CurveManager.h"
#include "CustomVariant.h"
#include "EditModeCamera.h"
#include "RendererManager.h"
#include "ShaderManager.h"
#include "Vectorization/Vectorizer.h"
#include "ViewModeCamera.h"

#include <QObject>

#include <imgui.h>
#include <QFileDialog>
#include <QFuture>
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

    void setWindow(Window *newWindow);

signals:
    void load(QString path);
    void draw();

private:
    void drawGUI();
    void drawPainter();

    enum class FileDialogType { //
        LoadFromXML,
        LoadFromJSON,
        SaveAsPNG,
        SaveAsJSON,
        LoadVectorizerImage,
    };

private:
    QThread mVectorizerThread;

    RendererManager *mRendererManager;
    CurveManager *mCurveManager;
    ShaderManager *mShaderManager;
    BitmapRenderer *mBitmapRenderer;
    Vectorizer *mVectorizer;

    QVector<Manager *> mManagers;

    Window *mWindow;
    EditModeCamera *mEditModeCamera;
    ViewModeCamera *mViewModeCamera;

    float mIfps;
    bool mImGuiWantsMouseCapture;
    bool mImGuiWantsKeyboardCapture;

    WorkMode mWorkMode;
    SubWorkMode mSubWorkMode;
    SubWorkMode mPreviousSubWorkMode;
    ActionMode mActionMode;
    RenderMode mRenderMode;
    float mWidth;
    float mHeight;
    float mPixelRatio;

    float mGlobalContourThickness;
    float mGlobalDiffusionWidth;
    QVector4D mGlobalContourColor;
    float mGlobalBlurStrength;

    int mSmoothIterations;
    int mQualityFactor;

    float mVoidThreshold;

    // GUI
    QPen mDashedPen;
    QPen mSolidPen;
    QPen mDenseDashedPen;
    QPointF mPreviousMousePosition;
    Qt::MouseButton mPressedButton;
    bool mVectorizerImageLoaded;
    bool mShowFileDailog;
    FileDialogType mFileDailogType;

    // Aux
    Bezier *mSelectedCurve;
    ControlPoint *mSelectedControlPoint;
    ColorPoint *mSelectedColorPoint;
    BlurPoint *mSelectedBlurPoint;

    QList<ControlPoint *> mControlPoints;
    QList<ColorPoint *> mColorPoints;
    QList<BlurPoint *> mBlurPoints;
};

#endif // CONTROLLER_H
