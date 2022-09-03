#include "RendererManager.h"
#include "CurveManager.h"
#include "ShaderManager.h"

#include <QImage>
#include <QMatrix4x4>

RendererManager::RendererManager(QObject *parent)
    : Manager{parent}
    , mWidth(1600)
    , mHeight(900)
    , mPixelRatio(1.0f)
    , mInitialFramebuffer(nullptr)
    , mFinalFramebuffer(nullptr)
    , mSmoothIterations(20)
    , mQualityFactor(1.0f)
    , mSave(false)
{}

bool RendererManager::init()
{
    initializeOpenGLFunctions();

    mShaderManager = ShaderManager::instance();
    mCurveManager = CurveManager::instance();
    mCamera = EditModeCamera::instance();

    mPoints = new Points;
    mQuad = new Quad;

    mDefaultFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mDefaultFramebufferFormat.setSamples(0);
    mDefaultFramebufferFormat.setMipmap(false);
    mDefaultFramebufferFormat.setTextureTarget(GL_TEXTURE_2D);
    mDefaultFramebufferFormat.setInternalTextureFormat(GL_RGBA8);

    mFinalFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mFinalFramebufferFormat.setSamples(0);
    mFinalFramebufferFormat.setMipmap(false);
    mFinalFramebufferFormat.setTextureTarget(GL_TEXTURE_2D);
    mFinalFramebufferFormat.setInternalTextureFormat(GL_RGBA8);

    mFinalMultisampleFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mFinalMultisampleFramebufferFormat.setSamples(16);
    mFinalMultisampleFramebufferFormat.setMipmap(false);
    mFinalMultisampleFramebufferFormat.setInternalTextureFormat(GL_RGBA8);

    // For color and blur textures
    mDrawBuffers = new GLenum[2];
    mDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
    mDrawBuffers[1] = GL_COLOR_ATTACHMENT1;

    createFramebuffers();

    return true;
}

void RendererManager::render()
{
    mCurves = mCurveManager->curves();

    if (mRenderMode == RenderMode::Diffusion)
    {
        renderDiffusion(nullptr, true);

        // If a curve is selected then render it
        if (mCurveManager->selectedCurve())
        {
            renderContour(nullptr, mCurveManager->selectedCurve(), false);
        }
    }

    if (mRenderMode == RenderMode::Contour)
    {
        renderContours(nullptr, true);
    }

    if (mRenderMode == RenderMode::ContourAndDiffusion)
    {
        renderDiffusion(nullptr, true);
        renderContours(nullptr, false);
    }

    if (mSave)
    {
        if (mRenderMode == RenderMode::Diffusion)
        {
            renderDiffusion(mFinalFramebuffer, true);
            mFinalFramebuffer->toImage().save(mSavePath);
        }

        if (mRenderMode == RenderMode::Contour)
        {
            renderContours(mFinalMultisampleFramebuffer, true);
            mFinalMultisampleFramebuffer->toImage().save(mSavePath);
        }

        if (mRenderMode == RenderMode::ContourAndDiffusion)
        {
            renderDiffusion(mFinalMultisampleFramebuffer, true);
            renderContours(mFinalMultisampleFramebuffer, false);
            mFinalMultisampleFramebuffer->toImage().save(mSavePath);
        }

        mSave = false;
    }
}

void RendererManager::resize(int width, int height)
{
    mWidth = width;
    mHeight = height;

    deleteFramebuffers();
    createFramebuffers();
}

RendererManager *RendererManager::instance()
{
    static RendererManager instance;
    return &instance;
}

void RendererManager::setRenderMode(RenderMode newRenderMode)
{
    mRenderMode = newRenderMode;
}

