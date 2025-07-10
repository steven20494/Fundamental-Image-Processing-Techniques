#include "Utils.h"
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

bool ReadBMP(const std::string& filename, BMPHeader& bmpheader, unique_ptr<DIBHeader>& dibHeaderPtr, vector<vector<Pixel>>& image) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Cannot open file: " << filename << endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(&bmpheader), sizeof(bmpheader));

    uint32_t dibHeaderSize = 0;
    file.read(reinterpret_cast<char*>(&dibHeaderSize), 4);

    if (dibHeaderSize == 40) {
        auto hdr = make_unique<DIBHeader>();
        file.seekg(-4, ios::cur);
        file.read(reinterpret_cast<char*>(hdr.get()), sizeof(DIBHeader));
        dibHeaderPtr = move(hdr);
    } else if (dibHeaderSize == 52) {
        auto hdr = make_unique<DIBHeaderV2>();
        file.seekg(-4, ios::cur);
        file.read(reinterpret_cast<char*>(hdr.get()), sizeof(DIBHeaderV2));
        dibHeaderPtr = move(hdr);
    } else if (dibHeaderSize == 56) {
        auto hdr = make_unique<DIBHeaderV3>();
        file.seekg(-4, ios::cur);
        file.read(reinterpret_cast<char*>(hdr.get()), sizeof(DIBHeaderV3));
        dibHeaderPtr = move(hdr);
    } else if (dibHeaderSize == 108) {
        auto hdr = make_unique<DIBHeaderV4>();
        file.seekg(-4, ios::cur);
        file.read(reinterpret_cast<char*>(hdr.get()), sizeof(DIBHeaderV4));
        dibHeaderPtr = move(hdr);
    } else if (dibHeaderSize == 124) {
        auto hdr = make_unique<DIBHeaderV5>();
        file.seekg(-4, ios::cur);
        file.read(reinterpret_cast<char*>(hdr.get()), sizeof(DIBHeaderV5));
        dibHeaderPtr = move(hdr);
    } else {
        cerr << "Unsupported DIB header size: " << dibHeaderSize << endl;
        return false;
    }

    file.seekg(bmpheader.offsetData, ios::beg);

    int width = dibHeaderPtr->width;
    int height = dibHeaderPtr->height;
    int bpp = dibHeaderPtr->bitsPerPixel;
    int rowSize = ((bpp * width + 31) / 32) * 4;

    vector<uint8_t> row(rowSize);
    image.resize(height, vector<Pixel>(width));

    for (int y = 0; y < height; y++) {
        file.read(reinterpret_cast<char*>(row.data()), rowSize);
        for (int x = 0; x < width; x++) {
            Pixel px;
            if (bpp == 24) {
                px.blue = row[x * 3 + 0];
                px.green = row[x * 3 + 1];
                px.red = row[x * 3 + 2];
                px.alpha = 255;
            } else if (bpp == 32) {
                px.blue = row[x * 4 + 0];
                px.green = row[x * 4 + 1];
                px.red = row[x * 4 + 2];
                px.alpha = row[x * 4 + 3];
            }
            image[height - 1 - y][x] = px;
        }
    }

    return true;
}


bool WriteBMP(const std::string& filename, const BMPHeader& bmpheader, const DIBHeader* dibHeader, const std::vector<std::vector<Pixel>>& image) {
    ofstream outfile(filename, ios::binary);
    if (!outfile) {
        cerr << "Cannot create output file: " << filename << endl;
        return false;
    }

    int width = dibHeader->width;
    int height = dibHeader->height;
    int bpp = dibHeader->bitsPerPixel;
    int rowSize = ((bpp * width + 31) / 32) * 4;
    int bytesPerPixel = bpp / 8;

    outfile.write(reinterpret_cast<const char*>(&bmpheader), sizeof(bmpheader));
    outfile.write(reinterpret_cast<const char*>(dibHeader), dibHeader->headerSize);

    vector<uint8_t> padding(rowSize - (width * bytesPerPixel), 0);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const Pixel& px = image[height - 1 - y][x];
            if (bpp == 24) {
                outfile.write(reinterpret_cast<const char*>(&px.blue), 1);
                outfile.write(reinterpret_cast<const char*>(&px.green), 1);
                outfile.write(reinterpret_cast<const char*>(&px.red), 1);
            } else if (bpp == 32) {
                outfile.write(reinterpret_cast<const char*>(&px.blue), 1);
                outfile.write(reinterpret_cast<const char*>(&px.green), 1);
                outfile.write(reinterpret_cast<const char*>(&px.red), 1);
                outfile.write(reinterpret_cast<const char*>(&px.alpha), 1);
            }
        }
        if (bpp == 24 && !padding.empty()) {
            outfile.write(reinterpret_cast<const char*>(padding.data()), padding.size());
        }
    }

    return true;
}


