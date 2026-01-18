#include "VideoDecodeThread.h"

#include "Player/FFmpegPlayer.h"
#include "Log/log.h"

static double synchronize_video(FFmpegPlayerCtx *is, AVFrame *src_frame, double pts)
{
    double frame_delay;

    if(pts != 0) {
        // if we have pts, set video clock to it
        is->video_clock = pts;
    } else {
        // if we aren't given a pts, set it to the clock
        pts = is->video_clock;
    }
    // update the video clock
    frame_delay = av_q2d(is->vCodecCtx->time_base);
    // if we are repeating a frame, adjust clock accordingly
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    is->video_clock += frame_delay;

    return pts;
}

VideoDecodeThread::VideoDecodeThread()
{

}

void VideoDecodeThread::setPlayerCtx(FFmpegPlayerCtx *ctx)
{
    playerCtx = ctx;
}

void VideoDecodeThread::run()
{
    int ret = video_entry();
    qDebug()<<"VideoDecodeThread finished, ret="<< ret;
}

int VideoDecodeThread::video_entry()
{
    FFmpegPlayerCtx *is = playerCtx;
    AVPacket *packet = av_packet_alloc();
    AVCodecContext *pCodecCtx = is->vCodecCtx;
    int ret = -1;
    double pts = 0;

    AVFrame * pFrame = av_frame_alloc();
    AVFrame * pFrameRGB = av_frame_alloc();

    av_image_alloc(pFrameRGB->data, pFrameRGB->linesize, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, 32);

    for (;;) {

        if (m_stop) {
            break;
        }

        if (is->pause == PAUSE) {
            //SDL_Delay(5);
            QThread::sleep(5);
            continue;
        }

        if (is->flush_vctx) {
            ff_log_line("avcodec_flush_buffers(vCodecCtx) for seeking");
            avcodec_flush_buffers(is->vCodecCtx);
            is->flush_vctx = false;
            continue;
        }

        av_packet_unref(packet);

        if (is->videoq.packetGet(packet, m_stop) < 0) {
            break;
        }

        // Decode video frame
        ret = avcodec_send_packet(pCodecCtx, packet);
        if (ret == 0) {
            ret = avcodec_receive_frame(pCodecCtx, pFrame);
        }

        if (packet->dts == AV_NOPTS_VALUE
            && pFrame->opaque && *(uint64_t*)pFrame->opaque != AV_NOPTS_VALUE) {
            pts = (double)*(uint64_t *)pFrame->opaque;
        } else if(packet->dts != AV_NOPTS_VALUE) {
            pts = (double)packet->dts;
        } else {
            pts = 0;
        }
        pts *= av_q2d(is->video_st->time_base);

        // frame ready
        if (ret == 0) {

            ret = sws_scale(is->sws_ctx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0,
                            pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

            pts = synchronize_video(is, pFrame, pts);

            if (ret == pCodecCtx->height) {
                if (queue_picture(is, pFrameRGB, pts) < 0) {
                    break;
                }
            }
        }
    }

    av_frame_free(&pFrame);
    av_frame_free(&pFrameRGB);
    av_packet_free(&packet);

    return 0;
}

int VideoDecodeThread::queue_picture(FFmpegPlayerCtx *is, AVFrame *pFrame, double pts)
{
    VideoPicture *vp;

    // wait until we have space for a new pic
    //is->pictq_mutex->lock();
    std::unique_lock<std::mutex> lock(*is->pictq_mutex);
    while (is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE) {

        /*std::unique_lock<std::mutex> lock(*is->pictq_mutex);
        is->pictq_cond->wait_for(lock,std::chrono::milliseconds(500));*/
        is->pictq_cond->wait_for(lock, std::chrono::milliseconds(500));

        if (m_stop) {
            break;
        }
    }
    //is->pictq_mutex->unlock();

    if (m_stop) {
        return 0;
    }

    // windex is set to 0 initially
    vp = &is->pictq[is->pictq_windex];

    if (!vp->bmp) {
        // 分配内存前先释放锁，避免长时间持有锁
        lock.unlock();
       /* vp->bmp = av_frame_alloc();
        av_image_alloc(vp->bmp->data, vp->bmp->linesize, is->vCodecCtx->width, is->vCodecCtx->height, AV_PIX_FMT_RGB24, 32);*/
        vp->bmp = av_frame_alloc();
        av_image_alloc(vp->bmp->data, vp->bmp->linesize,
            is->vCodecCtx->width, is->vCodecCtx->height,
            AV_PIX_FMT_YUV420P, 32); // 使用 YUV420P

        // 如果需要转换格式，添加 sws_scale 上下文
        if (!is->sws_ctx) {
            is->sws_ctx = sws_getContext(
                is->vCodecCtx->width, is->vCodecCtx->height, is->vCodecCtx->pix_fmt,
                is->vCodecCtx->width, is->vCodecCtx->height, AV_PIX_FMT_YUV420P,
                SWS_BILINEAR, NULL, NULL, NULL);
        }

        // 重新获取锁以继续操作共享资源
        lock.lock();
    }

    // Copy the pic data and set pts
    /*memcpy(vp->bmp->data[0], pFrame->data[0], is->vCodecCtx->height * pFrame->linesize[0]);*/
       // 使用 sws_scale 转换格式
    if (is->sws_ctx) {
        const int stride[] = { pFrame->linesize[0], pFrame->linesize[1], pFrame->linesize[2] };
        sws_scale(is->sws_ctx, pFrame->data, stride, 0, is->vCodecCtx->height,
            vp->bmp->data, vp->bmp->linesize);
    }
    else {
        // 如果格式匹配，直接复制 (不太可能)
        av_frame_copy(vp->bmp, pFrame);
    }
    vp->pts = pts;

    // now we inform our display thread that we have a pic ready
    if(++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE) {
        is->pictq_windex = 0;
    }
  /*  is->pictq_mutex->lock();*/
    is->pictq_size++;
    //is->pictq_mutex->unlock();

    return 0;
}
