#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include "Common.h"
#include "Manager.h"
#include <QOpenGLFunctions>

#include <QMatrix3x3>

class Shader;

class ShaderManager : public Manager
{
private:
    explicit ShaderManager(QObject *parent = nullptr);

public:
    bool init() override;
    bool bind(ShaderType shader);
    void release();

    void setUniformValue(const QString &name, int value);
    void setUniformValue(const QString &name, unsigned int value);
    void setUniformValue(const QString &name, float value);
    void setUniformValue(const QString &name, const QVector3D &value);
    void setUniformValue(const QString &name, const QVector4D &value);
    void setUniformValue(const QString &name, const QMatrix4x4 &value);
    void setUniformValue(const QString &name, const QMatrix3x3 &value);
    void setUniformValueArray(const QString &name, const QVector<float> &values);
    void setUniformValueArray(const QString &name, const QVector<QVector4D> &values);
    void setUniformValueArray(const QString &name, const QVector<QVector3D> &values);
    void setUniformValueArray(const QString &name, const QVector<QVector2D> &values);
    void setSampler(const QString &name, unsigned int unit, unsigned int id, GLenum target = GL_TEXTURE_2D);

    static ShaderManager *instance();

private:
    ShaderType mActiveShader;
    QMap<ShaderType, Shader *> mShaders;
};

#endif // SHADERMANAGER_H
