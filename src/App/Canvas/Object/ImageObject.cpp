#include "ImageObject.h"

#include <QPainter>

#include <algorithm>

ImageObject::ImageObject(const QImage &image)
    : m_image(image)
{
}

ImageObject::ImageObject(ObjectId id, const QImage &image)
    : m_image(image)
{
    setId(id);
}

ObjectType ImageObject::type() const
{
    return ObjectType::Image;
}

const QImage &ImageObject::image() const
{
    return m_image;
}

void ImageObject::setImage(const QImage &image)
{
    m_image = image;
}

QString ImageObject::sourcePath() const
{
    return m_sourcePath;
}

void ImageObject::setSourcePath(const QString &path)
{
    m_sourcePath = path;
}

QSizeF ImageObject::displaySize() const
{
    if (m_displaySize.isValid() && !m_displaySize.isEmpty()) {
        return m_displaySize;
    }

    return QSizeF(m_image.size());
}

void ImageObject::setDisplaySize(const QSizeF &size)
{
    m_displaySize = QSizeF(std::max<qreal>(1.0, size.width()), std::max<qreal>(1.0, size.height()));
}

void ImageObject::clearDisplaySize()
{
    m_displaySize = QSizeF();
}

bool ImageObject::hasCustomDisplaySize() const
{
    return m_displaySize.isValid() && !m_displaySize.isEmpty();
}

std::unique_ptr<BaseObject> ImageObject::clone() const
{
    return std::make_unique<ImageObject>(*this);
}

QRectF ImageObject::localBounds() const
{
    return QRectF(QPointF(), displaySize());
}

void ImageObject::paintLocal(QPainter &painter) const
{
    if (m_image.isNull()) {
        return;
    }

    painter.drawImage(localBounds(), m_image);
}
