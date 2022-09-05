// This code taken from https://github.com/zhuethanca/DiffusionCurves and adopted.
#include "GaussianStack.h"

GaussianStack::GaussianStack()
    : mProgress(0.0f)
{}

/**
 * Construct a Gaussian scale space representing the image passed in.
 *
 * Repeatedly applies a Gaussian filter with radius sigma starting from 0
 * and increasing by <sigmaStep> (default 0.4) each level. The stack stops
 * when the image's standard deviation drops below <stdDevCutoff> (default
 * 40) or, if a max height is specified, at that height.
 *
 * param image: An OpenCV matrix containing an RGB image.
 * param stdDevCutoff: The minimum standard deviation of a blurred image
 *                     that will be used in the stack.
 * param maxHeight: The maximum height of the stack.
 * param sigmaStep: The increase in Gaussian filter widths between each level.
 */
void GaussianStack::run(cv::Mat image, double stdDevCutoff, int maxHeight, double sigmaStep)
{
    this->mLevels.clear();

    double sigma = 0.4;
    //    double stdDev;

    int remainingHeight = maxHeight;

    do
    {
        this->mProgress = float(maxHeight - remainingHeight) / float(maxHeight);

        // Define parameters for the next level of Gaussian filter.
        const int radius = std::ceil(2 * sigma);
        const int width = 2 * radius + 1;

        cv::Mat blurred;
        cv::GaussianBlur(image, blurred, cv::Size(width, width), sigma, sigma);

        // Compute the image's standard deviation, averaged over three channels.
        //        cv::Scalar mean, stdDevChannels;
        //        cv::meanStdDev(blurred, mean, stdDevChannels);
        //        stdDev = (stdDevChannels[0] + stdDevChannels[1] + stdDevChannels[2]) / 3.0;

        sigma += 0.4;

        this->mLevels.push_back(blurred);

        remainingHeight -= 1;

    } while (remainingHeight != 0);
}

/*
 * Returns the number of levels in the scale space.
 */
int GaussianStack::height()
{
    return this->mLevels.size();
}

/*
 * Restricts the height of the stack to <layers> layers, removing any
 * images above that level.
 */
void GaussianStack::restrict(int layers)
{
    this->mLevels.resize(layers);
}

/*
 * Returns the blurred RGB image at the <layer>'th layer.
 */
cv::Mat GaussianStack::layer(int layer)
{
    return this->mLevels.at(layer);
}

float GaussianStack::progress() const
{
    return this->mProgress;
}
