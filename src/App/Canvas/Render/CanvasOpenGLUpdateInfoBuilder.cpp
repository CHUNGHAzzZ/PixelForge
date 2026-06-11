#include "CanvasOpenGLUpdateInfoBuilder.h"

#include "App/Canvas/Object/BaseObject.h"

#include <QPainter>
#include <QPen>

#include <algorithm>
#include <cmath>

namespace PixelForge {

namespace {
QRect growRect(const QRect &rect, int border)
{
    return rect.adjusted(-border, -border, border, border);
}

QRect cacheBoundsForCanvasAndObjects(const CanvasObject &canvas, const CanvasObjectList &objects)
{
    QRectF bounds = canvas.canvasBounds();
    for (const auto &object : objects) {
        if (object && object->type() != ObjectType::Image) {
            bounds = bounds.united(object->canvasBounds());
        }
    }

    const QRect alignedBounds = bounds.adjusted(-8.0, -8.0, 8.0, 8.0).toAlignedRect();
    const int left = std::floor(static_cast<double>(alignedBounds.left()) / CanvasDefaultTileEffectiveSize) * CanvasDefaultTileEffectiveSize;
    const int top = std::floor(static_cast<double>(alignedBounds.top()) / CanvasDefaultTileEffectiveSize) * CanvasDefaultTileEffectiveSize;
    const int right = std::ceil(static_cast<double>(alignedBounds.right() + 1) / CanvasDefaultTileEffectiveSize) * CanvasDefaultTileEffectiveSize;
    const int bottom = std::ceil(static_cast<double>(alignedBounds.bottom() + 1) / CanvasDefaultTileEffectiveSize) * CanvasDefaultTileEffectiveSize;
    return QRect(left, top, std::max(1, right - left), std::max(1, bottom - top));
}
}

CanvasOpenGLUpdateInfo CanvasOpenGLUpdateInfoBuilder::buildUpdateInfo(
    const CanvasDirtyUpdateInfo &dirtyInfo,
    const CanvasObject &canvas,
    const CanvasObjectList &objects,
    ObjectId selectedObjectId) const
{
    CanvasOpenGLUpdateInfo info;
    if (!dirtyInfo.isValid()) {
        return info;
    }

    const QRect sceneBounds = calculateCacheBounds(canvas, objects);
    const QRect updateRect = dirtyInfo.dirtySceneRect & sceneBounds;
    if (updateRect.isEmpty()) {
        return info;
    }

    const QRect artificialRect = growRect(updateRect, m_textureBorder) & sceneBounds;
    if (artificialRect.isEmpty()) {
        return info;
    }

    const int firstColumn = xToColumn(artificialRect.left() - sceneBounds.left());
    const int lastColumn = xToColumn(artificialRect.right() - sceneBounds.left());
    const int firstRow = yToRow(artificialRect.top() - sceneBounds.top());
    const int lastRow = yToRow(artificialRect.bottom() - sceneBounds.top());

    info.dirtySceneRect = dirtyInfo.dirtySceneRect;
    info.levelOfDetail = dirtyInfo.levelOfDetail;
    info.tileList.reserve(static_cast<size_t>((lastColumn - firstColumn + 1) * (lastRow - firstRow + 1)));

    for (int column = firstColumn; column <= lastColumn; ++column) {
        for (int row = firstRow; row <= lastRow; ++row) {
            const QRect effectiveTileRect = calculateEffectiveTileRect(column, row, sceneBounds);
            if (effectiveTileRect.isEmpty()) {
                continue;
            }

            const QRect physicalTileRect = calculatePhysicalTileRect(column, row, sceneBounds, dirtyInfo.levelOfDetail);
            const QRect patchRect = physicalTileRect & updateRect;
            if (patchRect.isEmpty()) {
                continue;
            }

            CanvasTextureTileUpdateInfo tileInfo;
            tileInfo.column = column;
            tileInfo.row = row;
            tileInfo.effectiveTileSceneRect = effectiveTileRect;
            tileInfo.physicalTileSceneRect = physicalTileRect;
            tileInfo.patchSceneRect = patchRect;
            tileInfo.currentSceneRect = sceneBounds;
            tileInfo.levelOfDetail = dirtyInfo.levelOfDetail;
            tileInfo.patchPixels = renderPatch(patchRect, canvas, objects, selectedObjectId);
            if (tileInfo.isValid()) {
                info.tileList.push_back(std::move(tileInfo));
            }
        }
    }

    return info;
}

QRect CanvasOpenGLUpdateInfoBuilder::calculateEffectiveTileRect(int column, int row, const QRect &sceneBounds) const
{
    const QRect tileRect(
        sceneBounds.left() + column * m_effectiveTextureSize.width(),
        sceneBounds.top() + row * m_effectiveTextureSize.height(),
        m_effectiveTextureSize.width(),
        m_effectiveTextureSize.height());
    return sceneBounds & tileRect;
}

QRect CanvasOpenGLUpdateInfoBuilder::calculatePhysicalTileRect(int column, int row, const QRect &sceneBounds, int levelOfDetail) const
{
    Q_UNUSED(levelOfDetail)
    const QRect effectiveTileRect = calculateEffectiveTileRect(column, row, sceneBounds);
    return growRect(effectiveTileRect, m_textureBorder) & sceneBounds;
}

QRect CanvasOpenGLUpdateInfoBuilder::calculateCacheBounds(const CanvasObject &canvas, const CanvasObjectList &objects) const
{
    return cacheBoundsForCanvasAndObjects(canvas, objects);
}

int CanvasOpenGLUpdateInfoBuilder::xToColumn(int x) const
{
    return std::max(0, x / std::max(1, m_effectiveTextureSize.width()));
}

int CanvasOpenGLUpdateInfoBuilder::yToRow(int y) const
{
    return std::max(0, y / std::max(1, m_effectiveTextureSize.height()));
}

void CanvasOpenGLUpdateInfoBuilder::setTextureBorder(int border)
{
    m_textureBorder = std::max(0, border);
}

void CanvasOpenGLUpdateInfoBuilder::setEffectiveTextureSize(const QSize &size)
{
    m_effectiveTextureSize = QSize(std::max(1, size.width()), std::max(1, size.height()));
}

QImage CanvasOpenGLUpdateInfoBuilder::renderPatch(
    const QRect &patchSceneRect,
    const CanvasObject &canvas,
    const CanvasObjectList &objects,
    ObjectId selectedObjectId) const
{
    QImage image(patchSceneRect.size(), QImage::Format_RGBA8888);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.translate(-patchSceneRect.topLeft());
    painter.setClipRect(patchSceneRect);

    canvas.paintBoard(painter);
    for (const auto &object : objects) {
        if (!object || !object->canvasBounds().intersects(patchSceneRect)) {
            continue;
        }

        if (object->type() == ObjectType::Image) {
            continue;
        }

        object->paint(painter);
    }

    for (const auto &object : objects) {
        if (!object || object->id() != selectedObjectId || !object->canvasBounds().intersects(patchSceneRect)) {
            continue;
        }

        if (object->type() == ObjectType::Image) {
            continue;
        }

        QPen selectionPen(QColor(QStringLiteral("#4B7BEC")), 1.5);
        selectionPen.setCosmetic(true);
        painter.setPen(selectionPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(object->canvasBounds().adjusted(0.75, 0.75, -0.75, -0.75));
    }
    painter.end();

    return image;
}

}
