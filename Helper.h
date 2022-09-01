#ifndef HELPER_H
#define HELPER_H

#include "Bezier.h"

#include <QByteArray>

class Helper
{
    Helper();

public:
    static QByteArray getBytes(QString path);
    static QVector<Bezier *> loadCurveDataFromXML(const QString &filename);
    static QVector<Bezier *> loadCurveDataFromJSON(const QString &filename);
    static bool saveCurveDataToJSON(const QVector<Bezier *> &curves, const QString &filename);
};

#endif // HELPER_H