void RendererManager::renderContours(QOpenGLFramebufferObject *target, bool clearTarget)
{
    if (target)
    {
        mCamera->resize(target->width(), target->height());

        target->bind();
        glViewport(0, 0, target->width(), target->height());

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }

    } else
    {
        // Render to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    mShaderManager->bind(ShaderType::ContourShader);
    mPoints->bind();
    mShaderManager->setUniformValue("projection", mCamera->projection());
    mShaderManager->setUniformValue("pointDelta", mPoints->delta());
    mShaderManager->setUniformValue("zoom", mCamera->zoom());

    for (const auto &curve : mCurves)
    {
        QVector<QVector2D> controlPoints = curve->getControlPointPositions();

        mShaderManager->setUniformValue("color", curve->mContourColor);
        mShaderManager->setUniformValue("thickness", curve->mContourThickness);
        mShaderManager->setUniformValue("controlPointsCount", (int) controlPoints.size());
        mShaderManager->setUniformValueArray("controlPoints", controlPoints);

        glDrawArrays(GL_POINTS, 0, mPoints->size());
    }

    mPoints->release();
    mShaderManager->release();

    if (target)
    {
        // Restore
        target->release();
        mCamera->resize(mWidth, mHeight);
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);
    }
}

void RendererManager::renderDiffusion(QOpenGLFramebufferObject *target, bool clearTarget)
{
    // First pass (render colors to initial framebuffer)
    renderColors(mInitialFramebuffer);

    // Downsample 0
    downsample(mDownsampleFramebuffers[0], mInitialFramebuffer);

    // Downsample 1,2,3...
    for (int i = 1; i < mDownsampleFramebuffers.size(); ++i)
    {
        downsample(mDownsampleFramebuffers[i], mDownsampleFramebuffers[i - 1]);
    }

    // Blit mDownsampleFramebuffers.last() ----> mUpsampleFramebuffers.last()
    QOpenGLFramebufferObject::blitFramebuffer(mUpsampleFramebuffers.last(), //
                                              QRect(0, 0, mUpsampleFramebuffers.last()->width(), mUpsampleFramebuffers.last()->height()),
                                              mDownsampleFramebuffers.last(),
                                              QRect(0, 0, mDownsampleFramebuffers.last()->width(), mDownsampleFramebuffers.last()->height()),
                                              GL_COLOR_BUFFER_BIT,
                                              GL_NEAREST,
                                              0,
                                              0);

    QOpenGLFramebufferObject::blitFramebuffer(mUpsampleFramebuffers.last(), //
                                              QRect(0, 0, mUpsampleFramebuffers.last()->width(), mUpsampleFramebuffers.last()->height()),
                                              mDownsampleFramebuffers.last(),
                                              QRect(0, 0, mDownsampleFramebuffers.last()->width(), mDownsampleFramebuffers.last()->height()),
                                              GL_COLOR_BUFFER_BIT,
                                              GL_NEAREST,
                                              1,
                                              1);

    // Upsample and Smooth
    for (int i = mUpsampleFramebuffers.size() - 2; 0 <= i; --i)
    {
        upsample(mUpsampleFramebuffers[i], mTemporaryFrameBuffers[i], mUpsampleFramebuffers[i + 1], mDownsampleFramebuffers[i]);
    }

    // Last Pass Blur
    drawFinalBlurCurves(mUpsampleFramebuffers[0]);

    // Post processing (now we apply the actual blur and create the final image)
    if (target)
    {
        target->bind();
        glViewport(0, 0, target->width(), target->height());

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        mShaderManager->bind(ShaderType::BlurShader);
        mShaderManager->setSampler("colorTexture", 0, mUpsampleFramebuffers[0]->textures().at(0));
        mShaderManager->setSampler("blurTexture", 1, mUpsampleFramebuffers[0]->textures().at(1));
        mShaderManager->setUniformValue("widthRatio", mQualityFactor * float(target->width()) / mUpsampleFramebuffers[0]->width());
        mShaderManager->setUniformValue("heightRatio", mQualityFactor * float(target->height()) / mUpsampleFramebuffers[0]->height());
        mQuad->render();
        mShaderManager->release();
        target->release();

        // Restore
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);

    } else
    {
        // Else render to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        mShaderManager->bind(ShaderType::BlurShader);
        mShaderManager->setSampler("colorTexture", 0, mUpsampleFramebuffers[0]->textures().at(0));
        mShaderManager->setSampler("blurTexture", 1, mUpsampleFramebuffers[0]->textures().at(1));
        mShaderManager->setUniformValue("widthRatio", mQualityFactor * float(mWidth) / mUpsampleFramebuffers[0]->width());
        mShaderManager->setUniformValue("heightRatio", mQualityFactor * float(mHeight) / mUpsampleFramebuffers[0]->height());
        mQuad->render();
        mShaderManager->release();
    }
}

