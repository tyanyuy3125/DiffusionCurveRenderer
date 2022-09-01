#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions_4_2_Core>

class Framebuffer : protected QOpenGLFunctions_4_2_Core
{
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    enum FBORenderTarget {
        MULTISAMPLING_FBO,
        MULTISAMPLING_TEXTURE,
        MULTISAMPLING_COLOR_RBO,
        MULTISAMPLING_DEPTH_RBO,
    };

    void bind();
    unsigned int texture() const;
    int width() const;
    int height() const;

private:
    GLuint mRenderRelatedIds[8];

    int mWidth;
    int mHeight;
};

#endif // FRAMEBUFFER_H
