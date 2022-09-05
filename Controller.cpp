#include "Controller.h"
#include "Helper.h"
#include "Window.h"

#include <QApplication>
#include <QDebug>
#include <QOpenGLPaintDevice>
#include <QPaintDevice>
#include <QPainter>
#include <QtConcurrent>

Controller::Controller(QObject *parent)
    : QObject(parent)
    , mIfps(0.0f)
    , mWorkMode(WorkMode::Edit)
    , mSubWorkMode(SubWorkMode::ViewOriginalImage)
    , mPreviousSubWorkMode(SubWorkMode::ViewOriginalImage)
    , mActionMode(ActionMode::Select)
    , mRenderMode(RenderMode::Contour)
    , mGlobalContourThickness(DEFAULT_CONTOUR_THICKNESS)
    , mGlobalDiffusionWidth(DEFAULT_DIFFUSION_WIDTH)
    , mGlobalContourColor(DEFAULT_CONTOUR_COLOR)
    , mGlobalBlurStrength(DEFAULT_BLUR_STRENGTH)
    , mSmoothIterations(20)
    , mQualityFactor(1)
    , mVoidThreshold(0.0f)
    , mVectorizerImageLoaded(false)
    , mShowFileDailog(false)
    , mSelectedCurve(nullptr)
    , mSelectedControlPoint(nullptr)
    , mSelectedColorPoint(nullptr)
    , mSelectedBlurPoint(nullptr)
{
    mDashedPen.setDashPattern({8, 8});
    mDashedPen.setWidthF(1.0f);
    mDashedPen.setJoinStyle(Qt::MiterJoin);

    mDenseDashedPen.setDashPattern({4, 4});
    mDenseDashedPen.setWidthF(1.0f);
    mDenseDashedPen.setJoinStyle(Qt::MiterJoin);

    mSolidPen.setWidthF(1.0f);
    mSolidPen.setJoinStyle(Qt::MiterJoin);
}

Controller::~Controller()
{
    qDebug() << Q_FUNC_INFO;
}

void Controller::init()
{
    initializeOpenGLFunctions();
    //    glEnable(GL_BLEND);
    //    glBlendFunc(GL_ONE, GL_ZERO);

    mRendererManager = RendererManager::instance();
    mCurveManager = CurveManager::instance();
    mShaderManager = ShaderManager::instance();
    mBitmapRenderer = BitmapRenderer::instance();

    mVectorizer = Vectorizer::instance();
    mVectorizer->moveToThread(&mVectorizerThread);
    connect(this, &Controller::draw, mVectorizer, &Vectorizer::draw, Qt::DirectConnection);
    connect(this, &Controller::load, mVectorizer, &Vectorizer::load, Qt::QueuedConnection);
    mVectorizerThread.start();

    mEditModeCamera = EditModeCamera::instance();
    mEditModeCamera->setPixelRatio(mWindow->devicePixelRatio());

    mViewModeCamera = ViewModeCamera::instance();
    mViewModeCamera->setPixelRatio(mWindow->devicePixelRatio());

    mManagers << mRendererManager //
              << mCurveManager    //
              << mShaderManager   //
              << mBitmapRenderer;

    for (auto manager : mManagers)
        manager->init();

    connect(mWindow, &Window::destroyed, this, [=]() { //
        qDebug() << Q_FUNC_INFO;
        if (mVectorizerThread.isRunning())
            mVectorizerThread.quit();
    });

    QVector<Bezier *> curves = Helper::loadCurveDataFromXML(":Resources/CurveData/zephyr.xml");
    if (!curves.isEmpty())
    {
        mCurveManager->clear();
        mCurveManager->addCurves(curves);
    }
}

