#include <math.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <stdint.h> // REQUIRED for uint64_t
#include <stdio.h>
#include <stdlib.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void handle_errors() {
  ERR_print_errors_fp(stderr);
  exit(1);
}

// Renamed to match main
uint64_t compute_ahash(unsigned char *pixels, int w, int h, int channels) {
  uint64_t hash = 0;
  double total = 0;
  unsigned char small[64];
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      int orig_x = x * w / 8;
      int orig_y = y * h / 8;
      int idx = (orig_y * w + orig_x) * channels;
      small[y * 8 + x] = (pixels[idx] + pixels[idx + 1] + pixels[idx + 2]) / 3;
      total += small[y * 8 + x];
    }
  }
  double avg = total / 64.0;
  for (int i = 0; i < 64; i++) {
    if (small[i] >= avg) {
      hash |= (1ULL << i);
    }
  }
  return hash;
}

int main() {
  int width, height, channels;
  unsigned char *pixels = stbi_load("input.jpg", &width, &height, &channels, 0);
  if (!pixels) {
    printf("Error: input.jpg\n");
    return 1;
  }

  // Save Perceptual Hash
  uint64_t p_hash = compute_ahash(pixels, width, height, channels);
  FILE *h_file = fopen("ahash.bin", "wb");
  fwrite(&p_hash, sizeof(uint64_t), 1, h_file);
  fclose(h_file);

  // Poisoning...
  srand(42);
  for (int i = 0; i < width * height * channels; i++) {
    int noise = rand() % 256;
    pixels[i] = (unsigned char)fmin(255.0, (double)pixels[i] + (noise * 0.05));
  }

  stbi_write_jpg("protected.jpg", width, height, channels, pixels, 90);
  stbi_image_free(pixels); // Original pixels gone

  // Reload for Signing
  int w2, h2, c2;
  unsigned char *saved_pixels = stbi_load("protected.jpg", &w2, &h2, &c2, 0);
  size_t pixel_size = w2 * h2 * c2;

  // RSA Signing... (Keep your existing signing code here)
  // ... (rest of your signing code) ...

  stbi_image_free(saved_pixels);
  printf("Hemlock Protection Complete.\n");
  return 0;
}
