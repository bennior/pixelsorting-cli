#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h> 

#include "hbs.h"
#include "pixelsort.h"
#include "quicksort.h"
#include "mask.h" 
#include "image_format.h"
#include "pixel_stream_context.h"


enum AVPixelFormat correct_for_deprecated_pixel_format(enum AVPixelFormat pix_fmt) {
    // Fix swscaler deprecated pixel format warning
    // (YUVJ has been deprecated, change pixel format to regular YUV)
    switch (pix_fmt) {
        case AV_PIX_FMT_YUVJ420P: return AV_PIX_FMT_YUV420P;
        case AV_PIX_FMT_YUVJ422P: return AV_PIX_FMT_YUV422P;
        case AV_PIX_FMT_YUVJ444P: return AV_PIX_FMT_YUV444P;
        case AV_PIX_FMT_YUVJ440P: return AV_PIX_FMT_YUV440P;
        default:                  return pix_fmt;
    }
}


void pixelsort_image(const char* input, const char* output, void (*mask)(char*, pixel_stream_context*, int*, int, int, int), void (*rotate_image)(char*, int, int, int), float (*hbs)(char, char, char)) {

   int desired_comp = 3;
   int width = 0; int height = 0; int channels = 0;
  
   char* image = (char*) stbi_load(input, &width, &height, &channels, desired_comp);
   if(image == NULL) {
     printf("Error in loading the image\n");
     exit(1);
   }

   const char* format = get_image_format(input);

   if(strcmp(format, "") == 0) {
     printf("Image format is not supported. Supported format are 'png', 'pgm', 'ppm', 'jpg', 'jpeg' and 'bmp'\n");
     exit(1);
   }

   if(strcmp(format, "pgm") == 0 && hbs != get_brightness) {
     printf("Option not available for image format 'pgm'. Use '--sort-by-brightness' or '-B' instead\n");
     exit(1);
   }

   int columns = height;
   int rows = width;
 
   pixel_stream_context px_str_ctx = {NULL, 0, 0};

   if(rotate_image) {
     rotate_image(image, width, height, desired_comp);
     columns = width;
     rows = height;
   }

   if(mask) {
     int size = 0;

     mask(image, &px_str_ctx, &size, rows, columns, desired_comp);

     char* p = image + px_str_ctx.array[0].offset * desired_comp;

     for(int i = 0; i < size; i++, p += px_str_ctx.array[i].offset * desired_comp) {
       quicksort(p, px_str_ctx.array[i].length * desired_comp, desired_comp, hbs);
     }

   } else {

     for(int i = 0; i < columns; i++) {
       quicksort(image + rows * desired_comp * i, rows * desired_comp, desired_comp, hbs);
     }

   }

   free_arr(&px_str_ctx);

   if(rotate_image) {
     rotate_image(image, height, width, desired_comp);
   }

   if(strcmp(format, "png") == 0)
    stbi_write_png(output, width, height, desired_comp, image, width * desired_comp);
   if(strcmp(format, "pgm") == 0)
    stbi_write_png(output, width, height, desired_comp, image, width * desired_comp);
   if(strcmp(format, "ppm") == 0)
    stbi_write_png(output, width, height, desired_comp, image, width * desired_comp);
   if(strcmp(format, "jpg") == 0)
    stbi_write_jpg(output, width, height, desired_comp, image, 100);
   if(strcmp(format, "bmp") == 0)
    stbi_write_bmp(output, width, height, desired_comp, image);
}

