#include <SDL2/SDL.h>
#include <iostream>
#include <fftw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


fftw_complex* performFFT(unsigned char* img, int width, int height, int channels) {
    int N = width * height;

    double* in = fftw_alloc_real(N);
    fftw_complex* out = fftw_alloc_complex(N);

    // Convert image to grayscale and populate the input array
    for (int i = 0; i < N; i++) {
        in[i] = img[i * channels];
    }

    // Plan and execute FFT 
    fftw_plan plan = fftw_plan_dft_r2c_2d(height, width, in, out, FFTW_ESTIMATE);
    fftw_execute(plan);

    fftw_destroy_plan(plan);
    fftw_free(in);

    return out;
}


void reconstructImageFromFFT(unsigned char* img, fftw_complex* fftCoeffs, int width, int height, int channels, int x) {
    int N = width * height;
    double* out = fftw_alloc_real(N);

    fftw_complex* tempCoeffs = fftw_alloc_complex(N);
    memcpy(tempCoeffs, fftCoeffs, sizeof(fftw_complex) * N);

    // Zero out coefficients beyond the first x coefficients
    for (int i = x; i < N; i++) {
        tempCoeffs[i][0] = 0; // Real
        tempCoeffs[i][1] = 0; // Imaginary
    }

    // Plan and execute IFFT
    fftw_plan plan = fftw_plan_dft_c2r_2d(height, width, tempCoeffs, out, FFTW_ESTIMATE);
    fftw_execute(plan);

    // Convert the output back to image format
    for (int i = 0; i < N; i++) {
        // Normalize assign to img
        double val = out[i] / N; 
        unsigned char pixelVal = (unsigned char)val;
        for (int c = 0; c < channels; c++) {
            img[i * channels + c] = pixelVal; 
        }
    }

    fftw_destroy_plan(plan);
    fftw_free(out);
    fftw_free(tempCoeffs);
}


int main(int argc, char *argv[]) {
    const char *img_filename = argv[1];
    
    int width, height, channels;
    unsigned char *img = stbi_load(img_filename, &width, &height, &channels, 0);
    if (img == NULL) {
        std::cerr << "Error in loading the image" << std::endl;
        exit(1);
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Marble is fat",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          width, height,
                                          0);
    SDL_SetWindowResizable(window, SDL_FALSE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


    fftw_complex* fftCoeffs = performFFT(img, width, height, channels);
    SDL_Texture* texture = nullptr;

    SDL_bool running = SDL_TRUE;
    SDL_Event event;
    bool done = false;
    bool starting = true;
    int x = 1;
    while (running) {
        if(starting==true){
            SDL_Delay(5000);
            starting = false;
        }
        // Process events
        while (SDL_PollEvent(&event)) {
            // Handle exit event
            if (event.type == SDL_QUIT) {
                running = SDL_FALSE;
            }
        }

        if(x <= (width*height*0.55)){
            reconstructImageFromFFT(img, fftCoeffs, width, height, channels, x);
            x += 1000;
            //std::cout << "\nC: " << x;
        }else{
            if(!done){
            std::cout << "done\n";
            }
            done = true;
        }

        // Convert pixel data to SDL_Texture
        SDL_Surface *surf = SDL_CreateRGBSurfaceFrom((void *)img,
                                                 width, height,
                                                 channels * 8, width * channels,
                                                 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        
        SDL_DestroyTexture(texture);
        texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        SDL_DestroyTexture(texture);

        //SDL_Delay(1000); 

        // Update window
        SDL_RenderPresent(renderer);
    }

    fftw_free(fftCoeffs);
    stbi_image_free(img);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}