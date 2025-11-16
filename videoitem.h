#ifndef VIDEOITEM_H
#define VIDEOITEM_H
#include <QQuickItem>
#include <QQuickFramebufferObject>
#include "ffmpegplayer.h"
class VideoItem : public QQuickFramebufferObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    VideoItem(QQuickItem *parent = nullptr);
    bool infoDirty() const
    {
        return m_infoChanged;
    }
    void makeInfoDirty(bool dirty)
    {
        m_infoChanged = dirty;
    }
public slots:
    void setUrl(QString url);
    void start();
    void pause();
    void seek(int rel);
    void update_img();
public:
    Renderer *createRenderer() const override;

    YUVData getFrame();
public:
    bool m_infoChanged = true;

private:
    FFmpegPlayer* m_player;

};

#endif // VIDEOITEM_Hi
