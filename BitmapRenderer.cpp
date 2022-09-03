#include "BitmapRenderer.h"

BitmapRenderer::BitmapRenderer(QObject *parent)
    : Manager(parent)
    , mTexture(0)
    , mScreenWidth(1600)
    , mScreeHeight(900)
    , mPixelRatio(1.0f)
{}

BitmapRenderer *BitmapRenderer::instance()
{
    static BitmapRenderer instance;
    return &instance;
}

bool BitmapRenderer::init()
{
    initializeOpenGLFunctions();

    mCamera = ViewModeCamera::instance();
    mShaderManager = ShaderManager::instance();

    mQuad = new Quad;

    return true;
}

void BitmapRenderer::render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, mPixelRatio * mScreenWidth, mPixelRatio * mScreeHeight);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    mShaderManager->bind(ShaderType::BitmapShader);
    mShaderManager->setUniformValue("projection", mCamera->projection());
    mShaderManager->setUniformValue("transformation", mImageTransformation);
    mShaderManager->setSampler("sourceTexture", 0, mTexture);
    mQuad->render();
    mShaderManager->release();
}

void BitmapRenderer::setData(cv::Mat image, int width, int height)
{
    if (mTexture)
        glDeleteTextures(1, &mTexture);

    mTextureWidth = width;
    mTextureHeight = height;

    mImageTransformation.setToIdentity();
    mImageTransformation.scale(QVector3D(mTextureWidth / 2.0, mTextureHeight / 2.0, 1));
    mImageTransformation.setColumn(3, QVector4D(0, 0, 0, 1));

    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, mTextureWidth, mTextureHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, image.ptr());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void BitmapRenderer::resize(int width, int height)
{
    mScreenWidth = width;
    mScreeHeight = height;
}

void BitmapRenderer::setPixelRatio(float newPixelRatio)
{
    mPixelRatio = newPixelRatio;
}