void Controller::onAction(Action action, CustomVariant value)
{
    switch (action)
    {
    case Action::Select: {
        mCurveManager->select(value.toVector2D(), mEditModeCamera->zoom() * 15.0f);
        break;
    }
    case Action::AddControlPoint: {
        mCurveManager->addControlPoint(value.toVector2D());
        break;
    }
    case Action::AddColorPoint: {
        mCurveManager->addColorPoint(value.toVector2D());
        break;
    }
    case Action::AddBlurPoint: {
        mCurveManager->addBlurPoint(value.toVector2D());
        break;
    }
    case Action::RemoveCurve:
        mCurveManager->removeSelectedCurve();
        break;
    case Action::RemoveControlPoint:
        mCurveManager->removeSelectedControlPoint();
        break;
    case Action::RemoveColorPoint:
        mCurveManager->removeSelectedColorPoint();
        break;
    case Action::RemoveBlurPoint:
        mCurveManager->removeSelectedBlurPoint();
        break;
    case Action::UpdateControlPointPosition:
        if (mSelectedControlPoint)
            mSelectedControlPoint->mPosition = value.toVector2D();
        break;
    case Action::UpdateColorPointPosition:
        if (mSelectedColorPoint)
            mSelectedColorPoint->mPosition = mSelectedCurve->parameterAt(value.toVector2D(), 10000);
        break;
    case Action::UpdateBlurPointPosition:
        if (mSelectedBlurPoint)
            mSelectedBlurPoint->mPosition = mSelectedCurve->parameterAt(value.toVector2D(), 10000);
        break;
    case Action::ClearCanvas:
        mCurveManager->clear();
        break;
    case Action::LoadFromXML: {
        QVector<Bezier *> curves = Helper::loadCurveDataFromXML(value.toString());
        if (!curves.isEmpty())
        {
            mCurveManager->clear();
            mCurveManager->addCurves(curves);
        }
        break;
    }
    case Action::SaveAsPNG: {
        mRendererManager->save(value.toString());
        break;
    }
    case Action::LoadFromJSON: {
        QVector<Bezier *> curves = Helper::loadCurveDataFromJSON(value.toString());
        if (!curves.isEmpty())
        {
            mCurveManager->clear();
            mCurveManager->addCurves(curves);
        }
        break;
    }
    case Action::LoadVectorizerImage: {
        emit load(value.toString());
        mVectorizerImageLoaded = true;
        mWorkMode = WorkMode::Vectorization;
        mSubWorkMode = SubWorkMode::ViewOriginalImage;
        break;
    }
    case Action::SaveAsJSON: {
        Helper::saveCurveDataToJSON(mCurveManager->curves(), value.toString());
        break;
    }
    case Action::ShowLoadFromJSONDialog:
        mFileDailogType = FileDialogType::LoadFromJSON;
        mShowFileDailog = true;
        break;
    case Action::ShowSaveAsJSONDialog:
        mFileDailogType = FileDialogType::SaveAsJSON;
        mShowFileDailog = true;
        break;
    case Action::ShowLoadFromXMLDialog:
        mFileDailogType = FileDialogType::LoadFromXML;
        mShowFileDailog = true;
        break;
    case Action::ShowSaveAsPNGDialog:
        mFileDailogType = FileDialogType::SaveAsPNG;
        mShowFileDailog = true;
        break;
    case Action::ShowLoadImageDialog:
        mFileDailogType = FileDialogType::LoadVectorizerImage;
        mShowFileDailog = true;
        break;
    }
}

