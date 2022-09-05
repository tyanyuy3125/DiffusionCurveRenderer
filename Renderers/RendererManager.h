#ifndef RENDERERMANAGER_H
#define RENDERERMANAGER_H

#include "Common.h"

#include "ColorRenderer.h"
#include "ContourRenderer.h"
#include "DiffusionRenderer.h"
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
    void setSmoothIterations(int newSmoothIterations);
    void setQualityFactor(float newQualityFactor);
    void setPixelRatio(float newPixelRatio);
    void save(const QString &path);

private:
    void createFramebuffers();
    void deleteFramebuffers();

private:
    CurveManager *mCurveManager;

    ContourRenderer *mContourRenderer;
    ColorRenderer *mColorRenderer;
    DiffusionRenderer *mDiffusionRenderer;

    Points *mPoints;
    Quad *mQuad;

    QOpenGLFramebufferObjectFormat mInitialFramebufferFormat;
    QOpenGLFramebufferObjectFormat mFinalFramebufferFormat;            // For mFinalFramebuffer
    QOpenGLFramebufferObjectFormat mFinalMultisampleFramebufferFormat; // For mFinalMultisampleFramebuffer

    QOpenGLFramebufferObject *mInitialFramebuffer;          // Contains initial color and blur info
    QOpenGLFramebufferObject *mFinalFramebuffer;            // Final single sampled result for colors and blur in order to save the image
    QOpenGLFramebufferObject *mFinalMultisampleFramebuffer; // We render contours to this FBO
    GLenum *mDrawBuffers;

    RenderMode mRenderMode;
    int mSmoothIterations;

    float mQualityFactor;
    int mWidth;
    int mHeight;
    float mPixelRatio;

    bool mSave;
    QString mSavePath;
};

#endif // RENDERERMANAGER_H
