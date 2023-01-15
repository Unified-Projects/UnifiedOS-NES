#include "2C02.h"

using namespace UnifiedEmulation;
using namespace NES;

PPU2C02::PPU2C02()
    : sprScreen(ivec2(256, 240)), sprNameTable{PixelImage(ivec2(256, 256)), PixelImage(ivec2(256, 256))}, sprPatternTable{PixelImage(ivec2(128, 128)), PixelImage(ivec2(128, 128))}
{
    palScreen[0x00] = vec3(84, 84, 84);
	palScreen[0x01] = vec3(0, 30, 116);
	palScreen[0x02] = vec3(8, 16, 144);
	palScreen[0x03] = vec3(48, 0, 136);
	palScreen[0x04] = vec3(68, 0, 100);
	palScreen[0x05] = vec3(92, 0, 48);
	palScreen[0x06] = vec3(84, 4, 0);
	palScreen[0x07] = vec3(60, 24, 0);
	palScreen[0x08] = vec3(32, 42, 0);
	palScreen[0x09] = vec3(8, 58, 0);
	palScreen[0x0A] = vec3(0, 64, 0);
	palScreen[0x0B] = vec3(0, 60, 0);
	palScreen[0x0C] = vec3(0, 50, 60);
	palScreen[0x0D] = vec3(0, 0, 0);
	palScreen[0x0E] = vec3(0, 0, 0);
	palScreen[0x0F] = vec3(0, 0, 0);

	palScreen[0x10] = vec3(152, 150, 152);
	palScreen[0x11] = vec3(8, 76, 196);
	palScreen[0x12] = vec3(48, 50, 236);
	palScreen[0x13] = vec3(92, 30, 228);
	palScreen[0x14] = vec3(136, 20, 176);
	palScreen[0x15] = vec3(160, 20, 100);
	palScreen[0x16] = vec3(152, 34, 32);
	palScreen[0x17] = vec3(120, 60, 0);
	palScreen[0x18] = vec3(84, 90, 0);
	palScreen[0x19] = vec3(40, 114, 0);
	palScreen[0x1A] = vec3(8, 124, 0);
	palScreen[0x1B] = vec3(0, 118, 40);
	palScreen[0x1C] = vec3(0, 102, 120);
	palScreen[0x1D] = vec3(0, 0, 0);
	palScreen[0x1E] = vec3(0, 0, 0);
	palScreen[0x1F] = vec3(0, 0, 0);

	palScreen[0x20] = vec3(236, 238, 236);
	palScreen[0x21] = vec3(76, 154, 236);
	palScreen[0x22] = vec3(120, 124, 236);
	palScreen[0x23] = vec3(176, 98, 236);
	palScreen[0x24] = vec3(228, 84, 236);
	palScreen[0x25] = vec3(236, 88, 180);
	palScreen[0x26] = vec3(236, 106, 100);
	palScreen[0x27] = vec3(212, 136, 32);
	palScreen[0x28] = vec3(160, 170, 0);
	palScreen[0x29] = vec3(116, 196, 0);
	palScreen[0x2A] = vec3(76, 208, 32);
	palScreen[0x2B] = vec3(56, 204, 108);
	palScreen[0x2C] = vec3(56, 180, 204);
	palScreen[0x2D] = vec3(60, 60, 60);
	palScreen[0x2E] = vec3(0, 0, 0);
	palScreen[0x2F] = vec3(0, 0, 0);

	palScreen[0x30] = vec3(236, 238, 236);
	palScreen[0x31] = vec3(168, 204, 236);
	palScreen[0x32] = vec3(188, 188, 236);
	palScreen[0x33] = vec3(212, 178, 236);
	palScreen[0x34] = vec3(236, 174, 236);
	palScreen[0x35] = vec3(236, 174, 212);
	palScreen[0x36] = vec3(236, 180, 176);
	palScreen[0x37] = vec3(228, 196, 144);
	palScreen[0x38] = vec3(204, 210, 120);
	palScreen[0x39] = vec3(180, 222, 120);
	palScreen[0x3A] = vec3(168, 226, 144);
	palScreen[0x3B] = vec3(152, 226, 180);
	palScreen[0x3C] = vec3(160, 214, 228);
	palScreen[0x3D] = vec3(160, 162, 160);
	palScreen[0x3E] = vec3(0, 0, 0);
	palScreen[0x3F] = vec3(0, 0, 0);
}

PPU2C02::~PPU2C02(){

}


