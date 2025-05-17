#include "pixelsort.h"
#include "hbs.h"
#include "rotate_image.h"
#include "mask.h"
#include "cargs.h"
#include "pixel_stream_context.h"

static struct cag_option options[] = {


      {.identifier = 'I',
	.access_letters = "I",
	.access_name = "image",
	.description = "Pixelsorts an image"},

      {.identifier = 'V',
	.access_letters = "V",
	.access_name = "video",
	.description = "Pixelsorts a video frame by frame"},

      {.identifier = 'i',
	.access_letters = "i",
	.access_name = "input",
	.value_name = "FILENAME",
	.description = "Input file"},

      {.identifier = 'o',
	.access_letters = "o",
	.access_name = "output",
	.value_name = "FILENAME",
	.description = "Output file"},

      {.identifier = 'm',
	.access_letters = "m",
	.access_name = "disable-mask",
	.description = "Disables masking"},

      {.identifier = 'M',
	.access_letters = "M",
	.access_name = "enable-mask",
	.description = "Enables masking"},

      {.identifier = 'S',
	.access_letters = "S",
	.access_name = "saturation",
	.description = "Sorts pixels by saturation"},

      {.identifier = 'H',
	.access_letters = "H",
	.access_name = "hue",
	.description = "Sorts pixels by hue"},

      {.identifier = 'B',
	.access_letters = "B",
	.access_name = "brightness",
	.description = "Sorts pixels by brightness"},

      {.identifier = 'R',
	.access_letters = "R",
	.access_name = "rows",
	.description = "Sorts pixels horizontally"},

      {.identifier = 'C',
	.access_letters = "C",
	.access_name = "columns",
	.description = "Sorts pixels vertically"},

      {.identifier = 'h',
	.access_letters = "h",
	.access_name = "help",
	.description = "Shows the command help"}
    };

int main(int argc, char** argv) {

    float (*hbs)(char, char, char) = &get_brightness;
    void (*rotate_image)(char*, int, int, int) = NULL;
    void (*mask)(char*, pixel_stream_context*, int*, int, int, int) = NULL;
    void (*pixelsort)(const char*, const char*, void (*)(char*, pixel_stream_context*, int*, int, int, int), void (*)(char*, int, int, int), float (*)(char, char, char)) = &pixelsort_image; 
    const char* input;
    const char* output;

    cag_option_context context;
    cag_option_init(&context, options, CAG_ARRAY_SIZE(options), argc, argv);

    while(cag_option_fetch(&context)) {
      switch(cag_option_get_identifier(&context)) {

      case 'V':
	pixelsort = &pixelsort_video;
	break;
      case 'i':
	input = cag_option_get_value(&context); 
	break;
      case 'o':
	output = cag_option_get_value(&context);
	break;
      case 'M':
	mask = &create_mask;
	break;
      case 'S':
	hbs = &get_saturation;
	break;
      case 'H':
	hbs = &get_hue;
	break;
      case 'C':
	rotate_image = &horizontal_to_vertical;
	break;
      case 'h':
	printf("Usage: pixelsort [-i/--input] <filename> [-o/--output] <filename> [options...]\n\n");
	cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
	printf("\nIf no options provided pixelsort with automatically use [-I/--image], [-B/--brightness], [-R/--rows] and [-m/--disable-mask]\n"); 
	return 0; case '?':
	cag_option_print_error(&context, stdout);
	break;
      }
    }

    pixelsort(input, output, mask, rotate_image, hbs);

    return 0;
}