void Controller::render(float ifps)
{
    mIfps = ifps;

    // Update member variables
    mSelectedCurve = mCurveManager->selectedCurve();
    mSelectedControlPoint = mCurveManager->selectedControlPoint();
    mSelectedColorPoint = mCurveManager->selectedColorPoint();
    mSelectedBlurPoint = mCurveManager->selectedBlurPoint();
    mPixelRatio = mWindow->devicePixelRatio();

    // Update member classes
    mEditModeCamera->setPixelRatio(mPixelRatio);
    mViewModeCamera->setPixelRatio(mPixelRatio);
    mBitmapRenderer->setPixelRatio(mPixelRatio);
    mRendererManager->setPixelRatio(mPixelRatio);
    mRendererManager->setRenderMode(mRenderMode);

    if (mSelectedCurve)
    {
        mControlPoints = mSelectedCurve->controlPoints();
        mColorPoints = mSelectedCurve->getAllColorPoints();
        mBlurPoints = mSelectedCurve->blurPoints();
    } else
    {
        if (mActionMode == ActionMode::AddColorPoint)
            mActionMode = ActionMode::Select;

        mControlPoints.clear();
        mColorPoints.clear();
        mBlurPoints.clear();
    }

    // Render
    if (mWorkMode == WorkMode::Edit)
    {
        mEditModeCamera->update(ifps);
        mRendererManager->render();
    } else if (mWorkMode == WorkMode::Vectorization)
    {
        mViewModeCamera->update(ifps);
        mBitmapRenderer->render();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);

    drawPainter();
    drawGUI();

    // ImGui Stuff
    mImGuiWantsMouseCapture = ImGui::GetIO().WantCaptureMouse;
    mImGuiWantsKeyboardCapture = ImGui::GetIO().WantCaptureKeyboard;

    // We must show file dialog in the render loop; otherwise the app crashes.
    if (mShowFileDailog)
    {
        mShowFileDailog = false;

        switch (mFileDailogType)
        {
        case FileDialogType::LoadFromXML: {
            QString path = QFileDialog::getOpenFileName(nullptr, "Select a XML file", QString(), "*.xml");
            if (!path.isNull())
                onAction(Action::LoadFromXML, path);
            break;
        }
        case FileDialogType::LoadFromJSON: {
            QString path = QFileDialog::getOpenFileName(nullptr, "Select a JSON file", QString(), "*.json");
            if (!path.isNull())
                onAction(Action::LoadFromJSON, path);
            break;
        }
        case FileDialogType::SaveAsPNG: {
            QString path = QFileDialog::getSaveFileName(nullptr, "Save as PNG", "", "*.png");
            if (!path.isNull())
                onAction(Action::SaveAsPNG, path);
            break;
        }
        case FileDialogType::SaveAsJSON: {
            QString path = QFileDialog::getSaveFileName(nullptr, "Save as JSON", "", "*.json");
            if (!path.isNull())
                onAction(Action::SaveAsJSON, path);
            break;
        }
        case FileDialogType::LoadVectorizerImage: {
            QString path = QFileDialog::getOpenFileName(nullptr, "Select an Image", "", "*.png *.jpg *.jpeg *.bmp");
            if (!path.isNull())
                onAction(Action::LoadVectorizerImage, path);

            break;
        }
        }
    }
}

