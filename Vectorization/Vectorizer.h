#ifndef VECTORIZER_H
#define VECTORIZER_H

#include "Bezier.h"
#include "BitmapRenderer.h"
#include "CurveManager.h"
#include "EdgeStack.h"
#include "GaussianStack.h"
#include "PixelChain.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <QMap>
#include <QObject>
#include <QString>
#include <QThread>

class Vectorizer : public QObject
{
    Q_OBJECT
private:
    explicit Vectorizer(QObject *parent = nullptr);

public:
    static Vectorizer *instance();

public slots:
    void load(QString path);
    void draw();

private:
    void clear();
    void traceEdgePixels(QVector<PixelChain> &chains, cv::Mat edges, int lengthThreshold);
    void potrace(QVector<Point> &polyline, PixelChain chain);
    void findBestPath(QVector<Point> &bestPath, PixelChain chain, Eigen::MatrixXd penalties);
    void constructCurves(QVector<Bezier *> &curves, const QVector<QVector<Point>> &polylines);
    Bezier *constructCurve(const QVector<Point> &polyline, double tension = 2.0);

private:
    BitmapRenderer *mBitmapRenderer;
    CurveManager *mCurveManager;

    double mCannyUpperThreshold;
    double mCannyLowerThreshold;

    // Updated when an image is loaded
    cv::Mat mOriginalImage;
    cv::Mat mEdgeImage;
    GaussianStack *mGaussianStack;
    EdgeStack *mEdgeStack;
    QVector<PixelChain> mChains;
    QVector<QVector<Point>> mPolylines;

    SubWorkMode mSubWorkMode;
    VectorizationStatus mVectorizationStatus;

    // GUI Stuff
    int mSelectedGaussianLayer;
    int mSelectedEdgeLayer;
    bool mInit;
    bool mUpdateInitialData;
    float mProgress;
};

#endif // VECTORIZER_H
