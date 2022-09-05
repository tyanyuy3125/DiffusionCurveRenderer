#ifndef CONTOURRENDERER_H
#define CONTOURRENDERER_H

#include "RendererBase.h"

class ContourRenderer : public RendererBase
{
public:
    ContourRenderer();

    void render(QOpenGLFramebufferObject *target, bool clearTarget = true);
    void render(QOpenGLFramebufferObject *target, Bezier *curve, bool clearTarget = true);
};

#endif // CONTOURRENDERER_H
