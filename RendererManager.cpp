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
    , mSave(false)
{}

bool RendererManager::init()
{
    initializeOpenGLFunctions();

    mShaderManager = ShaderManager::instance();
    mCurveManager = CurveManager::instance();
    mCamera = Camera::instance();

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
    {
        mCamera->resize(mInitialFramebuffer->width(), mInitialFramebuffer->height());

        mInitialFramebuffer->bind();
        glViewport(0, 0, mInitialFramebuffer->width(), mInitialFramebuffer->height());
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

            auto controlPoints = curve->getControlPointPositions();

            auto leftColors = curve->getLeftColors();
            auto leftColorPositions = curve->getLeftColorPositions();

            auto rightColors = curve->getRightColors();
            auto rightColorPositions = curve->getRightColorPositions();

            auto blurPointPositions = curve->getBlurPointPositions();
            auto blurPointStrengths = curve->getBlurPointStrengths();

            mShaderManager->setUniformValue("diffusionWidth", curve->mDiffusionWidth);

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
        }

        mPoints->release();
        mShaderManager->release();

        // Restore camera
        mCamera->resize(mWidth, mHeight);
    }

    // Downsample 0
    {
        mDownsampleFramebuffers[0]->bind();
        glViewport(0, 0, mDownsampleFramebuffers[0]->width(), mDownsampleFramebuffers[0]->height());
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        mShaderManager->bind(ShaderType::DownsampleShader);
        mShaderManager->setSampler("colorTexture", 0, mInitialFramebuffer->textures().at(0));
        mShaderManager->setSampler("blurTexture", 1, mInitialFramebuffer->textures().at(1));
        mQuad->render();
        mShaderManager->release();
        mDownsampleFramebuffers[0]->release();
    }

    // Downsample 1,2,3...

    for (int i = 1; i < mDownsampleFramebuffers.size(); ++i)
    {
        mDownsampleFramebuffers[i]->bind();
        glViewport(0, 0, mDownsampleFramebuffers[i]->width(), mDownsampleFramebuffers[i]->height());
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        mShaderManager->bind(ShaderType::DownsampleShader);
        mShaderManager->setSampler("colorTexture", 0, mDownsampleFramebuffers[i - 1]->textures().at(0));
        mShaderManager->setSampler("blurTexture", 1, mDownsampleFramebuffers[i - 1]->textures().at(1));
        mQuad->render();
        mShaderManager->release();
    }

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

    // Upsample and Smooth second last
    {
        int i = mUpsampleFramebuffers.size() - 2;
        mUpsampleFramebuffers[i]->bind();
        glViewport(0, 0, mUpsampleFramebuffers[i]->width(), mUpsampleFramebuffers[i]->height());
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        mShaderManager->bind(ShaderType::UpsampleShader);
        mShaderManager->setSampler("colorSourceTexture", 0, mUpsampleFramebuffers.last()->textures().at(0));
        mShaderManager->setSampler("colorTargetTexture", 1, mDownsampleFramebuffers[i]->textures().at(0));
        mShaderManager->setSampler("blurSourceTexture", 2, mUpsampleFramebuffers.last()->textures().at(1));
        mShaderManager->setSampler("blurTargetTexture", 3, mDownsampleFramebuffers[i]->textures().at(1));
        mQuad->render();
        mShaderManager->release();

        mShaderManager->bind(ShaderType::JacobiShader);
        mShaderManager->setSampler("colorConstrainedTexture", 0, mDownsampleFramebuffers.last()->textures().at(0));
        mShaderManager->setSampler("colorTargetTexture", 1, mUpsampleFramebuffers[i]->textures().at(0));
        mShaderManager->setSampler("blurConstrainedTexture", 2, mDownsampleFramebuffers.last()->textures().at(1));
        mShaderManager->setSampler("blurTargetTexture", 3, mUpsampleFramebuffers[i]->textures().at(1));
        mQuad->render();
        mShaderManager->release();

        for (int j = 0; j < mSmoothIterations; j++)
        {
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

            mShaderManager->bind(ShaderType::JacobiShader);
            mShaderManager->setSampler("colorConstrainedTexture", 0, mDownsampleFramebuffers.last()->textures().at(0));
            mShaderManager->setSampler("colorTargetTexture", 1, mUpsampleFramebuffers[i]->textures().at(0));
            mShaderManager->setSampler("blurConstrainedTexture", 2, mDownsampleFramebuffers.last()->textures().at(1));
            mShaderManager->setSampler("blurTargetTexture", 3, mUpsampleFramebuffers[i]->textures().at(1));
            mQuad->render();
            mShaderManager->release();
        }

        mUpsampleFramebuffers[i]->release();
    }

    // Upsample and Smooth
    for (int i = mUpsampleFramebuffers.size() - 3; 0 <= i; --i)
    {
        mUpsampleFramebuffers[i]->bind();
        glViewport(0, 0, mUpsampleFramebuffers[i]->width(), mUpsampleFramebuffers[i]->height());
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        mShaderManager->bind(ShaderType::UpsampleShader);
        mShaderManager->setSampler("colorSourceTexture", 0, mUpsampleFramebuffers[i + 1]->textures().at(0));
        mShaderManager->setSampler("colorTargetTexture", 1, mDownsampleFramebuffers[i]->textures().at(0));
        mShaderManager->setSampler("blurSourceTexture", 2, mUpsampleFramebuffers[i + 1]->textures().at(1));
        mShaderManager->setSampler("blurTargetTexture", 3, mDownsampleFramebuffers[i]->textures().at(1));
        mQuad->render();
        mShaderManager->release();

        for (int j = 0; j < mSmoothIterations; j++)
        {
            QOpenGLFramebufferObject::blitFramebuffer(mTemporaryFrameBuffers[i], //
                                                      QRect(0, 0, mTemporaryFrameBuffers[i]->width(), mTemporaryFrameBuffers[i]->height()),
                                                      mUpsampleFramebuffers[i],
                                                      QRect(0, 0, mUpsampleFramebuffers[i]->width(), mUpsampleFramebuffers[i]->height()),
                                                      GL_COLOR_BUFFER_BIT,
                                                      GL_NEAREST,
                                                      0,
                                                      0);

            QOpenGLFramebufferObject::blitFramebuffer(mTemporaryFrameBuffers[i], //
                                                      QRect(0, 0, mTemporaryFrameBuffers[i]->width(), mTemporaryFrameBuffers[i]->height()),
                                                      mUpsampleFramebuffers[i],
                                                      QRect(0, 0, mUpsampleFramebuffers[i]->width(), mUpsampleFramebuffers[i]->height()),
                                                      GL_COLOR_BUFFER_BIT,
                                                      GL_NEAREST,
                                                      1,
                                                      1);

            mShaderManager->bind(ShaderType::JacobiShader);
            mShaderManager->setSampler("colorConstrainedTexture", 0, mDownsampleFramebuffers[i]->textures().at(0));
            mShaderManager->setSampler("colorTargetTexture", 1, mTemporaryFrameBuffers[i]->textures().at(0));
            mShaderManager->setSampler("blurConstrainedTexture", 2, mDownsampleFramebuffers[i]->textures().at(1));
            mShaderManager->setSampler("blurTargetTexture", 3, mTemporaryFrameBuffers[i]->textures().at(1));
            mQuad->render();
            mShaderManager->release();
        }

        mUpsampleFramebuffers[i]->release();
    }

    //    mUpsampleFramebuffers[0]->toImage(true, 0).save("000.bmp");
    //    mUpsampleFramebuffers[0]->toImage(true, 1).save("001.bmp");

    // Last Pass Blur
    {
        mCamera->resize(mUpsampleFramebuffers[0]->width(), mUpsampleFramebuffers[0]->height());

        mUpsampleFramebuffers[0]->bind();
        glViewport(0, 0, mUpsampleFramebuffers[0]->width(), mUpsampleFramebuffers[0]->height());

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

        // Restore camera
        mCamera->resize(mWidth, mHeight);
    }

    //    mUpsampleFramebuffers[0]->toImage(true, 0).save("000.bmp");
    //    mUpsampleFramebuffers[0]->toImage(true, 1).save("001.bmp");

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
        mShaderManager->setUniformValue("widthRatio", float(target->width()) / mUpsampleFramebuffers[0]->width());
        mShaderManager->setUniformValue("heightRatio", float(target->height()) / mUpsampleFramebuffers[0]->height());
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
        mShaderManager->setUniformValue("widthRatio", float(mWidth) / mUpsampleFramebuffers[0]->width());
        mShaderManager->setUniformValue("heightRatio", float(mHeight) / mUpsampleFramebuffers[0]->height());
        mQuad->render();
        mShaderManager->release();
    }
}

