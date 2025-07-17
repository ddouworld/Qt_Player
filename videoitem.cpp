#include "videoitem.h"
#include "videofboitem.h"
VideoItem::VideoItem(QQuickItem *parent):  QQuickFramebufferObject(parent) {
    m_player = new FFmpegPlayer;
    connect(m_player,&FFmpegPlayer::refresh,this,&VideoItem::update_img, Qt::QueuedConnection);
}

void VideoItem::setUrl(QString url)
{
    qDebug()<<"设置播放链接";
    m_player->setFilePath("C:\\Users\\zha\\Desktop\\testvideo\\test.mp4");
    if (m_player->initPlayer() != 0) {
        return ;
    }
}

QQuickFramebufferObject::Renderer *VideoItem::createRenderer() const
{
    return new VideoFboItem;
}


void VideoItem::start()
{
    qDebug()<<"开始播放";
    m_player->start();
}

void VideoItem::update_img()
{

    //m_infoChanged= true;
    update();
}

YUVData VideoItem::getFrame()
{
    return m_player->getFrame();
}

