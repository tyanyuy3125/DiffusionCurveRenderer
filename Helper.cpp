#include "Helper.h"

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Helper::Helper() {}

QByteArray Helper::getBytes(QString path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly))
    {
        return file.readAll();
    } else
    {
        qWarning() << QString("Could not open '%1'").arg(path);
        return QByteArray();
    }
}

QVector<Bezier *> Helper::loadCurveDataFromXML(const QString &filename)
{
    QDomDocument document;

    // Read the file
    {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            qCritical() << "Error occured while loading the file:" << filename;
            return QVector<Bezier *>();
        }

        document.setContent(&file);
        file.close();
    }

    QDomElement root = document.documentElement();
    QDomElement component = root.firstChild().toElement();
    QVector<Bezier *> curves;
    while (!component.isNull())
    {
        Bezier *curve = new Bezier;
        if (component.tagName() == "curve")
        {
            QDomElement child = component.firstChild().toElement();
            while (!child.isNull())
            {
                if (child.tagName() == "control_points_set")
                {
                    QDomElement element = child.firstChild().toElement();
                    while (!element.isNull())
                    {
                        float x = element.attribute("y").toDouble();
                        float y = element.attribute("x").toDouble();
                        ControlPoint *controlPoint = new ControlPoint;
                        controlPoint->mPosition = QVector2D(x, y);
                        curve->addControlPoint(controlPoint);
                        element = element.nextSibling().toElement();
                    }
                } else if (child.tagName() == "left_colors_set" || child.tagName() == "right_colors_set")
                {
                    QDomElement element = child.firstChild().toElement();
                    int maxGlobalID = 0;

                    QVector<ColorPoint *> colorPoints;

                    while (!element.isNull())
                    {
                        uint8_t r = element.attribute("B").toUInt();
                        uint8_t g = element.attribute("G").toUInt();
                        uint8_t b = element.attribute("R").toUInt();
                        int globalID = element.attribute("globalID").toInt();

                        ColorPoint *colorPoint = new ColorPoint;
                        colorPoint->setParent(curve);
                        colorPoint->mColor = QVector4D(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
                        colorPoint->mDirection = child.tagName() == "left_colors_set" ? ColorPoint::Direction::Left : ColorPoint::Direction::Right;
                        colorPoint->mPosition = globalID;
                        colorPoints << colorPoint;

                        if (globalID >= maxGlobalID)
                            maxGlobalID = globalID;

                        element = element.nextSibling().toElement();
                    }

                    for (int i = 0; i < colorPoints.size(); ++i)
                    {
                        colorPoints[i]->mPosition = colorPoints[i]->mPosition / maxGlobalID;
                        curve->addColorPoint(colorPoints[i]);
                    }
                }
                child = child.nextSibling().toElement();
            }
        }
        curves << curve;
        component = component.nextSibling().toElement();
    }

    return curves;
}

