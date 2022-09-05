#ifndef COLORSAMPLER_H
#define COLORSAMPLER_H

#include "Bezier.h"
#include <opencv2/core.hpp>

#include <QRandomGenerator>
#include <QVector2D>
#include <QVector4D>

class ColorSampler
{
public:
    ColorSampler();

    /*
     * Set the colours alongside a Bezier curve by sampling nearby points in an
     * image. The number of sample points is determined by <sampleDensity> and
     * the approximate length of the curve.
     *
     * param curve: A cubic Bezier curve.
     * param colours: An object signifying colour constraints around the curve.
     * param image: A BGR image.
     * param imageLab: The same image represented in the  Lab colour scheme.
     * param sampleDensity: The number of samples to take per pixel on the curve.
     */
    void run(const QVector<Bezier *> &curves, cv::Mat &image, cv::Mat &imageLab, const double sampleDensity);

private:
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
    void sampleAlongNormal(Bezier *curve, float parameter, ColorPoint::Direction direction, cv::Mat &image, cv::Mat &imageLab, const double distance = 3.0);

private:
    float mProgress;
    QRandomGenerator mRandomGenerator;
};

#endif // COLORSAMPLER_H
