#include "audioplayer.h"


AudioPlay::AudioPlay(AudioDecodeThread* audiodecode,QObject *parent): QObject(parent)
{
    m_audiodecode = audiodecode;
    connect(m_audiodecode,&AudioDecodeThread::bufferReady,this,&AudioPlay::startPlay);

}

int AudioPlay::openDevice(const QAudioFormat &format)
{
    audioSink = new QAudioSink(format);

    audioSink->setBufferSize(1024);

    return 1;
}

void AudioPlay::start()
{
    audioDevice  = audioSink->start();
    connect(audioSink,&QAudioSink::stateChanged,this,&AudioPlay::play);

}

void AudioPlay::stop()
{
    audioSink->stop();
}

void AudioPlay::onBufferReady(QAudioBuffer buffer)
{
    if (!buffer.isValid()) {
        qWarning() << "无效的音频缓冲区";
        return;
    }

    // **将音频数据写入 QAudioSink**
    // QByteArray data = QByteArray(reinterpret_cast<const char*>(buffer.constData()), buffer.byteCount());
    // audioDevice->write(data);
}

void AudioPlay::onDecodingFinished()
{

}

void AudioPlay::onDecodingError(QString error)
{

}

void AudioPlay::play(QAudio::State state)
{
    if(state == QAudio::IdleState)
    {
        int freesize = audioSink->bytesFree();
        stream = new unsigned char[freesize];
        m_audiodecode->getAudioData(stream,freesize);
        audioDevice->write((char*)stream,freesize);
    }
}

void AudioPlay::startPlay()
{
    int freesize = audioSink->bytesFree();
    stream = new unsigned char[freesize];
    m_audiodecode->getAudioData(stream,freesize);
    audioDevice->write((char*)stream,freesize);
}