void RendererManager::renderContour(QOpenGLFramebufferObject *target, Bezier *curve, bool clearTarget)
{
    if (target)
    {
        mCamera->resize(target->width(), target->height());

        target->bind();
        glViewport(0, 0, target->width(), target->height());

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }

    } else
    {
        // Render to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    mShaderManager->bind(ShaderType::ContourShader);
    mPoints->bind();
    mShaderManager->setUniformValue("projection", mCamera->projection());
    mShaderManager->setUniformValue("pointDelta", mPoints->delta());
    mShaderManager->setUniformValue("zoom", mCamera->zoom());

    if (curve)
    {
        QVector<QVector2D> controlPoints = curve->getControlPointPositions();
        mShaderManager->setUniformValue("color", curve->mContourColor);
        mShaderManager->setUniformValue("thickness", curve->mContourThickness);
        mShaderManager->setUniformValue("controlPointsCount", (int) controlPoints.size());
        mShaderManager->setUniformValueArray("controlPoints", controlPoints);

        glDrawArrays(GL_POINTS, 0, mPoints->size());
    }

    mPoints->release();
    mShaderManager->release();

    if (target)
    {
        // Restore
        target->release();
        mCamera->resize(mWidth, mHeight);
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);
    }
}

void RendererManager::renderColors(QOpenGLFramebufferObject *draw)
{
    mCamera->resize(draw->width(), draw->height());
    mCamera->setLeft(mCamera->left() * mQualityFactor);
    mCamera->setTop(mCamera->top() * mQualityFactor);

    draw->bind();
    glViewport(0, 0, draw->width(), draw->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    mShaderManager->bind(ShaderType::ColorShader);
    mPoints->bind();

    mShaderManager->setUniformValue("projection", mCamera->projection());
    mShaderManager->setUniformValue("pointsDelta", mPoints->delta());
    mShaderManager->setUniformValue("zoom", mCamera->zoom());

    for (auto &curve : mCurves)
    {
        if (curve == nullptr)
            continue;

        curve->scale(mQualityFactor);

        auto controlPoints = curve->getControlPointPositions();
        auto leftColors = curve->getLeftColors();
        auto leftColorPositions = curve->getLeftColorPositions();
        auto rightColors = curve->getRightColors();
        auto rightColorPositions = curve->getRightColorPositions();
        auto blurPointPositions = curve->getBlurPointPositions();
        auto blurPointStrengths = curve->getBlurPointStrengths();

        mShaderManager->setUniformValue("diffusionWidth", mQualityFactor * curve->mDiffusionWidth);
        mShaderManager->setUniformValueArray("controlPoints", controlPoints);
        mShaderManager->setUniformValue("controlPointsCount", (int) controlPoints.size());
        mShaderManager->setUniformValueArray("leftColors", leftColors);
        mShaderManager->setUniformValueArray("leftColorPositions", leftColorPositions);
        mShaderManager->setUniformValue("leftColorsCount", (int) leftColorPositions.size());
        mShaderManager->setUniformValueArray("rightColors", rightColors);
        mShaderManager->setUniformValueArray("rightColorPositions", rightColorPositions);
        mShaderManager->setUniformValue("rightColorsCount", (int) rightColorPositions.size());
        mShaderManager->setUniformValueArray("blurPointPositions", blurPointPositions);
        mShaderManager->setUniformValueArray("blurPointStrengths", blurPointStrengths);
        mShaderManager->setUniformValue("blurPointsCount", (int) blurPointPositions.size());

        glDrawArrays(GL_POINTS, 0, mPoints->size());

        curve->scale(1.0f / mQualityFactor);
    }

    mPoints->release();
    mShaderManager->release();

    // Restore camera
    mCamera->resize(mWidth, mHeight);
    mCamera->setLeft(mCamera->left() / mQualityFactor);
    mCamera->setTop(mCamera->top() / mQualityFactor);

    draw->release();
}

void RendererManager::downsample(QOpenGLFramebufferObject *draw, QOpenGLFramebufferObject *read)
{
    draw->bind();
    glViewport(0, 0, draw->width(), draw->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    mShaderManager->bind(ShaderType::DownsampleShader);
    mShaderManager->setSampler("colorTexture", 0, read->textures().at(0));
    mShaderManager->setSampler("blurTexture", 1, read->textures().at(1));
    mQuad->render();
    mShaderManager->release();
    draw->release();
}

void RendererManager::upsample(QOpenGLFramebufferObject *draw, QOpenGLFramebufferObject *drawBuffer, QOpenGLFramebufferObject *source, QOpenGLFramebufferObject *target)
{
    draw->bind();
    glViewport(0, 0, draw->width(), draw->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    mShaderManager->bind(ShaderType::UpsampleShader);
    mShaderManager->setSampler("colorSourceTexture", 0, source->textures().at(0));
    mShaderManager->setSampler("colorTargetTexture", 1, target->textures().at(0));
    mShaderManager->setSampler("blurSourceTexture", 2, source->textures().at(1));
    mShaderManager->setSampler("blurTargetTexture", 3, target->textures().at(1));
    mQuad->render();
    mShaderManager->release();

    for (int j = 0; j < mSmoothIterations; j++)
    {
        QOpenGLFramebufferObject::blitFramebuffer(drawBuffer, //
                                                  QRect(0, 0, drawBuffer->width(), drawBuffer->height()),
                                                  draw,
                                                  QRect(0, 0, draw->width(), draw->height()),
                                                  GL_COLOR_BUFFER_BIT,
                                                  GL_NEAREST,
                                                  0,
                                                  0);

        QOpenGLFramebufferObject::blitFramebuffer(drawBuffer, //
                                                  QRect(0, 0, drawBuffer->width(), drawBuffer->height()),
                                                  draw,
                                                  QRect(0, 0, draw->width(), draw->height()),
                                                  GL_COLOR_BUFFER_BIT,
                                                  GL_NEAREST,
                                                  1,
                                                  1);

        mShaderManager->bind(ShaderType::JacobiShader);
        mShaderManager->setSampler("colorConstrainedTexture", 0, target->textures().at(0));
        mShaderManager->setSampler("colorTargetTexture", 1, drawBuffer->textures().at(0));
        mShaderManager->setSampler("blurConstrainedTexture", 2, target->textures().at(1));
        mShaderManager->setSampler("blurTargetTexture", 3, drawBuffer->textures().at(1));
        mQuad->render();
        mShaderManager->release();
    }

    draw->release();
}

void RendererManager::drawFinalBlurCurves(QOpenGLFramebufferObject *draw)
{
    mCamera->resize(draw->width(), draw->height());

    draw->bind();
    glViewport(0, 0, draw->width(), draw->height());

    mShaderManager->bind(ShaderType::LastPassBlurShader);
    mPoints->bind();

    mShaderManager->setUniformValue("projection", mCamera->projection());
    mShaderManager->setUniformValue("pointsDelta", mPoints->delta());
    mShaderManager->setUniformValue("zoom", mCamera->zoom());

    for (auto &curve : mCurves)
    {
        if (curve == nullptr)
            continue;

        auto controlPoints = curve->getControlPointPositions();
        auto blurPointPositions = curve->getBlurPointPositions();
        auto blurPointStrengths = curve->getBlurPointStrengths();

        mShaderManager->setUniformValue("diffusionWidth", curve->mDiffusionWidth);
        mShaderManager->setUniformValueArray("controlPoints", controlPoints);
        mShaderManager->setUniformValue("controlPointsCount", (int) controlPoints.size());
        mShaderManager->setUniformValueArray("blurPointPositions", blurPointPositions);
        mShaderManager->setUniformValueArray("blurPointStrengths", blurPointStrengths);
        mShaderManager->setUniformValue("blurPointsCount", (int) blurPointPositions.size());

        glDrawArrays(GL_POINTS, 0, mPoints->size());
    }

    mPoints->release();
    mShaderManager->release();
    draw->release();

    // Restore camera
    mCamera->resize(mWidth, mHeight);
}

void RendererManager::createFramebuffers()
{
    int size = mQualityFactor * qMax(mWidth, mHeight);

    mInitialFramebuffer = new QOpenGLFramebufferObject(size, size, mDefaultFramebufferFormat);
    mInitialFramebuffer->addColorAttachment(size, size); // For blur info
    mInitialFramebuffer->bind();
    glDrawBuffers(2, mDrawBuffers);
    mInitialFramebuffer->release();

    mFinalFramebuffer = new QOpenGLFramebufferObject(size, size, mFinalFramebufferFormat);
    mFinalMultisampleFramebuffer = new QOpenGLFramebufferObject(size, size, mFinalMultisampleFramebufferFormat);

    while (size != 0)
    {
        mDownsampleFramebuffers << new QOpenGLFramebufferObject(size, size, mDefaultFramebufferFormat);
        mDownsampleFramebuffers.last()->addColorAttachment(size, size); // For blur info
        mDownsampleFramebuffers.last()->bind();
        glDrawBuffers(2, mDrawBuffers);
        mDownsampleFramebuffers.last()->release();

        mUpsampleFramebuffers << new QOpenGLFramebufferObject(size, size, mDefaultFramebufferFormat);
        mUpsampleFramebuffers.last()->addColorAttachment(size, size); // For blur info
        mUpsampleFramebuffers.last()->bind();
        glDrawBuffers(2, mDrawBuffers);
        mUpsampleFramebuffers.last()->release();

        mTemporaryFrameBuffers << new QOpenGLFramebufferObject(size, size, mDefaultFramebufferFormat);
        mTemporaryFrameBuffers.last()->addColorAttachment(size, size); // For blur info
        mTemporaryFrameBuffers.last()->bind();
        glDrawBuffers(2, mDrawBuffers);
        mTemporaryFrameBuffers.last()->release();

        size /= 2;
    }
}

void RendererManager::deleteFramebuffers()
{
    if (mInitialFramebuffer)
        delete mInitialFramebuffer;

    if (mFinalFramebuffer)
        delete mFinalFramebuffer;

    if (mFinalMultisampleFramebuffer)
        delete mFinalMultisampleFramebuffer;

    for (int i = 0; i < mDownsampleFramebuffers.size(); ++i)
        delete mDownsampleFramebuffers[i];

    for (int i = 0; i < mUpsampleFramebuffers.size(); ++i)
        delete mUpsampleFramebuffers[i];

    for (int i = 0; i < mTemporaryFrameBuffers.size(); ++i)
        delete mTemporaryFrameBuffers[i];

    mDownsampleFramebuffers.clear();
    mUpsampleFramebuffers.clear();
    mTemporaryFrameBuffers.clear();
}

void RendererManager::setSmoothIterations(int newSmoothIterations)
{
    mSmoothIterations = newSmoothIterations;
}

void RendererManager::setQualityFactor(float newQualityFactor)
{
    if (qFuzzyCompare(mQualityFactor, newQualityFactor))
        return;

    mQualityFactor = newQualityFactor;
    deleteFramebuffers();
    createFramebuffers();
}

void RendererManager::save(const QString &path)
{
    mSave = true;
    mSavePath = path;
}

void RendererManager::setPixelRatio(float newPixelRatio)
{
    if (qFuzzyCompare(mPixelRatio, newPixelRatio))
        return;

    mPixelRatio = newPixelRatio;

    deleteFramebuffers();
    createFramebuffers();
}
