#include "videobox.h"
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}
#include "mainwindow.h"
#include <QDebug>
#include "Sound/singlesound.h"

VideoBox::VideoBox(const QString &filePath, BoxesGroup *parent) :
    BoundingBox(parent, TYPE_IMAGE) {
    setName("Video");

    setFilePath(filePath);
}

void VideoBox::updateAfterFrameChanged(int currentFrame) {
    BoundingBox::updateAfterFrameChanged(currentFrame);

    mCurrentVideoFrame = qMin(mFramesCount - 2, mCurrentFrame);
    auto searchCurrentFrame = mVideoFramesCache.find(mCurrentVideoFrame);
    if(searchCurrentFrame == mVideoFramesCache.end()) {
        schedulePixmapReload();
    } else {
        mPixmapReloadScheduled = false;
        scheduleUpdate(false);
    }
}

void VideoBox::afterSuccessfulUpdate() {
    mPixmapReloadScheduled = false;
    if(mUpdatePixmapReloadScheduled) {
        auto searchLastFrame = mVideoFramesCache.find(mUpdateVideoFrame);
        if(searchLastFrame == mVideoFramesCache.end()) {
            mVideoFramesCache.insert({mUpdateVideoFrame, mUpdateVideoImage});
        }
    }
    mRelBoundingRect = mUpdateVideoImage.rect();
}

void VideoBox::updateBoundingRect() {
    //mRelBoundingRect = mOldVideoImage.rect();

    BoundingBox::updateBoundingRect();
}

void VideoBox::drawSelected(QPainter *p,
                            const CanvasMode &)
{
    if(mVisible) {
        p->save();

//        QPainterPath mapped;
//        mapped.addRect(mPixmap.rect());
//        mapped = mCombinedTransformMatrix.map(mapped);
//        QPen pen = p->pen();
//        p->setPen(QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine));
//        p->setBrush(Qt::NoBrush);
//        p->drawPath(mapped);
//        p->setPen(pen);

        drawBoundingRect(p);
        p->restore();
    }
}

bool VideoBox::relPointInsidePath(QPointF point)
{
    return mRelBoundingRect.contains(point.toPoint());
}

void VideoBox::makeDuplicate(BoundingBox *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
}

BoundingBox *VideoBox::createNewDuplicate(BoxesGroup *parent) {
    return new VideoBox(mSrcFilePath, parent);
}

void VideoBox::draw(QPainter *p)
{
    if(mVisible) {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->drawImage(0, 0, mUpdateVideoImage);
    }
}

