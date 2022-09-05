// This code taken from https://github.com/zhuethanca/DiffusionCurves
#include "EdgeStack.h"
#include "GaussianStack.h"

#include "opencv2/imgproc/imgproc.hpp"

EdgeStack::EdgeStack()
    : mProgress(0.0f)
{}

/**
 * Creates a stack of edge images from a Gaussian scale space.
 *
 * Runs Canny edge detection on each image in the Gaussian stack.
 * Edge detection uses low and high thresholds as specified by
 * parameters.
 *
 * param stack: A stack of images from a Gaussian scale space.
 * param lowThreshold: Low edge strength threshold for Canny edges.
 * param highThreshold: High edge strength threshold for Canny edges.
 */
void EdgeStack::run(GaussianStack *stack, double lowThreshold, double highThreshold)
{
    int stackHeight = stack->height();

    // Meaningless, nonzero initial value for number of detected edge pixels.
    int nonzeros = 1;

    for (int layer = 0; layer < stackHeight && nonzeros > 0; layer++)
    {
        this->mProgress = float(layer) / float(stackHeight - 1);

        cv::Mat image = stack->layer(layer);

        cv::Mat edges;
        cv::Canny(image, edges, lowThreshold, highThreshold);

        this->mLevels.push_back(edges);

        nonzeros = cv::countNonZero(edges);
    }

    if (nonzeros == 0)
    {
        // Remove the top level if it had no detectable edge pixels.
        this->mLevels.pop_back();
    }

    // Trim any blurred images from the Gaussian stack beyond the point
    // where edges stopped being detectable.
    int edgeHeight = this->height();
    if (edgeHeight < stackHeight)
    {
        stack->restrict(edgeHeight);
    }
}

/*
 * Returns the number of levels in the edge stack.
 */
int EdgeStack::height()
{
    return this->mLevels.size();
}

/*
 * Returns the image of edges at the <layer>'th layer.
 */
cv::Mat EdgeStack::layer(int layer)
{
    return this->mLevels.at(layer);
}

float EdgeStack::progress() const
{
    return this->mProgress;
}
