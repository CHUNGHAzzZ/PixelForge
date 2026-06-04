#include "CanvasItem.h"

#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>

namespace {
class CanvasRenderer final : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
public:
    void render() override
    {
        initializeOpenGLFunctions();

        glViewport(0, 0, m_size.width(), m_size.height());
        glClearColor(0.17F, 0.18F, 0.20F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override
    {
        m_size = size;

        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setSamples(4);
        return new QOpenGLFramebufferObject(size, format);
    }

private:
    QSize m_size;
};
}

CanvasItem::CanvasItem(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    setMirrorVertically(true);
}

QQuickFramebufferObject::Renderer *CanvasItem::createRenderer() const
{
    return new CanvasRenderer();
}
