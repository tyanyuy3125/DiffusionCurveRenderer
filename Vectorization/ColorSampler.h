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

    void run(const QVector<Bezier *> &curves, cv::Mat &image, cv::Mat &imageLab, const double sampleDensity);

private:
    void sampleAlongNormal(Bezier *curve, float parameter, ColorPoint::Direction direction, cv::Mat &image, cv::Mat &imageLab, const double distance = 3.0);

private:
    float mProgress;
    QRandomGenerator mRandomGenerator;
};

#endif // COLORSAMPLER_H
