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
    void onVectorize();
    void updateEdges();

    void traceEdgePixels(ProgressStatus &progressStatus, QVector<PixelChain> &chains, cv::Mat edges, int lengthThreshold);
    void potrace(QVector<Point> &polyline, PixelChain chain);
    void findBestPath(QVector<Point> &bestPath, PixelChain chain, Eigen::MatrixXd penalties);
    void constructCurves(ProgressStatus &progressStatus, QVector<Bezier *> &curves, const QVector<QVector<Point>> &polylines);
    Bezier *constructCurve(const QVector<Point> &polyline, double tension = 2.0);
    void sampleColors(Bezier *curve, cv::Mat &image, cv::Mat &imageLab, double sampleDensity);

signals:
    void vectorize();

private:
    BitmapRenderer *mBitmapRenderer;
    CurveManager *mCurveManager;

    float mCannyUpperThreshold;
    float mCannyLowerThreshold;

    // Updated when an image is loaded
    cv::Mat mOriginalImage;
    cv::Mat mBlurredImage;
    cv::Mat mEdgeImage;
    cv::Mat mLabImage;

    GaussianStack *mGaussianStack;
    EdgeStack *mEdgeStack;
    QList<PixelChain> mChains;
    QList<QVector<Point>> mPolylines;

    SubWorkMode mSubWorkMode;
    VectorizationStatus mVectorizationStatus;

    // GUI Stuff
    int mSelectedGaussianLayer;
    int mSelectedEdgeLayer;
    bool mInit;
    bool mUpdateInitialData;
    ProgressStatus mProgressStatus;
};

#endif // VECTORIZER_H
