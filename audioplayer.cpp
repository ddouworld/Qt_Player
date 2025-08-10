#include "audioplayer.h"
AudioPlay::AudioPlay(QObject *parent)
{

}


int AudioPlay::openDevice(AudioDecodeThread* audio_decode_thread,const QAudioFormat &format)
{
    // 创建自定义音频设备
    audioDevice = new AudioDevice(audio_decode_thread);

    audioSink = new QAudioSink(format);
    audioSink->setBufferSize(4096);


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