uint8_t clamp_float(float val) {
    if (val < 0.0f) return 0;
    if (val > 255.0f) return 255;
    return static_cast<uint8_t>(val + 0.5f);
}


void BGR2YCbCr(const std::vector<std::vector<Pixel>>& image, std::vector<std::vector<Pixel_YCbCr>>& image_YCbCr){
    for (int y=0; y<image.size(); y++){
        for (int x=0; x<image[0].size(); x++){
            image_YCbCr[y][x].Y = 0 + 0.299*image[y][x].red + 0.587*image[y][x].green + 0.114*image[y][x].blue;
            image_YCbCr[y][x].Cb = 128 - 0.168736*image[y][x].red - 0.331264*image[y][x].green + 0.5*image[y][x].blue;
            image_YCbCr[y][x].Cr = 128 + 0.5*image[y][x].red - 0.418688*image[y][x].green - 0.081312*image[y][x].blue;
        }
    }
}


void YCbCr2BGR(const std::vector<std::vector<Pixel_YCbCr>>& image_YCbCr, std::vector<std::vector<Pixel>>& image){
    for (int y=0; y<image.size(); y++){
        for (int x=0; x<image[0].size(); x++){
            image[y][x].blue = clamp_float(image_YCbCr[y][x].Y + 1.722*(image_YCbCr[y][x].Cb-128));
            image[y][x].green = clamp_float(image_YCbCr[y][x].Y - 0.344136*(image_YCbCr[y][x].Cb-128) - 0.714136*(image_YCbCr[y][x].Cr-128));
            image[y][x].red = clamp_float(image_YCbCr[y][x].Y + 1.402*(image_YCbCr[y][x].Cr-128));
        }
    }
}


std::vector<std::vector<Pixel>> Normalize_RGB(const std::vector<std::vector<Pixel>>& image){
    int height = image.size();
    int width = image[0].size();

    int maximum = 0;
    int minimum = 255;

    for (int y=0; y<height; y++){
        for (int x=0; x<width; x++){
            int pixel_minimum = std::min(image[y][x].blue, std::min(image[y][x].green, image[y][x].red));
            int pixel_maximum = std::max(image[y][x].blue, std::max(image[y][x].green, image[y][x].red));
            if (pixel_minimum < minimum){
                minimum = pixel_minimum;
            }
            if (pixel_maximum > maximum){
                maximum = pixel_maximum;
            }
        }
    }

    std::vector<std::vector<Pixel>> result (height, std::vector<Pixel>(width));
    for (int y=0; y<height; y++){
        for (int x=0; x<width; x++){
            result[y][x].blue = (image[y][x].blue - minimum) * 255 / (maximum - minimum);
            result[y][x].green = (image[y][x].green - minimum) * 255 / (maximum - minimum);
            result[y][x].red = (image[y][x].red- minimum) * 255 / (maximum - minimum);
        }
    }

    return result;
}

std::vector<std::vector<Pixel>> Normalize_RGB_float(const std::vector<std::vector<Pixel_float>>& image){
    int height = image.size();
    int width = image[0].size();

    float maximum = 0;
    float minimum = 255;

    for (int y=0; y<height; y++){
        for (int x=0; x<width; x++){
            float pixel_minimum = std::min(image[y][x].blue, std::min(image[y][x].green, image[y][x].red));
            float pixel_maximum = std::max(image[y][x].blue, std::max(image[y][x].green, image[y][x].red));
            if (pixel_minimum < minimum){
                minimum = pixel_minimum;
            }
            if (pixel_maximum > maximum){
                maximum = pixel_maximum;
            }
        }
    }

    std::vector<std::vector<Pixel>> result (height, std::vector<Pixel>(width));
    for (int y=0; y<height; y++){
        for (int x=0; x<width; x++){
            result[y][x].blue = (image[y][x].blue - minimum) * 255 / (maximum - minimum);
            result[y][x].green = (image[y][x].green - minimum) * 255 / (maximum - minimum);
            result[y][x].red = (image[y][x].red- minimum) * 255 / (maximum - minimum);
        }
    }

    return result;
}