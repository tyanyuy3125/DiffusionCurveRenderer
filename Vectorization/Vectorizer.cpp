#include "Vectorizer.h"
#include "imgui.h"

#include <Dependencies/Eigen/SparseCore>

#include <QDebug>

Vectorizer::Vectorizer(QObject *parent)
    : QObject(parent)
    , mCannyUpperThreshold(200.0f)
    , mCannyLowerThreshold(20.0f)
    , mGaussianStack(nullptr)
    , mEdgeStack(nullptr)
    , mSubWorkMode(SubWorkMode::ViewOriginalImage)
    , mVectorizationStatus(VectorizationStatus::Ready)
    , mSelectedGaussianLayer(0)
    , mSelectedEdgeLayer(0)
    , mInit(false)
    , mUpdateInitialData(false)

{
    mBitmapRenderer = BitmapRenderer::instance();
    mCurveManager = CurveManager::instance();

    // onVectorize runs in a seperate thread
    connect(this, &Vectorizer::vectorize, this, &Vectorizer::onVectorize, Qt::QueuedConnection);
}

void Vectorizer::load(QString path)
{
    if (mGaussianStack)
        delete mGaussianStack;

    if (mEdgeStack)
        delete mEdgeStack;

    mSelectedGaussianLayer = 0;
    mSelectedEdgeLayer = 0;
    mProgressStatus.progress = 0.0f;

    mOriginalImage = cv::imread(path.toStdString(), cv::IMREAD_COLOR);
    mBlurredImage = mOriginalImage;

    cv::Canny(mOriginalImage, mEdgeImage, mCannyUpperThreshold, mCannyLowerThreshold);

    mProgressStatus.progress = 0.0f;
    mVectorizationStatus = VectorizationStatus::CreatingGaussianStack;
    mProgressStatus.start = 0.0f;
    mProgressStatus.end = 0.5f;
    mGaussianStack = new GaussianStack(mProgressStatus, mOriginalImage);

    mVectorizationStatus = VectorizationStatus::CreatingEdgeStack;
    mProgressStatus.start = 0.5f;
    mProgressStatus.end = 1.0f;
    mEdgeStack = new EdgeStack(mProgressStatus, mGaussianStack, mCannyLowerThreshold, mCannyUpperThreshold);

    mProgressStatus.progress = 1.0f;
    mSubWorkMode = SubWorkMode::ViewOriginalImage;
    mVectorizationStatus = VectorizationStatus::Ready;
    mInit = true;
    mUpdateInitialData = true;
}

void Vectorizer::onVectorize()
{
    mChains.clear();
    mPolylines.clear();

    mVectorizationStatus = VectorizationStatus::TracingEdges;
    mProgressStatus.start = 0.0f;
    mProgressStatus.end = 0.45f;
    traceEdgePixels(mProgressStatus, mChains, mEdgeStack->layer(mSelectedEdgeLayer), 10);

    qInfo() << "Chains detected."
            << "Number of chains is:" << mChains.size();

    // Create polylines
    mVectorizationStatus = VectorizationStatus::CreatingPolylines;
    mProgressStatus.start = 0.45f;
    mProgressStatus.end = 0.9f;
    for (int i = 0; i < mChains.size(); i++)
    {
        mProgressStatus.progress = mProgressStatus.start + (mProgressStatus.end - mProgressStatus.start) * float(i) / mChains.size();

        PixelChain chain = mChains[i];
        QList<Point> polyline;
        potrace(polyline, chain);
        mPolylines << polyline;
    }

    // Now construct curves using polylines
    mVectorizationStatus = VectorizationStatus::ConstructingCurves;
    mProgressStatus.start = 0.9f;
    mProgressStatus.end = 1.0f;
    QVector<Bezier *> curves;
    constructCurves(mProgressStatus, curves, mPolylines);

    // Set new curves
    mCurveManager->clear();
    mCurveManager->addCurves(curves);

    mProgressStatus.progress = 1.0f;
    mSubWorkMode = SubWorkMode::ViewOriginalImage;
    mVectorizationStatus = VectorizationStatus::Finished;
    mUpdateInitialData = true;
}

