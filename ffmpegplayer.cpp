#include "FFmpegPlayer.h"

#include "demuxthread.h"
#include "videodecodethread.h"
#include "audiodecodethread.h"
#include "audioplayer.h"
#include "log.h"

#define FREE(x) \
delete x; \
    x = nullptr

    static double get_audio_clock(FFmpegPlayerCtx *is)
{
    double pts;
    int hw_buf_size, bytes_per_sec, n;

    pts = is->audio_clock;
    hw_buf_size = is->audio_buf_size - is->audio_buf_index;
    bytes_per_sec = 0;
    n = is->aCodecCtx->ch_layout.nb_channels * 2;

    if(is->audio_st) {
        bytes_per_sec = is->aCodecCtx->sample_rate * n;
    }

    if (bytes_per_sec) {
        pts -= (double)hw_buf_size / bytes_per_sec;
    }
    return pts;
}


// static void schedule_refresh(FFmpegPlayerCtx *is, int delay)
// {
//     //SDL_AddTimer(delay, sdl_refresh_timer_cb, is);
// }

void FFmpegPlayer::video_display()
{
    VideoPicture *vp =  &playerCtx.pictq[playerCtx.pictq_rindex];
    //调用回调显示视频
    if (vp->bmp ) {
        emit refresh();
        //is->imgCb(vp->bmp->data[0], is->vCodecCtx->width, is->vCodecCtx->height, is->cbData);
    }
}

YUVData FFmpegPlayer::getFrame()
{
    FFmpegPlayerCtx * is = &playerCtx;
    VideoPicture *vp =  &is->pictq[is->pictq_rindex];
  
    if (vp->bmp ) {
       /* m_yuvData.Y.resize(vp->bmp->linesize[0]*is->vCodecCtx->height);
        m_yuvData.Y =QByteArray((char*)vp->bmp->data[0],m_yuvData.Y.size());
        m_yuvData.U.resize(vp->bmp->linesize[1]*is->vCodecCtx->height/2);
        m_yuvData.U =QByteArray((char*)vp->bmp->data[1],m_yuvData.Y.size()/4);
        m_yuvData.V.resize(vp->bmp->linesize[2]*is->vCodecCtx->height/2);
        m_yuvData.V =QByteArray((char*)vp->bmp->data[2],m_yuvData.Y.size()/4);
        m_yuvData.yLineSize =vp->bmp->linesize[0];
        m_yuvData.uLineSize = vp->bmp->linesize[1];
        m_yuvData.vLineSize = vp->bmp->linesize[2];
        m_yuvData.height = is->vCodecCtx->height;*/

        int width = is->vCodecCtx->width;
        int height = is->vCodecCtx->height;

        // 正确提取 YUV 数据
        int y_size = width * height;
        int uv_size = y_size / 4;

        // 分配并复制 Y 分量
        m_yuvData.Y.resize(y_size);
        memcpy(m_yuvData.Y.data(), vp->bmp->data[0], y_size);

        // 分配并复制 U 分量
        m_yuvData.U.resize(uv_size);
        memcpy(m_yuvData.U.data(), vp->bmp->data[1], uv_size);

        // 分配并复制 V 分量
        m_yuvData.V.resize(uv_size);
        memcpy(m_yuvData.V.data(), vp->bmp->data[2], uv_size);

        // 设置行大小和高度
        m_yuvData.yLineSize = vp->bmp->linesize[0];
        m_yuvData.uLineSize = vp->bmp->linesize[1];
        m_yuvData.vLineSize = vp->bmp->linesize[2];
        m_yuvData.height = height;
        //is->imgCb(vp->bmp->data[0], is->vCodecCtx->width, is->vCodecCtx->height, is->cbData);
    }
    return m_yuvData;
}

static void FN_Audio_Cb(void *userdata, qint8 *stream, int len)
{
    AudioDecodeThread *dt = (AudioDecodeThread*)userdata;
    //dt->getAudioData(stream, len);
}

void stream_seek(FFmpegPlayerCtx *is, int64_t pos, int rel)
{
    if (!is->seek_req) {
        is->seek_pos = pos;
        is->seek_flags = rel < 0 ? AVSEEK_FLAG_BACKWARD : 0;
        is->seek_req = 1;
    }
}

FFmpegPlayer::FFmpegPlayer(QObject *parent): QObject(parent)
{
}

void FFmpegPlayer::setFilePath(const char *filePath)
{
    m_filePath = filePath;
}

void FFmpegPlayer::setImageCb(Image_Cb cb, void *userData)
{
    playerCtx.imgCb  = cb;
    playerCtx.cbData = userData;
}

