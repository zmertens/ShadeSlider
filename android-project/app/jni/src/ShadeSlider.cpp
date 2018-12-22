// #include <iostream>

#include <SDL.h>

int main(int argc, char * argv[]) {

    static int counter = 0;
    while (counter < 10000) { 
        SDL_Log("%s", "Hello ShadeSlider Activity");
        counter += 1;
    }

    return 0;
}