#include "BMPTypes.h"
#include "Utils.h"
#include <iostream>

using namespace std;

int main() {
    BMPHeader header;
    unique_ptr<DIBHeader> dibHeader;
    vector<vector<Pixel>> image;

    if (!ReadBMP("input1.bmp", header, dibHeader, image)) {
        cerr << "Failed to read BMP file.\n";
        return 1;
    }


    // Flip the image
    int height = dibHeader->height;
    int width = dibHeader->width;
    vector<vector<Pixel>> image_flip(height, vector<Pixel>(width));
    for (int y=0; y<height; y++){
        for (int x=0; x<width; x++){
            image_flip[y][x] = image[y][width-1-x];
        }
    }


    if (!WriteBMP("output.bmp", header, dibHeader.get(), image_flip)) {
        cerr << "Failed to write BMP file.\n";
        return 1;
    }

    return 0;
}
