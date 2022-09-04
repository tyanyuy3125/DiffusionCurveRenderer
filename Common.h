#ifndef COMMON_H
#define COMMON_H

#include <QVector2D>

#define MAX_CONTROL_POINT_COUNT 32

#define DEFAULT_CONTOUR_THICKNESS 3.0f
#define DEFAULT_DIFFUSION_WIDTH 3.0f
#define DEFAULT_CONTOUR_COLOR QVector4D(0, 0, 0, 1)
#define DEFAULT_BLUR_STRENGTH 0.25f

#define BLUR_POINT_VISUAL_GAP 25.0f
#define COLOR_POINT_VISUAL_GAP 5.0f

enum class ShaderType { //
    ContourShader,
    ColorShader,
    ScreenShader,
    DownsampleShader,
    UpsampleShader,
    JacobiShader,
    BlurShader,
    LastPassBlurShader,
    BitmapShader
};

enum class WorkMode { //
    View,
    Edit,
};

enum class SubWorkMode { //
    ViewOriginalImage,
    ViewEdges,
    ViewGaussianStack,
    ViewEdgeStack
};

enum class ActionMode { //
    Select,
    AddControlPoint,
    AddColorPoint,
    AddBlurPoint,
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
    AddBlurPoint,
    RemoveCurve,
    RemoveControlPoint,
    RemoveColorPoint,
    RemoveBlurPoint,
    UpdateControlPointPosition,
    UpdateColorPointPosition,
    UpdateBlurPointPosition,
    LoadFromXML,
    LoadFromJSON,
    SaveAsPNG,
    SaveAsJSON,
    LoadVectorizerImage,
    ShowLoadFromXMLDialog,
    ShowLoadFromJSONDialog,
    ShowSaveAsPNGDialog,
    ShowSaveAsJSONDialog,
    ShowLoadImageDialog,
    ClearCanvas

};

enum class VectorizationStatus {
    Ready,
    Finished,
    CreatingGaussianStack,
    CreatingEdgeStack,
    TracingEdges,
    CreatingPolylines,
    ConstructingCurves

};

#endif // COMMON_H
