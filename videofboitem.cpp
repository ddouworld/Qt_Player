#include "videofboitem.h"
#include "videoitem.h"
VideoFboItem::VideoFboItem() {
    m_render.init();
}


void VideoFboItem::render()
{
    m_render.paint();
    QQuickOpenGLUtils::resetOpenGLState();
}



QOpenGLFramebufferObject *VideoFboItem::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    m_render.resize(size.width(), size.height());
    return new QOpenGLFramebufferObject(size, format);
}



void VideoFboItem::synchronize(QQuickFramebufferObject *item)
{
    VideoItem *pItem = qobject_cast<VideoItem *>(item);
    if (pItem->infoDirty())
    {
        //更新纹理，也就是更新视频
        m_render.updateTextureInfo(1920, 1080);
        pItem->makeInfoDirty(false);
    }
    ba = pItem->getFrame();
    m_render.updateTextureData(ba);

}