QVector<Bezier *> Helper::loadCurveDataFromJSON(const QString &filename)
{
    QJsonDocument document;

    // Read the file
    {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            qCritical() << "Error occured while loading the file:" << filename;
            return QVector<Bezier *>();
        }

        document = QJsonDocument::fromJson(file.readAll());
        file.close();
    }

    QVector<Bezier *> curves;
    QJsonArray curvesArray = document.array();

    for (auto element : curvesArray)
    {
        Bezier *curve = new Bezier;

        QJsonObject curveObject = element.toObject();
        QJsonArray leftColorsArray = curveObject["left_color_points"].toArray();
        QJsonArray rightColorsArray = curveObject["right_color_points"].toArray();
        QJsonArray controlPointsArray = curveObject["control_points"].toArray();
        int z = curveObject["z"].toInt();

        // Control points
        for (auto controlPointElement : controlPointsArray)
        {
            QJsonObject controlPointObject = controlPointElement.toObject();
            QJsonObject positionObject = controlPointObject["position"].toObject();
            float x = positionObject["x"].toDouble();
            float y = positionObject["y"].toDouble();

            ControlPoint *controlPoint = new ControlPoint;
            controlPoint->mPosition = QVector2D(x, y);
            curve->addControlPoint(controlPoint);
        }

        // Left colors
        for (auto leftColorElement : leftColorsArray)
        {
            QJsonObject leftColorObject = leftColorElement.toObject();
            QJsonObject colorObject = leftColorObject["color"].toObject();
            float r = colorObject["r"].toDouble();
            float g = colorObject["g"].toDouble();
            float b = colorObject["b"].toDouble();
            float a = colorObject["a"].toDouble();
            float position = leftColorObject["position"].toDouble();

            ColorPoint *colorPoint = new ColorPoint;
            colorPoint->mDirection = ColorPoint::Direction::Left;
            colorPoint->mColor = QVector4D(r, g, b, a);
            colorPoint->mPosition = position;
            colorPoint->setParent(curve);
            curve->addColorPoint(colorPoint);
        }

        // Right colors
        for (auto rightColorElement : rightColorsArray)
        {
            QJsonObject rightColorObject = rightColorElement.toObject();
            QJsonObject colorObject = rightColorObject["color"].toObject();
            float r = colorObject["r"].toDouble();
            float g = colorObject["g"].toDouble();
            float b = colorObject["b"].toDouble();
            float a = colorObject["a"].toDouble();
            float position = rightColorObject["position"].toDouble();

            ColorPoint *colorPoint = new ColorPoint;
            colorPoint->mDirection = ColorPoint::Direction::Right;
            colorPoint->mColor = QVector4D(r, g, b, a);
            colorPoint->mPosition = position;
            colorPoint->setParent(curve);
            curve->addColorPoint(colorPoint);
        }

        curve->mDepth = z;
        curves << curve;
    }

    return curves;
}

bool Helper::saveCurveDataToJSON(const QVector<Bezier *> &curves, const QString &filename)
{
    QJsonArray curvesArray;

    for (const auto curve : curves)
    {
        QJsonObject curveObject;
        QJsonArray controlPointsArray;
        QJsonArray leftColorsArray;
        QJsonArray rightColorsArray;

        // Control point
        QVector<ControlPoint *> controlPoints = curve->controlPoints();

        for (const auto controlPoint : controlPoints)
        {
            QJsonObject controlPointObject;

            QJsonObject position;
            position.insert("x", controlPoint->mPosition.x());
            position.insert("y", controlPoint->mPosition.y());

            controlPointObject.insert("position", position);

            controlPointsArray << controlPointObject;
        }

        // Left colors
        QVector<ColorPoint *> leftColors = curve->getLeftColorPoints();

        for (const auto leftColor : leftColors)
        {
            QJsonObject leftColorObject;

            QJsonObject color;
            color.insert("r", leftColor->mColor.x());
            color.insert("g", leftColor->mColor.y());
            color.insert("b", leftColor->mColor.z());
            color.insert("a", leftColor->mColor.w());

            leftColorObject.insert("color", color);
            leftColorObject.insert("position", leftColor->mPosition);

            leftColorsArray << leftColorObject;
        }

        // Right colors
        QVector<ColorPoint *> rightColors = curve->getRightColorPoints();

        for (const auto rightColor : rightColors)
        {
            QJsonObject rightColorObject;

            QJsonObject color;
            color.insert("r", rightColor->mColor.x());
            color.insert("g", rightColor->mColor.y());
            color.insert("b", rightColor->mColor.z());
            color.insert("a", rightColor->mColor.w());

            rightColorObject.insert("color", color);
            rightColorObject.insert("position", rightColor->mPosition);

            rightColorsArray << rightColorObject;
        }

        curveObject.insert("z", curve->mDepth);
        curveObject.insert("control_points", controlPointsArray);
        curveObject.insert("left_color_points", leftColorsArray);
        curveObject.insert("right_color_points", rightColorsArray);
        curvesArray << curveObject;
    }

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
    {
        QJsonDocument document;
        document.setArray(curvesArray);
        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        stream << document.toJson(QJsonDocument::Indented);
        stream.flush();
        file.close();
        return true;
    } else
    {
        qCritical() << Q_FUNC_INFO << "Couldn't write to file" << filename;
        return false;
    }
}
