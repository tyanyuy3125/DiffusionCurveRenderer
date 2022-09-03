QT       += core gui xml openglwidgets opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(Dependencies/qtimgui/qtimgui.pri)

SOURCES += \
    Bezier.cpp \
    BitmapRenderer.cpp \
    BlurPoint.cpp \
    ColorPoint.cpp \
    ControlPoint.cpp \
    Controller.cpp \
    Curve.cpp \
    CurveManager.cpp \
    CustomVariant.cpp \
    EditModeCamera.cpp \
    Framebuffer.cpp \
    GaussianStack.cpp \
    Helper.cpp \
    Main.cpp \
    Manager.cpp \
    Points.cpp \
    Quad.cpp \
    RendererManager.cpp \
    Shader.cpp \
    ShaderManager.cpp \
    Vectorizer.cpp \
    ViewModeCamera.cpp \
    Window.cpp

HEADERS += \
    Bezier.h \
    BitmapRenderer.h \
    BlurPoint.h \
    ColorPoint.h \
    Common.h \
    ControlPoint.h \
    Controller.h \
    Curve.h \
    CurveManager.h \
    CustomVariant.h \
    EditModeCamera.h \
    Framebuffer.h \
    GaussianStack.h \
    Helper.h \
    Manager.h \
    Points.h \
    Quad.h \
    RendererManager.h \
    Shader.h \
    ShaderManager.h \
    Vectorizer.h \
    ViewModeCamera.h \
    Window.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resources.qrc


INCLUDEPATH += $$PWD/Dependencies/opencv-4.6.0/include
DEPENDPATH += $$PWD/Dependencies/opencv-4.6.0/include

LIBS += \
-L$$PWD/Dependencies/opencv-4.6.0/x64/vc16/lib/ -lopencv_core460 \
-L$$PWD/Dependencies/opencv-4.6.0/x64/vc16/lib/ -lopencv_imgproc460 \
-L$$PWD/Dependencies/opencv-4.6.0/x64/vc16/lib/ -lopencv_highgui460 \
-L$$PWD/Dependencies/opencv-4.6.0/x64/vc16/lib/ -lopencv_imgcodecs460
