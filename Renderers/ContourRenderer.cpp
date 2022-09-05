#include "ContourRenderer.h"

#include <QOpenGLFramebufferObject>

ContourRenderer::ContourRenderer() {}

void ContourRenderer::render(QOpenGLFramebufferObject *target, bool clearTarget)
{
    auto curves = mCurveManager->curves();

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

    for (const auto &curve : curves)
    {
        if (curve == nullptr)
            continue;

        if (curve->mVoid)
            continue;

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

void ContourRenderer::render(QOpenGLFramebufferObject *target, Bezier *curve, bool clearTarget)
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
