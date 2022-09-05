#include "RendererManager.h"

#include <QImage>
#include <QMatrix4x4>

RendererManager::RendererManager(QObject *parent)
    : Manager{parent}
    , mInitialFramebuffer(nullptr)
    , mFinalFramebuffer(nullptr)
    , mSmoothIterations(20)
    , mQualityFactor(1.0f)
    , mWidth(1600)
    , mHeight(900)
    , mPixelRatio(1.0f)
    , mSave(false)
{}

bool RendererManager::init()
{
    initializeOpenGLFunctions();
    // TODO

    mCurveManager = CurveManager::instance();

    mPoints = new Points;
    mQuad = new Quad;

    mContourRenderer = new ContourRenderer;
    mContourRenderer->setPoints(mPoints);
    mContourRenderer->setQuad(mQuad);
    mContourRenderer->init();

    mColorRenderer = new ColorRenderer;
    mColorRenderer->setPoints(mPoints);
    mColorRenderer->setQuad(mQuad);
    mColorRenderer->init();

    mDiffusionRenderer = new DiffusionRenderer;
    mDiffusionRenderer->setPoints(mPoints);
    mDiffusionRenderer->setQuad(mQuad);
    mDiffusionRenderer->init();

    mInitialFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mInitialFramebufferFormat.setSamples(0);
    mInitialFramebufferFormat.setMipmap(false);
    mInitialFramebufferFormat.setTextureTarget(GL_TEXTURE_2D);
    mInitialFramebufferFormat.setInternalTextureFormat(GL_RGBA8);

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
    if (mRenderMode == RenderMode::Diffusion)
    {
        mColorRenderer->render(mInitialFramebuffer);
        mDiffusionRenderer->render(mInitialFramebuffer, nullptr, true);

        // If a curve is selected then render it
        if (mCurveManager->selectedCurve())
        {
            mContourRenderer->render(nullptr, mCurveManager->selectedCurve(), false);
        }

    } else if (mRenderMode == RenderMode::Contour)
    {
        mContourRenderer->render(nullptr, true);

    } else if (mRenderMode == RenderMode::ContourAndDiffusion)
    {
        mColorRenderer->render(mInitialFramebuffer);
        mDiffusionRenderer->render(mInitialFramebuffer, nullptr, true);
        mContourRenderer->render(nullptr, false);
    }

    if (mSave)
    {
        if (mRenderMode == RenderMode::Diffusion)
        {
            mColorRenderer->render(mInitialFramebuffer);
            mDiffusionRenderer->render(mInitialFramebuffer, mFinalFramebuffer, true);
            mFinalFramebuffer->toImage().save(mSavePath);
        }

        if (mRenderMode == RenderMode::Contour)
        {
            mContourRenderer->render(mFinalMultisampleFramebuffer, true);
            mFinalMultisampleFramebuffer->toImage().save(mSavePath);
        }

        if (mRenderMode == RenderMode::ContourAndDiffusion)
        {
            mColorRenderer->render(mInitialFramebuffer);
            mDiffusionRenderer->render(mInitialFramebuffer, mFinalMultisampleFramebuffer, true);
            mContourRenderer->render(mFinalMultisampleFramebuffer, false);
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

    mContourRenderer->resize(mWidth, mHeight);
    mColorRenderer->resize(mWidth, mHeight);
    mDiffusionRenderer->resize(mWidth, mHeight);
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

void RendererManager::createFramebuffers()
{
    int size = mQualityFactor * qMax(mWidth, mHeight);

    mInitialFramebuffer = new QOpenGLFramebufferObject(size, size, mInitialFramebufferFormat);
    mInitialFramebuffer->addColorAttachment(size, size); // For blur info
    mInitialFramebuffer->bind();
    glDrawBuffers(2, mDrawBuffers);
    mInitialFramebuffer->release();

    mFinalFramebuffer = new QOpenGLFramebufferObject(size, size, mFinalFramebufferFormat);
    mFinalMultisampleFramebuffer = new QOpenGLFramebufferObject(size, size, mFinalMultisampleFramebufferFormat);
}

void RendererManager::deleteFramebuffers()
{
    if (mInitialFramebuffer)
        delete mInitialFramebuffer;

    if (mFinalFramebuffer)
        delete mFinalFramebuffer;

    if (mFinalMultisampleFramebuffer)
        delete mFinalMultisampleFramebuffer;
}

void RendererManager::setQualityFactor(float newQualityFactor)
{
    if (qFuzzyCompare(mQualityFactor, newQualityFactor))
        return;

    mQualityFactor = newQualityFactor;
    deleteFramebuffers();
    createFramebuffers();

    mContourRenderer->setQualityFactor(mQualityFactor);
    mColorRenderer->setQualityFactor(mQualityFactor);
    mDiffusionRenderer->setQualityFactor(mQualityFactor);
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

    mContourRenderer->setPixelRatio(mPixelRatio);
    mColorRenderer->setPixelRatio(mPixelRatio);
    mDiffusionRenderer->setPixelRatio(mPixelRatio);
}

void RendererManager::setSmoothIterations(int newSmoothIterations)
{
    mSmoothIterations = newSmoothIterations;
    mDiffusionRenderer->setSmoothIterations(mSmoothIterations);
}
