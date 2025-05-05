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

  const AVOutputFormat *ofmt = NULL;
  AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
  AVCodecParameters *video_codec_params = NULL;
  AVCodec *decoder = NULL, *encoder = NULL;
  AVCodecContext *decoder_ctx = NULL, *encoder_ctx = NULL;
  AVPacket *pkt = NULL, *out_pkt = NULL;
  AVFrame *frame = NULL;
  int stream_index = 0, video_stream_index = -1;
  int* stream_mapping = NULL;
  int stream_mapping_size = 0;


  //allocate input format context
  ifmt_ctx = avformat_alloc_context();
  if(!ifmt_ctx) {
    printf("Could not allocate InputFormatContext\n");
    exit(1);
  }

  //open video file
  if(avformat_open_input(&ifmt_ctx, input.c_str(), NULL, NULL) != 0) {
    printf("Could not open the video file\n");
    exit(1);
  }

  //find stream info
  if(avformat_find_stream_info(ifmt_ctx, NULL) < 0) {
    printf("Could not find stream info\n");
    exit(1);
  }

  //allocate output format context
  avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, output.c_str());
  if (!ofmt_ctx) {
    printf("Could not allocate OutputFormatContext\n");
    exit(1);
  }

  stream_mapping_size = ifmt_ctx->nb_streams;
  stream_mapping = (int*) av_calloc(stream_mapping_size, sizeof(*stream_mapping));
  if (!stream_mapping) {
    exit(1);
  }

  ofmt = ofmt_ctx->oformat;

  for(unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
    AVStream* out_stream;
    AVStream* in_stream = ifmt_ctx->streams[i];
    AVCodecParameters *in_codecpar = in_stream->codecpar;

    //find video stream
    if(in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;
      video_codec_params = in_stream->codecpar;
      decoder = (AVCodec*) avcodec_find_decoder(in_codecpar->codec_id);
      encoder = (AVCodec*) avcodec_find_encoder(in_codecpar->codec_id);
      if(!decoder || !encoder) {
	printf("Could not find codec\n");
	exit(1);
      }

      stream_mapping[i] = stream_index++;

      out_stream = avformat_new_stream(ofmt_ctx, NULL);
      if(!out_stream) {
	printf("Failed allocating output stream\n");
	exit(1);
      }

      if(avcodec_parameters_copy(out_stream->codecpar, in_codecpar) < 0) {
	printf("Failed to copy codec parameters\n");
	exit(1);
      }

      out_stream->codecpar->codec_tag = 0;

      continue;
    }

    //audio or subtitle stream
    if(in_codecpar->codec_type == AVMEDIA_TYPE_AUDIO || in_codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
      stream_mapping[i] = stream_index++;

      out_stream = avformat_new_stream(ofmt_ctx, NULL);
      if(!out_stream) {
	printf("Failed allocating output stream\n");
	exit(1);
      }

      if(avcodec_parameters_copy(out_stream->codecpar, in_codecpar) < 0) {
	printf("Failed to copy codec parameters\n");
	exit(1);
      }

      out_stream->codecpar->codec_tag = 0;

      continue;
    }

    //different stream
    stream_mapping[i] = -1;
  }

  //allocate codec context
  encoder_ctx = avcodec_alloc_context3(encoder);
  decoder_ctx = avcodec_alloc_context3(decoder);
  if(!encoder_ctx || !decoder_ctx) {
    printf("Could not allocate AVCodecContext\n");
    exit(1);
  }

  //copy codec parameters to codec context
  if(avcodec_parameters_to_context(encoder_ctx, video_codec_params) < 0) {
    printf("Could not initalize EncoderContext\n");
    exit(1);
  }
  if(avcodec_parameters_to_context(decoder_ctx, video_codec_params) < 0) {
    printf("Could not initalize DecoderContext\n");
    exit(1);
  }

  AVRational input_frame_rate = av_guess_frame_rate(ifmt_ctx, ifmt_ctx->streams[video_stream_index], NULL);
  encoder_ctx->time_base = av_inv_q(input_frame_rate);

  //open codecs
  if(avcodec_open2(encoder_ctx, encoder, NULL) < 0) {
    printf("Could not open EncoderCodec\n");
    exit(1);
  }
  if(avcodec_open2(decoder_ctx, decoder, NULL) < 0) {
    printf("Could not open DecoderCodec\n");
    exit(1);
  }

  if (!(ofmt->flags & AVFMT_NOFILE)) {
    //opens pb (byte stream) using avio open and sets up output file for writing
    if(avio_open(&ofmt_ctx->pb, output.c_str(), AVIO_FLAG_WRITE) < 0) {
      printf("Could not open output file\n");
      exit(1);
    }
  }
    
  //writes container header for guessed container in avformat_alloc_output_context2()
  if (avformat_write_header(ofmt_ctx, NULL) < 0) {
    printf("Error occurred when opening output file\n");
    exit(1);
  }

  //allocate frame
  frame = av_frame_alloc();
  if(!frame) {
    printf("Could not allocate AVFrame\n");
    exit(1);
  }

  //allocate packet
  pkt = av_packet_alloc();
  out_pkt = av_packet_alloc();
  if(!pkt || !out_pkt) {
    printf("Could not allocate AVPacket\n");
    exit(1);
  }

  while(av_read_frame(ifmt_ctx, pkt) >= 0) {
    AVStream *in_stream, *out_stream;

    if (pkt->stream_index >= stream_mapping_size ||
        stream_mapping[pkt->stream_index] < 0) {
        av_packet_unref(pkt);
        continue;
	//continue if no subtile, audio or videostream and releases the references held by packet
    }
    
    if(pkt->stream_index == video_stream_index) {
      if(avcodec_send_packet(decoder_ctx, pkt) < 0) {
	printf("Failed to decode packet\n");
	exit(1);
      }
      if(avcodec_receive_frame(decoder_ctx, frame) < 0) {
        printf("Failed to receive frame\n");
        exit(1);
      }

      if(avcodec_send_frame(encoder_ctx, frame) < 0) {
	printf("Failed to encode frame\n");
        exit(1);
      }
      if(avcodec_receive_packet(encoder_ctx, out_pkt) < 0) {
	printf("Failed to receive packet\n");
        exit(1);
      }

      printf("Previous stream index: %d | New stream index: %d\n", pkt->stream_index, out_pkt->stream_index);

      //assigns the stream - the packet belongs to - to the in_stream
      in_stream  = ifmt_ctx->streams[pkt->stream_index];

      //assigns the next index of our valid streams to out_packet->stream_index
      out_pkt->stream_index = stream_mapping[pkt->stream_index];

      //makes out_stream point to next available stream in ofmt_ctx
      out_stream = ofmt_ctx->streams[out_pkt->stream_index];

      //synchronizes timestamps or different streams or translates them depending on container
      av_packet_rescale_ts(out_pkt, in_stream->time_base, out_stream->time_base);
      out_pkt->pos = -1; //basically a method of unreffing a packet
    
      //finally packets of compressed (video, audio or subtitle) data are now written to output format context
      if(av_interleaved_write_frame(ofmt_ctx, out_pkt) < 0) {
        printf("Error muxing packet\n");
        exit(1);
      }

      av_packet_unref(pkt);
      av_packet_unref(out_pkt);
      continue;
    }

    //assigns the stream - the packet belongs to - to the in_stream
    in_stream  = ifmt_ctx->streams[pkt->stream_index];

    //assigns the next index of our valid streams to out_packet->stream_index
    pkt->stream_index = stream_mapping[pkt->stream_index];

    //makes out_stream point to next available stream in ofmt_ctx
    out_stream = ofmt_ctx->streams[pkt->stream_index];
 
    //synchronizes timestamps or different streams or translates them depending on container
    av_packet_rescale_ts(pkt, in_stream->time_base, out_stream->time_base);
    pkt->pos = -1; //basically a method of unreffing a packet
    
    //finally packets of compressed (video, audio or subtitle) data are now written to output format context
    if(av_interleaved_write_frame(ofmt_ctx, pkt) < 0) {
      printf("Error muxing packet\n");
      exit(1);
    }

    av_packet_unref(pkt);
  }

  //finalizes the output file
  av_write_trailer(ofmt_ctx);

  //freeing memory
  av_packet_free(&out_pkt);
  av_packet_free(&pkt);
  av_frame_free(&frame);
  avcodec_free_context(&encoder_ctx);
  avcodec_free_context(&decoder_ctx);

  avformat_close_input(&ifmt_ctx);

  if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
    avio_closep(&ofmt_ctx->pb);
  avformat_free_context(ofmt_ctx);

  av_freep(&stream_mapping);

  printf("Sucess\n");
}
