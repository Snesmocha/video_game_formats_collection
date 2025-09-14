#include "tfss_read_write.h"
#include <stdio.h>
#include <stb_image.h>

#define TEST_READ

#ifdef TEST_READ
int main() {
    const char* tfss_file = "test.tfss";
    const char* reference_png = "SamplePNGImage_30mbmb.png";

    
    int w_ref = 0, h_ref = 0, bpp_ref = 0;

    // Load the reference image
    uint8_t* expected_data = stbi_load(reference_png, &w_ref, &h_ref, &bpp_ref, 0);
    if (!expected_data) {
        fprintf(stderr, "Failed to load reference PNG image\n");
        return 1;
    }

    printf("Reference image: %dx%d, bpp=%d\n", w_ref, h_ref, bpp_ref);

    // Allocate output buffer
    uint8_t* image_data = malloc(w_ref * h_ref * bpp_ref);
    if (!image_data) {
        fprintf(stderr, "Failed to allocate memory for TFSS image\n");
        stbi_image_free(expected_data);
        return 1;
    }
	
	uint32_t whd[3] = {0,0,0};
	uint32_t flag = 0;
	uint8_t format = 0;
	uint8_t mip_count = 0;
	uint8_t face_count = 0;
    // Load TFSS image
    read_tfss(tfss_file, image_data, &flag, whd, &format, &mip_count, &face_count);

    printf("TFSS image: %dx%d, bpp=%d\n", whd[0], whd[1], 0);

    // Sanity check
    if (whd[0] != w_ref || whd[1] != h_ref ) {
        fprintf(stderr, "Size or format mismatch between images\n");
    }

    // Compare data
    size_t size = w_ref * h_ref * bpp_ref;
    int same = memcmp(image_data, expected_data, size) == 0;
    printf("Pixel data %s\n", same ? "MATCHES" : "DIFFERS");

    if (!same) {
        for (size_t i = 0; i < size; ++i) {
            if (image_data[i] != expected_data[i]) {
                printf("Mismatch at byte %zu: got %d, expected %d\n", i, image_data[i], expected_data[i]);
                break;
            }
        }
    }

    // Cleanup
    free(image_data);
    stbi_image_free(expected_data);

    return 0;
}

#elif defined(TEST_WRITE)
int main(void) {
    int width = 0;
    int height = 0;
    int bpp = 3; // RGB
    int compression_level = 22;
    uint8_t* data = stbi_load("SamplePNGImage_30mbmb.png", &width, &height, &bpp, 0);
    size_t data_size = width * height * bpp;

    if (!data) {
        fprintf(stderr, "Failed to allocate image memory\n");
        return 1;
    }

    //generate_checkerboard(data, width, height, bpp);
	uint32_t whd[3] = {width, height, 0};
    write_tfss("test.tfss", data, 15, whd, TF_RGBA8, 0, compression_level, 0, 0);
    stbi_image_free(data);

    printf("test.tfss saved successfully!\n");
    return 0;
}

#endif


#ifdef AAAA

#define WIDTH 128
#define HEIGHT 512

int main(void) {
    int width = WIDTH;
    int height = HEIGHT;

    // Each pixel is 2 bytes (RGBA4444)
    uint16_t* data = malloc(width * height * sizeof(uint16_t));
    if (!data) {
        fprintf(stderr, "Failed to allocate image memory\n");
        return 1;
    }

    for (int y = 0; y < height; y++) {
        float fy = (float)y / (height - 1);
        for (int x = 0; x < width; x++) {
            float fx = (float)x / (width - 1);

            // Compute RG->GB gradient in 0-15 range (4 bits)
            uint8_t r4 = (uint8_t)((1.0f - fx) * 15.0f); // red decreases left->right
            uint8_t g4 = (uint8_t)(((1.0f - fx) * (1.0f - fy) + fx * fy) * 15.0f); // blended green
            uint8_t b4 = (uint8_t)(fx * 15.0f);        // blue increases left->right
            uint8_t a4 = 0xF;                          // fully opaque

            // Pack into RGBA4444: rrrr gggg bbbb aaaa
            uint16_t pixel = (r4 << 12) | (g4 << 8) | (b4 << 4) | a4;
            data[y * width + x] = pixel;
        }
    }

    uint32_t whd[3] = {width, height, 0};
    int compression_level = 22;

    // Assuming write_tfss can handle RGBA4444
    write_tfss("test.tfss", (uint8_t*)data, 0, whd, TF_RGBA4444, 0, compression_level, 0);

    free(data);
    printf("test.tfss saved successfully!\n");
    return 0;
}

#endif


#ifdef BBBB
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define WIDTH 128
#define HEIGHT 512
#define DEPTH 32   // new depth dimension

int main(void) {
    int width = WIDTH;
    int height = HEIGHT;
    int depth = DEPTH;

    // Each pixel is 4 bytes (RGBA8)
    uint8_t* data = malloc(width * height * depth * 4);
    if (!data) {
        fprintf(stderr, "Failed to allocate image memory\n");
        return 1;
    }

    for (int z = 0; z < depth; z++) {
        float fz = (float)z / (depth - 1);  // normalize depth
        for (int y = 0; y < height; y++) {
            float fy = (float)y / (height - 1);
            for (int x = 0; x < width; x++) {
                float fx = (float)x / (width - 1);

                // Compute gradient in 0-255 range
                uint8_t r = (uint8_t)((1.0f - fx) * 255.0f);             // red decreases left->right
                uint8_t g = (uint8_t)(((1.0f - fx) * (1.0f - fy) + fx * fy) * 255.0f); // green blend
                uint8_t b = (uint8_t)(fz * 255.0f);                      // blue changes with depth
                uint8_t a = 0xFF;                                        // fully opaque

                size_t index = ((z * height + y) * width + x) * 4;
                data[index + 0] = r;
                data[index + 1] = g;
                data[index + 2] = b;
                data[index + 3] = a;
            }
        }
    }

    uint32_t whd[3] = {width, height, depth};
    int compression_level = 22;

    // Assuming write_tfss can handle RGBA8 and 3D data
    write_tfss("test_3d.tfss", data, 0, whd, TF_RGBA8, 0, compression_level, 0);

    free(data);
    printf("test_3d.tfss saved successfully!\n");
    return 0;
}

#endif