void RendererManager::createFramebuffers()
{
    int size = qMax(mWidth, mHeight);

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

int RendererManager::smoothIterations() const
{
    return mSmoothIterations;
}

void RendererManager::setSmoothIterations(int newSmoothIterations)
{
    mSmoothIterations = newSmoothIterations;
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

void RendererManager::applyBlur(QOpenGLFramebufferObject *read, QOpenGLFramebufferObject *draw, int times)
{
    mShaderManager->bind(ShaderType::BlurShader);
    mShaderManager->setUniformValue("width", read->width());
    mShaderManager->setUniformValue("height", read->height());
    mShaderManager->release();

    for (int i = 0; i < times; i++)
    {
        draw->bind();
        glClear(GL_COLOR_BUFFER_BIT);
        mShaderManager->bind(ShaderType::BlurShader);
        mShaderManager->setSampler("screenTexture", 0, read->texture());
        mShaderManager->setUniformValue("horizontal", true);
        mQuad->render();
        mShaderManager->release();

        fillFramebuffer(draw, read);

        draw->bind();
        glClear(GL_COLOR_BUFFER_BIT);
        mShaderManager->bind(ShaderType::BlurShader);
        mShaderManager->setSampler("screenTexture", 0, read->texture());
        mShaderManager->setUniformValue("horizontal", false);
        mQuad->render();
        mShaderManager->release();

        fillFramebuffer(draw, read);
    }
}

void RendererManager::fillFramebuffer(QOpenGLFramebufferObject *source, QOpenGLFramebufferObject *target)
{
    target->bind();
    glClear(GL_COLOR_BUFFER_BIT);
    mShaderManager->bind(ShaderType::ScreenShader);
    mShaderManager->setUniformValue("widthRatio", 1.0f);
    mShaderManager->setUniformValue("heightRatio", 1.0f);
    mShaderManager->setSampler("screenTexture", 0, source->texture());
    mQuad->render();
    mShaderManager->release();
}
