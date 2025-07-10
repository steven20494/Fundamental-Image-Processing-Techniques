#include "BMPTypes.h"
#include "Utils.h"
#include <iostream>
#include <cmath>

using namespace std;


uint8_t clamp_float(float val) {
    if (val < 0.0f) return 0;
    if (val > 255.0f) return 255;
    return static_cast<uint8_t>(val + 0.5f);
}


int main() {
    BMPHeader header;
    unique_ptr<DIBHeader> dibHeader;
    vector<vector<Pixel>> image;

    if (!ReadBMP("input1.bmp", header, dibHeader, image)) {
        cerr << "Failed to read BMP file.\n";
        return 1;
    }

    // Resize the image
    float resize_factor = 2.5;
    int old_height = (int)(dibHeader->height);
    int old_width = (int)(dibHeader->width);
    int new_height = (int)(dibHeader->height * resize_factor);
    int new_width = (int)(dibHeader->width * resize_factor);
    vector<vector<Pixel>> image_resize(new_height, vector<Pixel>(new_width));

    float old_x = 0;
    float old_y = 0;
    for (int y=0; y<new_height; y++){
        for (int x=0; x<new_width; x++){
            old_x = x / resize_factor;
            old_y = y / resize_factor;
            int x1 = floor(old_x);
            int x2 = min(x1 + 1, old_width - 1);
            int y1 = floor(old_y);
            int y2 = min(y1 + 1, old_height - 1);

            image_resize[y][x].blue = clamp_float(
                image[y1][x1].blue * (x2-old_x) * (y2-old_y) +
                image[y1][x2].blue * (old_x-x1) * (y2-old_y) +
                image[y2][x1].blue * (x2-old_x) * (old_y-y1) +
                image[y2][x2].blue * (old_x-x1) * (old_y-y1));

            image_resize[y][x].green = clamp_float(
                image[y1][x1].green * (x2-old_x) * (y2-old_y) +
                image[y1][x2].green * (old_x-x1) * (y2-old_y) +
                image[y2][x1].green * (x2-old_x) * (old_y-y1) +
                image[y2][x2].green * (old_x-x1) * (old_y-y1));
                
            image_resize[y][x].red = clamp_float(
                image[y1][x1].red * (x2-old_x) * (y2-old_y) +
                image[y1][x2].red * (old_x-x1) * (y2-old_y) +
                image[y2][x1].red * (x2-old_x) * (old_y-y1) +
                image[y2][x2].red * (old_x-x1) * (old_y-y1));
                
            if (dibHeader->bitsPerPixel == 32){
                image_resize[y][x].alpha = clamp_float(
                    image[y1][x1].alpha * (x2-old_x) * (y2-old_y) +
                    image[y1][x2].alpha * (old_x-x1) * (y2-old_y) +
                    image[y2][x1].alpha * (x2-old_x) * (old_y-y1) +
                    image[y2][x2].alpha * (old_x-x1) * (old_y-y1));
            }
        }
    }


    header.fileSize = 14 + dibHeader->headerSize + new_height * new_width * (dibHeader->bitsPerPixel/8);
    dibHeader->width = new_width;
    dibHeader->height = new_height;
    
    if (!WriteBMP("output.bmp", header, dibHeader.get(), image_resize)) {
        cerr << "Failed to write BMP file.\n";
        return 1;
    }

    return 0;
}