void pixelsort_video(const char* input, const char* output, void (*mask)(char*, pixel_stream_context*, int*, int, int, int), void (*rotate_image)(char*, int, int, int), float (*hbs)(char, char, char)) {

  AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
  const AVCodec *decoder = NULL, *encoder = NULL;
  AVCodecContext *dec_ctx = NULL, *enc_ctx = NULL;
  AVPacket *dec_pkt = NULL, *enc_pkt = NULL;
  AVFrame *dec_frame = NULL, *enc_frame = NULL;
  SwsContext *sws_fmt_to_rgb = NULL, *sws_rgb_to_fmt = NULL;
  enum AVPixelFormat source_pix_fmt;

  int video_stream_index = -1;
  int stream_index = 0;
  int ret = 0;

  //allocate input format context
  ifmt_ctx = avformat_alloc_context();
  if(!ifmt_ctx) {
    av_log(NULL, AV_LOG_ERROR, "Could not allocate (AVFormatContext) ifmt_ctx\n");
    exit(1);
  }

  //open video file
  if(avformat_open_input(&ifmt_ctx, input, NULL, NULL) != 0) {
    av_log(NULL, AV_LOG_ERROR, "Could not open the video file\n");
    exit(1);
  }

  //find stream info
  if(avformat_find_stream_info(ifmt_ctx, NULL) < 0) {
    av_log(NULL, AV_LOG_ERROR, "Could not find stream info\n");
    exit(1);
  }

  //allocate output format context
  avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, output);
  if (!ofmt_ctx) {
    av_log(NULL, AV_LOG_ERROR, "Could not allocate (AVFormatContext) ofmt_ctx\n");
    exit(1);
  }

  for(unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
    AVStream* out_stream;
    AVStream* in_stream = ifmt_ctx->streams[i];
    AVCodecParameters *codecpar = in_stream->codecpar;

    //allocate out_stream
    out_stream = avformat_new_stream(ofmt_ctx, NULL);
    if(!out_stream) {
      printf("Failed allocating output stream\n");
      exit(1);
    }

    //find video stream
    if(codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;
      
      //find decoder
      decoder = avcodec_find_decoder(codecpar->codec_id);
      if(!decoder) {
	av_log(NULL, AV_LOG_ERROR, "Could not find (AVCodec) decoder\n");
	exit(1);
      }

      //allocate decoder ctx
      dec_ctx = avcodec_alloc_context3(decoder);
      if(!dec_ctx) {
	av_log(NULL, AV_LOG_ERROR, "Could not allocate (AVCodecContext) dec_ctx\n");
	exit(1);
      }

      //copy codec parameters to codec context
      if(avcodec_parameters_to_context(dec_ctx, codecpar) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Could not initalize (AVCodecContext) dec_ctx\n");
        exit(1);
      }

      dec_ctx->pkt_timebase = in_stream->time_base; 
      dec_ctx->framerate = av_guess_frame_rate(ifmt_ctx, in_stream, NULL);

      //open decoder
      if(avcodec_open2(dec_ctx, decoder, NULL) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Could not open (AVCodec) decoder\n");
	exit(1);
      }


      //find encoder
      encoder = avcodec_find_encoder(dec_ctx->codec_id);
      if(!encoder) {
	av_log(NULL, AV_LOG_ERROR, "Could not find (AVCodec) encoder\n");
	exit(1);
      }

      //allocate encoder ctx
      enc_ctx = avcodec_alloc_context3(encoder);
      if(!enc_ctx) {
	av_log(NULL, AV_LOG_ERROR, "Could not allocate (AVCodecContext) enc_ctx\n");
	exit(1);
      }
      
      const enum AVPixelFormat * pix_fmts = NULL;

      enc_ctx->height = dec_ctx->height;
      enc_ctx->width = dec_ctx->width;
      enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;

      ret = avcodec_get_supported_config(dec_ctx, NULL, AV_CODEC_CONFIG_PIX_FORMAT, 0,
					(const void**)&pix_fmts, NULL);

      enc_ctx->pix_fmt = (ret >= 0 && pix_fmts) ? pix_fmts[0] : dec_ctx->pix_fmt;
      enc_ctx->time_base = av_inv_q(dec_ctx->framerate);

      if(ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

      //open encoder
      if(avcodec_open2(enc_ctx, encoder, NULL)) {
	av_log(NULL, AV_LOG_ERROR, "Could not open (AVCodec) encoder\n");
	exit(1);
      }

      //initialize out_stream->codec_params
      if(avcodec_parameters_from_context(out_stream->codecpar, enc_ctx)) {
	av_log(NULL, AV_LOG_ERROR, "Could not copy AVCodecParameters from (AVCodecContext) enc_ctx to (AVStream) out_stream");
        exit(1);
      }
      
      out_stream->time_base = enc_ctx->time_base;

    }else if(codecpar->codec_type == AVMEDIA_TYPE_AUDIO || codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {

      //remux
      if(avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Could not copy AVCodecParameters to (AVStream) out_stream");
	exit(1);
      }

      out_stream->time_base = in_stream->time_base;
    }else {
      //terminate
      av_log(NULL, AV_LOG_FATAL, "Elementary stream is of unknown type, cannot proceed\n");
      exit(1);
    }
  }

  if(video_stream_index < 0) {
    av_log(NULL, AV_LOG_ERROR, "Could not find video stream");
    exit(1);
  }

  //open output file
  if(!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
    if(avio_open(&ofmt_ctx->pb, output, AVIO_FLAG_WRITE)) {
      av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", output);
      exit(1);
    }
  }

  //write file header
  if(avformat_write_header(ofmt_ctx, NULL)) {
    av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
    exit(1);
  }

  //allocate frame
  dec_frame = av_frame_alloc();
  enc_frame = av_frame_alloc();

  if(!dec_frame || !enc_frame) {
    av_log(NULL, AV_LOG_ERROR, "Could not allocate AVFrame\n");
    exit(1);
  }

  enc_frame->format = enc_ctx->pix_fmt;
  enc_frame->width = dec_ctx->width;
  enc_frame->height = dec_ctx->height;

  if(av_frame_get_buffer(enc_frame, 32) < 0) {
    av_log(NULL, AV_LOG_ERROR, "Could not allocate buffer for enc_frame\n");
    exit(1);
  }

  //allocate packet
  dec_pkt = av_packet_alloc();
  enc_pkt = av_packet_alloc();

  if(!dec_pkt || !enc_pkt) {
    av_log(NULL, AV_LOG_ERROR, "Could not allocate AVPacket\n");
    exit(1);
  }

  source_pix_fmt = correct_for_deprecated_pixel_format(dec_ctx->pix_fmt);      

  sws_fmt_to_rgb = sws_getContext(dec_ctx->width, dec_ctx->height, source_pix_fmt,
				  dec_ctx->width, dec_ctx->height, AV_PIX_FMT_RGB24,
				  SWS_FAST_BILINEAR, NULL, NULL, NULL);
  if(!sws_fmt_to_rgb) {
    av_log(NULL, AV_LOG_ERROR, "Could not initialze SwsContext\n");
    exit(1);
  }
  
  sws_rgb_to_fmt = sws_getContext(dec_ctx->width, dec_ctx->height, AV_PIX_FMT_RGB24,
				  dec_ctx->width, dec_ctx->height, enc_ctx->pix_fmt,
				  SWS_FAST_BILINEAR, NULL, NULL, NULL);
  if(!sws_rgb_to_fmt) {
    av_log(NULL, AV_LOG_ERROR, "Could not initialze SwsContext\n");
    exit(1);
  }

  int rgb_size = dec_ctx->height * dec_ctx->width * 3;
  unsigned char *rgb_buffer = (u_int8_t*) av_malloc(rgb_size);
  unsigned char* dst_data[4] = {rgb_buffer, NULL, NULL, NULL};
  int dst_linesize[4] = {dec_ctx->width * 3, 0, 0, 0}; 

  char error[AV_ERROR_MAX_STRING_SIZE];

  while(av_read_frame(ifmt_ctx, dec_pkt) >= 0) {

    stream_index = dec_pkt->stream_index;

    if(dec_pkt->stream_index == video_stream_index) {

      if((ret = avcodec_send_packet(dec_ctx, dec_pkt)) < 0) {
	av_make_error_string(error, AV_ERROR_MAX_STRING_SIZE, ret);
 	av_log(NULL, AV_LOG_ERROR, "Failed to decode packet: %s\n", error);
 	goto end;
      }

      ret = avcodec_receive_frame(dec_ctx, dec_frame);
      if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
	continue;
      } else if (ret < 0) {
	av_log(NULL, AV_LOG_ERROR, "Failed to receive frame\n");
	goto end;
      }

      dec_frame->pts = dec_frame->best_effort_timestamp;

      //converting to rgb24
      sws_scale(sws_fmt_to_rgb, (const u_int8_t* const*)dec_frame->data, dec_frame->linesize, 0, dec_frame->height, dst_data, dst_linesize);

      for(int i = 0; i < rgb_size; i += 3) {
	dst_data[0][i] = 255;
	dst_data[0][i + 1] = 255;
	dst_data[0][i + 2] = 0;
      }
      
      //convert back to frame
      sws_scale(sws_rgb_to_fmt, (const u_int8_t* const*)dst_data, dst_linesize, 0, dec_frame->height, enc_frame->data, enc_frame->linesize);
      
      enc_frame->time_base = dec_frame->time_base;
      enc_frame->pict_type = AV_PICTURE_TYPE_NONE;
      enc_frame->pts = av_rescale_q(dec_frame->pts, ifmt_ctx->streams[video_stream_index]->time_base, enc_ctx->time_base);

      if(avcodec_send_frame(enc_ctx, enc_frame) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Failed to encode frame\n");
	goto end;
      }
      
      ret = avcodec_receive_packet(enc_ctx, enc_pkt);
      if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
	continue;
      } else if (ret < 0) {
	av_log(NULL, AV_LOG_ERROR, "Failed to receive packet\n");
	goto end;
      }

      enc_pkt->stream_index = stream_index;
      
      av_packet_rescale_ts(enc_pkt, enc_ctx->time_base, ofmt_ctx->streams[stream_index]->time_base);

      if(av_interleaved_write_frame(ofmt_ctx, enc_pkt) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Failed to write packet to (AVFormatContext) ofmt_ctx");
	goto end;
      }
    }else {
      //remux
      av_packet_rescale_ts(dec_pkt, ifmt_ctx->streams[stream_index]->time_base, ofmt_ctx->streams[stream_index]->time_base);

      if(av_interleaved_write_frame(ofmt_ctx, dec_pkt)) {
	av_log(NULL, AV_LOG_ERROR, "Failed to write packet to (AVFormatContext) ofmt_ctx");
	goto end;
      }
    }

     av_packet_unref(dec_pkt);
     av_packet_unref(enc_pkt);
  }

  av_packet_unref(dec_pkt);
  av_packet_unref(enc_pkt);

  av_frame_unref(dec_frame);
  av_frame_unref(enc_frame);

  //flush decoder and encoder
  avcodec_send_packet(dec_ctx, NULL);
  avcodec_send_packet(enc_ctx, NULL);

  //finalizes the output file
  av_write_trailer(ofmt_ctx);

