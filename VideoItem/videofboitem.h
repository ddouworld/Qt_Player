#ifndef VIDEOFBOITEM_H
#define VIDEOFBOITEM_H
#include <QOpenGLFramebufferObject>
#include <QQuickFramebufferObject>
#include <QQuickOpenGLUtils>
#include "i420render.h"
class VideoFboItem: public QQuickFramebufferObject::Renderer
{
public:
    VideoFboItem();
public:
    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
    void synchronize(QQuickFramebufferObject *item) override;
private:
    I420Render m_render;
    QQuickWindow *m_window = nullptr;

    YUVData ba;
};

#endif // VIDEOFBOITEM_H
