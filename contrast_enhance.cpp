#include "BMPTypes.h"
#include "Utils.h"
#include <iostream>
#include <cmath>
#include <map>
#include <utility>

using namespace std;


struct PatchInfo {
    int x_start, x_end;
    int y_start, y_end;
    int x_center, y_center;
    map<int, float> cdf;
};


map<int, float> CountYValues(const std::vector<std::vector<Pixel_YCbCr>>& image_YCbCr) {
    map<int, float> cdf_map;
    int height = image_YCbCr.size();
    int width = image_YCbCr[0].size();

    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            int Y = image_YCbCr[y][x].Y;
            cdf_map[Y]++;
        }
    }

    float cumulative_sum = 0.;
    for (auto& pair : cdf_map) {
        float prob = pair.second / static_cast<float>(height * width);
        cumulative_sum += prob;
        pair.second = cumulative_sum;
        // cout << "Y:" << pair.first << " CDF:" << pair.second << endl;
    }

    return cdf_map;
}


void fillMissingCDF(std::map<int, float>& cdf) {
    int first_key = cdf.begin()->first;
    for (int i = 0; i < first_key; ++i)
        cdf[i] = 0.0f;

    auto it = cdf.begin();
    while (it != cdf.end()) {
        int start = it->first;
        float value = it->second;
        auto next = std::next(it);
        int end = next != cdf.end() ? next->first - 1 : 255;

        for (int i = start + 1; i <= end; ++i) {
            if (cdf.find(i) == cdf.end()) {
                cdf[i] = value;
            }
        }
        it = next;
    }

    for (int i = cdf.rbegin()->first + 1; i <= 255; ++i)
        cdf[i] = 1.0f;
}


pair<int, int> find_patch_index(int y, int x, const vector<vector<PatchInfo>>& patches){
    int patch_x, patch_y;

    patch_y = patches.size()-1;
    for (int j=0; j<patches.size()-1; j++){
        if (patches[j][0].y_start <= y && y < patches[j+1][0].y_start){
            patch_y = j;
            break;
        }
    }

    patch_x = patches[0].size()-1;
    for (int i=0; i<patches[0].size()-1; i++){
        if (patches[0][i].x_start <= x && x < patches[0][i+1].x_start){
            patch_x = i;
            break;
        }
    }

    return {patch_y, patch_x};
}


