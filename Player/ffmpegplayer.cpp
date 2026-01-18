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




void FFmpegPlayer::video_display()
{
    VideoPicture *vp =  &playerCtx.pictq[playerCtx.pictq_rindex];
    //调用回调显示视频
    if (vp->bmp ) {
        emit refresh();

    }
}

YUVData FFmpegPlayer::getFrame()
{
    FFmpegPlayerCtx * is = &playerCtx;
    VideoPicture *vp =  &is->pictq[is->pictq_rindex];
  
    if (vp->bmp ) {


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

    }
    else
    {
        int width = 1920;
        int height = 1080;

        // 正确提取 YUV 数据
        int y_size = width * height;
        int uv_size = y_size / 4;
        m_yuvData.Y.resize(y_size);
        m_yuvData.U.resize(uv_size);
        m_yuvData.V.resize(uv_size);
        memset(m_yuvData.Y.data(), 0, y_size); // 填充中性值
        memset(m_yuvData.U.data(), 128, uv_size); // 填充中性值
        memset(m_yuvData.V.data(), 128, uv_size); // 填充中性值
    }
    return m_yuvData;
}

// void stream_seek(FFmpegPlayerCtx *is, int64_t pos, int rel)
// {
//     if (!is->seek_req) {
//         is->seek_pos = pos;
//         is->seek_flags = rel < 0 ? AVSEEK_FLAG_BACKWARD : 0;
//         is->seek_req = 1;
//     }
// }

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
    m_schedule_refresh->start(40); // 初始40ms
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

void FFmpegPlayer::pause()
{
    if (playerCtx.pause == UNPAUSE) {
        playerCtx.pause = PAUSE;
    } else {
         playerCtx.pause = UNPAUSE;
    }
    //playerCtx.pause = state;

    // reset frame_timer when restore pause state
    playerCtx.frame_timer = av_gettime() / 1000000.0;
}

void FFmpegPlayer::stream_seek(int rel)
{
    int pos = get_audio_clock(&playerCtx);
    pos += rel;
    qDebug()<<"seek to %lf v:%lf a:%lf" <<pos << get_audio_clock(&playerCtx)<< get_audio_clock(&playerCtx);
    FFmpegPlayerCtx *is = &playerCtx;
    if (!is->seek_req) {
        is->seek_pos = (int64_t)(pos * AV_TIME_BASE);
        is->seek_flags = rel < 0 ? AVSEEK_FLAG_BACKWARD : 0;
        is->seek_req = 1;
    }
}

double FFmpegPlayer::getPlayProgress()
{
    FFmpegPlayerCtx *is = &playerCtx;

    // 1. 获取当前播放位置（优先用音频时钟，与seek逻辑对齐）
    double current_pos = is->audio_clock;

    // 2. 计算媒体总时长（从formatCtx的duration转换）
    double total_duration = 0.0;
    if (is->formatCtx != nullptr && is->formatCtx->duration != AV_NOPTS_VALUE) {
        // 将AVFormatContext的duration（微秒）转为秒
        total_duration = static_cast<double>(is->formatCtx->duration) / AV_TIME_BASE;
    }

    // 3. 边界校验：避免进度值超出合理范围
    if (current_pos < 0.0) {
        current_pos = 0.0;
    }
    if (total_duration > 0.0 && current_pos > total_duration) {
        current_pos = total_duration;
    }

    // 4. 调试输出：对齐原stream_seek的日志风格，补充关键进度信息
    double progress_percent = (total_duration > 0.0) ? (current_pos / total_duration) * 100.0 : 0.0;
    qDebug() << "play progress - current: " << current_pos << "s, total: " << total_duration
             << "s, percent: " << progress_percent << "%, audio_clock: " << is->audio_clock
             << "s, video_clock: " << is->video_clock << "s";

    // 5. 返回当前播放进度（秒）
    return progress_percent;
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

            m_schedule_refresh->setInterval((int)(actual_delay * 1000 + 0.5));
            video_display();

            if (++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE) {
                is->pictq_rindex = 0;
            }
            is->pictq_mutex->lock();
            is->pictq_size--;
            is->pictq_cond->notify_all();
            is->pictq_mutex->unlock();
        }
    } else {
        m_schedule_refresh->setInterval(10);
    }
}
