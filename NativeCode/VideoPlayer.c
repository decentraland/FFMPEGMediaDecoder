#include "VideoPlayer.h"
#include "Logger.h"

#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>

static int lastID = 0;

int fill_stream_info(AVStream *avs, AVCodec **avc, AVCodecContext **avcc) {
  *avc = avcodec_find_decoder(avs->codecpar->codec_id);
  if (!*avc) {logging("failed to find the codec"); return -1;}

  *avcc = avcodec_alloc_context3(*avc);
  if (!*avcc) {logging("failed to alloc memory for codec context"); return -1;}

  if (avcodec_parameters_to_context(*avcc, avs->codecpar) < 0) {logging("failed to fill codec context"); return -1;}

  if (avcodec_open2(*avcc, *avc, NULL) < 0) {logging("failed to open codec"); return -1;}
  return 0;
}

void prepare_swr(VideoPlayerContext *vpc) {
  
}

int prepare_decoder(VideoPlayerContext *vpc) {
  logging("preparing decoder");
  for (int i = 0; i < vpc->pFormatContext->nb_streams; i++) {
    if (vpc->pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      vpc->video_avs = vpc->pFormatContext->streams[i];
      vpc->video_index = i;

      if (fill_stream_info(vpc->video_avs, &vpc->video_avc, &vpc->video_avcc)) {return -1;}
    } else if (vpc->pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      vpc->audio_avs = vpc->pFormatContext->streams[i];
      vpc->audio_index = i;

      if (fill_stream_info(vpc->audio_avs, &vpc->audio_avc, &vpc->audio_avcc)) {return -1;}
    } else {
      logging("skipping streams other than audio and video");
    }
  }

  logging("decoder prepared");
  return 0;
}

VideoPlayerContext* create(const char* url)
{
  VideoPlayerContext* vpc = (VideoPlayerContext*) calloc(1, sizeof(VideoPlayerContext));
  logging("initializing all the containers, codecs and protocols.");

  // AVFormatContext holds the header information from the format (Container)
  // Allocating memory for this component
  // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
  vpc->pFormatContext = avformat_alloc_context();
  if (!vpc->pFormatContext) {
    logging("ERROR could not allocate memory for Format Context");
    return NULL;
  }

  logging("opening the input file (%s) and loading format (container) header", url);
  // Open the file and read its header. The codecs are not opened.
  // The function arguments are:
  // AVFormatContext (the component we allocated memory for),
  // url (filename),
  // AVInputFormat (if you pass NULL it'll do the auto detect)
  // and AVDictionary (which are options to the demuxer)
  // http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
  if (avformat_open_input(&vpc->pFormatContext, url, NULL, NULL) != 0) {
    logging("ERROR could not open the file");
    return NULL;
  }

  // now we have access to some information about our file
  // since we read its header we can say what format (container) it's
  // and some other information related to the format itself.
  logging("format %s, duration %lld us, bit_rate %lld", vpc->pFormatContext->iformat->name, vpc->pFormatContext->duration, vpc->pFormatContext->bit_rate);

  logging("finding stream info from format");
  // read Packets from the Format to get stream information
  // this function populates vpc->pFormatContext->streams
  // (of size equals to vpc->pFormatContext->nb_streams)
  // the arguments are:
  // the AVFormatContext
  // and options contains options for codec corresponding to i-th stream.
  // On return each dictionary will be filled with options that were not found.
  // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
  if (avformat_find_stream_info(vpc->pFormatContext,  NULL) < 0) {
    logging("ERROR could not get the stream info");
    return NULL;
  }

  if (prepare_decoder(vpc)) {
    logging("ERROR could not prepare the decoder");
    return NULL;
  }
  // https://ffmpeg.org/doxygen/trunk/structAVFrame.html
  AVFrame *pFrame = av_frame_alloc();
  if (!pFrame)
  {
    logging("failed to allocated memory for AVFrame");
    return NULL;
  }
  // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
  AVPacket *pPacket = av_packet_alloc();
  if (!pPacket)
  {
    logging("failed to allocated memory for AVPacket");
    return NULL;
  }

  vpc->pFrame = pFrame;
  vpc->pPacket = pPacket;
  return vpc;
}


static void save_ppm_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
  FILE *f;
  int i;
  f = fopen(filename, "w");
  // writing the minimal required header for a pgm file format
  // portable graymap format -> https://en.wikipedia.org/wiki/Netpbm_format#PGM_example
  fprintf(f, "P6\n%d %d\n%d\n", xsize, ysize, 255);

  // writing line by line
  for (i = 0; i < ysize; i++)
      fwrite(buf + i * wrap, 3, xsize, f);
  fclose(f);
}


