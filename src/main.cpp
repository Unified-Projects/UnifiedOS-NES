// THE MAIN FILE FOR SETUPS AND MANAGEMENT OF THE PROCESS
#include <BUS.h>

#include <LibUnified/GUI/Window.h>
using namespace LibUnified::GUI;
using namespace LibUnified::Graphics;

surface_t GameWindow;
std::shared_ptr<Cartridge> cart;
Bus* nes;

void GameOnDraw(surface_t* window){
    // TOT COPY VIDEOBUFFER TO THE WINDOW
    window->Blit(&GameWindow);
}

int main(int argc, char *argv[]){
    // CHECK FOR ARGS IF NOT FAIL
    if (argc <= 1 || !argv){
        return -1; // No file inputed
    }

    if(!argv[1] /*File*/){
        return -1;
    }

    // SETUP EMULATOR
    nes = new Bus;

    // LOAD THE CARTRIDGE
	cart = std::make_shared<Cartridge>(argv[1]);
    nes->insertCartridge(cart);
    nes->reset();

    // Setup game surface
    GameWindow.width = nes->ppu.GetScreen().size.x;
    GameWindow.height = nes->ppu.GetScreen().size.y;
    GameWindow.buffer = nes->ppu.GetScreen().buffer;

    // Create a window
    Window* window = new Window((std::string("NES Emulator")).c_str(), {0, 0}, {0, 0}, WINDOW_TYPE_LOCKED);
    window->OnDraw = GameOnDraw;

    for(;;){ // GAME LOOP
        // POLL WINDOW (For input purposes)
        window->Poll();

        // CLOCK EMULATOR
        do { nes->clock(); } while (!nes->ppu.frame_complete);
			nes->ppu.frame_complete = false;

        // RENDER WINDOW
        window->Render();
    }

    return 0;
}