void Controller::drawGUI()
{
    QtImGui::newFrame();

    ImGui::SetNextWindowSize(ImVec2(420, 820), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls", NULL, ImGuiWindowFlags_MenuBar);

    // Menu
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Load an Image for Vectorization"))
                onAction(Action::ShowLoadImageDialog);

            ImGui::Separator();

            if (ImGui::MenuItem("Load from XML"))
                onAction(Action::ShowLoadFromXMLDialog);

            if (ImGui::MenuItem("Load from JSON"))
                onAction(Action::ShowLoadFromJSONDialog);

            ImGui::Separator();

            if (ImGui::MenuItem("Save as PNG"))
                onAction(Action::ShowSaveAsPNGDialog);

            if (ImGui::MenuItem("Save as JSON"))
                onAction(Action::ShowSaveAsJSONDialog);

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Work Modes
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Work Modes");

        int mode = (int) mWorkMode;
        ImGui::BeginDisabled(!mVectorizerImageLoaded);
        ImGui::RadioButton("Vectorize##WorkMode", &mode, 0);
        ImGui::EndDisabled();
        ImGui::RadioButton("Edit##WorkMode", &mode, 1);
        mWorkMode = WorkMode(mode);
    }

    if (mWorkMode == WorkMode::Vectorization)
    {
        emit draw();
    } else if (mWorkMode == WorkMode::Edit)
    {
        // Action Modes
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Actions Modes");

            int mode = (int) mActionMode;
            ImGui::RadioButton("Select##ActionMode", &mode, 0);
            ImGui::RadioButton("Add Control Point (Ctrl)##ActionMode", &mode, 1);
            ImGui::BeginDisabled(!mSelectedCurve);
            ImGui::RadioButton("Add Color Point (Alt)##ActionMode", &mode, 2);
            ImGui::RadioButton("Add Blur Point##ActionMode", &mode, 3);
            ImGui::EndDisabled();
            mActionMode = ActionMode(mode);
        }

        ImGui::Spacing();

        // Render Settings
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Render Settings");
            bool b0 = mRenderMode == RenderMode::Contour;
            bool b1 = mRenderMode == RenderMode::Diffusion;
            bool b2 = mRenderMode == RenderMode::ContourAndDiffusion;

            if (ImGui::Checkbox("Contours", &b0))
                mRenderMode = RenderMode::Contour;

            if (ImGui::Checkbox("Diffusion", &b1))
                mRenderMode = RenderMode::Diffusion;

            if (ImGui::Checkbox("Contours and Diffusion", &b2))
                mRenderMode = RenderMode::ContourAndDiffusion;

            if (ImGui::SliderInt("Smooth Iterations", &mSmoothIterations, 2, 50))
                mRendererManager->setSmoothIterations(mSmoothIterations);

            const char *names[3] = {"Empty", "Default", "High"};
            const char *elem_name = names[mQualityFactor];

            if (ImGui::SliderInt("Render Quality", &mQualityFactor, 1, 2, elem_name))
                mRendererManager->setQualityFactor(mQualityFactor);

            if (ImGui::SliderFloat("Global Blur Strength", &mGlobalBlurStrength, 0.0f, 1.0f))
                mCurveManager->setGlobalBlurStrength(mGlobalBlurStrength);

            if (ImGui::SliderFloat("Global Thickness", &mGlobalContourThickness, 2, 10))
                mCurveManager->setGlobalContourThickness(mGlobalContourThickness);

            if (ImGui::SliderFloat("Global Diffusion Width", &mGlobalDiffusionWidth, 2, 40))
                mCurveManager->setGlobalDiffusionWidth(mGlobalDiffusionWidth);

            if (ImGui::ColorEdit4("Global Contour Color", &mGlobalContourColor[0]))
                mCurveManager->setGlobalContourColor(mGlobalContourColor);

            if (ImGui::SliderFloat("Void Threshold", &mVoidThreshold, 0.0f, 32.0f))
                mCurveManager->makeVoid(mVoidThreshold);
        }

        ImGui::Spacing();

        // Curve
        if (mSelectedCurve)
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Curve");
            ImGui::Text("Number of Control Points: %d", (int) mSelectedCurve->controlPoints().size());
            ImGui::Text("Number of Color Points: %d", (int) mSelectedCurve->getAllColorPoints().size());
            if (ImGui::InputInt("Depth", &mSelectedCurve->mDepth))
                mCurveManager->sortCurves();
            ImGui::SliderFloat("Thickness", &mSelectedCurve->mContourThickness, 2, 50);
            ImGui::SliderFloat("Diffusion Width", &mSelectedCurve->mDiffusionWidth, 2, 50);
            ImGui::ColorEdit4("Contour Color", &mSelectedCurve->mContourColor[0]);
            if (ImGui::Button("Remove Curve"))
                onAction(Action::RemoveCurve);
        }

        ImGui::Spacing();

        // Control Point
        if (mSelectedControlPoint)
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Control Point");
            ImGui::InputFloat2("Position (x,y)", &mSelectedControlPoint->mPosition[0]);
            if (ImGui::Button("Remove Control Point"))
                onAction(Action::RemoveControlPoint);
        }

        ImGui::Spacing();

        // Color Point
        if (mSelectedColorPoint)
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Color Point");

            ImGui::Text("Direction: %s", mSelectedColorPoint->mDirection == ColorPoint::Direction::Left ? "Left" : "Right");
            ImGui::SliderFloat("Position", &mSelectedColorPoint->mPosition, 0.0f, 1.0f);
            ImGui::ColorEdit4("Color", &mSelectedColorPoint->mColor[0]);

            if (ImGui::Button("Remove Color Point"))
                onAction(Action::RemoveColorPoint);
        }

        ImGui::Spacing();

        // Blur Point
        if (mSelectedBlurPoint)
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Blur Point");

            ImGui::SliderFloat("Position", &mSelectedBlurPoint->mPosition, 0.0f, 1.0f);
            ImGui::SliderFloat("Strength", &mSelectedBlurPoint->mStrength, 0.0f, 1.0f);

            if (ImGui::Button("Remove Blur Point"))
                onAction(Action::RemoveBlurPoint);
        }

        ImGui::Spacing();

        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Info");
        ImGui::Text("Number of Curves: %d", (int) mCurveManager->curves().size());
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        if (ImGui::Button("Clear Canvas"))
            onAction(Action::ClearCanvas);
    }

    ImGui::End();

    ImGui::Render();
    QtImGui::render();
}