void convert_to_rgb24(AVFrame *srcFrame, int frameNumber)
{
  int width = srcFrame->width;
  int height = srcFrame->height;

  enum AVPixelFormat dstFormat = AV_PIX_FMT_RGB24;
  int bufSize  = av_image_get_buffer_size(dstFormat, width, height, 1);
  uint8_t *buf = (uint8_t*) av_malloc(bufSize);

  AVFrame *dstFrame = av_frame_alloc();

  av_frame_copy_props(dstFrame, srcFrame);
  dstFrame->format = dstFormat;

  av_image_fill_arrays(dstFrame->data, dstFrame->linesize, buf, dstFormat, width, height, 1);

  struct SwsContext* conversion = sws_getContext(width,
                                          height,
                                          srcFrame->format,
                                          width,
                                          height,
                                          dstFormat,
                                          SWS_FAST_BILINEAR,
                                          NULL,
                                          NULL,
                                          NULL);
  sws_scale(conversion, (const uint8_t**)srcFrame->data, srcFrame->linesize, 0, height, dstFrame->data, dstFrame->linesize);
  sws_freeContext(conversion);

  dstFrame->format = dstFormat;
  dstFrame->width = srcFrame->width;
  dstFrame->height = srcFrame->height;

  char frame_filename[1024];
  snprintf(frame_filename, sizeof(frame_filename), "out/%s-%d.ppm", "frame", frameNumber);

  save_ppm_frame(dstFrame->data[0], dstFrame->linesize[0], dstFrame->width, dstFrame->height, frame_filename);
}

int decode_packet(AVCodecContext *avcc, AVPacket *pPacket, AVFrame *pFrame)
{
  // Supply raw packet data as input to a decoder
  // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
  int response = avcodec_send_packet(avcc, pPacket);

  if (response < 0) {
    logging("Error while sending a packet to the decoder: %s", av_err2str(response));
    return response;
  }

  while (response >= 0)
  {
    // Return decoded output data (into a frame) from a decoder
    // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
    response = avcodec_receive_frame(avcc, pFrame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      logging("Error while receiving a frame from the decoder: %s", av_err2str(response));
      return response;
    }

    if (response >= 0) {
      logging(
          "Frame %d (type=%c, size=%d bytes, format=%d) pts %d key_frame %d [DTS %d]",
          avcc->frame_number,
          av_get_picture_type_char(pFrame->pict_type),
          pFrame->pkt_size,
          pFrame->format,
          pFrame->pts,
          pFrame->key_frame,
          pFrame->coded_picture_number
      );
      return 0;
    }
  }
  return -1;
}

void process_video_frame(AVFrame* frame, int frameNumber)
{
  convert_to_rgb24(frame, frameNumber);
}

void process_audio_frame(AVFrame* frame, int frameNumber)
{

}

int process_frame(VideoPlayerContext* vpContext)
{
  int res = -1;
  // fill the Packet with data from the Stream
  // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
  if (av_read_frame(vpContext->pFormatContext, vpContext->pPacket) >= 0)
  {
    if (vpContext->pPacket->stream_index == vpContext->video_index) {
      logging("[VIDEO] AVPacket->pts %" PRId64, vpContext->pPacket->pts);
      res = decode_packet(vpContext->video_avcc, vpContext->pPacket, vpContext->pFrame);
      if (res < 0)
        return -1;
      process_video_frame(vpContext->pFrame, vpContext->audio_avcc->frame_number);
    } else if (vpContext->pPacket->stream_index == vpContext->audio_index) {
      logging("[AUDIO] AVPacket->pts %" PRId64, vpContext->pPacket->pts);
      res = decode_packet(vpContext->audio_avcc, vpContext->pPacket, vpContext->pFrame);
      if (res < 0)
        return -1;
      process_audio_frame(vpContext->pFrame, vpContext->audio_avcc->frame_number);
    }

    res = 0;
    // https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html#ga63d5a489b419bd5d45cfd09091cbcbc2
    av_packet_unref(vpContext->pPacket);
  }

  return res;
}

void destroy(VideoPlayerContext* vpContext)
{
  logging("releasing all the resources");

  avformat_close_input(&vpContext->pFormatContext);
  av_packet_free(&vpContext->pPacket);
  av_frame_free(&vpContext->pFrame);
  avcodec_free_context(&vpContext->video_avcc);
  avcodec_free_context(&vpContext->audio_avcc);

  free(vpContext);
}