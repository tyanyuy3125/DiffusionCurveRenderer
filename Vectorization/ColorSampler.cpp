#include "ColorSampler.h"
#include "qdebug.h"

#include <random>

ColorSampler::ColorSampler()
    : mProgress(0.0f)
{
    mRandomGenerator = QRandomGenerator::securelySeeded();
}

void ColorSampler::run(const QVector<Bezier *> &curves, cv::Mat &image, cv::Mat &imageLab, const double sampleDensity)
{
    for (int i = 0; i < curves.size(); i++)
    {
        mProgress = float(i) / (curves.size() - 1);

        Bezier *curve = curves[i];

        sampleAlongNormal(curve, 0.0f, ColorPoint::Direction::Left, image, imageLab);
        sampleAlongNormal(curve, 0.0f, ColorPoint::Direction::Right, image, imageLab);

        sampleAlongNormal(curve, 0.5, ColorPoint::Direction::Left, image, imageLab);
        sampleAlongNormal(curve, 0.5, ColorPoint::Direction::Right, image, imageLab);

        sampleAlongNormal(curve, 1.0f, ColorPoint::Direction::Left, image, imageLab);
        sampleAlongNormal(curve, 1.0f, ColorPoint::Direction::Right, image, imageLab);

        int nSamples = sampleDensity * curve->length();

        for (int i = 0; i < nSamples - 3; i++)
        {
            sampleAlongNormal(curve, mRandomGenerator.bounded(1.0f), ColorPoint::Direction::Left, image, imageLab);
            sampleAlongNormal(curve, mRandomGenerator.bounded(1.0f), ColorPoint::Direction::Right, image, imageLab);
        }
    }
}

/*
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
void ColorSampler::sampleAlongNormal(Bezier *curve, float parameter, ColorPoint::Direction direction, cv::Mat &image, cv::Mat &imageLab, const double distance)
{
    const int width = image.cols;
    const int height = image.rows;
    QVector2D point = curve->valueAt(parameter);
    QVector2D normal = curve->normalAt(parameter);

    if (direction == ColorPoint::Direction::Right)
        normal = -normal;

    // Traverse the (normalized) normal to a sample point.
    QVector2D sample = (point + distance * normal);
    sample[0] = int(sample[0]);
    sample[1] = int(sample[1]);

    // Check that the sample point is inside the image.
    bool xOutOfBounds = sample.x() < 0 || sample.x() >= width;
    bool yOutOfBounds = sample.y() < 0 || sample.y() >= height;
    if (xOutOfBounds || yOutOfBounds)
    {
        // Return null to signify that no colour could be sampled here.
        return;
    }

    auto color = &image.at<cv::Vec3b>(sample.y(), sample.x());

    if (color != NULL)
    {
        // Colour sample is valid.
        ColorPoint *point = new ColorPoint;
        point->mColor = QVector4D(int(color->val[2]) / 255.0f, int(color->val[1]) / 255.0f, int(color->val[0]) / 255.0f, 1.0);
        point->mDirection = direction;
        point->mPosition = parameter;
        point->setParent(curve);
        curve->addColorPoint(point);
    }
}
