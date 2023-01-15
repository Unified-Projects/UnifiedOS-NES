#pragma once
#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#include <memory>

#include "../Mappers/000/Mapper_000.h"
#include "../Mappers/001/Mapper_001.h"
#include "../Mappers/002/Mapper_002.h"
#include "../Mappers/003/Mapper_003.h"
#include "../Mappers/004/Mapper_004.h"
#include "../Mappers/066/Mapper_066.h"

namespace UnifiedEmulation {
    namespace NES {
        class Cartridge{
        public:
            Cartridge(const std::string& sFileName);
            ~Cartridge();

        public:
	        bool ImageValid();

        private:
	        bool bImageValid = false;
            MIRROR hw_mirror = HORIZONTAL;

            std::vector<uint8_t> vPRGMemory;
            std::vector<uint8_t> vCHRMemory;

            uint8_t nMapperID = 0;
            uint8_t nPRGBanks = 0;
            uint8_t nCHRBanks = 0;

            std::shared_ptr<Mapper> pMapper;

        public:
            //Comunication with main bus
            bool cpuRead(uint16_t addr, uint8_t &data);
            bool cpuWrite(uint16_t addr, uint8_t data);

            //Comunication with ppu bus
            bool ppuRead(uint16_t addr, uint8_t &data);
            bool ppuWrite(uint16_t addr, uint8_t data);

            // Permits system rest of mapper to know state
	        void reset();

            // Get Mirror configuration
	        MIRROR Mirror();

            std::shared_ptr<Mapper> GetMapper();
        };
    }
}