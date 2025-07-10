#include "BMPTypes.h"
#include "Utils.h"
#include <iostream>
#include <cmath>
#include <algorithm>

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
            float sum = 0;

            for (int n = 0; n < kH; n++) {
                for (int m = 0; m < kW; m++) {
                    int y = j + n - padH;
                    int x = i + m - padW;
            
                    // Clamp padding
                    int y_clamped = max(0, min(height - 1, y));
                    int x_clamped = max(0, min(width - 1, x));
                    sum += image_YCbCr[y_clamped][x_clamped].Y * kernel[n][m];
                }
            }
            result[j][i].Y = clamp_float(sum);
            result[j][i].Cb = image_YCbCr[j][i].Cb;
            result[j][i].Cr = image_YCbCr[j][i].Cr;
        }
    }
    return result;
}


vector<vector<float>> generateGaussianKernel(int kernelSize, float sigma) {
    vector<vector<float>> kernel(kernelSize, vector<float>(kernelSize));
    float sum = 0.0f;

    int halfSize = kernelSize / 2;
    float variance = 2.0f * sigma * sigma;
    float normalize_factor = 3.14159 * variance;

    for (int y = -halfSize; y <= halfSize; y++) {
        for (int x = -halfSize; x <= halfSize; x++) {
            float exponent = -(x * x + y * y) / variance;
            float value = exp(exponent) / normalize_factor;
            kernel[y + halfSize][x + halfSize] = value;
            sum += value;
        }
    }

    for (int y = 0; y < kernelSize; y++) {
        for (int x = 0; x < kernelSize; x++) {
            kernel[y][x] /= sum;
        }
    }

    return kernel;
}


vector<vector<Pixel_YCbCr>> median_filter(const vector<vector<Pixel_YCbCr>>& image_YCbCr, int kernel_size){
    int height = image_YCbCr.size();
    int width = image_YCbCr[0].size();

    int padH = kernel_size / 2;
    int padW = kernel_size / 2;

    vector<vector<Pixel_YCbCr>> result(height, vector<Pixel_YCbCr>(width));
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {
            vector<float> patch_Y;
            vector<float> patch_Cb;
            vector<float> patch_Cr;
            for (int n = 0; n < kernel_size; n++) {
                for (int m = 0; m < kernel_size; m++) {
                    int y = j + n - padH;
                    int x = i + m - padW;

                    if (y >= 0 && y < height && x >= 0 && x < width) {
                    patch_Y.push_back(image_YCbCr[y][x].Y);
                    patch_Cb.push_back(image_YCbCr[y][x].Cb);
                    patch_Cr.push_back(image_YCbCr[y][x].Cr);
                    }
                }
            }

            nth_element(patch_Y.begin(), patch_Y.begin() + patch_Y.size() / 2, patch_Y.end());
            nth_element(patch_Cb.begin(), patch_Cb.begin() + patch_Cb.size() / 2, patch_Cb.end());
            nth_element(patch_Cr.begin(), patch_Cr.begin() + patch_Cr.size() / 2, patch_Cr.end());
            result[j][i].Y = patch_Y[patch_Y.size() / 2];
            result[j][i].Cb = patch_Cb[patch_Cb.size() / 2];
            result[j][i].Cr = patch_Cr[patch_Cr.size() / 2];
        }
    }
    return result;
}


int main() {
    BMPHeader header;
    unique_ptr<DIBHeader> dibHeader;
    vector<vector<Pixel>> image;

    if (!ReadBMP("input3.bmp", header, dibHeader, image)) {
        cerr << "Failed to read BMP file.\n";
        return 1;
    }

    int height = (int)(dibHeader->height);
    int width = (int)(dibHeader->width);
    vector<vector<Pixel_YCbCr>> image_YCbCr(height, vector<Pixel_YCbCr>(width));
    BGR2YCbCr(image, image_YCbCr);

    int kernel_size = 9;
    if (kernel_size%2 == 0) {
        cerr << "Please use an odd number for the kernel size.\n";
    }

    // Gaussian filter
    float sigma = 2;
    vector<vector<float>> kernel_1 = generateGaussianKernel(kernel_size, sigma);
    vector<vector<Pixel_YCbCr>> result_1 = convolution(image_YCbCr, kernel_1);

    YCbCr2BGR(result_1, image);
    if (!WriteBMP("output_denoise1.bmp", header, dibHeader.get(), image)) {
        cerr << "Failed to write BMP file.\n";
        return 1;
    }

    // Median filter
    vector<vector<Pixel_YCbCr>> result_2 = median_filter(image_YCbCr, kernel_size);

    YCbCr2BGR(result_2, image);
    if (!WriteBMP("output_denoise2.bmp", header, dibHeader.get(), image)) {
        cerr << "Failed to write BMP file.\n";
        return 1;
    }

    return 0;
}