// THE MAIN FILE FOR SETUPS AND MANAGEMENT OF THE PROCESS
#include <BUS.h>

#include <stdio.h>

#include <LibUnified/GUI/Window.h>
using namespace LibUnified::GUI;
using namespace LibUnified::Graphics;

surface_t GameWindow;
std::shared_ptr<Cartridge> cart;
Bus* nes;

void GameOnDraw(surface_t* window){
    // COPY VIDEOBUFFER TO THE WINDOW
    // window->Blit(&GameWindow);

    memset(window->buffer, 0xFF, window->width * window->height * 4);

    // return;
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
    Window* window = new Window((std::string("NES Emulator")).c_str(), {0, 0}, {0, 0}, WINDOW_TYPE_LOCKED);
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
        // do { nes->clock(); } while (!nes->ppu.frame_complete);
		// 	nes->ppu.frame_complete = false;

        // RENDER WINDOW
        window->Render();

        printf("NES: frame\n");
        fflush(stdout);
    }

    return 0;
}