#include "ImGuiWindow.h"

#include "Curve/CurveContainer.h"
#include "Renderer/RendererManager.h"
#include "Util/Chronometer.h"
#include "Util/Logger.h"

#include <QFileDialog>
#include <QtImGui.h>
#include <cmath>
#include <imgui.h>

DiffusionCurveRenderer::ImGuiWindow::ImGuiWindow(QObject* parent)
    : QObject(parent)
{
}

void DiffusionCurveRenderer::ImGuiWindow::Draw()
{
    mGlobalContourThickness = mCurveContainer->GetGlobalContourThickness();
    mGlobalDiffusionWidth = mCurveContainer->GetGlobalDiffusionWidth();
    mGlobalDiffusionGap = mCurveContainer->GetGlobalDiffusionGap();
    mGlobalBlurStrength = mCurveContainer->GetGlobalBlurStrength();
    mSmoothIterations = mRendererManager->GetSmoothIterations();
    mFrambufferSize = mRendererManager->GetFramebufferSize();
    mFrambufferSizeIndex = std::log2(mFrambufferSize / 512);

    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_MenuBar);
    DrawMenuBar();
    DrawWorkModes();
    if (mWorkMode == WorkMode::Vectorization)
    {
        DrawVectorizationOptions();
    }
    else
    {
        DrawCurveEditingSettings();
    }
    ImGui::End();
}

