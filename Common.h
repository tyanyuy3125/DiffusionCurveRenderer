#ifndef COMMON_H
#define COMMON_H

#include <QVector2D>

#define MAX_CONTROL_POINT_COUNT 32

#define DEFAULT_CONTOUR_THICKNESS 3.0f
#define DEFAULT_DIFFUSION_WIDTH 3.0f
#define DEFAULT_CONTOUR_COLOR QVector4D(0, 0, 0, 1)

enum class ShaderType { //
    ContourShader,
    ColorShader,
    ScreenShader,
    DownsampleShader,
    UpsampleShader,
    JacobiShader,
    BlurShader
};

enum class Mode { //
    Select,
    AddControlPoint,
    AddColorPoint,
};

enum class RenderMode { //
    Contour,
    Diffusion,
    ContourAndDiffusion,
};

enum class Action { //
    Select,
    AddControlPoint,
    AddColorPoint,
    RemoveCurve,
    RemoveControlPoint,
    RemoveColorPoint,
    UpdateControlPointPosition,
    UpdateColorPointPosition,
    LoadFromXML,
    LoadFromJSON,
    SaveAsPNG,
    SaveAsJSON,
    ShowLoadFromXMLDialog,
    ShowLoadFromJSONDialog,
    ShowSaveAsPNGDialog,
    ShowSaveAsJSONDialog,
    ClearCanvas

};

#endif // COMMON_H