void Controller::drawPainter()
{
    if (mWorkMode == WorkMode::Edit && mSelectedCurve)
    {
        mDashedPen.setColor(QColor(0, 0, 0));
        mSolidPen.setColor(QColor(0, 0, 0));

        QOpenGLPaintDevice device(mPixelRatio * mWidth, mPixelRatio * mHeight);
        QPainter painter(&device);
        painter.setRenderHint(QPainter::Antialiasing, true);

        // Control polygon
        painter.setPen(mDashedPen);
        painter.setBrush(QBrush());
        for (int i = 0; i < mControlPoints.size() - 1; ++i)
        {
            QPointF p0 = mEditModeCamera->toGUI(mControlPoints[i]->mPosition);
            QPointF p1 = mEditModeCamera->toGUI(mControlPoints[i + 1]->mPosition);
            painter.drawLine(p0, p1);
        }

        // Blur Points

        for (int i = 0; i < mBlurPoints.size(); ++i)
        {
            QPointF visualPosition = mEditModeCamera->toGUI(mBlurPoints[i]->getPosition2D(mEditModeCamera->zoom() * BLUR_POINT_VISUAL_GAP));
            QPointF actualPosition = mEditModeCamera->toGUI(mSelectedCurve->valueAt(mBlurPoints[i]->mPosition));

            // Draw a dashed line actual position to visual position
            painter.setPen(mDenseDashedPen);
            painter.drawLine(actualPosition, visualPosition);

            // Outer disk
            float outerRadius = mBlurPoints[i]->mSelected ? 36 : 24;
            outerRadius = qMin(outerRadius, outerRadius / mEditModeCamera->zoom());
            painter.setBrush(QColor(128, 128, 128, 128));
            painter.setPen(QColor(0, 0, 0, 0));
            painter.drawEllipse(visualPosition, outerRadius, outerRadius);

            // Inner disk
            float innerRadius = 8;
            innerRadius = qMin(innerRadius, innerRadius / mEditModeCamera->zoom());
            painter.setBrush(QColor(255, 255, 255));
            painter.setPen(QColor(0, 0, 0, 0));
            painter.drawEllipse(visualPosition, innerRadius, innerRadius);
        }

        // Control Points

        for (int j = 0; j < mControlPoints.size(); ++j)
        {
            QPointF center = mEditModeCamera->toGUI(mControlPoints[j]->mPosition);

            // Outer disk
            float outerRadius = mControlPoints[j]->mSelected ? 16 : 12;
            outerRadius = qMin(outerRadius, outerRadius / mEditModeCamera->zoom());
            painter.setBrush(QColor(128, 128, 128, 128));
            painter.setPen(QColor(0, 0, 0, 0));
            painter.drawEllipse(center, outerRadius, outerRadius);

            // Inner disk
            float innerRadius = 6;
            innerRadius = qMin(innerRadius, innerRadius / mEditModeCamera->zoom());
            painter.setBrush(QColor(255, 255, 255));
            painter.setPen(QColor(0, 0, 0, 0));
            painter.drawEllipse(center, innerRadius, innerRadius);
        }

        // Color Points
        for (int i = 0; i < mColorPoints.size(); ++i)
        {
            QPointF visualPosition = mEditModeCamera->toGUI(mColorPoints[i]->getPosition2D(mEditModeCamera->zoom() * COLOR_POINT_VISUAL_GAP));
            QPointF actualPosition = mEditModeCamera->toGUI(mSelectedCurve->valueAt(mColorPoints[i]->mPosition));

            // Draw a dashed line actual position to visual position
            painter.setPen(mDenseDashedPen);
            painter.drawLine(actualPosition, visualPosition);

            // Outer disk
            float outerRadius = mColorPoints[i]->mSelected ? 16 : 12;
            outerRadius = qMin(outerRadius, outerRadius / mEditModeCamera->zoom());
            painter.setBrush(QColor(128, 128, 128, 128));
            painter.setPen(QColor(0, 0, 0, 0));
            painter.drawEllipse(visualPosition, outerRadius, outerRadius);

            // Inner disk
            float innerRadius = 6;
            innerRadius = qMin(innerRadius, innerRadius / mEditModeCamera->zoom());
            painter.setBrush(QColor(255 * mColorPoints[i]->mColor.x(), //
                                    255 * mColorPoints[i]->mColor.y(),
                                    255 * mColorPoints[i]->mColor.z(),
                                    255 * mColorPoints[i]->mColor.w()));
            painter.setPen(QColor(0, 0, 0, 0));
            painter.drawEllipse(visualPosition, innerRadius, innerRadius);
        }
    }
}