uint8_t PPU2C02::cpuRead(uint16_t addr, bool rdonly)
{
	uint8_t data = 0x00;

	if (rdonly)
	{
		switch (addr)
		{
		case 0x0000: // Control
			data = control.reg;
			break;
		case 0x0001: // Mask
			data = mask.reg;
			break;
		case 0x0002: // Status
			data = status.reg;
			break;
		case 0x0003: // OAM Address
			break;
		case 0x0004: // OAM Data
			break;
		case 0x0005: // Scroll
			break;
		case 0x0006: // PPU Address
			break;
		case 0x0007: // PPU Data
			break;
		}
	}
	else
	{
		switch (addr)
		{
			// Control - Not readable
		case 0x0000: break;
		
			// Mask - Not Readable
		case 0x0001: break;
		
			// Status
		case 0x0002:
			data = (status.reg & 0xE0) | (ppu_data_buffer & 0x1F);

			// Clear the vertical blanking flag
			status.vertical_blank = 0;

			// Reset Loopy's Address latch flag
			address_latch = 0;
			break;

			// OAM Address - Not Readable
		case 0x0003: break;

			// OAM Data
		case 0x0004: 
			data = pOAM[oam_addr];
			break;

			// Scroll - Not Readable
		case 0x0005: break;

			// PPU Address - Not Readable
		case 0x0006: break;

			// PPU Data
		case 0x0007: 
			data = ppu_data_buffer;

			ppu_data_buffer = ppuRead(vram_addr.reg);

			if (vram_addr.reg >= 0x3F00) data = ppu_data_buffer;

			vram_addr.reg += (control.increment_mode ? 32 : 1);
			break;
		}
	}

	return data;
}

void PPU2C02::cpuWrite(uint16_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0x0000: // Control
		control.reg = data;
		tram_addr.nametable_x = control.nametable_x;
		tram_addr.nametable_y = control.nametable_y;
		break;
	case 0x0001: // Mask
		mask.reg = data;
		break;
	case 0x0002: // Status
		break;
	case 0x0003: // OAM Address
		oam_addr = data;
		break;
	case 0x0004: // OAM Data
		pOAM[oam_addr] = data;
		break;
	case 0x0005: // Scroll
		if(address_latch == 0){
			fine_x = data & 0x07;
			tram_addr.coarse_x = data >> 3;
			address_latch = 1;
		}
		else{
			tram_addr.fine_y = data & 0x07;
			tram_addr.coarse_y = data >> 3;
			address_latch = 0;
		}
		break;
	case 0x0006: // PPU Address
		if(address_latch == 0){
			tram_addr.reg = (tram_addr.reg & 0x00FF) | (data << 8);
			address_latch = 1;
		}
		else{
			tram_addr.reg = (tram_addr.reg & 0xFF00) | data;
			vram_addr = tram_addr;
			address_latch = 0;
		}
		break;
	case 0x0007: // PPU Data
		ppuWrite(vram_addr.reg, data);
		vram_addr.reg += (control.increment_mode ? 32 : 1);
		break;
	}
}

uint8_t PPU2C02::ppuRead(uint16_t addr, bool rdonly)
{
	uint8_t data = 0x00;
	addr &= 0x3FFF;

	if (cart->ppuRead(addr, data))
	{

	}
	else if(addr >= 0x0000 && addr <= 0x1FFF){
		data = tblPattern[(addr & 0x1000) >> 12][addr & 0x0FFF];
	}
	else if (addr >= 0x2000 && addr <= 0x3EFF)
	{
		addr &= 0x0FFF;

		if (cart->Mirror() == MIRROR::VERTICAL)
		{
			// Vertical
			if (addr >= 0x0000 && addr <= 0x03FF)
				data = tblName[0][addr & 0x03FF];
			if (addr >= 0x0400 && addr <= 0x07FF)
				data = tblName[1][addr & 0x03FF];
			if (addr >= 0x0800 && addr <= 0x0BFF)
				data = tblName[0][addr & 0x03FF];
			if (addr >= 0x0C00 && addr <= 0x0FFF)
				data = tblName[1][addr & 0x03FF];
		}
		else if (cart->Mirror() == MIRROR::HORIZONTAL)
		{
			// Horizontal
			if (addr >= 0x0000 && addr <= 0x03FF)
				data = tblName[0][addr & 0x03FF];
			if (addr >= 0x0400 && addr <= 0x07FF)
				data = tblName[0][addr & 0x03FF];
			if (addr >= 0x0800 && addr <= 0x0BFF)
				data = tblName[1][addr & 0x03FF];
			if (addr >= 0x0C00 && addr <= 0x0FFF)
				data = tblName[1][addr & 0x03FF];
		}
	}
	else if (addr >= 0x3F00 && addr <= 0x3FFF)
	{
		addr &= 0x001F;
		if (addr == 0x0010) addr = 0x0000;
		if (addr == 0x0014) addr = 0x0004;
		if (addr == 0x0018) addr = 0x0008;
		if (addr == 0x001C) addr = 0x000C;
		data = tblPalette[addr] & (mask.grayscale ? 0x30 : 0x3F);
	}

	return data;
}

