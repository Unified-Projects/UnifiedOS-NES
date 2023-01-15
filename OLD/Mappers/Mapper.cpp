#include "Mapper.h"

using namespace UnifiedEmulation;
using namespace NES;

Mapper::Mapper(uint8_t prgBanks, uint8_t chrBanks){
    nPRGBanks = prgBanks;
    nCHRBanks = chrBanks;
}

Mapper::~Mapper(){
    
}

MIRROR Mapper::mirror()
{
	return MIRROR::HARDWARE;
}

bool Mapper::irqState()
{
	return false;
}

void Mapper::irqClear()
{
}

void Mapper::scanline()
{
}