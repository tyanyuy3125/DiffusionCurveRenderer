#ifndef VECTORIZER_H
#define VECTORIZER_H

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include <QMap>
#include <QString>

class Vectorizer
{
private:
    Vectorizer();

public:
    enum class ImageType {
        Original,
    };

    static Vectorizer *instance();

    void load(const QString &path);
    cv::Mat image(ImageType type);

private:
    void clear();

private:
    QMap<ImageType, cv::Mat> mImages;
};

#endif // VECTORIZER_H
