#include "RendererBase.h"

RendererBase::RendererBase()
    : mQualityFactor(1.0f)
    , mPixelRatio(1.0f)
    , mWidth(1600)
    , mHeight(900)
{}

void RendererBase::init()
{
    initializeOpenGLFunctions();
    mCurveManager = CurveManager::instance();
    mShaderManager = ShaderManager::instance();
    mCamera = EditModeCamera::instance();
}

void RendererBase::resize(int w, int h)
{
    mWidth = w;
    mHeight = h;
}

void RendererBase::setQualityFactor(float newQualityFactor)
{
    if (qFuzzyCompare(mQualityFactor, newQualityFactor))
        return;

    mQualityFactor = newQualityFactor;
}

void RendererBase::setPixelRatio(float newPixelRatio)
{
    if (qFuzzyCompare(mPixelRatio, newPixelRatio))
        return;

    mPixelRatio = newPixelRatio;
}

void RendererBase::setPoints(Points *newPoints)
{
    mPoints = newPoints;
}

void RendererBase::setQuad(Quad *newQuad)
{
    mQuad = newQuad;
}