int main() {
    BMPHeader header;
    unique_ptr<DIBHeader> dibHeader;
    vector<vector<Pixel>> image;

    if (!ReadBMP("input1.bmp", header, dibHeader, image)) {
        cerr << "Failed to read BMP file.\n";
        return 1;
    }

    int height = (int)(dibHeader->height);
    int width = (int)(dibHeader->width);

    vector<vector<Pixel_YCbCr>> image_YCbCr(height, vector<Pixel_YCbCr>(width));
    BGR2YCbCr(image, image_YCbCr);

    string method = "HE";

    // Histogram Equalization (HE)
    if (method == "HE"){
        map<int, float> cdf_map;
        cdf_map = CountYValues(image_YCbCr); // Calculate the CDF

        for (int y=0; y<height; y++){
            for (int x=0; x<width; x++){
                image_YCbCr[y][x].Y = clamp_float(cdf_map[image_YCbCr[y][x].Y]*255.); // Mapping the original Y value to new one
            }
        }
        YCbCr2BGR(image_YCbCr, image);
        
        if (!WriteBMP("output_HE.bmp", header, dibHeader.get(), image)) {
            cerr << "Failed to write BMP file.\n";
            return 1;
        }
    }

    // Contrast Limited Adaptive Histogram Equalization (CLAHE)
    else if (method == "CLAHE"){
        int patch_rows = 8;
        int patch_cols = 8;
        int patch_height = height / patch_rows;
        int patch_width = width / patch_cols;

        vector<vector<PatchInfo>> patches(patch_rows, vector<PatchInfo>(patch_cols));
        
        for (int ty = 0; ty < patch_rows; ++ty) {
            for (int tx = 0; tx < patch_cols; ++tx) {
                // Calculate the boundary of each patch
                int y_start = ty * patch_height;
                int y_end;
                if (ty == patch_rows - 1) {
                    y_end = height;
                } else {
                    y_end = y_start + patch_height;
                }
        
                int x_start = tx * patch_width;
                int x_end;
                if (tx == patch_cols - 1) {
                    x_end = width;
                } else {
                    x_end = x_start + patch_width;
                }
        
                vector<vector<Pixel_YCbCr>> patch(y_end - y_start, vector<Pixel_YCbCr>(x_end - x_start));
                for (int j = y_start; j < y_end; j++){
                    for (int i = x_start; i < x_end; i++){
                        patch[j - y_start][i - x_start] = image_YCbCr[j][i];
                    }
                }
                map<int, float> cdf;
                cdf = CountYValues(patch);
                fillMissingCDF(cdf);
                
                patches[ty][tx].x_start = x_start;
                patches[ty][tx].x_end = x_end;
                patches[ty][tx].y_start = y_start;
                patches[ty][tx].y_end = y_end;
                patches[ty][tx].x_center = (x_start + x_end) / 2;
                patches[ty][tx].y_center = (y_start + y_end) / 2;
                patches[ty][tx].cdf = cdf;
            }
        }
        // cout << "here" << endl;
        int tx1, ty1, tx2, ty2;
        int x1, x2, y1, y2;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                pair<int, int> patch_index = find_patch_index(y, x, patches);
                int patch_y = patch_index.first;
                int patch_x = patch_index.second;
                // cout << "position: " << patch_y << "," << patch_x << endl;

                if (y<patches[patch_y][patch_x].y_center){
                    ty1 = patch_y - 1;
                    ty2 = patch_y;
                }
                else{
                    ty1 = patch_y;
                    ty2 = patch_y + 1;
                }

                if (x<patches[patch_y][patch_x].x_center){
                    tx1 = patch_x - 1;
                    tx2 = patch_x;
                }
                else{
                    tx1 = patch_x;
                    tx2 = patch_x + 1;
                }

                tx1 = max(0, tx1);
                tx2 = min(tx2, patch_cols- 1 );
                ty1 = max(0, ty1);
                ty2 = min(ty2, patch_rows- 1 );
                // cout << "t1: "<< ty1 << "," << tx1 << endl;
                // cout << "t2: "<< ty2 << "," << tx2 << endl;

                x1 = patches[ty1][tx1].x_center;
                x2 = patches[ty1][tx2].x_center;
                y1 = patches[ty1][tx1].y_center;
                y2 = patches[ty2][tx1].y_center;


                if (x1 == x2 && y1 == y2){
                    image_YCbCr[y][x].Y = (patches[patch_y][patch_x].cdf[image_YCbCr[y][x].Y])*255.;
                }
                else if (y1 == y2){
                    image_YCbCr[y][x].Y = clamp_float(
                        ((x2-x) * patches[patch_y][tx1].cdf[image_YCbCr[y][x].Y] +
                        (x-x1) * patches[patch_y][tx2].cdf[image_YCbCr[y][x].Y]) / (x2-x1) * 255.);
                }
                else if (x1 == x2){
                    image_YCbCr[y][x].Y = clamp_float(
                        ((y2-y) * patches[ty1][patch_x].cdf[image_YCbCr[y][x].Y] +
                        (y-y1) * patches[ty2][patch_x].cdf[image_YCbCr[y][x].Y]) / (y2-y1) * 255.);
                }
                else {
                    image_YCbCr[y][x].Y = clamp_float(
                        ((x2-x) * (y2-y) * patches[ty1][tx1].cdf[image_YCbCr[y][x].Y] +
                        (x-x1) * (y2-y) * patches[ty1][tx2].cdf[image_YCbCr[y][x].Y] +
                        (x2-x) * (y-y1) * patches[ty2][tx1].cdf[image_YCbCr[y][x].Y] +
                        (x-x1) * (y-y1) * patches[ty2][tx2].cdf[image_YCbCr[y][x].Y]) /
                        ((x2 - x1) * (y2 - y1)) * 255.);
                }
            }
        }
        YCbCr2BGR(image_YCbCr, image);
        
        if (!WriteBMP("output_CLAHE.bmp", header, dibHeader.get(), image)) {
            cerr << "Failed to write BMP file.\n";
            return 1;
        }
    }
    return 0;
}