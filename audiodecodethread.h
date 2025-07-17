#ifndef AUDIODECODETHREAD_H
#define AUDIODECODETHREAD_H

#include "ThreadBase.h"
#include <QObject>
#include <QAudioBuffer>
#include <QAudioSink>
// #include "audioplayer.h"
struct FFmpegPlayerCtx;

class AudioDecodeThread : public QObject,public ThreadBase
{
    Q_OBJECT
public:
    AudioDecodeThread();

    void setPlayerCtx(FFmpegPlayerCtx *ctx);

    void getAudioData(unsigned char *stream, int len);

    void run();
public slots:
    void play(QtAudio::State state);
signals:
    void bufferReady();

private:
    int audio_decode_frame(FFmpegPlayerCtx *is, double *pts_ptr);

private:
    FFmpegPlayerCtx *is = nullptr;
    //AudioPlay* m_audioPlay = nullptr;
    unsigned char *stream = nullptr;
    QAudioSink* audioSink;
    QIODevice *audioDevice;      // 音频设备 I/O

};

#endif // AUDIODECODETHREAD_H
