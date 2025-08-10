#ifndef AUDIOPLAY_H
#define AUDIOPLAY_H
#include <QAudioFormat>
#include <QAudioSink>
#include <QDebug>
#include "audiodevice.h"
class AudioPlay :public QObject
{
    Q_OBJECT
public:
    AudioPlay(QObject *parent = nullptr);

    int openDevice(AudioDecodeThread* audio_decode_thread,const QAudioFormat &format);

    void start();

    void play(const char *data, qint64 len);


private:
    QAudioSink* audioSink;
    AudioDevice *audioDevice;      // 音频设备 I/O
};

#endif // AUDIOPLAY_H
