#include <Mappers/001/Mapper_001.h>

Mapper_001::Mapper_001(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks)
{
	vRAMStatic.resize(32 * 1024);
}


Mapper_001::~Mapper_001()
{
}

bool Mapper_001::cpuMapRead(uint16_t addr, uint32_t &mapped_addr, uint8_t &data)
{
	if (addr >= 0x6000 && addr <= 0x7FFF)
	{
		// Read is from static ram on cartridge
		mapped_addr = 0xFFFFFFFF;

		// Read data from RAM
		data = vRAMStatic[addr & 0x1FFF];

		// Signal mapper has handled request
		return true;
	}

	if (addr >= 0x8000)
	{
		if (nControlRegister & 0b01000)
		{
			// 16K Mode
			if (addr >= 0x8000 && addr <= 0xBFFF)
			{
				mapped_addr = nPRGBankSelect16Lo * 0x4000 + (addr & 0x3FFF);
				return true;
			}

			if (addr >= 0xC000 && addr <= 0xFFFF)
			{
				mapped_addr = nPRGBankSelect16Hi * 0x4000 + (addr & 0x3FFF);
				return true;
			}
		}
		else
		{
			// 32K Mode
			mapped_addr = nPRGBankSelect32 * 0x8000 + (addr & 0x7FFF);
			return true;
		}
	}

	

	return false;
}

bool Mapper_001::cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data)
{
	if (addr >= 0x6000 && addr <= 0x7FFF)
	{
		// Write is to static ram on cartridge
		mapped_addr = 0xFFFFFFFF;

		// Write data to RAM
		vRAMStatic[addr & 0x1FFF] = data;

		// Signal mapper has handled request
		return true;
	}

	if (addr >= 0x8000)
	{
		if (data & 0x80)
		{
			// MSB is set, so reset serial loading
			nLoadRegister = 0x00;
			nLoadRegisterCount = 0;
			nControlRegister = nControlRegister | 0x0C;
		}
		else
		{
			// Load data in serially into load register
			// It arrives LSB first, so implant this at
			// bit 5. After 5 writes, the register is ready
			nLoadRegister >>= 1;
			nLoadRegister |= (data & 0x01) << 4;
			nLoadRegisterCount++;

			if (nLoadRegisterCount == 5)
			{
				// Get Mapper Target Register, by examining
				// bits 13 & 14 of the address
				uint8_t nTargetRegister = (addr >> 13) & 0x03;

				if (nTargetRegister == 0) // 0x8000 - 0x9FFF
				{
					// Set Control Register
					nControlRegister = nLoadRegister & 0x1F;

					switch (nControlRegister & 0x03)
					{
					case 0:	mirrormode = ONESCREEN_LO; break;
					case 1: mirrormode = ONESCREEN_HI; break;
					case 2: mirrormode = VERTICAL;     break;
					case 3:	mirrormode = HORIZONTAL;   break;
					}
				}
				else if (nTargetRegister == 1) // 0xA000 - 0xBFFF
				{
					// Set CHR Bank Lo
					if (nControlRegister & 0b10000) 
					{
						// 4K CHR Bank at PPU 0x0000
						nCHRBankSelect4Lo = nLoadRegister & 0x1F;
					}
					else
					{
						// 8K CHR Bank at PPU 0x0000
						nCHRBankSelect8 = nLoadRegister & 0x1E;
					}
				}
				else if (nTargetRegister == 2) // 0xC000 - 0xDFFF
				{
					// Set CHR Bank Hi
					if (nControlRegister & 0b10000)
					{
						// 4K CHR Bank at PPU 0x1000
						nCHRBankSelect4Hi = nLoadRegister & 0x1F;
					}
				}
				else if (nTargetRegister == 3) // 0xE000 - 0xFFFF
				{
					// Configure PRG Banks
					uint8_t nPRGMode = (nControlRegister >> 2) & 0x03;

					if (nPRGMode == 0 || nPRGMode == 1)
					{
						// Set 32K PRG Bank at CPU 0x8000
						nPRGBankSelect32 = (nLoadRegister & 0x0E) >> 1;
					}
					else if (nPRGMode == 2)
					{
						// Fix 16KB PRG Bank at CPU 0x8000 to First Bank
						nPRGBankSelect16Lo = 0;
						// Set 16KB PRG Bank at CPU 0xC000
						nPRGBankSelect16Hi = nLoadRegister & 0x0F;
					}
					else if (nPRGMode == 3)
					{
						// Set 16KB PRG Bank at CPU 0x8000
						nPRGBankSelect16Lo = nLoadRegister & 0x0F;
						// Fix 16KB PRG Bank at CPU 0xC000 to Last Bank
						nPRGBankSelect16Hi = nPRGBanks - 1;
					}
				}

				// 5 bits were written, and decoded, so
				// reset load register
				nLoadRegister = 0x00;
				nLoadRegisterCount = 0;
			}

		}

	}

	// Mapper has handled write, but do not update ROMs
	return false;
}

bool Mapper_001::ppuMapRead(uint16_t addr, uint32_t &mapped_addr)
{
	if (addr < 0x2000)
	{
		if (nCHRBanks == 0)
		{
			mapped_addr = addr;
			return true;
		}
		else
		{
			if (nControlRegister & 0b10000)
			{
				// 4K CHR Bank Mode
				if (addr >= 0x0000 && addr <= 0x0FFF)
				{
					mapped_addr = nCHRBankSelect4Lo * 0x1000 + (addr & 0x0FFF);
					return true;
				}

				if (addr >= 0x1000 && addr <= 0x1FFF)
				{
					mapped_addr = nCHRBankSelect4Hi * 0x1000 + (addr & 0x0FFF);
					return true;
				}
			}
			else
			{
				// 8K CHR Bank Mode
				mapped_addr = nCHRBankSelect8 * 0x2000 + (addr & 0x1FFF);
				return true;
			}
		}
	}	

	return false;
}

bool Mapper_001::ppuMapWrite(uint16_t addr, uint32_t &mapped_addr)
{
	if (addr < 0x2000)
	{
		if (nCHRBanks == 0)
		{
			mapped_addr = addr;
			return true;
		}

		return true;
	}
	else
		return false;
}

void Mapper_001::reset()
{
	nControlRegister = 0x1C;
	nLoadRegister = 0x00;
	nLoadRegisterCount = 0x00;
	
	nCHRBankSelect4Lo = 0;
	nCHRBankSelect4Hi = 0;
	nCHRBankSelect8 = 0;

	nPRGBankSelect32 = 0;
	nPRGBankSelect16Lo = 0;
	nPRGBankSelect16Hi = nPRGBanks - 1;
}

MIRROR Mapper_001::mirror()
{
	
	return mirrormode;
}