end:

  //freeing memory
  av_free(rgb_buffer);
  av_packet_free(&dec_pkt);
  av_packet_free(&enc_pkt);
  av_frame_free(&dec_frame);
  av_frame_free(&enc_frame);
  avcodec_free_context(&dec_ctx);
  avcodec_free_context(&enc_ctx);
  sws_free_context(&sws_rgb_to_fmt);
  sws_free_context(&sws_fmt_to_rgb);

  avformat_close_input(&ifmt_ctx);

  if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    avio_closep(&ofmt_ctx->pb);
  avformat_free_context(ofmt_ctx);

}

void pixelsort_video2(const char* input, const char* output, void (*mask)(char*, pixel_stream_context*, int*, int, int, int), void (*rotate_image)(char*, int, int, int), float (*hbs)(char, char, char)) {

  AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
  const AVCodec *decoder = NULL, *encoder = NULL;
  AVCodecContext *dec_ctx = NULL, *enc_ctx = NULL;
  AVPacket *dec_pkt = NULL, *enc_pkt = NULL;
  AVFrame *dec_frame = NULL, *enc_frame = NULL;
  SwsContext *sws_fmt_to_rgb = NULL, *sws_rgb_to_fmt = NULL;
  enum AVPixelFormat source_pix_fmt;

  int video_stream_index = -1;
  int ret = 0;

  //allocate input format context
  ifmt_ctx = avformat_alloc_context();
  if(!ifmt_ctx) {
    av_log(NULL, AV_LOG_ERROR, "Could not allocate (AVFormatContext) ifmt_ctx\n");
    exit(1);
  }

  //open video file
  if(avformat_open_input(&ifmt_ctx, input, NULL, NULL) != 0) {
    av_log(NULL, AV_LOG_ERROR, "Could not open the video file\n");
    exit(1);
  }

  //find stream info
  if(avformat_find_stream_info(ifmt_ctx, NULL) < 0) {
    av_log(NULL, AV_LOG_ERROR, "Could not find stream info\n");
    exit(1);
  }

  //allocate output format context
  avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, output);
  if (!ofmt_ctx) {
    av_log(NULL, AV_LOG_ERROR, "Could not allocate (AVFormatContext) ofmt_ctx\n");
    exit(1);
  }

  for(unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
    AVStream* out_stream;
    AVStream* in_stream = ifmt_ctx->streams[i];
    AVCodecParameters *codecpar = in_stream->codecpar;

    //allocate out_stream
    out_stream = avformat_new_stream(ofmt_ctx, NULL);
    if(!out_stream) {
      av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
      exit(1);
    }

    //find video stream
    if(codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;

      //find decoder
      decoder = avcodec_find_decoder(codecpar->codec_id);
      if(!decoder) {
	av_log(NULL, AV_LOG_ERROR, "Could not find (AVCodec) decoder\n");
	exit(1);
      }

      //allocate decoder ctx
      dec_ctx = avcodec_alloc_context3(decoder);
      if(!dec_ctx) {
	av_log(NULL, AV_LOG_ERROR, "Could not allocate (AVCodecContext) dec_ctx\n");
	exit(1);
      }

      //copy codec parameters to decoder context
      if(avcodec_parameters_to_context(dec_ctx, codecpar) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Could not initalize (AVCodecContext) dec_ctx\n");
        exit(1);
      }

      //open decoder
      if(avcodec_open2(dec_ctx, decoder, NULL) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Could not open (AVCodec) decoder\n");
	exit(1);
      }



      //find encoder
      encoder = avcodec_find_encoder(codecpar->codec_id);
      if(!encoder) {
	av_log(NULL, AV_LOG_ERROR, "Could not find (AVCodec) encoder\n");
	exit(1);
      }

      //allocate encoder context
      enc_ctx = avcodec_alloc_context3(encoder);
      if(!enc_ctx) {
	av_log(NULL, AV_LOG_ERROR, "Could not allocate (AVCodecContext) enc_ctx\n");
	exit(1);
      }

      //copy codec parameters to enc ctx
      if(avcodec_parameters_to_context(enc_ctx, codecpar) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Could not initialize (AVCodecContext) enc_ctx\n");
	exit(1);
      }

      //initialize enc_ctx->time_base
      AVRational input_frame_rate = av_guess_frame_rate(ifmt_ctx, in_stream, NULL);
      enc_ctx->time_base = av_inv_q(input_frame_rate);

      //open encoder
      if(avcodec_open2(enc_ctx, encoder, NULL)) {
	av_log(NULL, AV_LOG_ERROR, "Could not open (AVCodec) encoder\n");
	exit(1);
      }

      if(avcodec_parameters_copy(out_stream->codecpar, codecpar) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Failed to copy codec parameters\n");
	exit(1);
      }

    } else if(codecpar->codec_type == AVMEDIA_TYPE_AUDIO || codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {

      //audio or subtitle stream

      if(avcodec_parameters_copy(out_stream->codecpar, codecpar) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Failed to copy codec parameters\n");
	exit(1);
      }

    }else {

      //different streams
      av_log(NULL, AV_LOG_FATAL, "Elementary stream is of unknown type, cannot proceed\n");
      exit(1);
    }
  }

  if(video_stream_index < 0) {
    av_log(NULL, AV_LOG_ERROR, "Could not find video stream");
    exit(1);
  }

  if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
    //opens pb (byte stream) using avio open and sets up output file for writing
    if(avio_open(&ofmt_ctx->pb, output, AVIO_FLAG_WRITE) < 0) {
      av_log(NULL, AV_LOG_ERROR, "Could not open output file\n");
      exit(1);
    }
  }

  //writes container header for guessed container in avformat_alloc_output_context2()
  if (avformat_write_header(ofmt_ctx, NULL) < 0) {
    av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
    exit(1);
  }

  //allocate frame
  dec_frame = av_frame_alloc();
  enc_frame = av_frame_alloc();

  if(!dec_frame || !enc_frame) {
    av_log(NULL, AV_LOG_ERROR, "Could not allocate AVFrame\n");
    exit(1);
  }

  enc_frame->format = enc_ctx->pix_fmt;
  enc_frame->width = dec_ctx->width;
  enc_frame->height = dec_ctx->height;

  if(av_frame_get_buffer(enc_frame, 32) < 0) {
    av_log(NULL, AV_LOG_ERROR, "Could not allocate buffer for enc_frame\n");
    exit(1);
  }

  //allocate packet
  dec_pkt = av_packet_alloc();
  enc_pkt = av_packet_alloc();

  if(!dec_pkt || !enc_pkt) {
    av_log(NULL, AV_LOG_ERROR, "Could not allocate AVPacket\n");
    exit(1);
  }

  source_pix_fmt = correct_for_deprecated_pixel_format(dec_ctx->pix_fmt);      

  sws_fmt_to_rgb = sws_getContext(dec_ctx->width, dec_ctx->height, source_pix_fmt,
				  dec_ctx->width, dec_ctx->height, AV_PIX_FMT_RGB24,
				  SWS_FAST_BILINEAR, NULL, NULL, NULL);
  if(!sws_fmt_to_rgb) {
    av_log(NULL, AV_LOG_ERROR, "Could not initialize SwsContext\n");
    exit(1);
  }
  
  sws_rgb_to_fmt = sws_getContext(dec_ctx->width, dec_ctx->height, AV_PIX_FMT_RGB24,
				  dec_ctx->width, dec_ctx->height, enc_ctx->pix_fmt,
				  SWS_FAST_BILINEAR, NULL, NULL, NULL);
  if(!sws_rgb_to_fmt) {
    av_log(NULL, AV_LOG_ERROR, "Could not initialize SwsContext\n");
    exit(1);
  }

  int rgb_size = dec_ctx->height * dec_ctx->width * 3;
  unsigned char *rgb_buffer = (u_int8_t*) av_malloc(rgb_size);
  unsigned char* dst_data[4] = {rgb_buffer, NULL, NULL, NULL};
  int dst_linesize[4] = {dec_ctx->width * 3, 0, 0, 0}; 

  while(av_read_frame(ifmt_ctx, dec_pkt) >= 0) {

    AVStream *in_stream, *out_stream;

    if(dec_pkt->stream_index == video_stream_index) {

      if(avcodec_send_packet(dec_ctx, dec_pkt) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Failed to decode packet\n");
	goto end;
      }

      ret = avcodec_receive_frame(dec_ctx, dec_frame);
      if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
	  continue;
	} else if (ret < 0) {
	  av_log(NULL, AV_LOG_ERROR, "Failed to receive frame\n");
	  goto end;
      }

      enc_frame->pts = dec_frame->pts;
      
      //converting to rgb24
      sws_scale(sws_fmt_to_rgb, (const u_int8_t* const*)dec_frame->data, dec_frame->linesize, 0, dec_frame->height, dst_data, dst_linesize);

      for(int i = 0; i < rgb_size; i += 3) {
	dst_data[0][i] = 0;
	dst_data[0][i + 1] = 255;
	dst_data[0][i + 2] = 255;
      }
      sws_scale(sws_rgb_to_fmt, (const u_int8_t* const*)dst_data, dst_linesize, 0, dec_frame->height, enc_frame->data, enc_frame->linesize);

      if(avcodec_send_frame(enc_ctx, enc_frame) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Failed to encode frame\n");
        goto end;
      }

      ret = avcodec_receive_packet(enc_ctx, enc_pkt);
      if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
	  continue;
	} else if (ret < 0) {
	  av_log(NULL, AV_LOG_ERROR, "Failed to receive packet\n");
	  goto end;
      }

      //assings the streams
      in_stream  = ifmt_ctx->streams[dec_pkt->stream_index];

      enc_pkt->stream_index = dec_pkt->stream_index;

      out_stream = ofmt_ctx->streams[enc_pkt->stream_index];

      av_packet_rescale_ts(enc_pkt, in_stream->time_base, out_stream->time_base);

      enc_pkt->pos = -1;

      if(av_interleaved_write_frame(ofmt_ctx, enc_pkt) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Error muxing packet\n");
	goto end;
      }

    }else {
     //remuxing

     //assigns the streams
     in_stream  = ifmt_ctx->streams[dec_pkt->stream_index];
     out_stream = ofmt_ctx->streams[dec_pkt->stream_index];

     av_packet_rescale_ts(dec_pkt, in_stream->time_base, out_stream->time_base);

     enc_pkt->pos = -1;

     if(av_interleaved_write_frame(ofmt_ctx, dec_pkt) < 0) {
       av_log(NULL, AV_LOG_ERROR, "Error muxing packet\n");
       goto end;
     }
    }

    av_packet_unref(dec_pkt);
    av_packet_unref(enc_pkt);
  }
  
  av_packet_unref(dec_pkt);
  av_packet_unref(enc_pkt);

  av_frame_unref(dec_frame);
  av_frame_unref(enc_frame);

  //flush decoder and encoder
  avcodec_send_packet(dec_ctx, NULL);
  avcodec_send_packet(enc_ctx, NULL);

  //finalizes the output file
  av_write_trailer(ofmt_ctx);

end:

  //freeing memory
  av_free(rgb_buffer);
  av_packet_free(&dec_pkt);
  av_packet_free(&enc_pkt);
  av_frame_free(&dec_frame);
  av_frame_free(&enc_frame);
  avcodec_free_context(&dec_ctx);
  avcodec_free_context(&enc_ctx);
  sws_free_context(&sws_rgb_to_fmt);
  sws_free_context(&sws_fmt_to_rgb);

  avformat_close_input(&ifmt_ctx);

  if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    avio_closep(&ofmt_ctx->pb);
  avformat_free_context(ofmt_ctx);

  printf("Success\n");
}