void Vectorizer::draw()
{
    ImGui::Spacing();

    if (mVectorizationStatus == VectorizationStatus::CreatingGaussianStack)
    {
        ImGui::Text("Status: Creating Gaussian Stack...");
        ImGui::ProgressBar(mProgressStatus.progress);
        return;

    } else if (mVectorizationStatus == VectorizationStatus::CreatingEdgeStack)
    {
        ImGui::Text("Status: Creating Edge Stack...");
        ImGui::ProgressBar(mProgressStatus.progress);
        return;

    } else if (mVectorizationStatus == VectorizationStatus::TracingEdges)
    {
        ImGui::Text("Status: Tracing Edges...");
        ImGui::ProgressBar(mProgressStatus.progress);
        return;
    } else if (mVectorizationStatus == VectorizationStatus::CreatingPolylines)
    {
        ImGui::Text("Status: Creating Polylines...");
        ImGui::ProgressBar(mProgressStatus.progress);
        return;
    } else if (mVectorizationStatus == VectorizationStatus::ConstructingCurves)
    {
        ImGui::Text("Status: Constructing Curves...");
        ImGui::ProgressBar(mProgressStatus.progress);
        return;
    }

    if (!mInit)
        return;

    if (mUpdateInitialData)
    {
        mBitmapRenderer->setData(mOriginalImage, GL_BGR);
        mUpdateInitialData = false;
    }

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sub Work Modes");

    int mode = (int) mSubWorkMode;

    if (ImGui::RadioButton("View Original Image##SubWorkMode", &mode, 0))
        mBitmapRenderer->setData(mOriginalImage, GL_BGR);

    if (ImGui::RadioButton("View Edges##SubWorkMode", &mode, 1))
        mBitmapRenderer->setData(mEdgeImage, GL_RED);

    if (ImGui::RadioButton("View Gaussian Stack##SubWorkMode", &mode, 2))
        mBitmapRenderer->setData(mGaussianStack->layer(mSelectedGaussianLayer), GL_BGR);

    if (ImGui::RadioButton("Choose Edge Stack Level##SubWorkMode", &mode, 3))
        mBitmapRenderer->setData(mEdgeStack->layer(mSelectedEdgeLayer), GL_RED);

    if (mSubWorkMode == SubWorkMode::ViewGaussianStack)
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Gaussian Stack Layers");

        if (ImGui::SliderInt("Layer##Gaussian", &mSelectedGaussianLayer, 0, mGaussianStack->height() - 1))
            mBitmapRenderer->setData(mGaussianStack->layer(mSelectedGaussianLayer), GL_BGR);
    }

    if (mSubWorkMode == SubWorkMode::ChooseEdgeStackLevel)
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Edge Stack Layers");

        if (ImGui::SliderInt("Layer##Edge", &mSelectedEdgeLayer, 0, mEdgeStack->height() - 1))
            mBitmapRenderer->setData(mEdgeStack->layer(mSelectedEdgeLayer), GL_RED);

        if (ImGui::Button("Vectorize##Button"))
            emit vectorize();
    }

    mSubWorkMode = SubWorkMode(mode);
}

