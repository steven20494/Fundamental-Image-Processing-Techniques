#include "BMPTypes.h"
#include "Utils.h"
#include <iostream>
#include <cmath>
#include <map>
#include <utility>

using namespace std;

vector<vector<Pixel_YCbCr>> convolution(const vector<vector<Pixel_YCbCr>>& image_YCbCr, const vector<vector<float>> kernel){
    int height = image_YCbCr.size();
    int width = image_YCbCr[0].size();
    int kH = kernel.size();
    int kW = kernel[0].size();
    
    int padH = kH / 2;
    int padW = kW / 2;

    vector<vector<Pixel_YCbCr>> result(height, vector<Pixel_YCbCr>(width));
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {
            int sum = 0;

            for (int n = 0; n < kH; n++) {
                for (int m = 0; m < kW; m++) {
                    int y = j + n - padH;
                    int x = i + m - padW;
            
                    // Clamp padding
                    int y_clamped = max(0, min(height - 1, y));
                    int x_clamped = max(0, min(width - 1, x));
                    sum += image_YCbCr[y_clamped][x_clamped].Y * kernel[n][m];

                    // // Zero padding
                    // if (y >= 0 && y < height && x >= 0 && x < width) {
                    //     sum += image_YCbCr[y][x].Y * kernel[n][m];
                    // }
                }
            }
            result[j][i].Y = sum;
            result[j][i].Cb = image_YCbCr[j][i].Cb;
            result[j][i].Cr = image_YCbCr[j][i].Cr;
        }
    }
    return result;
}


int main() {
    BMPHeader header;
    unique_ptr<DIBHeader> dibHeader;
    vector<vector<Pixel>> image;

    if (!ReadBMP("input2.bmp", header, dibHeader, image)) {
        cerr << "Failed to read BMP file.\n";
        return 1;
    }

    int height = (int)(dibHeader->height);
    int width = (int)(dibHeader->width);
    vector<vector<Pixel_YCbCr>> image_YCbCr(height, vector<Pixel_YCbCr>(width));
    BGR2YCbCr(image, image_YCbCr);

    vector<vector<float>> kernel_1 = {
        {0, -1, 0},
        {-1, 5, -1},
        {0, -1, 0}
    };

    vector<vector<float>> kernel_2 = {
        {-1, -1, -1},
        {-1, 9, -1},
        {-1, -1, -1}
    };

    vector<vector<Pixel_YCbCr>> result_1 = convolution(image_YCbCr, kernel_1);
    vector<vector<Pixel_YCbCr>> result_2 = convolution(image_YCbCr, kernel_2);

    YCbCr2BGR(result_1, image);
    if (!WriteBMP("output_sharpness1.bmp", header, dibHeader.get(), image)) {
        cerr << "Failed to write BMP file.\n";
        return 1;
    }

    YCbCr2BGR(result_2, image);
    if (!WriteBMP("output_sharpness2.bmp", header, dibHeader.get(), image)) {
        cerr << "Failed to write BMP file.\n";
        return 1;
    }    

    return 0;
}