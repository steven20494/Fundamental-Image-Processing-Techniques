## Fundamental Image Processing Techniques
The objective of this project is to implement fundamental image processing techniques on BMP images using the C++ programming language. The operations include `flipping`, `resizing`, `contrast enhancement`, `sharpness enhancement`, `denoising`, `white balance adjustment`, and `image restoration`.

### Read BMP file
Performing basic image processing using the BMP file format facilitates a deeper understanding of image data structures. As BMP is an uncompressed format, it enables direct pixel-level access without the need for additional complex procedures such as those involved in image compression.

The BMP file format consists of a bitmap file header and a DIB (Device Independent Bitmap) header. The DIB header comes in several versions, each with a different structure name and size. Depending on the version, the header size varies—from the simplest V1 format with 40 bytes to the more advanced V5 format with 124 bytes. For more detailed information, please refer to the relevant [Wikipedia entry](https://en.wikipedia.org/wiki/BMP_file_format).

### Resize
Resizing is a common operation in image processing. In this project, we implement bilinear interpolation as an example, which estimates pixel values based on the four nearest neighboring pixels. In practical applications, other commonly used methods include nearest neighbor and bicubic interpolation.

In image resizing tasks, the forward mapping approach involves calculating the new position of each original pixel after scaling and then performing interpolation at each new location. However, this method can be cumbersome, as the mapping of original pixels to new coordinates is not always straightforward and may result in 'holes' or undefined pixels in the output image. Therefore, the inverse mapping strategy is more commonly used, where each pixel in the target image is mapped back to the corresponding position in the original image, and interpolation is performed using the surrounding pixels.

<img src="asset\resize concept.png">
<img src="asset\resize result.png">

- In the process of downsampling the image by a factor of 0.5, aliasing effects can be observed upon close examination of fine details.


### Contrast enhancement
In this work, we employ histogram equalization to perform contrast enhancement. Histogram equalization computes the cumulative distribution function (CDF) of the original image’s intensity values and maps these values to a new intensity range. Consequently, if the original image has intensities concentrated within a narrow range, the CDF will exhibit a steep increase at that region. This steep increase results in a relatively flattened area in the output image after transformation.
The concept is illustrated in the figure below: each original pixel value is mapped to CDF × max_pixel_value, effectively spreading out the intensity distribution. As a result, the probability density function (PDF) after histogram equalization becomes more uniform.

<img src="asset\histogram equalization concept.png">

In this work, we implement Histogram Equalization (HE) and Contrast Limited Adaptive Histogram Equalization (CLAHE).

<img src="asset\histogram equalization result.png">


### Sharpness enhancement
By computing the second-order derivatives, regions in the image with significant intensity variations can be identified. To enhance edge sharpness, the second-order derivative values are multiplied by a negative sign and added back to the original image. This process is equivalent to applying the following filters through convolution. The difference between the two filters shown below is that one considers only horizontal and vertical directions (filter 1), while the other additionally incorporates diagonal directions (filter 2).

<img src="asset\filters.png" width="40%">

The entire process is illustrated in the figure. For better visualization, the second-order derivative values are intentionally amplified by a factor of 2000. As shown, contrast around the edges increases, resulting in improved sharpness.

<img src="asset\sharpness enhancement concept.png">

<img src="asset\sharpness enhancement result.png">

### Denoise
To address various types of image noise such as Gaussian noise, salt-and-pepper noise, and speckle noise, a range of denoising techniques can be applied, including Gaussian filtering, median filtering, geometric mean filtering, arithmetic mean filtering, and bilateral filtering. In this work, we focus on Gaussian noise and apply Gaussian filtering and median filtering for noise reduction.

<img src="asset\denoise result.png">

### White balance adjustment
Unlike the human visual system, which can adapt to varying lighting conditions, camera sensors often capture images with color deviations due to the lack of automatic adaptation to different light sources. Therefore, color correction using white balance algorithms is necessary. In this work, we implement three white balance methods: Max-RGB, Gray World, and Multi-Scale Retinex (MSR). It is worth noting that although MSR was not originally designed for white balance, it does offer some degree of color correction. For more accurate white balancing, however, the Multi-Scale Retinex with Color Restoration (MSRCR) is recommended. Future work may include the implementation of MSRCR to improve color fidelity

<img src="asset\white balance result1.png">
<img src="asset\white balance result2.png">

### Image restoration
In this work, we implement the Wiener filter, a commonly used method in image restoration, to recover degraded images. In our case, the degradation model 
𝐻. 𝐻 can be computed directly from the original image. However, in real-world scenarios, obtaining the exact degradation model is often challenging, and iterative methods are typically required to estimate it. The released code applies the Wiener filter separately to each RGB channel. By processing the color channels independently with the Wiener filter, improved restoration results can be achieved.

<img src="asset\restoration result.png">