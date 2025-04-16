#include <iostream>
#include <vector>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h> 
}

#include "hbs.hpp"
#include "pixelsort.hpp"
#include "quicksort.hpp"
#include "mask.hpp" 
#include "util.hpp"
#include "image_format.hpp"



void pixelsort_image(std::string& input, std::string& output, void (*mask_ptr)(char*, std::vector<pixel_stream>&, int&, int, int, int), void (*rotate_image)(char*, int&, int&, int&), float (*hbs)(char, char, char)) {
   int desired_channels = 3;
   int width = 0; int height = 0; int channels = 0;

  
   char* image = (char*) stbi_load(input.c_str(), &width, &height, &channels, desired_channels);
   if(image == NULL) {
     std::cout << "Error in loading the image" << std::endl;
     exit(1);
   }

   std::string format = get_image_format(input);

   if(format.empty()) {
     std::cout << "Image format is not supported. Supported format are 'png', 'pgm', 'ppm', 'jpg', 'jpeg' and 'bmp'" << std::endl;
     exit(1);
   }

   if(format == "pgm" && hbs != get_brightness) {
     std::cout << "Option not available for image format 'pgm'. Use '--sort-by-brightness' or '-B' instead"<< std::endl;
     exit(1);
   }

   int columns = height;
   int rows = width;
 
   std::vector<pixel_stream> px_str;
   px_str.reserve(1);
   px_str.emplace_back(pixel_stream{width, width});

   if(rotate_image) {
     rotate_image(image, width, height, desired_channels);
     columns = width;
     rows = height;
     px_str[0] = pixel_stream{height, height};
   }

   if(mask_ptr) {
     mask_ptr(image, px_str, columns, rows, columns, desired_channels);
   }

   char* p = image + assert_ptr(mask_ptr) * px_str[0].offset * desired_channels;

   for(int i = 0; i < columns; i++, p += px_str[i * assert_ptr(mask_ptr)].offset * desired_channels) {
        quicksort(p, px_str[i * assert_ptr(mask_ptr)].length * desired_channels, desired_channels, hbs); 
   }

   if(rotate_image) {
     rotate_image(image, height, width, desired_channels);
   }

   switch(const_hash(format.c_str())) {
     case const_hash("png"): stbi_write_png(output.c_str(), width, height, desired_channels, image, width * desired_channels);
			     break;
     case const_hash("pgm"): stbi_write_png(output.c_str(), width, height, desired_channels, image, width * desired_channels);
			     break;
     case const_hash("ppm"): stbi_write_png(output.c_str(), width, height, desired_channels, image, width * desired_channels);
			     break;
     case const_hash("jpg"): stbi_write_jpg(output.c_str(), width, height, desired_channels, image, 100);
			     break;
     case const_hash("bmp"): stbi_write_bmp(output.c_str(), width, height, desired_channels, image);
			     break;
   }
}

void pixelsort_video(std::string& input, std::string& output, void (*mask_ptr)(char*, std::vector<pixel_stream>&, int&, int, int, int), void (*rotate_image)(char*, int&, int&, int&), float (*hbs)(char, char, char)) {

  AVFormatContext *av_format_ctx = avformat_alloc_context();
  if(!av_format_ctx) {
    printf("Could not create AVFormatContext\n");
    exit(1);
  }

  // Open video file
  if(avformat_open_input(&av_format_ctx, input.c_str(), NULL, NULL) != 0) {
    printf("Error in loading the video\n");
    exit(1);
  }

  int video_stream_index = -1;
  AVCodecParameters *av_codec_params;
  const AVCodec *av_codec;

  //Find the first video stream
  for(unsigned int i = 0; i < av_format_ctx->nb_streams; i++) { 
    av_codec_params = av_format_ctx->streams[i]->codecpar;
    av_codec = avcodec_find_decoder(av_codec_params->codec_id);
    if(!av_codec) {
      continue;
    }

    if(av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;
      break;
    }
  }

  if(video_stream_index == -1) {
    printf("Could not find a video stream\n");
    exit(1);
  }

  //Set up a codec context for the decoder
  AVCodecContext *av_codec_ctx = avcodec_alloc_context3(av_codec);
  if(!av_codec_ctx) {
    printf("Could not create AVCodecContext\n");
    exit(1);
  }

  if(avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0) {
    printf("Could not initialize AVCodecContext\n");
    exit(1);
  }

  if(avcodec_open2(av_codec_ctx, av_codec, NULL) < 0) {
    printf("Could not open codec\n");
    exit(1);
  }

  AVFrame *av_frame = av_frame_alloc();
  if(!av_frame) {
    printf("Could not allocate AVFrame\n");
    exit(1);
  }

  AVPacket *av_packet = av_packet_alloc();
  if(!av_packet) {
    printf("Could not allocate AVPacket\n");
    exit(1);
  }

  int response = 0;
  char error[AV_ERROR_MAX_STRING_SIZE];

  while(av_read_frame(av_format_ctx, av_packet) >= 0) {
    if(av_packet->stream_index != video_stream_index) {
      continue;
    }

    response = avcodec_send_packet(av_codec_ctx, av_packet);
    if(response < 0) {
      av_make_error_string(error, AV_ERROR_MAX_STRING_SIZE, response);
      printf("Failed to decode packet: %s\n", error);
      exit(1);
    }

    response = avcodec_receive_frame(av_codec_ctx, av_frame);
    if(response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      continue;
    } else if (response < 0) {
      av_make_error_string(error, AV_ERROR_MAX_STRING_SIZE, response);
      printf("Failed to decode frame: %s\n", error);
      exit(1);
    }

    av_packet_unref(av_packet);
    break;
  }

  printf("Width: %d\n", av_frame->width);
  printf("Height: %d\n", av_frame->height);

  avformat_close_input(&av_format_ctx);
  avformat_free_context(av_format_ctx);
  av_frame_free(&av_frame);
  av_packet_free(&av_packet);
  avcodec_free_context(&av_codec_ctx);
  
}
