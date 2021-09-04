#include <stdio.h>
#include <chrono>
#include <thread>
#include "stdint.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <windows.h>
#include "chip8.h"
using namespace std;

uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

int main(int argc, char **argv){

    if(argc != 2){
        cout << "Incorrect Usage";
        return 1;
    }

    Chip8 chip8 = Chip8();
    int width = 1024;
    int height = 512;

    SDL_Window* window = nullptr;
    if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
        cerr << "Error in initialising SDL " << SDL_GetError() << endl;
        SDL_Quit();
        exit(1);
    }

    window = SDL_CreateWindow(
            "chip-8 Emulator",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height, SDL_WINDOW_SHOWN);

    if (window == nullptr){
        cerr << "Error in creating window" << SDL_GetError() << endl;
        SDL_Quit();
        exit(1);
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr){
        cerr << "Error in rendering" << SDL_GetError() << endl;
        SDL_Quit();
        exit(1);
    }

    SDL_RenderSetLogicalSize(renderer, width, height);

     SDL_Texture* sdlTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
     if (sdlTexture == nullptr){
        cerr << "Error in setting up textures   " << SDL_GetError() << endl;
        SDL_Quit();
        exit(1);
    }

    uint32_t pixels[2048];

    load:
    if (!chip8.load(argv[1]))
        return 2;

    while (true){
        chip8.emulateCycle();

        SDL_Event ev;
        while(SDL_PollEvent(&ev)){
            if (ev.type == SDL_QUIT) exit(0);

            if (ev.type == SDL_KEYDOWN){
                if (ev.key.keysym.sym == SDLK_ESCAPE){
                    exit(0);
                }


                for (int i = 0; i < 16; i++){
                    if (ev.key.keysym.sym == keymap[i]){
                        chip8.kypd[i] = 1;
                    }
                }
            }

            if (ev.type == SDL_KEYUP){
                for (int i = 0; i < 16; i++){
                    if (ev.key.keysym.sym == keymap[i]){
                        chip8.kypd[i] = 0;
                    }
                }
            }
        }

        if (chip8.drawFlag){
            chip8.drawFlag = false;

            for (int i = 0; i < 2048; ++i) {
                uint8_t pixel = chip8.gfx[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }

            SDL_UpdateTexture(sdlTexture, nullptr, pixels, 64 * sizeof(Uint32));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, sdlTexture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
        Sleep(1.2);//CHANGE FOR THE GAME THAT IS BEING PLAYED

    }
}