Vectorizer *Vectorizer::instance()
{
    static Vectorizer instance;
    return &instance;
}

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
void Vectorizer::traceEdgePixels(ProgressStatus &progressStatus, QVector<PixelChain> &chains, cv::Mat edges, int lengthThreshold)
{
    const int width = edges.cols;
    const int height = edges.rows;
    const int nEdgePixels = cv::countNonZero(edges);

    // Matrix denoting which edges have already been included in a pixel chain.
    Eigen::SparseMatrix<int> visited(width, height);

    // Matrix giving fast access to edge pixels.
    cv::Mat nonZeros;
    cv::findNonZero(edges, nonZeros);

    std::vector<PixelChain> growingChains;

    for (int i = 0; i < nEdgePixels; i++)
    {
        progressStatus.progress = progressStatus.start + (progressStatus.end - progressStatus.start) * float(i) / nEdgePixels;

        int row = nonZeros.at<cv::Point>(i).y;
        int col = nonZeros.at<cv::Point>(i).x;

        if (visited.coeffRef(col, row) != 0)
        {
            // Skip pixels that have already been used in an edge, to avoid
            // double-counting.
            continue;
        }

        PixelChain points;
        bool neighborFound = false;

        // Search through neighbours of this pixel until the end of the chain
        // has been reached.
        do
        {
            points.append(Point(col, row));
            visited.coeffRef(col, row) = 1;

            // The neighbourhood consists of pixels within one space of the
            // current pixel.
            const int startCol = col == 0 ? col : col - 1;
            const int endCol = col == width - 1 ? col : col + 1;
            const int startRow = row == 0 ? row : row - 1;
            const int endRow = row == height - 1 ? row : row + 1;

            neighborFound = false;

            // Search for unused pixels in the neighbourhood (adjacent) to the
            // current pixel until a first one is found.
            for (int x = startCol; x <= endCol && !neighborFound; x++)
            {
                for (int y = startRow; y <= endRow && !neighborFound; y++)
                {
                    uchar pixelValue = edges.at<uchar>(y, x);

                    if (pixelValue != 0 && visited.coeffRef(x, y) == 0.0)
                    {
                        // Note down the new pixel and stop looking.
                        row = y;
                        col = x;
                        neighborFound = true;
                    }
                }
            }
        } while (neighborFound);

        bool newChain = true;

        // Look for any existing chains that connect to this one, and merge
        // them into one longer chain.
        for (int j = 0; j < growingChains.size(); j++)
        {
            Point chainHead = growingChains.at(j).head();
            Point chainTail = growingChains.at(j).tail();

            Point newcomerHead = points.head();
            Point newcomerTail = points.tail();

            if (chainTail.isNeighbour(newcomerHead))
            {
                // Insert the new chain at the end of the old chain.
                growingChains.at(j).insertBack(points);
                newChain = false;
            } else if (chainTail.isNeighbour(newcomerTail))
            {
                // Insert the new chain backwards at the end of the old chain.
                growingChains.at(j).insertBack(points.reversed());
                newChain = false;
            } else if (chainHead.isNeighbour(newcomerTail))
            {
                // Insert the new chain at the front of the old one.
                growingChains.at(j).insertFront(points);
                newChain = false;
            } else if (chainHead.isNeighbour(newcomerHead))
            {
                // Insert the new chain backwards at the front of the old one.
                growingChains.at(j).insertFront(points.reversed());
                newChain = false;
            }
        }

        if (newChain)
        {
            growingChains.push_back(points);
        }
    }

    // Only keep chains that are at least as long as the threshold.
    for (int i = 0; i < growingChains.size(); i++)
    {
        PixelChain candidate = growingChains.at(i);

        if (candidate.length() >= lengthThreshold)
        {
            chains.push_back(candidate);
        }
    }
}

/*
 * This code taken from https://github.com/zhuethanca/DiffusionCurves
 *
 * Runs a modified Potrace (polygon trace) algorithm to transform a continuous
 * pixel chain into a polyline with minimal segments while still respecting the
 * shape of the pixel chain.
 *
 * For each pixel, the algorithm attempts to construct a straight line to every
 * other pixel. Each attempt has a penalty based on the standard deviation of
 * pixels between the start and end from the line segment. Lines with too high
 * a penalty are discarded. Short polylines are prioritized over low error
 * among valid paths.
 *
 * param polyline: Return value which will hold the points in the polyline.
 * param chain: Continuous chain of pixels to be approximated by a polyline.
 */
