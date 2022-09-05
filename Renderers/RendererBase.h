#ifndef RENDERERBASE_H
#define RENDERERBASE_H

#include "CurveManager.h"
#include "EditModeCamera.h"
#include "Points.h"
#include "Quad.h"
#include "ShaderManager.h"

class RendererBase : protected QOpenGLExtraFunctions
{
public:
    RendererBase();

    virtual void init();
    virtual void resize(int w, int h);
    virtual void setQualityFactor(float newQualityFactor);
    virtual void setPixelRatio(float newPixelRatio);

    void setPoints(Points *newPoints);
    void setQuad(Quad *newQuad);

protected:
    CurveManager *mCurveManager;
    ShaderManager *mShaderManager;
    EditModeCamera *mCamera;

    Points *mPoints;
    Quad *mQuad;

    float mQualityFactor;
    float mPixelRatio;
    int mWidth;
    int mHeight;
};

#endif // RENDERERBASE_H
