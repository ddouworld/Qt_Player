#include "audioplayer.h"
AudioPlay::AudioPlay(QObject *parent)
{

}


int AudioPlay::openDevice(AudioDecodeThread* audio_decode_thread,const QAudioFormat &format)
{
    // 创建自定义音频设备
    audioDevice = new AudioDevice(audio_decode_thread);

    audioSink = new QAudioSink(format);
    int bufferSize = 48000 * 2 * (16/8) * 0.5; // 计算缓冲大小
    audioSink->setBufferSize(bufferSize);


    return 0;
}

void AudioPlay::start()
{
    audioSink->start(audioDevice);

}


void AudioPlay::play(const char *data, qint64 len)
{
    //audioDevice->write(data,len);
}