void Vectorizer::potrace(QVector<Point> &polyline, PixelChain chain)
{
    const int nPoints = chain.length();

    // Each entry i, j represents the penalty of a straight line
    // from pixel i directly to pixel j.
    Eigen::MatrixXd penalties(nPoints, nPoints);
    // Set a recognizable dummy value to signify no penalty is set.
    penalties.setConstant(-1.0);

    // First, find a penalty between each pair of pixels, or leave the dummy
    // value if two pixels cannot be connected by a straight line.

    // Iterate over all triplets i, j, k, such that 0 <= i < j < k <= nPoints.
    for (int i = 0; i < nPoints; i++)
    {
        for (int k = i + 1; k < nPoints; k++)
        {
            bool isStraightPath = true;

            Eigen::Vector2f pointI = chain.get(i).toVector();
            Eigen::Vector2f pointK = chain.get(k).toVector();

            // Check that all points between I and K are close enough to the
            // straight line connecting them.
            for (int j = i + 1; j < k; j++)
            {
                Eigen::Vector2f pointJ = chain.get(j).toVector();

                Eigen::Vector2f lineItoK = pointK - pointI;
                Eigen::Vector2f lineItoJ = pointJ - pointI;

                double coefficient = lineItoJ.dot(lineItoK) / lineItoK.squaredNorm();
                Eigen::Vector2f dispJFromLine = lineItoJ - coefficient * lineItoK;

                // Discard the line if any point J is further than one unit
                // off the line.
                if (dispJFromLine.norm() >= 1.0)
                {
                    isStraightPath = false;
                    break;
                }
            }

            if (isStraightPath)
            {
                // Now that this line is known to be valid, compute the penalty
                // for this path segment.

                // Use an approximation of penalty from the Potrace paper.
                const double x = pointK.x() - pointI.x();
                const double y = pointK.y() - pointI.y();

                const double xBar = (pointK + pointI).x() / 2.0;
                const double yBar = (pointK + pointI).y() / 2.0;

                // Compute expected values of all the terms below.
                double expectedX = 0.0;
                double expectedY = 0.0;
                double expectedXY = 0.0;
                double expectedXSquare = 0.0;
                double expectedYSquare = 0.0;

                for (int j = i; j <= k; j++)
                {
                    const int xAtj = chain.get(j).x;
                    const int yAtj = chain.get(j).y;

                    expectedX += xAtj;
                    expectedY += yAtj;
                    expectedXY += xAtj * yAtj;
                    expectedXSquare += xAtj * xAtj;
                    expectedYSquare += yAtj * yAtj;
                }

                expectedX /= (k - i + 1);
                expectedY /= (k - i + 1);
                expectedXY /= (k - i + 1);
                expectedXSquare /= (k - i + 1);
                expectedYSquare /= (k - i + 1);

                // Evaluate the penalty approximation from the Potrace paper.
                const double a = expectedXSquare - 2 * xBar * expectedX + pow(xBar, 2.0);
                const double b = expectedXY - xBar * expectedX - yBar * expectedY + xBar * yBar;
                const double c = expectedYSquare - 2 * yBar * expectedY + pow(yBar, 2.0);

                const double interior = c * pow(x, 2.0) + 2 * b * x * y + a * pow(y, 2.0);
                const double penalty = std::sqrt(interior);

                penalties(i, k) = penalty;
            }
        }
    }

    // Search for the shortest and least-penalty path using the penalties
    // matrix. Invalid paths are now identified by dummy values left over
    // in the matrix.
    findBestPath(polyline, chain, penalties);
}

