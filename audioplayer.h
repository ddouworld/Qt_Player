#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H
#include <QAudioFormat>
#include <QAudioSink>
#include <QAudioBuffer>
#include <QDebug>
#include "audiodecodethread.h"
class AudioPlay :public QObject
{
    Q_OBJECT
public:
    AudioPlay(AudioDecodeThread* audiodecode,QObject *parent = nullptr);

    int openDevice(const QAudioFormat &format);

    void start();

    void stop();
public slots:
    // **当 QAudioDecoder 有新的音频数据时调用此函数**
    void onBufferReady(QAudioBuffer buffer);
    // **当解码完成时调用**
    void onDecodingFinished();
    //播放错误的时候调用此函数
    void onDecodingError(QString error);
    void play(QAudio::State state);
    void startPlay();



public:
    QAudioSink* audioSink;
    QIODevice *audioDevice;      // 音频设备 I/O
    unsigned char *stream = nullptr;
    AudioDecodeThread* m_audiodecode;

};

#endif // AUDIOPLAYER_H
