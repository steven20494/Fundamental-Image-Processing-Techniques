#ifndef BMPTYPES_H
#define BMPTYPES_H

#include <cstdint>
#include <complex>

// Use pragma to ensure 1 byte and avoid padding
#pragma pack(push, 1)

struct BMPHeader {
    char signature[2];
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offsetData;
};

struct DIBHeader {
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t importantColors;
};

struct DIBHeaderV2 : public DIBHeader {
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
};

struct DIBHeaderV3 : public DIBHeaderV2 {
    uint32_t alphaMask;
};

struct DIBHeaderV4 : public DIBHeaderV3 {
    uint32_t colorSpaceType;
    uint32_t colorSpaceEndpoints[9];
    uint32_t gammaRed;
    uint32_t gammaGreen;
    uint32_t gammaBlue;
};

struct DIBHeaderV5 : public DIBHeaderV4 {
    uint32_t intent;
    uint32_t profileData;
    uint32_t profileSize;
    uint32_t reserved;
};

#pragma pack(pop)


struct Pixel {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
};

struct Pixel_float {
    float blue;
    float green;
    float red;
    float alpha;
};

struct Pixel_complex {
    std::complex<double> blue;
    std::complex<double> green;
    std::complex<double> red;
    std::complex<double> alpha;
};

struct Pixel_YCbCr {
    float Y;
    float Cb;
    float Cr;
};

#endif // BMPTYPES_H