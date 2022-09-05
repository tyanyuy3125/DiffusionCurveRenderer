#ifndef COLORRENDERER_H
#define COLORRENDERER_H

#include "RendererBase.h"

#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>

class ColorRenderer : public RendererBase
{
public:
    ColorRenderer();

    void render(QOpenGLFramebufferObject *draw);
};

#endif // COLORRENDERER_H
