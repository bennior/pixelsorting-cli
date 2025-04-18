#include <iostream>
#include <cxxopts.hpp>

#include "pixelsort.hpp"
#include "hbs.hpp"
#include "rotate_image.hpp"
#include "mask.hpp"

int main(int argc, char** argv) {

    cxxopts::Options options("pixelsort", "A CLI Pixelsorting Program\n");

    options.add_options()
      ("h, help", "Help page")
      ("I, image-to-image", "Pixelsorts an image [DEFAULT]")
      ("V, video-to-video", "Pixelsorts every frame of a video")
      ("o, output", "Output file", cxxopts::value<std::string>())
      ("i, input", "Input file", cxxopts::value<std::string>())
      ("m, disable-mask", "Disables masking [DEFAULT]")
      ("M, enable-mask", "Enables masking")
      ("S, sort-by-saturation", "Sorts pixels by saturation")
      ("H, sort-by-hue", "Sorts pixels by hue [DEFAULT]")
      ("B, sort-by-brightness", "Sorts pixels by brightness")
      ("R, horizontal", "Sorts pixels by rows [DEFAULT]")
      ("C, vertical", "Sorts pixels by columns")
      ;
    
    options.parse_positional({"input", "output"});
 
    float (*hbs)(char, char, char) = get_hue;
    void (*rotate_image)(char*, int&, int&, int&) = nullptr;
    void (*mask_ptr)(char*, std::vector<pixel_stream>&, int&, int, int, int) = nullptr;
    void (*pixelsort)(std::string&, std::string&, void (*)(char*, std::vector<pixel_stream>&, int&, int, int, int), void (*)(char*, int&, int&, int&), float (*)(char, char, char)) = pixelsort_image; 
    std::string input;
    std::string output;
    
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
      
      if(result.count("input")) {
	input = result["input"].as<std::string>();
      }

      if(result.count("output")) {
	output = result["output"].as<std::string>();
      }

      if(result.count("vertical")) {
 	rotate_image = &horizontal_to_vertical;
      }

      if(result.count("enable-mask")) {
	mask_ptr = &mask;
      }

      if(result.count("video-to-video")) {
	pixelsort = &pixelsort_video; 
      }

    }
    catch(const std::exception& e) {
      std::cerr << "Error processing user's input: " << e.what() << std::endl;
      return 1;
    }

    pixelsort(input, output, mask_ptr, rotate_image, hbs);

    return 0;
}
