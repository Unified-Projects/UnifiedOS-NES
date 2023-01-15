#pragma once
#include <cstdint>
#include <cstring>
#include <memory>
#include "../Cartridge/Cartridge.h"

#include <UnifiedEngine/Core/Renderer/PixelRender.h>
using namespace UnifiedEngine;

namespace UnifiedEmulation {
    namespace NES {
        class PPU2C02{
        public:
            PPU2C02();
            ~PPU2C02();
        private:
            //Vram
            uint8_t tblName[2][1024];
            uint8_t tblPalette[32];
            uint8_t tblPattern[2][4096];

        public:
            //Comunication with main bus
            uint8_t cpuRead(uint16_t addr, bool readOnly = false);
            void cpuWrite(uint16_t addr, uint8_t data);

            //Comunication with ppu bus
            uint8_t ppuRead(uint16_t addr, bool readOnly = false);
            void ppuWrite(uint16_t addr, uint8_t data);

        private:
            //Cartridge
            std::shared_ptr<Cartridge> cart;
        public:
            //Interface
            void ConnectCartridge(const std::shared_ptr<Cartridge>& cartridge);
            void clock();
            void reset();

            bool nmi = false;

            bool scanline_trigger = false;
        
        private:
            vec3 palScreen[0x40];
            PixelImage sprScreen;
            PixelImage sprNameTable[2];
            PixelImage sprPatternTable[2];
        public:
            PixelImage& GetScreen();
            PixelImage& GetNameTable(uint8_t i);
            PixelImage& GetPatternTable(uint8_t i, uint8_t palette);
            bool frame_complete = false;
            vec3 GetColorFromPaletteRam(uint8_t palette, uint8_t pixel);

        private:
            int16_t scanline = 0;
            int16_t cycle = 0;
            bool odd_frame = false;

            union{
                struct{
                    uint8_t unused : 5;
                    uint8_t spriteoverflow : 1;
                    uint8_t sprite_zero_hit : 1;
                    uint8_t vertical_blank : 1;
                };

                uint8_t reg;
            } status;

            union{
                struct{
                    uint8_t grayscale : 1;
                    uint8_t render_background_left : 1;
                    uint8_t render_sprites_left : 1;
                    uint8_t render_background : 1;
                    uint8_t render_sprites : 1;
                    uint8_t enhance_red : 1;
                    uint8_t enhance_green : 1;
                    uint8_t enhance_blue : 1;
                };

                uint8_t reg;
            } mask;

            union PPUCTRL{
                struct{
                    uint8_t nametable_x : 1;
                    uint8_t nametable_y : 1;
                    uint8_t increment_mode : 1;
                    uint8_t pattern_sprite : 1;
                    uint8_t pattern_background : 1;
                    uint8_t sprite_size : 1;
                    uint8_t slave_mode : 1; // unused
                    uint8_t enable_nmi : 1;
                };

                uint8_t reg;
            } control;

            uint8_t address_latch = 0x00;
            uint8_t ppu_data_buffer = 0x00;

            union loopy_register
            {
                struct
                {

                    uint16_t coarse_x : 5;
                    uint16_t coarse_y : 5;
                    uint16_t nametable_x : 1;
                    uint16_t nametable_y : 1;
                    uint16_t fine_y : 3;
                    uint16_t unused : 1;
                };

                uint16_t reg = 0x0000;
            };
            
            
            loopy_register vram_addr;
            loopy_register tram_addr;

            uint8_t fine_x = 0x00;

            uint8_t bg_next_tile_id     = 0x00;
            uint8_t bg_next_tile_attrib = 0x00;
            uint8_t bg_next_tile_lsb    = 0x00;
            uint8_t bg_next_tile_msb    = 0x00;
            uint16_t bg_shifter_pattern_lo = 0x0000;
            uint16_t bg_shifter_pattern_hi = 0x0000;
            uint16_t bg_shifter_attrib_lo  = 0x0000;
            uint16_t bg_shifter_attrib_hi  = 0x0000;
        
        private:
            struct sObjectAttributeEntry{
                uint8_t y;
                uint8_t id;
                uint8_t attribute;
                uint8_t x;
            } OAM[64];
        
        public:
            uint8_t* pOAM = (uint8_t*)OAM;

            uint8_t oam_addr = 0x00;

            sObjectAttributeEntry spriteScanline[8];
            uint8_t sprite_count;
            uint8_t sprite_shifter_pattern_lo[8];
            uint8_t sprite_shifter_pattern_hi[8];

            bool bSpriteZeroBeingRendered = false;
            bool bSpriteZeroHitPossible = false;
        };
    }
}