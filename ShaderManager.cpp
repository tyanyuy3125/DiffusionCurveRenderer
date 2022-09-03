#include "ShaderManager.h"
#include "Shader.h"

ShaderManager::ShaderManager(QObject *parent)
    : Manager{parent}
{}

bool ShaderManager::init()
{
    // Contour Shader
    {
        Shader *shader = new Shader(ShaderType::ContourShader);
        mShaders.insert(shader->type(), shader);
        shader->addPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Contour.vert");
        shader->addPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Contour.frag");
        shader->addPath(QOpenGLShader::Geometry, ":/Resources/Shaders/Contour.geom");
        shader->addUniform("projection");
        shader->addUniform("color");
        shader->addUniform("thickness");
        shader->addUniform("zoom");
        shader->addUniform("pointDelta");
        shader->addUniform("controlPoints");
        shader->addUniform("controlPointsCount");
        shader->addAttribute("point");

        if (!shader->init())
            return false;
    }

    // Color Shader
    {
        Shader *shader = new Shader(ShaderType::ColorShader);
        mShaders.insert(shader->type(), shader);
        shader->addPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Color.vert");
        shader->addPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Color.frag");
        shader->addPath(QOpenGLShader::Geometry, ":/Resources/Shaders/Color.geom");
        shader->addUniform("projection");
        shader->addUniform("pointsDelta");
        shader->addUniform("diffusionWidth");
        shader->addUniform("zoom");
        shader->addUniform("controlPoints");
        shader->addUniform("controlPointsCount");
        shader->addUniform("leftColors");
        shader->addUniform("leftColorPositions");
        shader->addUniform("leftColorsCount");
        shader->addUniform("rightColors");
        shader->addUniform("rightColorPositions");
        shader->addUniform("rightColorsCount");
        shader->addUniform("blurPointPositions");
        shader->addUniform("blurPointStrengths");
        shader->addUniform("blurPointsCount");
        shader->addAttribute("point");

        if (!shader->init())
            return false;
    }

    // Screen Shader
    {
        Shader *shader = new Shader(ShaderType::ScreenShader);
        mShaders.insert(shader->type(), shader);
        shader->addPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Screen.vert");
        shader->addPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Screen.frag");
        shader->addUniform("sourceTexture");
        shader->addUniform("widthRatio");
        shader->addUniform("heightRatio");
        shader->addAttribute("position");
        shader->addAttribute("textureCoords");

        if (!shader->init())
            return false;
    }

    // Downsample Shader
    {
        Shader *shader = new Shader(ShaderType::DownsampleShader);
        mShaders.insert(shader->type(), shader);
        shader->addPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Downsample.vert");
        shader->addPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Downsample.frag");
        shader->addUniform("colorTexture");
        shader->addUniform("blurTexture");
        shader->addAttribute("position");
        shader->addAttribute("textureCoords");

        if (!shader->init())
            return false;
    }

    // Upsample Shader
    {
        Shader *shader = new Shader(ShaderType::UpsampleShader);
        mShaders.insert(shader->type(), shader);
        shader->addPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Upsample.vert");
        shader->addPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Upsample.frag");
        shader->addUniform("colorSourceTexture");
        shader->addUniform("colorTargetTexture");
        shader->addUniform("blurSourceTexture");
        shader->addUniform("blurTargetTexture");

        shader->addAttribute("position");
        shader->addAttribute("textureCoords");

        if (!shader->init())
            return false;
    }

    // Jacobi Shader
    {
        Shader *shader = new Shader(ShaderType::JacobiShader);
        mShaders.insert(shader->type(), shader);
        shader->addPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Jacobi.vert");
        shader->addPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Jacobi.frag");
        shader->addUniform("colorConstrainedTexture");
        shader->addUniform("colorTargetTexture");
        shader->addUniform("blurConstrainedTexture");
        shader->addUniform("blurTargetTexture");
        shader->addAttribute("position");
        shader->addAttribute("textureCoords");

        if (!shader->init())
            return false;
    }

    // Blur Shader
    {
        Shader *shader = new Shader(ShaderType::BlurShader);
        mShaders.insert(shader->type(), shader);
        shader->addPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Blur.vert");
        shader->addPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Blur.frag");
        shader->addUniform("colorTexture");
        shader->addUniform("blurTexture");
        shader->addUniform("widthRatio");
        shader->addUniform("heightRatio");
        shader->addAttribute("position");
        shader->addAttribute("textureCoords");

        if (!shader->init())
            return false;
    }

    // Last Pass Blur Shader
    {
        Shader *shader = new Shader(ShaderType::LastPassBlurShader);
        mShaders.insert(shader->type(), shader);
        shader->addPath(QOpenGLShader::Vertex, ":/Resources/Shaders/LastPassBlur.vert");
        shader->addPath(QOpenGLShader::Fragment, ":/Resources/Shaders/LastPassBlur.frag");
        shader->addPath(QOpenGLShader::Geometry, ":/Resources/Shaders/LastPassBlur.geom");
        shader->addUniform("projection");
        shader->addUniform("pointsDelta");
        shader->addUniform("diffusionWidth");
        shader->addUniform("zoom");
        shader->addUniform("controlPoints");
        shader->addUniform("controlPointsCount");
        shader->addUniform("blurPointPositions");
        shader->addUniform("blurPointStrengths");
        shader->addUniform("blurPointsCount");
        shader->addAttribute("point");

        if (!shader->init())
            return false;
    }

    // Bitmap Shader
    {
        Shader *shader = new Shader(ShaderType::BitmapShader);
        mShaders.insert(shader->type(), shader);
        shader->addPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Bitmap.vert");
        shader->addPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Bitmap.frag");
        shader->addUniform("projection");
        shader->addUniform("transformation");
        shader->addUniform("sourceTexture");
        shader->addUniform("widthRatio");
        shader->addUniform("heightRatio");
        shader->addAttribute("position");
        shader->addAttribute("textureCoords");

        if (!shader->init())
            return false;
    }

    return true;
}

