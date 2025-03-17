#include <iostream>
#include "pixelsort.hpp"
#include "hbs.hpp"

#include <cxxopts.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
}

int main(int argc, char** argv) {

    cxxopts::Options options("pixelsort", "A CLI Pixelsorting Program\n");

    options.add_options()
      ("h, help", "Help page")
      ("I, image-to-image", "Pixelsorts an image")
      ("V, video-to-video", "Pixelsorts every frame of a video")
      ("i, input", "Input file", cxxopts::value<std::string>())
      ("o, output", "Output file", cxxopts::value<std::string>())
      ("m, disable-mask", "Disables masking")
      ("M, enable-mask", "Enables masking")
      ("S, sort-by-saturation", "Sorts pixels by saturation")
      ("H, sort-by-hue", "Sorts pixels by hue")
      ("B, sort-by-brightness", "Sorts pixels by brightness")
      ("X, horizontal", "Sorts pixels by rows")
      ("Y, vertical", "Sorts pixels by columns")
      ;
    
    options.parse_positional({"input", "ouput"});
  
    int (*hbs)(char, char, char) = get_hue;
//     void (*rotation)(char*);
//     void* (*read_file);

    try {
      auto result = options.parse(argc, argv);

      if(result.count("help")) {
	std::cout << options.help() << std::endl;
	return 0;
      }

      if(result.count("sort-by-saturation")) {
	hbs = &get_saturation;
      }

      if(result.count("sort-by-brightness")) {
	hbs = &get_brightness;
      }

    }
    catch(const std::exception& e) {
      std::cerr << "Error processing user's input: " << e.what() << std::endl;
      return 1;
    }



     int desired_channels = 3;
     int width = 0; int height = 0; int channels = 0;

     char* image = (char*) stbi_load("images/fantasy.png", &width, &height, &channels, desired_channels);

     for(int i = 0; i < height; i++) {
      pixelsort((image + i * width * desired_channels), width, hbs);
     }

     stbi_write_png("images/output.png", width, height, 3, image, width * 3);    

    return 0;
}
