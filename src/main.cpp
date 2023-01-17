// THE MAIN FILE FOR SETUPS AND MANAGEMENT OF THE PROCESS
#include <BUS.h>

#include <stdio.h>
#include <math.h>
#include <algorithm>

#include <LibUnified/GUI/Window.h>
#include <LibUnified/Input/Keys.h>
using namespace LibUnified;
using namespace LibUnified::GUI;
using namespace LibUnified::Graphics;

bool Scaled = false;
surface_t GameWindow;
surface_t GameWindow2;
std::shared_ptr<Cartridge> cart;
Bus* nes;
Window* window;
surface_t oldWindow;
uint64_t SCALE = 1;
bool Render = true;

void ScaleNES(surface_t* window){

    if(!Scaled || (oldWindow.buffer != window->buffer) /*Resized*/){
        uint64_t maxXScale = floor(window->width / GameWindow.width);
        uint64_t maxYScale = floor(window->height / GameWindow.height);

        // Now get minimum of the two
        SCALE = std::min(maxXScale, maxYScale);

        if(SCALE == 0){
            // Dissable render as we wont fit
            Render = false;
        }
        else{
            Render = true;
        }

        // If scaled delete old data
        if(Scaled){
            delete GameWindow2.buffer;
        }

        // Create new window for scaling
        GameWindow2 = {};
        GameWindow2.depth = 32;
        GameWindow2.width = GameWindow.width * SCALE;
        GameWindow2.height = GameWindow.height * SCALE;
        GameWindow2.buffer = new uint8_t[GameWindow2.width * GameWindow2.height * 4];

        oldWindow = *window;
        Scaled = true;
    }

    for(int y = 0; y < GameWindow.height; y++){
        for(int x = 0; x < GameWindow.width; x++){
            for(int j = 0; j < SCALE; j++){
                for(int i = 0; i < SCALE; i++){
                    memcpy(GameWindow2.buffer + (((x * SCALE) + i) * 4) + (GameWindow2.width * 4 * ((y * SCALE) + j)), GameWindow.buffer + ((x * 4) + (y * GameWindow.width * 4)), 4);
                }
            }
        }
    }
}

void GameOnDraw(surface_t* window){
    // COPY VIDEOBUFFER TO THE WINDOW
    ScaleNES(window);

    if(!Render){
        return; // Stop issues with blitting a invalid size
    }

    window->Blit(&GameWindow2);
}

void GameOnKeyboard(Graphics::surface_t* window, uint8_t keyCode, bool Pressed){
    if(keyCode == "x"){
        nes.controller[0] &= 0xFF - 0x80;
        nes.controller[0] |= (Pressed) ? 0x80 : 0x00; // A Button
    }
    else if (keyCode == "z"){
        nes.controller[0] &= 0xFF - 0x40;
        nes.controller[0] |= (Pressed) ? 0x40 : 0x00; // B Button
    }
    else if (keyCode == "a"){
        nes.controller[0] &= 0xFF - 0x20;
        nes.controller[0] |= (Pressed) ? 0x20 : 0x00; // Select
    }
    else if (keyCode == "s"){
        nes.controller[0] &= 0xFF - 0x10;
        nes.controller[0] |= (Pressed) ? 0x10 : 0x00; // Start
    }
    else if (keyCode == KEY_UP){
        nes.controller[0] &= 0xFF - 0x08;
        nes.controller[0] |= (Pressed) ? 0x08 : 0x00;
    }
    else if (keyCode == KEY_DOWN){
        nes.controller[0] &= 0xFF - 0x04;
        nes.controller[0] |= (Pressed) ? 0x04 : 0x00;
    }
    else if (keyCode == KEY_LEFT){
        nes.controller[0] &= 0xFF - 0x02;
        nes.controller[0] |= (Pressed) ? 0x02 : 0x00;
    }
    else if (keyCode == KEY_RIGHT){
        nes.controller[0] &= 0xFF - 0x01;
        nes.controller[0] |= (Pressed) ? 0x01 : 0x00;
    }
}

int main(int argc, char *argv[]){
    printf("NES Started\n");
    fflush(stdout);

    // CHECK FOR ARGS IF NOT FAIL
    if (argc <= 1 || !argv){
        printf("NES: No Args\n");
        fflush(stdout);
        return -1; // No file inputed
    }

    if(!argv[1] /*File*/){
        printf("NES: No File\n");
        fflush(stdout);
        return -1;
    }

    printf("NES: Creating Bus\n");
    fflush(stdout);

    // SETUP EMULATOR
    nes = new Bus;

    printf("NES: Bus %lx\n", (uint64_t)nes);
    fflush(stdout);

    // LOAD THE CARTRIDGE
    printf("NES: Loading %s\n", argv[1]);
    fflush(stdout);
	cart = std::make_shared<Cartridge>(argv[1]);
    nes->insertCartridge(cart);
    nes->reset();

    // Setup game surface
    GameWindow.width = nes->ppu.GetScreen().size.x;
    GameWindow.height = nes->ppu.GetScreen().size.y;
    GameWindow.buffer = nes->ppu.GetScreen().buffer;

    // Create a window
    printf("NES: Creating Window\n");
    fflush(stdout);
    window = new Window((std::string("NES Emulator")).c_str(), {0, 0}, {0, 0}, WINDOW_TYPE_LOCKED);
    window->OnDraw = GameOnDraw;

    if(!window->surface.width){ //Invalid window so failed
        printf("NES: Did not recieve window\n");
        fflush(stdout);
    }
    else{
        printf("NES: Window, Size: X: %lu, Y: %lu, Buffer: %lx\n", window->surface.width, window->surface.height, window->surface.buffer);
        fflush(stdout);
    }

    for(;;){ // GAME LOOP
        // POLL WINDOW (For input purposes)
        window->Poll();

        // CLOCK EMULATOR
        while (!nes->ppu.frame_complete){
            nes->clock();
        }
        nes->ppu.frame_complete = false;

        // RENDER WINDOW
        window->Render();

        // printf("NES: frame\n");
        // fflush(stdout);
    }

    return 0;
}