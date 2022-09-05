#ifndef DIFFUSIONRENDERER_H
#define DIFFUSIONRENDERER_H

#include "RendererBase.h"

#include <QOpenGLFramebufferObjectFormat>

class DiffusionRenderer : public RendererBase
{
public:
    DiffusionRenderer();

    void init() override;
    void resize(int w, int h) override;
    void setQualityFactor(float newQualityFactor) override;
    void setPixelRatio(float newPixelRatio) override;
    void setSmoothIterations(int newSmoothIterations);

    void render(QOpenGLFramebufferObject *initialFramebuffer, QOpenGLFramebufferObject *target, bool clearTarget = true);

private:
    void downsample(QOpenGLFramebufferObject *draw, QOpenGLFramebufferObject *read);
    void upsample(QOpenGLFramebufferObject *draw, QOpenGLFramebufferObject *drawBuffer, QOpenGLFramebufferObject *source, QOpenGLFramebufferObject *target);
    void drawFinalBlurCurves(QOpenGLFramebufferObject *draw);

    void deleteFramebuffers();
    void createFramebuffers();

private:
    QOpenGLFramebufferObjectFormat mDefaultFramebufferFormat;
    QVector<QOpenGLFramebufferObject *> mDownsampleFramebuffers;
    QVector<QOpenGLFramebufferObject *> mUpsampleFramebuffers;
    QVector<QOpenGLFramebufferObject *> mTemporaryFrameBuffers;
    GLenum *mDrawBuffers;

    int mSmoothIterations;
};

#endif // DIFFUSIONRENDERER_H
