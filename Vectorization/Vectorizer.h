#ifndef VECTORIZER_H
#define VECTORIZER_H

#include "BitmapRenderer.h"
#include "EdgeStack.h"
#include "GaussianStack.h"
#include "PixelChain.h"
#include "Window.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <QMap>
#include <QString>

class Vectorizer
{
private:
    Vectorizer();

public:
    static Vectorizer *instance();

    void load(const QString &path);
    void drawGui();

private:
    void clear();
    void traceEdgePixels(QVector<PixelChain> &chains, cv::Mat edges, int lengthThreshold);
    void potrace(QVector<Point> &polyline, PixelChain chain);
    void findBestPath(QVector<Point> &bestPath, PixelChain chain, Eigen::MatrixXd penalties);

private:
    BitmapRenderer *mBitmapRenderer;
    SubWorkMode mSubWorkMode;
    VectorizationStatus mVectorizationStatus;

    cv::Mat mOriginalImage;
    cv::Mat mEdgeImage;
    double mCannyUpperThreshold;
    double mCannyLowerThreshold;

    GaussianStack *mGaussianStack;
    EdgeStack *mEdgeStack;
    QVector<PixelChain> mChains;

    // GUI Stuff
    int mSelectedGaussianLayer;
    int mSelectedEdgeLayer;
    bool mInit;
    bool mUpdateInitialData;
};

#endif // VECTORIZER_H
