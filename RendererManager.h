#ifndef RENDERERMANAGER_H
#define RENDERERMANAGER_H

#include "Bezier.h"
#include "Camera.h"
#include "Common.h"

#include "Manager.h"
#include "Points.h"
#include "Quad.h"

#include <QOpenGLFramebufferObjectFormat>

class ShaderManager;
class CurveManager;

class RendererManager : public Manager, protected QOpenGLExtraFunctions
{
    explicit RendererManager(QObject *parent = nullptr);

public:
    static RendererManager *instance();

    bool init() override;
    void render();
    void resize(int width, int height);
    void setRenderMode(RenderMode newRenderMode);
    void setPixelRatio(float newPixelRatio);

    void setSmoothIterations(int newSmoothIterations);
    void setQualityFactor(float newQualityFactor);

    void save(const QString &path);

private:
    void renderContours(QOpenGLFramebufferObject *target, bool clearTarget = true);
    void renderDiffusion(QOpenGLFramebufferObject *target, bool clearTarget = true);
    void renderContour(QOpenGLFramebufferObject *target, Bezier *curve, bool clearTarget = true);
    void createFramebuffers();
    void deleteFramebuffers();

    void renderColors(QOpenGLFramebufferObject *draw);
    void downsample(QOpenGLFramebufferObject *draw, QOpenGLFramebufferObject *read);
    void upsample(QOpenGLFramebufferObject *draw, QOpenGLFramebufferObject *drawBuffer, QOpenGLFramebufferObject *source, QOpenGLFramebufferObject *target);
    void drawFinalBlurCurves(QOpenGLFramebufferObject *draw);

private:
    ShaderManager *mShaderManager;
    CurveManager *mCurveManager;
    Camera *mCamera;

    Points *mPoints;
    Quad *mQuad;
    QList<Bezier *> mCurves;

    RenderMode mRenderMode;

    int mWidth;
    int mHeight;
    float mPixelRatio;

    QOpenGLFramebufferObjectFormat mFinalFramebufferFormat;            // For mFinalFramebuffer
    QOpenGLFramebufferObjectFormat mDefaultFramebufferFormat;          // For other FBOs
    QOpenGLFramebufferObjectFormat mFinalMultisampleFramebufferFormat; // For mFinalMultisampleFramebuffer

    QOpenGLFramebufferObject *mInitialFramebuffer;          // Contains initial color and blur info
    QOpenGLFramebufferObject *mFinalFramebuffer;            // Final single sampled result for colors and blur in order to save the image
    QOpenGLFramebufferObject *mFinalMultisampleFramebuffer; // We render contours to this FBO

    QVector<QOpenGLFramebufferObject *> mDownsampleFramebuffers;
    QVector<QOpenGLFramebufferObject *> mUpsampleFramebuffers;
    QVector<QOpenGLFramebufferObject *> mTemporaryFrameBuffers;

    int mSmoothIterations;
    float mQualityFactor;

    bool mSave;
    QString mSavePath;

    GLenum *mDrawBuffers;
};

#endif // RENDERERMANAGER_H
