#pragma once

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

typedef struct VideoPlayerContext
{
  // AVFormatContext holds the header information from the format (Container)
  // Allocating memory for this component
  // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
  AVFormatContext *pFormatContext;

  AVCodec *video_avc;
  AVCodec *audio_avc;
  AVStream *video_avs;
  AVStream *audio_avs;
  AVCodecContext *video_avcc;
  AVCodecContext *audio_avcc;
  int video_index;
  int audio_index;

  // https://ffmpeg.org/doxygen/trunk/structAVFrame.html
  AVFrame *pFrame;

  // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
  AVPacket *pPacket;
} VideoPlayerContext;

VideoPlayerContext* create(const char* url);

int decode_packet(AVCodecContext *avcc, AVPacket *pPacket, AVFrame *pFrame);

int process_frame(VideoPlayerContext* vpContext);

void destroy(VideoPlayerContext* vpContext);