void PPU2C02::ppuWrite(uint16_t addr, uint8_t data)
{
	addr &= 0x3FFF;

	if (cart->ppuWrite(addr, data))
	{

	}
	else if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		tblPattern[(addr & 0x1000) >> 12][addr & 0x0FFF] = data;
	}
	else if (addr >= 0x2000 && addr <= 0x3EFF)
	{
		addr &= 0x0FFF;
		if (cart->Mirror() == MIRROR::VERTICAL)
		{
			// Vertical
			if (addr >= 0x0000 && addr <= 0x03FF)
				tblName[0][addr & 0x03FF] = data;
			if (addr >= 0x0400 && addr <= 0x07FF)
				tblName[1][addr & 0x03FF] = data;
			if (addr >= 0x0800 && addr <= 0x0BFF)
				tblName[0][addr & 0x03FF] = data;
			if (addr >= 0x0C00 && addr <= 0x0FFF)
				tblName[1][addr & 0x03FF] = data;
		}
		else if (cart->Mirror() == MIRROR::HORIZONTAL)
		{
			// Horizontal
			if (addr >= 0x0000 && addr <= 0x03FF)
				tblName[0][addr & 0x03FF] = data;
			if (addr >= 0x0400 && addr <= 0x07FF)
				tblName[0][addr & 0x03FF] = data;
			if (addr >= 0x0800 && addr <= 0x0BFF)
				tblName[1][addr & 0x03FF] = data;
			if (addr >= 0x0C00 && addr <= 0x0FFF)
				tblName[1][addr & 0x03FF] = data;
		}
	}
	else if (addr >= 0x3F00 && addr <= 0x3FFF)
	{
		addr &= 0x001F;
		if (addr == 0x0010) addr = 0x0000;
		if (addr == 0x0014) addr = 0x0004;
		if (addr == 0x0018) addr = 0x0008;
		if (addr == 0x001C) addr = 0x000C;
		tblPalette[addr] = data;
	}
}

void PPU2C02::reset()
{
	fine_x = 0x00;
	address_latch = 0x00;
	ppu_data_buffer = 0x00;
	scanline = 0;
	cycle = 0;
	bg_next_tile_id = 0x00;
	bg_next_tile_attrib = 0x00;
	bg_next_tile_lsb = 0x00;
	bg_next_tile_msb = 0x00;
	bg_shifter_pattern_lo = 0x0000;
	bg_shifter_pattern_hi = 0x0000;
	bg_shifter_attrib_lo = 0x0000;
	bg_shifter_attrib_hi = 0x0000;
	status.reg = 0x00;
	mask.reg = 0x00;
	control.reg = 0x00;
	vram_addr.reg = 0x0000;
	tram_addr.reg = 0x0000;
	scanline_trigger = false;
	odd_frame = false;
}

void PPU2C02::ConnectCartridge(const std::shared_ptr<Cartridge>& cartridge)
{
	this->cart = cartridge;
}

PixelImage& PPU2C02::GetScreen(){
    return this->sprScreen;
}

PixelImage& PPU2C02::GetNameTable(uint8_t i){
    return this->sprNameTable[i];
}

PixelImage& PPU2C02::GetPatternTable(uint8_t i, uint8_t palette){
	for(uint16_t nTileY = 0; nTileY < 16; nTileY++){
		for(uint16_t nTileX = 0; nTileX < 16; nTileX++){
			uint16_t nOffset = nTileY * 256 + nTileX * 16;

			for(uint16_t row = 0; row < 8; row++){
				uint8_t tile_lsb = ppuRead(i * 0x1000 + nOffset + row + 0x0000);
				uint8_t tile_msb = ppuRead(i * 0x1000 + nOffset + row + 0x0008);

				for(uint16_t col = 0; col < 8; col++){
					uint8_t pixel = ((tile_lsb & 0x01) << 1) | (tile_msb & 0x01);
					tile_lsb >>= 1; tile_msb >>= 1;

					sprPatternTable[i].SetPixel(
						ivec2(
							nTileX * 8 + (7 - col),
							nTileY * 8 + row
						),
						GetColorFromPaletteRam(palette, pixel)
					);
				}
			}
		}
	}

    return this->sprPatternTable[i];
}

