#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define FACTOR 8
#define SCREEN_WIDTH 32 * FACTOR
#define SCREEN_HEIGHT 16 * FACTOR


uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];


uint32_t white = 0xFFFFFFFF; // White
uint32_t black = 0x00000000; // Black

int ruleValue = 109;

int parseBase2(char *str) {
    int result = 0;
    int length = strlen(str); // Get the length of the binary string
    for (int i = 0; i < length; i++) {
        if (str[i] == '1') {
            // Calculate the power of 2 based on the position from the right
            int pos = length - i - 1;
            result += (1 << (pos));
        }
    }
    return result;
}

uint32_t calculateState(uint32_t past, uint32_t current, uint32_t next) { 
    char ruleset[9];
    
    for (int i = 0; i < 8; i++) {
        ruleset[i] = (ruleValue & (1 << (7 - i))) ? '1' : '0';
    }
    ruleset[8] = '\0';
    
    char rule[4];

    rule[0] = past == white ? '0' : '1';
    rule[1] = current == white ? '0' : '1';
    rule[2] = next == white ? '0' : '1';
    rule[3] = '\0';

    int index = 7 - parseBase2(rule);

    if (ruleset[index] == '0') {
        return white;
    } else {
        return black;
    }
}

uint32_t* calculateList(uint32_t* pixels) {
    uint32_t *newpixels = malloc(SCREEN_WIDTH * sizeof(uint32_t));
    if (newpixels == NULL) {
        // Handle memory allocation failure
        printf("Error: Failed to allocate memory\n");
        return NULL;
    }
    // Process all rows
    newpixels[0] = calculateState(pixels[SCREEN_WIDTH - 1], pixels[0], pixels[1]);
    for (int x = 1; x < SCREEN_WIDTH - 1; x++) {
        newpixels[x] = calculateState(pixels[x - 1], pixels[x], pixels[x + 1]);
    }
    newpixels[SCREEN_WIDTH - 1] = calculateState(pixels[SCREEN_WIDTH - 2], pixels[SCREEN_WIDTH - 1], pixels[0]);
    return newpixels;
};

int main(int argc, char** argv){
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Error: SDL failed to initialize\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Elementary Cellular Automata", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if(!window){
        printf("Error: Failed to open window\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer){
        printf("Error: Failed to create renderer\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }
    
    if (argc >= 2){
        // Assume next argument is the rule number
        ruleValue = atoi(argv[1]);
    } 

// ============= Setup ===================
    // Fill in pixels
    for (int i = 0; i < SCREEN_WIDTH; i++){
        pixels[i] = white;
    }
    pixels[SCREEN_WIDTH / 2] = black;

// ======================================

    bool running = true;
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    int y = 0;
    int finished = 0; // 0 = not finished, 1 = finished
    while(running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_WINDOWEVENT:
                    if(event.window.event == SDL_WINDOWEVENT_RESIZED){
                        int newWidth = event.window.data1;
                        int newHeight = event.window.data2;

                        // Calculate the new scale based on the resized window dimensions
                        float scaleX = (float)newWidth / SCREEN_WIDTH;
                        float scaleY = (float)newHeight / SCREEN_HEIGHT;

                        // Set the new scale for rendering
                        SDL_RenderSetScale(renderer, scaleX, scaleY);
                    }
                    break;
                default:
                    break;
            }
        }

        if (finished == 1) {
            SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            continue;
        }

        for (int y = 1; y < SCREEN_HEIGHT; y++) { // Start from the second row
            uint32_t *previousRow = &pixels[(y - 1) * SCREEN_WIDTH]; // Get the previous row
            uint32_t *newRow = calculateList(previousRow); // Calculate the new row based on the previous row
            if (newRow != NULL) {
                for (int i = 0; i < SCREEN_WIDTH; i++) {
                    pixels[i + (y * SCREEN_WIDTH)] = newRow[i]; // Update the current row with the new state
                }
                free(newRow);
                // Update the texture once after all rows are processed
                SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_Delay(10);
            }
        }
        if (finished == 0) {
            finished = 1;
        }
    }
    // Destroy the texture after the loop if it exists
    if (texture) {
        SDL_DestroyTexture(texture);
    }
    SDL_DestroyRenderer(renderer); // Destroy the renderer
    SDL_DestroyWindow(window); // Destroy the window
    SDL_Quit(); // Quit SDL
    return 0;
}