int FFmpegPlayer::initPlayer()
{
    // init ctx
    playerCtx.init();
    //strncpy(playerCtx.filename, m_filePath.c_str(), m_filePath.size());
    snprintf(playerCtx.filename, sizeof(playerCtx.filename), "%s", m_filePath.c_str());
    // create demux thread
    m_demuxThread = new DemuxThread;
    m_demuxThread->setPlayerCtx(&playerCtx);
    if (m_demuxThread->initDemuxThread() != 0) {
        ff_log_line("DemuxThread init Failed.");
        return -1;
    }

    // create audio decode thread
    m_audioDecodeThread = new AudioDecodeThread;
    m_audioDecodeThread->setPlayerCtx(&playerCtx);

    // create video decode thread
    m_videoDecodeThread = new VideoDecodeThread;
    m_videoDecodeThread->setPlayerCtx(&playerCtx);

    // render audio params
    // audio_wanted_spec.freq = 48000;
    // audio_wanted_spec.format = AUDIO_S16SYS;
    // audio_wanted_spec.channels = 2;
    // audio_wanted_spec.silence = 0;
    // audio_wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
    // audio_wanted_spec.callback = FN_Audio_Cb;
    // audio_wanted_spec.userdata = m_audioDecodeThread;
    QAudioFormat format;
    format.setSampleRate(48000); // 采样率 44.1kHz
    format.setChannelCount(2);   // 立体声
    format.setSampleFormat(QAudioFormat::Int16);
    // create and open audio play device
    m_audioPlay = new AudioPlay();
    if (m_audioPlay->openDevice(m_audioDecodeThread,format) <= 0) {
        ff_log_line("open audio device Failed.");
        return -1;
    }
    return 0;
}

void FFmpegPlayer::start()
{
    m_demuxThread->start();
    m_videoDecodeThread->start();
    m_audioDecodeThread->start();
    m_audioPlay->start();

    m_schedule_refresh = new QTimer(this);
    connect(m_schedule_refresh, &QTimer::timeout, this, &FFmpegPlayer::onRefreshEvent);
    m_schedule_refresh->start(13); // 初始13ms
    m_stop = false;
}


void FFmpegPlayer::stop()
{
    m_stop = true;

    // stop audio decode
    ff_log_line("audio decode thread clean...");
    if (m_audioDecodeThread) {
        m_audioDecodeThread->stop();
        FREE(m_audioDecodeThread);
    }
    ff_log_line("audio decode thread finished.");

    // stop audio thread
    ff_log_line("audio play thread clean...");
    // if (m_audioPlay) {
    //     m_audioPlay->stop();
    //     FREE(m_audioPlay);
    // }
    ff_log_line("audio device finished.");

    // stop video decode thread
    ff_log_line("video decode thread clean...");
    if (m_videoDecodeThread) {
        m_videoDecodeThread->stop();
        FREE(m_videoDecodeThread);
    }
    ff_log_line("video decode thread finished.");

    // stop demux thread
    ff_log_line("demux thread clean...");
    if (m_demuxThread) {
        m_demuxThread->stop();
        m_demuxThread->finiDemuxThread();
        FREE(m_demuxThread);
    }
    ff_log_line("demux thread finished.");

    ff_log_line("player ctx clean...");
    playerCtx.fini();
    ff_log_line("player ctx finished.");
}

void FFmpegPlayer::pause(PauseState state)
{
    playerCtx.pause = state;

    // reset frame_timer when restore pause state
    playerCtx.frame_timer = av_gettime() / 1000000.0;
}


void FFmpegPlayer::onRefreshEvent()
{
    if (m_stop) {
        return;
    }

    FFmpegPlayerCtx *is = &playerCtx;
    VideoPicture *vp;
    double actual_delay, delay, sync_threshold, ref_clock, diff;

    if(is->video_st) {
        if(is->pictq_size == 0) {
            //schedule_refresh(is, 1);
            m_schedule_refresh->setInterval(1);
        } else {
            vp = &is->pictq[is->pictq_rindex];

            delay = vp->pts - is->frame_last_pts;

            if(delay <= 0 || delay >= 1.0) {
                delay = is->frame_last_delay;
            }

            // save for next time
            is->frame_last_delay = delay;
            is->frame_last_pts = vp->pts;

            ref_clock = get_audio_clock(is);
            diff = vp->pts - ref_clock;

            sync_threshold = (delay > AV_SYNC_THRESHOLD) ? delay : AV_SYNC_THRESHOLD;
            if (fabs(diff) < AV_NOSYNC_THRESHOLD) {
                if (diff <= -sync_threshold) {
                    delay = 0;
                } else if (diff >= sync_threshold) {
                    delay = 2 * delay;
                }
            }

            is->frame_timer += delay;
            actual_delay = is->frame_timer - (av_gettime() / 1000000.0);
            if (actual_delay < 0.010) {
                actual_delay = 0.010;
            }

            //schedule_refresh(is, (int)(actual_delay * 1000 + 0.5));
            m_schedule_refresh->setInterval((int)(actual_delay * 1000 + 0.5));
            video_display();

            if (++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE) {
                is->pictq_rindex = 0;
            }
            //emit refresh();
            is->pictq_mutex->lock();
            is->pictq_size--;
            is->pictq_cond->notify_all();
            is->pictq_mutex->unlock();
        }
    } else {
        m_schedule_refresh->setInterval(10);
        //schedule_refresh(is, 100);
    }
}
