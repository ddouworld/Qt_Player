#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H
#include <QObject>
#include <QDebug>
#include "QIODevice"
#include "audiodecodethread.h"

class AudioDevice : public QIODevice
{
    Q_OBJECT
public:
    AudioDevice(AudioDecodeThread* audio_decode_thread,QObject *parent = nullptr);

    qint64 readData(char *data, qint64 maxlen) override;

    qint64 writeData(const char *data, qint64 len) override;

    qint64 bytesAvailable() const override;

private:
    AudioDecodeThread* m_audio_decode_thread;
    int m_free_size = 4096;
};

#endif // AUDIODEVICE_H
