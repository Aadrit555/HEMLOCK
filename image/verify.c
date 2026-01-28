#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>

// Include the image loading library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// --- PERCEPTUAL HASHING LOGIC ---
// This function creates a structural fingerprint of the image
uint64_t compute_ahash(unsigned char *pixels, int w, int h, int channels) {
    uint64_t hash = 0;
    double total = 0;
    unsigned char small[64]; 

    // Step 1: Downsample to 8x8 and Grayscale
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int orig_x = x * w / 8;
            int orig_y = y * h / 8;
            int idx = (orig_y * w + orig_x) * channels;
            
            // Average R, G, and B to get grayscale brightness
            small[y * 8 + x] = (pixels[idx] + pixels[idx+1] + pixels[idx+2]) / 3;
            total += small[y * 8 + x];
        }
    }

    // Step 2: Calculate Average Brightness
    double avg = total / 64.0;

    // Step 3: Set bits based on whether pixel is above or below average
    for (int i = 0; i < 64; i++) {
        if (small[i] >= avg) {
            hash |= (1ULL << i);
        }
    }
    return hash;
}

void handle_errors() {
    ERR_print_errors_fp(stderr);
    exit(1);
}

int main() {
    // 1. Load the protected image
    int width, height, channels;
    unsigned char *pixels = stbi_load("protected.jpg", &width, &height, &channels, 0);
    if (!pixels) {
        printf("Error: Could not load protected.jpg\n");
        return 1;
    }
    size_t pixel_size = width * height * channels;

    // 2. Load the RSA Signature (.sig file)
    FILE *sig_file = fopen("signature.sig", "rb");
    if (!sig_file) { printf("Error: Missing signature.sig\n"); return 1; }
    fseek(sig_file, 0, SEEK_END);
    size_t sig_len = ftell(sig_file);
    fseek(sig_file, 0, SEEK_SET);
    unsigned char *sig = malloc(sig_len);
    fread(sig, 1, sig_len, sig_file);
    fclose(sig_file);

    // 3. Load the Public Key (.pem file)
    FILE *pub_key_file = fopen("public.pem", "r");
    if (!pub_key_file) { printf("Error: Missing public.pem\n"); return 1; }
    EVP_PKEY *pub_key = PEM_read_PUBKEY(pub_key_file, NULL, NULL, NULL);
    fclose(pub_key_file);

    // 4. Load the original Perceptual Hash (.bin file)
    uint64_t saved_hash;
    FILE *h_file = fopen("ahash.bin", "rb");
    int has_ahash = 0;
    if (h_file) {
        fread(&saved_hash, sizeof(uint64_t), 1, h_file);
        fclose(h_file);
        has_ahash = 1;
    }

    printf("\n--- AEGIS VERIFICATION REPORT ---\n");

    // --- STEP A: RSA CRYPTOGRAPHIC CHECK (Identity/Bit-Integrity) ---
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    if (EVP_DigestVerifyInit(md_ctx, NULL, EVP_sha256(), NULL, pub_key) <= 0) handle_errors();
    if (EVP_DigestVerifyUpdate(md_ctx, pixels, pixel_size) <= 0) handle_errors();

    int rsa_result = EVP_DigestVerifyFinal(md_ctx, sig, sig_len);

    if (rsa_result == 1) {
        printf("[IDENTITY]: VERIFIED (File is bit-perfect and signed by owner)\n");
    } else {
        printf("[IDENTITY]: WARNING (File data has been modified or re-compressed)\n");
    }

    // --- STEP B: PERCEPTUAL CONTENT CHECK (AI/Manual Edit Detection) ---
    if (has_ahash) {
        uint64_t current_hash = compute_ahash(pixels, width, height, channels);
        
        // Calculate Hamming Distance (count how many bits are different)
        int diff = 0;
        uint64_t x = saved_hash ^ current_hash;
        while (x > 0) {
            if (x & 1) diff++;
            x >>= 1;
        }

        printf("[CONTENT]:  Structural Difference: %d bits\n", diff);

        if (diff == 0) {
            printf("[CONTENT]:  STATUS: IDENTICAL\n");
        } else if (diff < 5) {
            printf("[CONTENT]:  STATUS: AUTHENTIC (Minor noise/compression detected)\n");
        } else {
            printf("[CONTENT]:  STATUS: TAMPERED (AI or manual modification detected!)\n");
        }
    } else {
        printf("[CONTENT]:  Error: No ahash.bin found to compare visual content.\n");
    }
    printf("----------------------------------\n\n");

    // 5. Cleanup
    stbi_image_free(pixels);
    free(sig);
    EVP_PKEY_free(pub_key);
    EVP_MD_CTX_free(md_ctx);

    return 0;
}