#include "ColorRenderer.h"

ColorRenderer::ColorRenderer() {}

void ColorRenderer::render(QOpenGLFramebufferObject *draw)
{
    auto curves = mCurveManager->curves();

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

    for (auto &curve : curves)
    {
        if (curve == nullptr)
            continue;

        if (curve->mVoid)
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
        mShaderManager->setUniformValue("diffusionGap", mQualityFactor * curve->mDiffusionGap);
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
