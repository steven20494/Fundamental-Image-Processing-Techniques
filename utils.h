#ifndef BMPIMAGE_H
#define BMPIMAGE_H

#include "BMPTypes.h"
#include <string>
#include <vector>
#include <memory>

bool ReadBMP(const std::string& filename, BMPHeader& header, std::unique_ptr<DIBHeader>& dibHeader, std::vector<std::vector<Pixel>>& image);
bool WriteBMP(const std::string& filename, const BMPHeader& header, const DIBHeader* dibHeader, const std::vector<std::vector<Pixel>>& image);
uint8_t clamp_float(float val);
void BGR2YCbCr(const std::vector<std::vector<Pixel>>& image, std::vector<std::vector<Pixel_YCbCr>>& image_YCbCr);
void YCbCr2BGR(const std::vector<std::vector<Pixel_YCbCr>>& image_YCbCr, std::vector<std::vector<Pixel>>& image);
std::vector<std::vector<Pixel>> Normalize_RGB(const std::vector<std::vector<Pixel>>& image);
std::vector<std::vector<Pixel>> Normalize_RGB_float(const std::vector<std::vector<Pixel_float>>& image);

#endif // BMPIMAGE_H