// This code taken from https://github.com/zhuethanca/DiffusionCurves
#ifndef EDGESTACK_H
#define EDGESTACK_H

#include "Common.h"
#include "GaussianStack.h"

#include <QVector>

#include <opencv2/core/mat.hpp>

class EdgeStack
{
private:
    /*
         * A vector of edge images of increasingly Gaussian-blurred images.
         */
    std::vector<cv::Mat> levels;

public:
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
    EdgeStack(ProgressStatus &progressStatus, GaussianStack *stack, double lowThreshold, double highThreshold);

    /*
         * Returns the number of levels in the edge stack.
         */
    int height();

    /*
         * Returns the image of edges at the <layer>'th layer.
         */
    cv::Mat layer(int layer);
};
#endif // EDGESTACK_H
