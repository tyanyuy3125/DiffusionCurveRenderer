#ifndef VECTORIZER_H
#define VECTORIZER_H

#include "CurveManager.h"
#include "EdgeStack.h"
#include "EdgeTracer.h"
#include "GaussianStack.h"
#include "Potrace.h"
#include "Renderers/BitmapRenderer.h"
#include "Vectorization/CurveConstructor.h"

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

signals:
    void vectorize();

private:
    BitmapRenderer *mBitmapRenderer;
    CurveManager *mCurveManager;

    GaussianStack *mGaussianStack;
    EdgeStack *mEdgeStack;
    EdgeTracer *mEdgeTracer;
    Potrace *mPotrace;
    CurveConstructor *mCurveConstructor;

    float mCannyUpperThreshold;
    float mCannyLowerThreshold;

    cv::Mat mOriginalImage;
    cv::Mat mEdgeImage;
    cv::Mat mLabImage;

    SubWorkMode mSubWorkMode;
    VectorizationStatus mVectorizationStatus;

    // GUI Stuff
    int mSelectedGaussianLayer;
    int mSelectedEdgeLayer;
    bool mInit;
    bool mUpdateInitialData;
};

#endif // VECTORIZER_H
