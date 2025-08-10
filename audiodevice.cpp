#include "audiodevice.h"
AudioDevice::AudioDevice(AudioDecodeThread* audio_decode_thread, QObject *parent)
    : QIODevice(parent), m_audio_decode_thread(audio_decode_thread)
{
    if (!open(QIODevice::ReadOnly)) {
        qCritical() << "AudioDevice打开失败！无法以只读模式打开";
    }
    else
        qDebug() <<"打开成功";
}

qint64 AudioDevice::readData(char *data, qint64 maxlen)
{
    if (!m_audio_decode_thread) {
        qWarning() << "音频解码线程未初始化";
        return 0;
    }
    int size = m_audio_decode_thread->getAudioData((unsigned char*)data,maxlen);
    m_free_size= size;
    return size;
}

qint64 AudioDevice::writeData(const char *data, qint64 len)
{
    // 通常不需要实现，除非要支持写入操作
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}

qint64 AudioDevice::bytesAvailable() const
{
    return m_free_size + QIODevice::bytesAvailable();
}