/* This code taken from https://github.com/zhuethanca/DiffusionCurves
 *
 * Finds a sequence of pixels, forming a polyline, which approximates
 * the pixel chain <chain> with the minimum number of segments and with
 * the minimum penalty given the matrix <penalties>.
 *
 * param bestPath: Return value, a sequence of points approximating the chain.
 * param chain: Continuous chain of pixels to be approximated by a polyline.
 * param penalties: Matrix in which the entry i, j represents the penalty of
 *                  a straight line between pixels i and j, or -1 if they are
 *                  not connectable by a straight line.
 */
void Vectorizer::findBestPath(QVector<Point> &bestPath, PixelChain chain, Eigen::MatrixXd penalties)
{
    const int nPoints = chain.length();

    // Store a vector which identifies best paths and their penalties from
    // pixel i to the endpoint.
    Eigen::VectorXd bestPenalties(nPoints);
    std::vector<std::vector<int>> bestPaths;

    // Make sure to set initial values for both penalties and paths, including
    // a sensible initial value for the endpoint itself.
    bestPenalties.setConstant(-1);
    bestPenalties(nPoints - 1) = 0;

    for (int i = 0; i < nPoints; i++)
    {
        std::vector<int> nullPath;
        bestPaths.push_back(nullPath);
    }

    bestPaths.at(nPoints - 1).push_back(nPoints - 1);

    // Work backwards, finding best paths from the end back to the beginning
    // using a dynamic programming approach.
    for (int i = nPoints - 2; i >= 0; i--)
    {
        for (int j = i + 1; j < nPoints; j++)
        {
            if (penalties(i, j) != -1)
            {
                const double penaltyCandidate = penalties(i, j) + bestPenalties(j);
                std::vector<int> pathCandidate = bestPaths.at(j);
                pathCandidate.push_back(i);

                bool firstPath = bestPaths.at(i).size() == 0;
                bool shortPath = pathCandidate.size() < bestPaths.at(i).size();
                bool equalPath = pathCandidate.size() == bestPaths.at(i).size();
                bool cheapPath = penaltyCandidate < bestPenalties(i);

                // Check if this is a new best path for any of the above reasons.
                if (firstPath || shortPath || (equalPath && cheapPath))
                {
                    bestPenalties(i) = penaltyCandidate;
                    bestPaths.at(i) = pathCandidate;
                }
            }
        }
    }

    std::vector<int> bestPathIndices = bestPaths.front();
    std::reverse(bestPathIndices.begin(), bestPathIndices.end());

    // Convert the path indices into a polyline.
    bestPath.clear();
    for (int i = 0; i < bestPathIndices.size(); i++)
    {
        Point point = chain.get(bestPathIndices.at(i));
        bestPath.push_back(point);
    }
}

void Vectorizer::constructCurves(ProgressStatus &progressStatus, QVector<Bezier *> &curves, const QVector<QVector<Point>> &polylines)
{
    for (int i = 0; i < polylines.size(); i++)
    {
        progressStatus.progress = progressStatus.start + (progressStatus.end - progressStatus.start) * float(i) / polylines.size();

        Bezier *curve = constructCurve(polylines.at(i));

        if (curve)
        {
            // Split curve into several curves because we support Beizer curves having at most 32 control points
            if (curve->controlPoints().size() >= 29)
            {
                QList<ControlPoint *> controlPoints = curve->controlPoints();

                QList<Bezier *> newCurves;
                newCurves << new Bezier;
                newCurves.last()->addControlPoint(controlPoints.at(0)->deepCopy());
                newCurves.last()->addControlPoint(controlPoints.at(1)->deepCopy());

                for (int i = 2; i < controlPoints.size() - 2; i++)
                {
                    newCurves.last()->addControlPoint(controlPoints.at(i)->deepCopy());

                    if (newCurves.last()->controlPoints().size() == 29)
                    {
                        newCurves.last()->removeControlPoint(28);
                        i--;
                        newCurves << new Bezier;
                        newCurves.last()->addControlPoint(controlPoints.at(i)->deepCopy());
                        newCurves.last()->addControlPoint(controlPoints.at(i + 1)->deepCopy());
                    }
                }

                newCurves.last()->addControlPoint(controlPoints.at(controlPoints.size() - 2)->deepCopy());
                newCurves.last()->addControlPoint(controlPoints.at(controlPoints.size() - 1)->deepCopy());

                curves << newCurves;

                delete curve;

            } else
            {
                curves << curve;
            }
        }
    }
}

