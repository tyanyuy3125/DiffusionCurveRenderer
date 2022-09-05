#ifndef EDGETRACER_H
#define EDGETRACER_H

#include "PixelChain.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

#include <QList>

class EdgeTracer
{
public:
    EdgeTracer();

    /*
     * This code taken from https://github.com/zhuethanca/DiffusionCurves
     *
     * Returns a series of distinct chains of pixels from the edge images. Each
     * chain represents a whole edge, parametrized into a 1D representation.
     *
     * Edges of length less than <lengthThreshold> are discarded.
     *
     * param chains: Output vector of edge pixel chains.
     * param edges: Black-and-white image, where edges are identified by white pixels.
     * param lengthThreshold: Minimum length required for an edge to be returned.
     */
    void run(cv::Mat edges, int lengthThreshold);

    float progress() const;

    const QList<PixelChain> &chains() const;

private:
    float mProgress; // [0,..., 1]
    QList<PixelChain> mChains;
};

#endif // EDGETRACER_H
