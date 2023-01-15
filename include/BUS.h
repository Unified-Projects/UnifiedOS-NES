#pragma once
#include <cstdint>
#include <array>

#include <CPU.h>
#include <PPU.h>
#include <Cartridge.h>

class Bus{
public:
    Bus();
    ~Bus();

public: //Devices
    //CPU
    CPU6502 cpu;

    //PPU
    PPU2C02 ppu;

    //APU
    // APU2A03 apu; APU Dissabled as no audio drivers yet :)
    
    //Ram
    uint8_t cpuRam[2048];

    //Cartridge
    std::shared_ptr<Cartridge> cart;

    // Controllers
    uint8_t controller[2];

    //Clock Count
    uint32_t SystemClockCount = 0;
    uint32_t ClockSpeedCounter = 0;

public: //Bus Read and Write
    void cpuWrite(uint16_t addr, uint8_t data);
    uint8_t cpuRead(uint16_t addr, bool bReadOnly = false);

    double dAudioSample = 0.0;
    void SetSampleFrequency(uint32_t sample_rate);

private:
    double dAudioTimePerSystemSample = 0.0;
    double dAudioTimePerNESClock = 0.0;
    double dAudioTime = 0.0;
    double dAudioGlobalTime = 0.0;

public: //Interface
    void insertCartridge(const std::shared_ptr<Cartridge>& cartridge);
    void reset();
    bool clock();

private:
    //Clock Cycles Passed
    uint32_t nSystemClockCounter = 0;

    // Internal cache of controller state
    uint8_t controller_state[2];

    uint8_t dma_page = 0x00;
    uint8_t dma_addr = 0x00;
    uint8_t dma_data = 0x00;

    bool dma_transfer = false;
    bool dma_dummy = true;
};