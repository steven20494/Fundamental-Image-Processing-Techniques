#include "BMPTypes.h"
#include "Utils.h"
#include <iostream>
#include <cmath>
#include <complex>

using namespace std;
const double PI = acos(-1);

void dft1d(const vector<complex<double>>& input,vector<complex<double>>& output,bool inverse=false) {
    int N = input.size();
    output.resize(N);

    double sign;
    if (inverse) {
        sign = 1.0;
    } else {
        sign = -1.0;
    }

    const double coe = 2.0 * PI /N;
    for (int k = 0; k < N; ++k) {
        complex<double> sum(0.0, 0.0);

        for (int n = 0; n < N; ++n) {
            double angle = k * n * coe;
            complex<double> Wn(cos(sign * angle), sin(sign * angle));
            sum += input[n] * Wn;
        }

        if (inverse) {
            output[k] = sum / static_cast<double>(N);
        } 
        else {
            output[k] = sum;
        }
    }
}


void dft2d(const vector<vector<complex<double>>>& input, vector<vector<complex<double>>>& output, bool inverse=false) {
    int height = input.size();
    int width = input[0].size();

    vector<vector<complex<double>>> temp(height, vector<complex<double>>(width));

    for (int j=0; j<height; j++) {
        dft1d(input[j], temp[j], inverse);
    }

    output = vector<vector<complex<double>>>(height, vector<complex<double>>(width));
    for (int i=0; i<width; i++) {
        vector<complex<double>> column(height);
        for (int j=0; j<height; j++) {
            column[j] = temp[j][i];
        }

        vector<complex<double>> columnResult;
        dft1d(column, columnResult, inverse);

        for (int j=0; j<height; j++) {
            output[j][i] = columnResult[j];
        }
    }
}


vector<vector<vector<complex<double>>>> estimate_H(const vector<vector<vector<complex<double>>>>& degrad_image, const vector<vector<vector<complex<double>>>>& original_image) {
    int height = original_image[0].size();
    int width = original_image[0][0].size();

    vector<vector<vector<complex<double>>>> H (3, vector<vector<complex<double>>>(height, vector<complex<double>>(width)));

    const double epsilon = 1e-8;

    for (int c=0; c<3; c++) {
        for (int y=0; y<height; y++) {
            for (int x=0; x<width; x++) {
                complex<double> F = original_image[c][y][x];
                complex<double> G = degrad_image[c][y][x];

                if (abs(F) < epsilon) {
                    H[c][y][x] = complex<double>(0.0, 0.0);
                } else {
                    H[c][y][x] = G / F;
                }
            }
        }
    }

    return H;
}



vector<vector<Pixel>> Wiener_filter(const vector<vector<Pixel>>& image, const vector<vector<Pixel>>& degrade_image, const double K){
    int height = image.size();
    int width = image[0].size();

    vector<vector<vector<complex<double>>>> input (3, vector<vector<complex<double>>>(height, vector<complex<double>>(width)));
    vector<vector<vector<complex<double>>>> degrade_input (3, vector<vector<complex<double>>>(height, vector<complex<double>>(width)));
    for (int y=0; y<height; y++){
        for (int x=0; x<width; x++){
            input[0][y][x] = complex<double> (image[y][x].blue, 0);
            input[1][y][x] = complex<double> (image[y][x].green, 0);
            input[2][y][x] = complex<double> (image[y][x].red, 0);
        }
    }

    for (int y=0; y<height; y++){
        for (int x=0; x<width; x++){
            degrade_input[0][y][x] = complex<double> (degrade_image[y][x].blue, 0);
            degrade_input[1][y][x] = complex<double> (degrade_image[y][x].green, 0);
            degrade_input[2][y][x] = complex<double> (degrade_image[y][x].red, 0);
        }
    }

    // Estimate the degradation model
    vector<vector<vector<complex<double>>>> H (3, vector<vector<complex<double>>>(height, vector<complex<double>>(width)));
    H = estimate_H(degrade_input, input);

    // Apply DFT
    vector<vector<vector<complex<double>>>> dft_degrade_input (3, vector<vector<complex<double>>>(height, vector<complex<double>>(width)));
    for (int c=0; c<3; c++){
        dft2d(degrade_input[c], dft_degrade_input[c]);
    }

    // Apply Wiener filter
    vector<vector<vector<complex<double>>>> F (3, vector<vector<complex<double>>>(height, vector<complex<double>>(width)));
    for (int y=0; y<height; y++){
        for (int x=0; x<width; x++){
            double H_norm_b = norm(H[0][y][x]);
            double H_norm_g = norm(H[1][y][x]);
            double H_norm_r = norm(H[2][y][x]);
            F[0][y][x] = (H_norm_b * dft_degrade_input[0][y][x]) / (H[0][y][x] * (H_norm_b + K));
            F[1][y][x] = (H_norm_g * dft_degrade_input[1][y][x]) / (H[1][y][x] * (H_norm_g + K));
            F[2][y][x] = (H_norm_r * dft_degrade_input[2][y][x]) / (H[2][y][x] * (H_norm_r + K));
        }
    }

    // Applt IDFT
    vector<vector<vector<complex<double>>>> dft_degrade_output (3, vector<vector<complex<double>>>(height, vector<complex<double>>(width)));
    for (int c=0; c<3; c++){
        dft2d(F[c], dft_degrade_output[c], true);
    }
    
    vector<vector<Pixel>> output (height, vector<Pixel>(width));
    for (int y=0; y<height; y++){
        for (int x=0; x<width; x++){
            output[y][x].blue = (int) (dft_degrade_output[0][y][x].real());
            output[y][x].green = (int) (dft_degrade_output[1][y][x].real());
            output[y][x].red = (int) (dft_degrade_output[2][y][x].real());
        }
    }
    return output;
}



int main() {
    BMPHeader header;
    unique_ptr<DIBHeader> dibHeader;
    vector<vector<Pixel>> image;
    vector<vector<Pixel>> Degrade_image;


    if (!ReadBMP("image1_ori.bmp", header, dibHeader, image)) {
        cerr << "Failed to read BMP file.\n";
        return 1;
    }

    if (!ReadBMP("input1.bmp", header, dibHeader, Degrade_image)) {
        cerr << "Failed to read BMP file.\n";
        return 1;
    }

    int height = (int)(dibHeader->height);
    int width = (int)(dibHeader->width);

    const double K = 0.1;
    vector<vector<Pixel>> output (height, vector<Pixel>(width));
    output = Wiener_filter(image, Degrade_image, K);

    if (!WriteBMP("output_01.bmp", header, dibHeader.get(), output)) {
        cerr << "Failed to write BMP file.\n";
        return 1;
    }

    return 0;
}