void VideoBox::updateFrameCount(const char* path) {
    AVFormatContext* format = avformat_alloc_context();
    if (avformat_open_input(&format, path, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return;
    }
    if (avformat_find_stream_info(format, NULL) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return;
    }

    // Find the index of the first audio stream
    for (uint i = 0; i < format->nb_streams; i++) {
        const AVMediaType &mediaType = format->streams[i]->codec->codec_type;
        if(mediaType == AVMEDIA_TYPE_VIDEO) {
            mFramesCount = format->streams[i]->nb_frames;
            break;
        }
    }

    avformat_free_context(format);
}

int VideoBox::getImageAtFrame(const char* path,
                    const int &frameId) {
    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if (avformat_open_input(&format, path, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return -1;
    }
    if (avformat_find_stream_info(format, NULL) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return -1;
    }

    // Find the index of the first audio stream
    int videoStreamIndex = -1;
    for (uint i = 0; i < format->nb_streams; i++) {
        const AVMediaType &mediaType = format->streams[i]->codec->codec_type;
        if(mediaType == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if(videoStreamIndex == -1) {
        fprintf(stderr,
                "Could not retrieve video stream from file '%s'\n", path);
        return -1;
    }
    AVCodecContext *videoCodec = NULL;
    struct SwsContext *sws = NULL;

    AVStream *videoStream = format->streams[videoStreamIndex];
    videoCodec = videoStream->codec;
    if( avcodec_open2(videoCodec, avcodec_find_decoder(videoCodec->codec_id), NULL) < 0 ) {
        fprintf(stderr, "Failed to open decoder for stream #%u in file '%s'\n",
                videoStreamIndex, path);
        return -1;
    }

    sws = sws_getContext(videoCodec->width, videoCodec->height,
                         videoCodec->pix_fmt,
                         videoCodec->width, videoCodec->height,
                         AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);

    // prepare to read data
    AVPacket packet;
    av_init_packet(&packet);
    AVFrame* decodedFrame = av_frame_alloc();
    if (!decodedFrame) {
        fprintf(stderr, "Error allocating the frame\n");
        return -1;
    }

    int tsms = qRound((frameId)*1000*
            videoStream->avg_frame_rate.den/
            (double)videoStream->avg_frame_rate.num);

    int64_t frame = av_rescale(tsms, videoStream->time_base.den,
                               videoStream->time_base.num);

    frame /= 1000;

    if (avformat_seek_file(format, videoStreamIndex, 0,
                           frame, frame,
            AVSEEK_FLAG_FRAME)< 0) {
        return 0;
    }

    avcodec_flush_buffers(videoCodec);

    int64_t pts = 0;

    do {
        if (av_read_frame(format, &packet) >= 0) {
            int gotFrame;
            if (packet.stream_index == videoStreamIndex) {
                avcodec_decode_video2(videoCodec, decodedFrame, &gotFrame,
                &packet);
            }
            av_free_packet(&packet);
        } else {
            break;
        }

        // calculate PTS:
        pts = av_frame_get_best_effort_timestamp (decodedFrame);
        pts = av_rescale_q ( pts, videoStream->time_base, AV_TIME_BASE_Q );
    } while ( pts/1000 <= tsms);

//    if(mImage.width() != videoCodec->width ||
//       mImage.height() != videoCodec->height) {
//        mImage = QImage(videoCodec->width, videoCodec->height,
//                   QImage::Format_RGBA8888);
//    }
    mUpdateVideoImage = QImage(videoCodec->width, videoCodec->height,
                        QImage::Format_RGBA8888);

    /* 2. Convert and write into image buffer  */
    uint8_t *dst[] = {mUpdateVideoImage.bits()};
    int linesizes[4];

    av_image_fill_linesizes(linesizes, AV_PIX_FMT_RGBA, decodedFrame->width);

    sws_scale(sws, decodedFrame->data, decodedFrame->linesize,
              0, videoCodec->height,
              dst, linesizes);


    // clean up
    av_frame_free(&decodedFrame);

    sws_freeContext(sws);
    avcodec_close(videoCodec);

    avformat_free_context(format);

    // success
    return 0;
}

void VideoBox::setUpdateVars() {
    BoundingBox::setUpdateVars();
    mUpdatePixmapReloadScheduled = mPixmapReloadScheduled;
    mUpdateVideoFrame = mCurrentVideoFrame;
    if(!mUpdatePixmapReloadScheduled) {
        auto searchCurrentFrame = mVideoFramesCache.find(mUpdateVideoFrame);
        if(searchCurrentFrame != mVideoFramesCache.end()) {
            mUpdateVideoImage = searchCurrentFrame->second;
        }
    }
}

void VideoBox::schedulePixmapReload() {
    if(mPixmapReloadScheduled) return;
    mPixmapReloadScheduled = true;
    scheduleUpdate();
}

void VideoBox::preUpdatePixmapsUpdates() {
    reloadPixmapIfNeeded();
    BoundingBox::preUpdatePixmapsUpdates();
}

void VideoBox::reloadPixmapIfNeeded() {
    if(mPixmapReloadScheduled) {
        reloadPixmap();
    }
}

void VideoBox::reloadPixmap() {
    if(mSrcFilePath.isEmpty()) {
    } else {
        getImageAtFrame(mSrcFilePath.toLatin1().data(),
                        mUpdateVideoFrame);
    }

    if(!mPivotChanged) centerPivotPosition();
}

void VideoBox::setFilePath(QString path) {
    mSrcFilePath = path;
    reloadFile();
}

void VideoBox::reloadFile() {
    mVideoFramesCache.clear();
    updateFrameCount(mSrcFilePath.toLatin1().data());
    schedulePixmapReload();
    reloadSound();
}

bool hasSound(const char* path) {
    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if (avformat_open_input(&format, path, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return false;
    }
    if(avformat_find_stream_info(format, NULL) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return false;
    }

    // Find the index of the first audio stream
    for (uint i = 0; i < format->nb_streams; i++) {
        const AVMediaType &mediaType = format->streams[i]->codec->codec_type;
        if(mediaType == AVMEDIA_TYPE_AUDIO) {
            return true;
        }
    }

    avformat_free_context(format);

    // success
    return false;
}
#include "Sound/soundcomposition.h"
void VideoBox::reloadSound() {
    if(hasSound(mSrcFilePath.toLatin1().data())) {
        if(mSound == NULL) {
            mSound = new SingleSound(mSrcFilePath);
            getParentCanvas()->getSoundComposition()->addSound(mSound);
        }
        mSound->reloadDataFromFile();
    } else {
    }
}
