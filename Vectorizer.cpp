#include "Vectorizer.h"

#include <QDebug>

Vectorizer::Vectorizer() {}

void Vectorizer::load(const QString &path)
{
    clear();
    auto image = cv::imread(path.toStdString(), cv::IMREAD_COLOR);
    mImages.insert(ImageType::Original, image);
}

cv::Mat Vectorizer::image(ImageType type)
{
    return mImages.value(type);
}

Vectorizer *Vectorizer::instance()
{
    static Vectorizer instance;
    return &instance;
}

void Vectorizer::clear() {}