bool ShaderManager::bind(ShaderType shader)
{
    mActiveShader = shader;
    return mShaders.value(mActiveShader)->bind();
}

void ShaderManager::release()
{
    mShaders.value(mActiveShader)->release();
}

void ShaderManager::setUniformValue(const QString &name, int value)
{
    mShaders.value(mActiveShader)->setUniformValue(name, value);
}

void ShaderManager::setUniformValue(const QString &name, unsigned int value)
{
    mShaders.value(mActiveShader)->setUniformValue(name, value);
}

void ShaderManager::setUniformValue(const QString &name, float value)
{
    mShaders.value(mActiveShader)->setUniformValue(name, value);
}

void ShaderManager::setUniformValue(const QString &name, const QVector3D &value)
{
    mShaders.value(mActiveShader)->setUniformValue(name, value);
}

void ShaderManager::setUniformValue(const QString &name, const QVector4D &value)
{
    mShaders.value(mActiveShader)->setUniformValue(name, value);
}

void ShaderManager::setUniformValue(const QString &name, const QMatrix4x4 &value)
{
    mShaders.value(mActiveShader)->setUniformValue(name, value);
}

void ShaderManager::setUniformValue(const QString &name, const QMatrix3x3 &value)
{
    mShaders.value(mActiveShader)->setUniformValue(name, value);
}

void ShaderManager::setUniformValueArray(const QString &name, const QVector<float> &values)
{
    mShaders.value(mActiveShader)->setUniformValueArray(name, values);
}

void ShaderManager::setUniformValueArray(const QString &name, const QVector<QVector4D> &values)
{
    mShaders.value(mActiveShader)->setUniformValueArray(name, values);
}

void ShaderManager::setUniformValueArray(const QString &name, const QVector<QVector3D> &values)
{
    mShaders.value(mActiveShader)->setUniformValueArray(name, values);
}

void ShaderManager::setUniformValueArray(const QString &name, const QVector<QVector2D> &values)
{
    mShaders.value(mActiveShader)->setUniformValueArray(name, values);
}

void ShaderManager::setSampler(const QString &name, unsigned int unit, unsigned int id, GLenum target)
{
    mShaders.value(mActiveShader)->setSampler(name, unit, id, target);
}

ShaderManager *ShaderManager::instance()
{
    static ShaderManager instance;

    return &instance;
}
