#include "BMPTypes.h"
#include "Utils.h"
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

vector<vector<Pixel_float>> convolution(const vector<vector<Pixel>>& image, const vector<vector<float>> kernel){
    int height = image.size();
    int width = image[0].size();
    int kH = kernel.size();
    int kW = kernel[0].size();
    
    int padH = kH / 2;
    int padW = kW / 2;

    vector<vector<Pixel_float>> result(height, vector<Pixel_float>(width));
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {
            float sum_b = 0.;
            float sum_g = 0.;
            float sum_r = 0.;

            for (int n = 0; n < kH; n++) {
                for (int m = 0; m < kW; m++) {
                    int y = j + n - padH;
                    int x = i + m - padW;
            
                    // Clamp padding
                    int y_clamped = max(0, min(height - 1, y));
                    int x_clamped = max(0, min(width - 1, x));
                    sum_b += image[y_clamped][x_clamped].blue * kernel[n][m];
                    sum_g += image[y_clamped][x_clamped].green * kernel[n][m];
                    sum_r += image[y_clamped][x_clamped].red * kernel[n][m];
                }
            }
            result[j][i].blue = clamp_float(sum_b);
            result[j][i].green = clamp_float(sum_g);
            result[j][i].red = clamp_float(sum_r);
        }
    }
    return result;
}


vector<vector<float>> generateGaussianKernel(const float sigma) {
    int kernelSize = 2 * sigma + 1;
    cout << "kernel size:" << kernelSize << endl;
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


vector<vector<Pixel_float>> SSR(const vector<vector<Pixel>>& image, const int kernelSize, const float sigma){
    int height = image.size();
    int width = image[0].size();
    const float epsilon = 1;

    vector<vector<Pixel_float>> image_L (height, vector<Pixel_float>(width));
    vector<vector<Pixel_float>> result (height, vector<Pixel_float>(width));

    vector<vector<float>> gaussian_kernel = generateGaussianKernel(sigma);
    image_L = convolution(image, gaussian_kernel);

    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            result[y][x].blue = log(image[y][x].blue + epsilon) - log(image_L[y][x].blue + epsilon);
            result[y][x].green = log(image[y][x].green + epsilon) - log(image_L[y][x].green + epsilon);
            result[y][x].red = log(image[y][x].red + epsilon) - log(image_L[y][x].red + epsilon);
        }
    }

    return result;
}


vector<vector<Pixel>> MSR(const vector<vector<Pixel>>& image, const int kernelSize, vector<float>sigmas){
    int height = image.size();
    int width = image[0].size();

    int n = sigmas.size();
    vector<vector<vector<Pixel_float>>> SSRs (n);
    for (int i=0; i<n; i++){
        SSRs[i] = SSR(image, kernelSize, sigmas[i]);
    }

    vector<vector<Pixel_float>> result (height, vector<Pixel_float>(width));
    for (int y=0; y<height; y++){
        for (int x=0; x<width; x++){
            for (int i=0; i<n; i++){
                result[y][x].blue += SSRs[i][y][x].blue / n;
                result[y][x].green += SSRs[i][y][x].green / n;
                result[y][x].red += SSRs[i][y][x].red / n;
            }
        }
    }

    vector<vector<Pixel>> output (height, vector<Pixel>(width));
    output = Normalize_RGB_float(result);
    
    return output;
}


vector<vector<Pixel>> gray_world(const vector<vector<Pixel>>& image){
    int height = image.size();
    int width = image[0].size();
    int sum_b = 0.;
    int sum_g = 0.;
    int sum_r = 0.;

    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            sum_b += image[y][x].blue;
            sum_g += image[y][x].green;
            sum_r += image[y][x].red;
        }
    }

    float mean_b = sum_b / (height * width);
    float mean_g = sum_g / (height * width);
    float mean_r = sum_r / (height * width);
    float mean_gray = (mean_b + mean_g + mean_r) / 3;

    const float epsilon = 1e-6;
    vector<vector<Pixel>> result (height, vector<Pixel>(width));
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            result[y][x].blue = (int) clamp_float(image[y][x].blue * mean_gray / (mean_b + epsilon));
            result[y][x].green = (int) clamp_float(image[y][x].green * mean_gray / (mean_g + epsilon));
            result[y][x].red = (int) clamp_float(image[y][x].red * mean_gray / (mean_r + epsilon));
        }
    }

    return result;
}


vector<vector<Pixel>> MAX_RGB(const vector<vector<Pixel>>& image){
    int height = image.size();
    int width = image[0].size();
    int max_b = 0.;
    int max_g = 0.;
    int max_r = 0.;

    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            if (image[y][x].blue > max_b){
                max_b = image[y][x].blue;
            }
            if (image[y][x].green > max_g){
                max_g = image[y][x].green;
            }
            if (image[y][x].red > max_r){
                max_r = image[y][x].red;
            }
        }
    }

    float gain_b = 255 / max_b;
    float gain_g = 255 / max_g;
    float gain_r = 255 / max_r;
    vector<vector<Pixel>> result (height, vector<Pixel>(width));
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            result[y][x].blue = (int) clamp_float(image[y][x].blue * gain_b);
            result[y][x].green = (int) clamp_float(image[y][x].green * gain_g);
            result[y][x].red = (int) clamp_float(image[y][x].red * gain_r);
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
    int kernel_size = 9;
    if (kernel_size%2 == 0) {
        cerr << "Please use an odd number for the kernel size.\n";
    }

    vector<vector<Pixel>> result1 (height, vector<Pixel>(width));
    vector<float> sigmas = {5., 50., 80.};
    result1 = MSR(image, kernel_size, sigmas);

    if (!WriteBMP("output3_MSR.bmp", header, dibHeader.get(), result1)) {
        cerr << "Failed to write BMP file.\n";
        return 1;
    }


    vector<vector<Pixel>> result2 (height, vector<Pixel>(width));
    result2 = gray_world(image);
    if (!WriteBMP("output3_grayworld.bmp", header, dibHeader.get(), result2)) {
        cerr << "Failed to write BMP file.\n";
        return 1;
    }


    vector<vector<Pixel>> result3 (height, vector<Pixel>(width));
    result3 = MAX_RGB(image);
    if (!WriteBMP("output3_maxrgb.bmp", header, dibHeader.get(), result3)) {
        cerr << "Failed to write BMP file.\n";
        return 1;
    }

    return 0;
}