Bezier *Vectorizer::constructCurve(const QVector<Point> &polyline, double tension)
{
    const int nPoints = polyline.size();

    if (nPoints <= 1)
    {
        qInfo() << "Number of points in the polyline is less than 1. Number of points is" << nPoints;
        return nullptr;
    }

    Bezier *curve = new Bezier;

    Eigen::Vector2f first = polyline.at(0).toVector();
    Eigen::Vector2f second = polyline.at(1).toVector();
    Eigen::Vector2f secondLast = polyline.at(nPoints - 2).toVector();
    Eigen::Vector2f last = polyline.at(nPoints - 1).toVector();

    Eigen::MatrixXf derivatives(2, nPoints);
    derivatives.col(0) = (second - first) / tension;
    derivatives.col(nPoints - 1) = (last - secondLast) / tension;

    for (int i = 1; i < polyline.size() - 1; i++)
    {
        Eigen::Vector2f next = polyline.at(i + 1).toVector();
        Eigen::Vector2f prev = polyline.at(i - 1).toVector();

        derivatives.col(i) = (next - prev) / tension;
    }

    Eigen::Vector2f firstDerivative = derivatives.col(0);
    Eigen::Vector2f firstControl = first + firstDerivative / 3.0;

    ControlPoint *firstHandle = polyline.at(0).toControlPoint();
    ControlPoint *firstControlHandle = Point(firstControl(0), firstControl(1)).toControlPoint();

    curve->addControlPoint(firstHandle);
    curve->addControlPoint(firstControlHandle);

    for (int i = 1; i < nPoints - 1; i++)
    {
        Eigen::Vector2f curr = polyline.at(i).toVector();
        Eigen::Vector2f currDerivative = derivatives.col(i);

        Eigen::Vector2f prevControl = curr - currDerivative / 3.0;
        Eigen::Vector2f nextControl = curr + currDerivative / 3.0;

        Point currHandle = polyline.at(i);
        Point prevControlHandle(prevControl(0), prevControl(1));
        Point nextControlHandle(nextControl(0), nextControl(1));

        curve->addControlPoint(prevControlHandle.toControlPoint());
        curve->addControlPoint(currHandle.toControlPoint());
        curve->addControlPoint(nextControlHandle.toControlPoint());
    }

    // Correct the normal around the first handle.
    Eigen::Vector2f lastDerivative = derivatives.col(nPoints - 1);
    Eigen::Vector2f lastControl = last - lastDerivative / 3.0;

    Point lastHandle = polyline.at(nPoints - 1);
    Point lastControlHandle(lastControl(0), lastControl(1));

    curve->addControlPoint(lastControlHandle.toControlPoint());
    curve->addControlPoint(lastHandle.toControlPoint());

    return curve;
}

void Vectorizer::sampleColors(ProgressStatus &progressStatus, QVector<Bezier *> &curves, cv::Mat &image, cv::Mat &imageLab, double sampleDensity) {}

/*
 * This code taken from https://github.com/zhuethanca/DiffusionCurves
 *
 * Locates a sample point by traversing <distance> pixels from starting point
 * <point> along the unit normal given by <normal>, and returns the BGR colour
 * at that pixel in an image.
 *
 * If the pixel appears to be an outlier, or if it is outside the image, then
 * NULL is returned instead.
 *
 * param image: A BGR image.
 * param imageLab: The same image represented in the  Lab colour scheme.
 * param point: A pixel location in the images.
 * param normal: The direction in which to look for colour samples.
 * param distance: The distance to travel in the normal direction for a sample.
 */