void Controller::setWindow(Window *newWindow)
{
    mWindow = newWindow;
}

void Controller::onWheelMoved(QWheelEvent *event)
{
    if (mWorkMode == WorkMode::Edit)
        mEditModeCamera->onWheelMoved(event);
    else if (mWorkMode == WorkMode::Vectorization)
        mViewModeCamera->onWheelMoved(event);
}

void Controller::onMousePressed(QMouseEvent *event)
{
    if (mImGuiWantsMouseCapture)
        return;

    mPressedButton = event->button();

    if (event->button() == Qt::LeftButton)
    {
        if (mWorkMode == WorkMode::Edit)
            onAction((Action) mActionMode, mEditModeCamera->toOpenGL(event->position()));

    } else if (event->button() == Qt::MiddleButton)
    {
        if (mWorkMode == WorkMode::Edit)
            mEditModeCamera->onMousePressed(event);
        else if (mWorkMode == WorkMode::Vectorization)
            mViewModeCamera->onMousePressed(event);
    }
}

void Controller::onMouseReleased(QMouseEvent *event)
{
    mPressedButton = Qt::NoButton;

    mEditModeCamera->onMouseReleased(event);
    mViewModeCamera->onMouseReleased(event);
}

void Controller::onMouseMoved(QMouseEvent *event)
{
    if (mImGuiWantsMouseCapture)
        return;

    if (mPressedButton == Qt::LeftButton)
    {
        if (mWorkMode == WorkMode::Edit && mActionMode == ActionMode::Select)
        {
            if (mSelectedColorPoint)
                onAction(Action::UpdateColorPointPosition, mEditModeCamera->toOpenGL(event->position()));

            if (mSelectedControlPoint)
                onAction(Action::UpdateControlPointPosition, mEditModeCamera->toOpenGL(event->position()));

            if (mSelectedBlurPoint)
                onAction(Action::UpdateBlurPointPosition, mEditModeCamera->toOpenGL(event->position()));
        }
    }

    if (mWorkMode == WorkMode::Edit)
        mEditModeCamera->onMouseMoved(event);
    else if (mWorkMode == WorkMode::Vectorization)
        mViewModeCamera->onMouseMoved(event);
}

void Controller::onKeyPressed(QKeyEvent *event)
{
    if (mImGuiWantsKeyboardCapture)
        return;

    if (mWorkMode == WorkMode::Edit)
    {
        if (event->key() == Qt::Key_Delete)
        {
            if (mSelectedColorPoint)
                onAction(Action::RemoveColorPoint);
            else if (mSelectedControlPoint)
                onAction(Action::RemoveControlPoint);
            else if (mSelectedBlurPoint)
                onAction(Action::RemoveBlurPoint);
            else if (mSelectedCurve)
                onAction(Action::RemoveCurve);
        } else if (event->key() == Qt::Key_Alt)
        {
            if (mSelectedCurve)
                mActionMode = ActionMode::AddColorPoint;
        } else if (event->key() == Qt::Key_Control)
        {
            mActionMode = ActionMode::AddControlPoint;
        }
    }
}

void Controller::onKeyReleased(QKeyEvent *)
{
    if (mImGuiWantsKeyboardCapture)
        return;
}

void Controller::resize(int w, int h)
{
    mWindow->makeCurrent();
    mWidth = w;
    mHeight = h;
    mEditModeCamera->resize(w, h);
    mViewModeCamera->resize(w, h);
    mRendererManager->resize(w, h);
    mBitmapRenderer->resize(w, h);
    mWindow->doneCurrent();
}
