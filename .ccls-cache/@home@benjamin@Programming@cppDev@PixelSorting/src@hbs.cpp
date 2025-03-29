#include <algorithm>
#include <iostream>

#include "hbs.hpp"

float get_hue(char red, char green, char blue) {
    float R = red / 255.0;    
    float G = green / 255.0;
    float B = blue / 255.0;

    float min = std::min({R, G, B});
    float max = std::max({R, G, B});

    float hue = (max == R) * (G - B) / (max - min) +
                (max == G) * 2.0 + (max == G) * (B - R) / (max - min) + 
                (max == B) * 4.0 + (max == B) * (R - G) / (max - min);       

    hue *= 60.0;
    if (hue < 360) hue += 360.0;

    return hue; 
}

float get_saturation(char red, char green, char blue) {
    float R = red / 255.0;
    float G = green / 255.0;
    float B = blue / 255.0;

    float min = std::min({R, G, B});
    float max = std::max({R, G, B});

    float luminosity = max - min;
    float saturation = 0.0;

    if(max == 0)
      return saturation;

    if(luminosity < 0.5)
      saturation = (max - min) / (max + min);

    if(luminosity >= 0.5)
      saturation = (max - min) / (2 - max - min);

    return saturation;
}

float get_brightness(char red, char green, char blue) {
  float R = red / 255.0;
  float G = green / 255.0;
  float B = blue / 255.0;

  float brightness = 0.2126 * R + 0.7152 * G + 0.0722 * B;

  return brightness;
}
