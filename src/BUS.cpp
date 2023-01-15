#include <BUS.h>

Bus::Bus(){
    cpu.ConnectBus(this);

    //Clear Ram
    for (auto &i : cpuRam) i = 0x00;
}

Bus::~Bus(){

}

void Bus::cpuWrite(uint16_t addr, uint8_t data){
    if(cart->cpuWrite(addr, data)){

    }
    else if(addr >= 0x0000 && addr <= 0x1FFF){
        cpuRam[addr & 0x07FF] = data;
    }
    else if (addr >= 0x2000 && addr <= 0x3FFF){
        ppu.cpuWrite(addr & 0x0007, data);
    }
    else if ((addr >= 0x4000 && addr <= 0x4013) || addr == 0x4015 || addr == 0x4017){
		// apu.cpuWrite(addr, data);
	}
    else if(addr == 0x4014){
        dma_page = data;
        dma_addr = 0x00;
        dma_transfer = true;
    }
    else if(addr >= 0x4016 && addr <=0x4017){
        controller_state[addr & 0x001] = controller[addr & 0x0001];
    }
}

uint8_t Bus::cpuRead(uint16_t addr, bool bReadOnly){
	uint8_t data = 0x00;
	if (cart->cpuRead(addr, data))
	{
		// Cartridge Address Range
	}
	else if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		// System RAM Address Range, mirrored every 2048
		data = cpuRam[addr & 0x07FF];
	}
	else if (addr >= 0x2000 && addr <= 0x3FFF)
	{
		// PPU Address range, mirrored every 8
		data = ppu.cpuRead(addr & 0x0007, bReadOnly);
	}
    else if (addr == 0x4015)
	{
		// APU Read Status
		// data = apu.cpuRead(addr);
	}
    else if(addr >= 0x4016 && addr <=0x4017){
        data = (controller_state[addr & 0x0001] & 0x80) > 0;
        controller_state[addr & 0x0001] <<= 1;
    }

	return data;
}

void Bus::insertCartridge(const std::shared_ptr<Cartridge>& cartridge){
    this->cart = cartridge;
    ppu.ConnectCartridge(cartridge);
}

void Bus::reset(){
    cart->reset();
	cpu.reset();
	ppu.reset();
	nSystemClockCounter = 0;
	dma_page = 0x00;
	dma_addr = 0x00;
	dma_data = 0x00;
	dma_dummy = true;
	dma_transfer = false;
}

bool Bus::clock(){
    ppu.clock();

    // apu.clock();

    if(nSystemClockCounter % 3 == 0){
        if(dma_transfer){
            if(dma_dummy){
                if(nSystemClockCounter % 2 == 1){
                    dma_dummy = false;
                }
            }
            else{
                if(nSystemClockCounter % 2 == 0){
                    dma_data = cpuRead(dma_page << 8 | dma_addr);
                }
                else{
                    ppu.pOAM[dma_addr] = dma_data;
                    dma_addr++;

                    if(dma_addr == 0x00){
                        dma_transfer = false;
                        dma_dummy = true;
                    }
                }
            }
        }
        else{
            cpu.clock();
        }
    }

    //Sync with audio
    // bool bAudioSampleReady = false;
	// dAudioTime += dAudioTimePerNESClock;
	// if (dAudioTime >= dAudioTimePerSystemSample){
	// 	dAudioTime -= dAudioTimePerSystemSample;
	// 	dAudioSample = apu.GetOutputSample();
	// 	bAudioSampleReady = true;
	// }

    if(ppu.nmi){
        ppu.nmi = false;
        cpu.nmi();
    }

    //if (cart->GetMapper()->irqState())
	//{
		//cart->GetMapper()->irqClear();
		//cpu.irq();		
	//}


    nSystemClockCounter++;
    
    ClockSpeedCounter++;
    SystemClockCount++;

    return bAudioSampleReady;
}

void Bus::SetSampleFrequency(uint32_t sample_rate){
    dAudioTimePerSystemSample = 1.0 / (double)sample_rate;
    dAudioTimePerNESClock = 1.0 / 5369318.0;
}