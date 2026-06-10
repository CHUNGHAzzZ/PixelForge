#pragma once

#include "App/Canvas/Object/BaseObject.h"

#include <QImage>
#include <QSizeF>
#include <QString>

namespace PixelForge {

class ImageObject final : public BaseObject
{
public:
    ImageObject() = default;
    explicit ImageObject(const QImage &image);
    ImageObject(ObjectId id, const QImage &image);

    ObjectType type() const override;

    const QImage &image() const;
    void setImage(const QImage &image);

    QString sourcePath() const;
    void setSourcePath(const QString &path);

    QSizeF displaySize() const;
    void setDisplaySize(const QSizeF &size);
    void clearDisplaySize();
    bool hasCustomDisplaySize() const;

    std::unique_ptr<BaseObject> clone() const override;
    QRectF localBounds() const override;

protected:
    void paintLocal(QPainter &painter) const override;

private:
    QImage m_image;
    QString m_sourcePath;
    QSizeF m_displaySize;
};

}
