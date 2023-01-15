#include <Mappers/Mapper.h>

Mapper::Mapper(uint8_t prgBanks, uint8_t chrBanks){
    nPRGBanks = prgBanks;
    nCHRBanks = chrBanks;
    return;
}

Mapper::~Mapper(){
    return;
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
    return;
}

void Mapper::scanline()
{
    return;
}