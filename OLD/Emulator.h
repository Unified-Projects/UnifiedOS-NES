#include "./Bus/Bus.h"
#include "./CPU/6502.h"
#include "./PPU/2C02.h"
#include "./SoundPacer.h"

namespace UnifiedEmulation {
    namespace NES {
        class NESEmulator{
        private:
            Bus* system;
        public:
            NESEmulator(){
                this->system = new Bus;
            }

            ~NESEmulator(){

            }

            void Update(){

            }

            Bus* GetSystem(){
                return this->system;
            }
        };
    }
}