vec3 PPU2C02::GetColorFromPaletteRam(uint8_t palette, uint8_t pixel){
	return palScreen[ppuRead(0x3F00 + (palette << 2) + pixel) & 0x3F];
}

void PPU2C02::clock(){
	auto IncrementScrollX = [&]()
	{
		if (mask.render_background || mask.render_sprites)
		{
			if (vram_addr.coarse_x == 31)
			{
				vram_addr.coarse_x = 0;
				vram_addr.nametable_x = ~vram_addr.nametable_x;
			}
			else
			{
				vram_addr.coarse_x++;
			}
		}
	};

	auto IncrementScrollY = [&]()
	{

		if (mask.render_background || mask.render_sprites)
		{
			if (vram_addr.fine_y < 7)
			{
				vram_addr.fine_y++;
			}
			else
			{

				vram_addr.fine_y = 0;

				if (vram_addr.coarse_y == 29)
				{
					vram_addr.coarse_y = 0;
					vram_addr.nametable_y = ~vram_addr.nametable_y;
				}
				else if (vram_addr.coarse_y == 31)
				{
					vram_addr.coarse_y = 0;
				}
				else
				{
					vram_addr.coarse_y++;
				}
			}
		}
	};

	auto TransferAddressX = [&]()
	{
		if (mask.render_background || mask.render_sprites)
		{
			vram_addr.nametable_x = tram_addr.nametable_x;
			vram_addr.coarse_x    = tram_addr.coarse_x;
		}
	};

	auto TransferAddressY = [&]()
	{
		if (mask.render_background || mask.render_sprites)
		{
			vram_addr.fine_y      = tram_addr.fine_y;
			vram_addr.nametable_y = tram_addr.nametable_y;
			vram_addr.coarse_y    = tram_addr.coarse_y;
		}
	};

	auto LoadBackgroundShifters = [&]()
	{	

		bg_shifter_pattern_lo = (bg_shifter_pattern_lo & 0xFF00) | bg_next_tile_lsb;
		bg_shifter_pattern_hi = (bg_shifter_pattern_hi & 0xFF00) | bg_next_tile_msb;

		bg_shifter_attrib_lo  = (bg_shifter_attrib_lo & 0xFF00) | ((bg_next_tile_attrib & 0b01) ? 0xFF : 0x00);
		bg_shifter_attrib_hi  = (bg_shifter_attrib_hi & 0xFF00) | ((bg_next_tile_attrib & 0b10) ? 0xFF : 0x00);
	};

	auto UpdateShifters = [&]()
	{
		if (mask.render_background)
		{
			bg_shifter_pattern_lo <<= 1;
			bg_shifter_pattern_hi <<= 1;

			bg_shifter_attrib_lo <<= 1;
			bg_shifter_attrib_hi <<= 1;
		}

		if (mask.render_sprites && cycle >= 1 && cycle < 258){
			for (int i = 0; i < sprite_count; i++){
				if (spriteScanline[i].x > 0){
					spriteScanline[i].x--;
				}
				else{
					sprite_shifter_pattern_lo[i] <<= 1;
					sprite_shifter_pattern_hi[i] <<= 1;
				}
			}
		}
	};

	if(scanline >= -1 && scanline < 240){
		if (scanline == 0 && cycle == 0 && odd_frame && (mask.render_background || mask.render_sprites))
		{
			// "Odd Frame" cycle skip
			cycle = 1;
		}

		if(scanline == -1 && cycle == 1){
			status.vertical_blank = 0;
			status.sprite_zero_hit = 0;
			status.spriteoverflow = 0;

			for(int i = 0; i < 8; i++){
				sprite_shifter_pattern_lo[i] = 0;
				sprite_shifter_pattern_hi[i] = 0;
			}
		}
			

		if ((cycle >= 2 && cycle < 258) || (cycle >= 321 && cycle < 338))
		{
			UpdateShifters();
			
			switch ((cycle - 1) % 8)
			{
			case 0:
				LoadBackgroundShifters();

				bg_next_tile_id = ppuRead(0x2000 | (vram_addr.reg & 0x0FFF));

				break;
			case 2:				
				bg_next_tile_attrib = ppuRead(0x23C0 | (vram_addr.nametable_y << 11) 
					                                 | (vram_addr.nametable_x << 10) 
					                                 | ((vram_addr.coarse_y >> 2) << 3) 
					                                 | (vram_addr.coarse_x >> 2));
					
				if (vram_addr.coarse_y & 0x02) bg_next_tile_attrib >>= 4;
				if (vram_addr.coarse_x & 0x02) bg_next_tile_attrib >>= 2;

				bg_next_tile_attrib &= 0x03;
				break;
			case 4: 
				bg_next_tile_lsb = ppuRead((control.pattern_background << 12) 
					                       + ((uint16_t)bg_next_tile_id << 4) 
					                       + (vram_addr.fine_y) + 0);

				break;
			case 6:
				bg_next_tile_msb = ppuRead((control.pattern_background << 12)
					                       + ((uint16_t)bg_next_tile_id << 4)
					                       + (vram_addr.fine_y) + 8);
				break;
			case 7:
				IncrementScrollX();
				break;
			}
		}

		if(cycle == 256){
			IncrementScrollY();
		}

		if(cycle == 257){
			TransferAddressX();
		}

		if(scanline == -1 && cycle >= 280 && cycle < 305){
			TransferAddressY();
		}

		if(cycle == 338 || cycle == 340){
			bg_next_tile_id = ppuRead(0x2000 | (vram_addr.reg & 0x0FFF));
		}

		if(cycle == 257 && scanline >= 0){
			std::memset(spriteScanline, 0xFF, 8 * sizeof(sObjectAttributeEntry));
			sprite_count = 0;

			uint8_t nOAMEntry = 0;
			bSpriteZeroHitPossible = false;
			while (nOAMEntry < 64 && sprite_count < 9){
				int16_t diff ((int16_t)scanline - (int16_t)OAM[nOAMEntry].y);
				if(diff >= 0 && diff < (control.sprite_size ? 16 : 8)){
					if(sprite_count < 8){

						if(nOAMEntry == 0){
							bSpriteZeroHitPossible = true;
						}

						memcpy(&spriteScanline[sprite_count], &OAM[nOAMEntry], sizeof(sObjectAttributeEntry));
						sprite_count++;
					}
				}
				nOAMEntry++;
			}
			status.spriteoverflow = (sprite_count > 8);
		}

		if(cycle == 340){
			for (uint8_t i = 0; i < sprite_count; i++){
				uint8_t sprite_pattern_bits_lo, sprite_pattern_bits_hi;
				uint16_t sprite_pattern_addr_lo, sprite_pattern_addr_hi;

				if (!control.sprite_size)
				{
					if (!(spriteScanline[i].attribute & 0x80))
					{    
						sprite_pattern_addr_lo = 
						  (control.pattern_sprite << 12)
						| (spriteScanline[i].id << 4)
						| (scanline - spriteScanline[i].y);
												
					}
					else
					{
						sprite_pattern_addr_lo = 
						  (control.pattern_sprite << 12)
						| (spriteScanline[i].id << 4)
						| (7 - (scanline - spriteScanline[i].y));
					}

				}
				else
				{
					if (!(spriteScanline[i].attribute & 0x80))
					{
						if (scanline - spriteScanline[i].y < 8)
						{
							sprite_pattern_addr_lo = 
							  ((spriteScanline[i].id & 0x01) << 12)
							| ((spriteScanline[i].id & 0xFE) << 4 )
							| ((scanline - spriteScanline[i].y) & 0x07 );
						}
						else
						{
							sprite_pattern_addr_lo = 
							  ( (spriteScanline[i].id & 0x01) << 12)
							| (((spriteScanline[i].id & 0xFE) + 1) << 4)
							| ((scanline - spriteScanline[i].y) & 0x07);
						}
					}
					else
					{
						if (scanline - spriteScanline[i].y < 8)
						{
							sprite_pattern_addr_lo = 
							  ( (spriteScanline[i].id & 0x01) << 12)
							| (((spriteScanline[i].id & 0xFE) + 1) << 4)
							| (7 - (scanline - spriteScanline[i].y) & 0x07);
						}
						else
						{
							sprite_pattern_addr_lo = 
							  ((spriteScanline[i].id & 0x01) << 12)
							| ((spriteScanline[i].id & 0xFE) << 4 )
							| (7 - (scanline - spriteScanline[i].y) & 0x07);
						}
					}
				}

				sprite_pattern_addr_hi = sprite_pattern_addr_lo + 8;

				sprite_pattern_bits_lo = ppuRead(sprite_pattern_addr_lo);
				sprite_pattern_bits_hi = ppuRead(sprite_pattern_addr_hi);

				if(spriteScanline[i].attribute & 0x40){
					auto flipbyte = [](uint8_t b)
					{
						b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
						b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
						b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
						return b;
					};

					sprite_pattern_bits_lo = flipbyte(sprite_pattern_bits_lo);
					sprite_pattern_bits_hi = flipbyte(sprite_pattern_bits_hi);
				}
				
				sprite_shifter_pattern_lo[i] = sprite_pattern_bits_lo;
				sprite_shifter_pattern_hi[i] = sprite_pattern_bits_hi;
			}
		}
	}	

	if(scanline == 240){

	}

	if(scanline == 241 && cycle == 1){
		status.vertical_blank = 1;
		if(control.enable_nmi){
			nmi = true;
		}
	}

	uint8_t bg_pixel = 0x00;
	uint8_t bg_palette = 0x00;

	if(mask.render_background){
		uint16_t bit_mux = 0x8000 >> fine_x;

		uint8_t p0_pixel = (bg_shifter_pattern_lo & bit_mux) > 0;
		uint8_t p1_pixel = (bg_shifter_pattern_hi & bit_mux) > 0;

		bg_pixel = (p1_pixel << 1) | p0_pixel;

		// Get palette
		uint8_t bg_pal0 = (bg_shifter_attrib_lo & bit_mux) > 0;
		uint8_t bg_pal1 = (bg_shifter_attrib_hi & bit_mux) > 0;
		bg_palette = (bg_pal1 << 1) | bg_pal0;
	}

	uint8_t fg_pixel = 0x00;
	uint8_t fg_palette = 0x00;
	uint8_t fg_priority = 0x00;
	
	if (mask.render_sprites)
	{
		bSpriteZeroBeingRendered = false;

		for (uint8_t i = 0; i < sprite_count; i++)
		{
			if (spriteScanline[i].x == 0) 
			{
				uint8_t fg_pixel_lo = (sprite_shifter_pattern_lo[i] & 0x80) > 0;
				uint8_t fg_pixel_hi = (sprite_shifter_pattern_hi[i] & 0x80) > 0;
				fg_pixel = (fg_pixel_hi << 1) | fg_pixel_lo;

				fg_palette = (spriteScanline[i].attribute & 0x03) + 0x04;
				fg_priority = (spriteScanline[i].attribute & 0x20) == 0;

				if (fg_pixel != 0)
				{
					if (i == 0)
					{
						bSpriteZeroBeingRendered = true;
					}

					break;
				}				
			}
		}		
	}

	uint8_t pixel = 0x00;
	uint8_t palette = 0x00;

	if(bg_pixel == 0 && fg_pixel == 0){
		pixel = 0x00;
		palette = 0x00;
	}
	else if (bg_pixel == 0 && fg_pixel > 0)
	{
		pixel = fg_pixel;
		palette = fg_palette;
	}
	else if (bg_pixel > 0 && fg_pixel == 0)
	{
		pixel = bg_pixel;
		palette = bg_palette;
	}
	else if (bg_pixel > 0 && fg_pixel > 0)
	{
		if (fg_priority)
		{
			pixel = fg_pixel;
			palette = fg_palette;
		}
		else
		{
			pixel = bg_pixel;
			palette = bg_palette;
		}

		if (bSpriteZeroHitPossible && bSpriteZeroBeingRendered)
		{
			if (mask.render_background & mask.render_sprites)
			{
				if (~(mask.render_background_left | mask.render_sprites_left))
				{
					if (cycle >= 9 && cycle < 258)
					{
						status.sprite_zero_hit = 1;
					}
				}
				else
				{
					if (cycle >= 1 && cycle < 258)
					{
						status.sprite_zero_hit = 1;
					}
				}
			}
		}
	}

    //TextureGen
    sprScreen.SetPixel(ivec2(cycle-1, scanline), GetColorFromPaletteRam(palette, pixel));

    //Advance Renderer
    cycle++;
	if(mask.render_background || mask.render_sprites)
		if (cycle == 260 && scanline < 240)
		{
			cart->GetMapper()->scanline();
		}

	if (cycle >= 341){
		cycle = 0;
		scanline++;
		if (scanline >= 261){
			scanline = -1;
			frame_complete = true;
			odd_frame = !odd_frame;
		}
	}
}