cv::Vec3b *Vectorizer::sampleAlongNormal(cv::Mat &image, cv::Mat &imageLab, QVector2D point, QVector2D normal, double distance)
{
    const int width = image.cols;
    const int height = image.rows;

    // Traverse the (normalized) normal to a sample point.
    QVector2D sample = (point + distance * normal);
    sample[0] = int(sample[0]);
    sample[0] = int(sample[0]);

    // Check that the sample point is inside the image.
    bool xOutOfBounds = sample.x() < 0 || sample.x() >= width;
    bool yOutOfBounds = sample.y() < 0 || sample.y() >= height;
    if (xOutOfBounds || yOutOfBounds)
    {
        // Return null to signify that no colour could be sampled here.
        return NULL;
    }

    // Extract an image patch, at most 3 pixels across, around the sample point.
    const int startX = sample.x() == 0 ? 0 : sample.x() - 1;
    const int endX = sample.x() == width - 1 ? width - 1 : sample.x() + 1;
    const int startY = sample.y() == 0 ? 0 : sample.y() - 1;
    const int endY = sample.y() == height - 1 ? height - 1 : sample.y() + 1;

    const int patchWidth = endX - startX + 1;
    const int patchHeight = endY - startY + 1;

    // Compute the mean of each of the LAB channels in the patch.
    double luminanceMean = 0.0;
    double alphaMean = 0.0;
    double betaMean = 0.0;

    for (int dx = 0; dx < patchWidth; dx++)
    {
        for (int dy = 0; dy < patchHeight; dy++)
        {
            cv::Vec3b pixel = imageLab.at<cv::Vec3b>(startY + dy, startX + dx);

            luminanceMean += pixel(0);
            alphaMean += pixel(1);
            betaMean += pixel(2);
        }
    }

    luminanceMean /= (patchWidth * patchHeight);
    alphaMean /= (patchWidth * patchHeight);
    betaMean /= (patchWidth * patchHeight);

    // Compute the variance and standard deviation of LAB channels in the patch.
    double luminanceVariance = 0.0;
    double alphaVariance = 0.0;
    double betaVariance = 0.0;

    for (int dx = 0; dx < patchWidth; dx++)
    {
        for (int dy = 0; dy < patchHeight; dy++)
        {
            cv::Vec3b pixel = imageLab.at<cv::Vec3b>(startY + dy, startX + dx);

            luminanceVariance += pow(pixel(0) - luminanceMean, 2.0);
            alphaVariance += pow(pixel(1) - alphaMean, 2.0);
            betaVariance += pow(pixel(2) - betaMean, 2.0);
        }
    }

    luminanceVariance /= (patchWidth * patchHeight);
    alphaVariance /= (patchWidth * patchHeight);
    betaVariance /= (patchWidth * patchHeight);

    const double luminanceStdDev = std::sqrt(luminanceVariance);
    const double alphaStdDev = std::sqrt(alphaVariance);
    const double betaStdDev = std::sqrt(betaVariance);

    // Take the sample point's value in each of the LAB channels.
    cv::Vec3b sampleColour = imageLab.at<cv::Vec3b>(sample.y(), sample.x());
    const double luminance = sampleColour(0);
    const double alpha = sampleColour(1);
    const double beta = sampleColour(2);

    // Check that the sample point is within one standard deviation of the
    // mean for all three channels.
    const double luminanceDiff = std::abs(luminance - luminanceMean);
    const double alphaDiff = std::abs(alpha - alphaMean);
    const double betaDiff = std::abs(beta - betaMean);

    if (luminanceDiff > luminanceStdDev || alphaDiff > alphaStdDev || betaDiff > betaStdDev)
    {
        // Return null to signify that no colour could be sampled here.
        return NULL;
    } else
    {
        // Return the BGR colour at the sample point.
        return &image.at<cv::Vec3b>(sample.y(), sample.x());
    }
}