void DiffusionCurveRenderer::ImGuiWindow::DrawWorkModes()
{
    if (ImGui::CollapsingHeader("Work Modes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        int mode = (int) mWorkMode;
        // ImGui::BeginDisabled(!mImageLoaded);
        ImGui::RadioButton("Vectorization##WorkMode", &mode, 0);
        // ImGui::EndDisabled();
        ImGui::RadioButton("Curve Editing##WorkMode", &mode, 1);
        SetWorkMode(WorkMode(mode));
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawVectorizationOptions()
{
    if (mVectorizationState == VectorizationState::Ready || mVectorizationState == VectorizationState::Finished)
    {
        if (ImGui::CollapsingHeader("Vectorization Options", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int option = static_cast<int>(mVectorizationOption);

            ImGui::RadioButton("View Original Image", &option, 0);
            ImGui::RadioButton("View Edges", &option, 1);
            ImGui::RadioButton("View Gaussian Stack", &option, 2);
            ImGui::RadioButton("Choose Edge Stack Level", &option, 3);

            SetVectorizationOption(VectorizationOption(option));

            if (mVectorizationOption == VectorizationOption::ViewGaussianStack)
            {
                ImGui::Text("Gaussian Stack Layers");
                if (ImGui::SliderInt("Layer##Gaussian", &mGaussianStackLayer, 0, mMaximumGaussianStackLayer))
                {
                    emit GaussianStackLayerChanged(mGaussianStackLayer);
                }
            }

            if (mVectorizationOption == VectorizationOption::ChooseEdgeStackLevel)
            {
                ImGui::Text("Edge Stack Layers");
                if (ImGui::SliderInt("Layer##Edge", &mEdgeStackLayer, 0, mMaximumEdgeStackLayer))
                {
                    emit EdgeStackLayerChanged(mEdgeStackLayer);
                }

                if (ImGui::Button("Vectorize"))
                {
                    emit Vectorize(mEdgeStackLayer);
                }
            }
        }
    }
    else
    {
        DrawVectorizationProgressBar();
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawVectorizationProgressBar()
{
    if (mVectorizationState == VectorizationState::CreatingGaussianStack)
    {
        ImGui::Text("Status: Creating Gaussian Stack...");
    }
    else if (mVectorizationState == VectorizationState::CreatingEdgeStack)
    {
        ImGui::Text("Status: Creating Edge Stack...");
    }
    else if (mVectorizationState == VectorizationState::TracingEdges)
    {
        ImGui::Text("Status: Tracing Edges...");
    }
    else if (mVectorizationState == VectorizationState::CreatingPolylines)
    {
        ImGui::Text("Status: Creating Polylines...");
    }
    else if (mVectorizationState == VectorizationState::ConstructingCurves)
    {
        ImGui::Text("Status: Constructing Curves...");
    }

    ImGui::ProgressBar(mVectorizationProgress);
}

void DiffusionCurveRenderer::ImGuiWindow::DrawCurveEditingSettings()
{
    DrawHintTexts();
    DrawRenderMode();
    DrawCurveHeader();
    DrawRenderSettings();
    DrawStats();
}

void DiffusionCurveRenderer::ImGuiWindow::DrawMenuBar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Load an image for vectorization"))
            {
                QString path = QFileDialog::getOpenFileName(nullptr, "Select an image", "", "*.png *.jpg *.jpeg *.bmp");

                if (path.isNull() == false)
                {
                    qDebug() << "ImGuiWindow::DrawMenuBar: Path is" << path;
                    emit LoadImage(path);
                }
            }

            ImGui::Separator();

            ImGui::MenuItem("Import XML");
            ImGui::MenuItem("Import JSON");

            ImGui::Separator();

            ImGui::MenuItem("Save as PNG");
            ImGui::MenuItem("Export as JSON");

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawHintTexts()
{
    if (ImGui::CollapsingHeader("Actions", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Selection         : Left Click");
        ImGui::Text("Add Control Point : Right Click");
        ImGui::Text("Add Color Point   : CTRL + Right Click");
        ImGui::Text("Move              : Middle button");
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawRenderMode()
{
    if (ImGui::CollapsingHeader("Render Modes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        mRenderModeContour = mRenderModes.testAnyFlag(RenderMode::Contour);
        mRenderModeDiffusion = mRenderModes.testAnyFlag(RenderMode::Diffusion);

        ImGui::Checkbox("Contour", &mRenderModeContour);
        ImGui::Checkbox("Diffusion", &mRenderModeDiffusion);

        SetRenderMode(RenderMode::Contour, mRenderModeContour);
        SetRenderMode(RenderMode::Diffusion, mRenderModeDiffusion);
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawCurveHeader()
{
    ImGui::BeginDisabled(!mSelectedCurve);
    if (ImGui::CollapsingHeader("Curve", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (mSelectedCurve)
        {
            if (std::dynamic_pointer_cast<Bezier>(mSelectedCurve))
            {
                ImGui::Text("Curve Type: Bezier");
            }
            else
            {
                ImGui::Text("Curve Type: B-Spline");
            }

            ImGui::Text("Number of Control Points: %d", (int) mSelectedCurve->GetControlPoints().size());
            ImGui::SliderFloat("Thickness", &mSelectedCurve->GetContourThickness_NonConst(), 1, 20);
            ImGui::SliderFloat("Diffusion Width", &mSelectedCurve->GetDiffusionWidth_NonConst(), 1, 10);
            ImGui::SliderFloat("Diffusion Gap", &mSelectedCurve->GetDiffusionGap_NonConst(), 1, 10);
            ImGui::ColorEdit4("Contour Color", &mSelectedCurve->GetContourColor_NonConst()[0]);

            if (ImGui::Button("Remove Curve"))
            {
                mCurveContainer->RemoveCurve(mSelectedCurve);
                SetSelectedCurve(nullptr);
            }
        }

        ImGui::Spacing();

        if (mSelectedControlPoint)
        {
            ImGui::Text("Control Point");
            if (ImGui::InputFloat2("Position (x,y)", &mSelectedControlPoint->position[0]))
                mSelectedCurve->Update();

            if (ImGui::Button("Remove Control Point"))
            {
                mSelectedCurve->RemoveControlPoint(mSelectedControlPoint);
                SetSelectedControlPoint(nullptr);
            }
        }

        ImGui::Spacing();

        if (mSelectedColorPoint)
        {
            ImGui::Text("Color Point");

            ImGui::Text("Direction: %s", mSelectedColorPoint->type == ColorPointType::Left ? "Left" : "Right");
            ImGui::SliderFloat("Position", &mSelectedColorPoint->position, 0.0f, 1.0f);

            if (ImGui::ColorEdit4("Color", &mSelectedColorPoint->color[0]))
            {
                mSelectedCurve->Update();
            }

            if (ImGui::Button("Remove Color Point"))
            {
                mSelectedCurve->RemoveColorPoint(mSelectedColorPoint);
                SetSelectedColorPoint(nullptr);
            }
        }
    }
    ImGui::EndDisabled();
}

void DiffusionCurveRenderer::ImGuiWindow::DrawRenderSettings()
{
    if (ImGui::CollapsingHeader("Render Settings"))
    {
        if (ImGui::SliderInt("Smooth Iterations", &mSmoothIterations, 2, 50))
            mRendererManager->SetSmoothIterations(mSmoothIterations);

        if (ImGui::SliderInt("Frambuffer Size", &mFrambufferSizeIndex, 0, 3, FRAME_BUFFER_SIZES[mFrambufferSizeIndex]))
        {
            mFrambufferSize = 512 * std::exp2(mFrambufferSizeIndex);
            mRendererManager->SetFramebufferSize(mFrambufferSize);
        }

        if (ImGui::SliderFloat("Global Blur Strength", &mGlobalBlurStrength, 0.0f, 1.0f))
            mCurveContainer->SetGlobalBlurStrength(mGlobalBlurStrength);

        if (ImGui::SliderFloat("Global Thickness", &mGlobalContourThickness, 1.0f, 20.0f))
            mCurveContainer->SetGlobalContourThickness(mGlobalContourThickness);

        if (ImGui::SliderFloat("Global Diffusion Width", &mGlobalDiffusionWidth, 1.0f, 10.0f))
            mCurveContainer->SetGlobalDiffusionWidth(mGlobalDiffusionWidth);

        if (ImGui::SliderFloat("Global Diffusion Gap", &mGlobalDiffusionGap, 1.0f, 10.))
            mCurveContainer->SetGlobalDiffusionGap(mGlobalDiffusionGap);
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawStats()
{
    if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (const auto& ID : ALL_CHORONOMETER_IDs)
            ImGui::Text(Chronometer::Print(ID).c_str());

        ImGui::Text("# of curves: %zu", mCurveContainer->GetTotalNumberOfCurves());
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }
}

void DiffusionCurveRenderer::ImGuiWindow::SetSelectedCurve(CurvePtr selectedCurve)
{
    if (mSelectedCurve == selectedCurve)
        return;

    mSelectedCurve = selectedCurve;

    SetSelectedControlPoint(nullptr);
    SetSelectedColorPoint(nullptr);

    emit SelectedCurveChanged(selectedCurve);
}

void DiffusionCurveRenderer::ImGuiWindow::SetSelectedControlPoint(ControlPointPtr point)
{
    if (mSelectedControlPoint == point)
        return;

    mSelectedControlPoint = point;
    emit SelectedControlPointChanged(mSelectedControlPoint);
}

void DiffusionCurveRenderer::ImGuiWindow::SetSelectedColorPoint(ColorPointPtr point)
{
    if (mSelectedColorPoint == point)
        return;

    mSelectedColorPoint = point;
    emit SelectedColorPointChanged(mSelectedColorPoint);
}

void DiffusionCurveRenderer::ImGuiWindow::SetRenderMode(RenderMode mode, bool on)
{
    mRenderModes.setFlag(mode, on);
    emit RenderModesChanged(mRenderModes);
}

void DiffusionCurveRenderer::ImGuiWindow::SetWorkMode(WorkMode workMode)
{
    if (mWorkMode == workMode)
        return;

    mWorkMode = workMode;
    emit WorkModeChanged(mWorkMode);
}

void DiffusionCurveRenderer::ImGuiWindow::SetVectorizationState(VectorizationState state)
{
    mVectorizationState = state;
}

void DiffusionCurveRenderer::ImGuiWindow::SetVectorizationOption(VectorizationOption option)
{
    if (mVectorizationOption == option)
        return;

    mVectorizationOption = option;

    emit VectorizationOptionChanged(mVectorizationOption);
}

void DiffusionCurveRenderer::ImGuiWindow::SetGaussianStackLayer(int layer)
{
    if (mGaussianStackLayer == layer)
        return;

    mGaussianStackLayer = layer;
    emit GaussianStackLayerChanged(mGaussianStackLayer);
}