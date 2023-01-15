#include "2A03.h"

using namespace UnifiedEmulation;
using namespace NES;

struct AudioPointers{
	bool* enable;
 	bool* halt;
    double* sample;
	double* output;
    APU2A03::sequencer* seq;
    APU2A03::oscpulse* osc;
    APU2A03::envelope* env;
    APU2A03::lengthcounter* lc;
    APU2A03::sweeper* sweep;
};

AudioPointers* P1;
AudioPointers* P2;
AudioPointers* N1;

bool* FirstRun = new bool(true);

bool* P1ThreadFin = new bool(true);
bool* P2ThreadFin = new bool(true);
bool* N1ThreadFin = new bool(true);

double* GTime = new double(0.0);

bool* HalfFrame = new bool(false);
bool* QuarterFrame = new bool(false);

bool* ThreadActive = new bool(true);

void Pulse1AudioThread(){
	while(*ThreadActive){
		if(!*P1ThreadFin){
			// Quater frame "beats" adjust the volume envelope
			if (*QuarterFrame)
			{
				P1->env->clock(*P1->halt);
			}

			// hald frame "beats" adjust the volume envelope
			if (*HalfFrame)
			{
				P1->lc->clock(*P1->enable, *P1->halt);
				P1->sweep->clock(P1->seq->reload, 0);
			}

			P1->osc->frequency = 1789773.0 / (16.0 * (double)(P1->seq->reload + 1));
			P1->osc->amplitude = (double)(P1->env->output -1) / 16.0;
			*P1->sample = P1->osc->sample(*GTime);

			if (P1->lc->counter > 0 && P1->seq->timer >= 8 && !P1->sweep->mute && P1->env->output > 2)
				*P1->output += (*P1->sample - *P1->output) * 0.5;
			else
				*P1->output = 0;

			*P1ThreadFin = true;
		}
	}
	return;
}

void Pulse2AudioThread(){
	while(*ThreadActive){
		if(!*P2ThreadFin){
			// Quater frame "beats" adjust the volume envelope
			if (*QuarterFrame)
			{
				P2->env->clock(*P2->halt);
			}

			// hald frame "beats" adjust the volume envelope
			if (*HalfFrame)
			{
				P2->lc->clock(*P2->enable, *P2->halt);
				P2->sweep->clock(P2->seq->reload, 0);
			}

			P2->osc->frequency = 1789773.0 / (16.0 * (double)(P2->seq->reload + 1));
			P2->osc->amplitude = (double)(P2->env->output-1) / 16.0;
			*P2->sample = P2->osc->sample(*GTime);

			if (P2->lc->counter > 0 && P2->seq->timer >= 8 && !P2->sweep->mute && P2->env->output > 2)
				*P2->output += (*P2->sample - *P2->output) * 0.5;
			else
				*P2->output = 0;
			*P2ThreadFin = true;
		}
	}
	return;
}

void NoiseAudioThread(){
	while(*ThreadActive){
		if(!*N1ThreadFin){
			// Quater frame "beats" adjust the volume envelope
			if (*QuarterFrame)
			{
				N1->env->clock(*N1->halt);
			}

			// hald frame "beats" adjust the volume envelope
			if (*HalfFrame)
			{
				N1->lc->clock(*N1->enable, *N1->halt);
			}

			N1->seq->clock(*N1->enable, [](uint32_t &s)
				{
					s = (((s & 0x0001) ^ ((s & 0x0002) >> 1)) << 14) | ((s & 0x7FFF) >> 1);
				});

			if (N1->lc->counter > 0 && N1->seq->timer >= 8)
			{
				*N1->output = (double)N1->seq->output * ((double)(N1->env->output-1) / 16.0);
			}
			*N1ThreadFin = true;
		}
	}
	return;
}

std::thread* P1T;
std::thread* P2T;
std::thread* N1T;

void GenerateOutputsThread(){
	if(*FirstRun){
		P1T = new std::thread(Pulse1AudioThread);
		P2T = new std::thread(Pulse2AudioThread);
		N1T = new std::thread(NoiseAudioThread);
		*FirstRun = false;
	}

	P1T->detach();
	P2T->detach();
	N1T->detach();

	while(ThreadActive){}

	return;
}

std::thread* MainGenThread;

uint8_t APU2A03::length_table[] = {  10, 254, 20,  2, 40,  4, 80,  6,
							        160,   8, 60, 10, 14, 12, 26, 14,
							         12,  16, 24, 18, 48, 20, 96, 22,
							        192,  24, 72, 26, 16, 28, 32, 30 };

APU2A03::APU2A03(){
	noise_seq.sequence = 0xDBDB;
}

APU2A03::~APU2A03(){
	*ThreadActive = false;
}

void APU2A03::cpuWrite(uint16_t addr, uint8_t data){
    switch (addr)
	{
	case 0x4000:
		switch ((data & 0xC0) >> 6){
		case 0x00: pulse1_seq.new_sequence = 0b01000000; pulse1_osc.dutycycle = 0.125; break;
		case 0x01: pulse1_seq.new_sequence = 0b01100000; pulse1_osc.dutycycle = 0.250; break;
		case 0x02: pulse1_seq.new_sequence = 0b01111000; pulse1_osc.dutycycle = 0.500; break;
		case 0x03: pulse1_seq.new_sequence = 0b10011111; pulse1_osc.dutycycle = 0.750; break;
		}
		pulse1_seq.sequence = pulse1_seq.new_sequence;
		pulse1_halt = (data & 0x20);
		pulse1_env.volume = (data & 0x0F);
		pulse1_env.disable = (data & 0x10);
		break;

	case 0x4001:
		pulse1_sweep.enabled = data & 0x80;
		pulse1_sweep.period = (data & 0x70) >> 4;
		pulse1_sweep.down = data & 0x08;
		pulse1_sweep.shift = data & 0x07;
		pulse1_sweep.reload = true;
		break;

	case 0x4002:
		pulse1_seq.reload = (pulse1_seq.reload & 0xFF00) | data;
		break;

	case 0x4003:
		pulse1_seq.reload = (uint16_t)((data & 0x07)) << 8 | (pulse1_seq.reload & 0x00FF);
		pulse1_seq.timer = pulse1_seq.reload;
		pulse1_seq.sequence = pulse1_seq.new_sequence;
		pulse1_lc.counter = length_table[(data & 0xF8) >> 3];
		pulse1_env.start = true;
		break;

	case 0x4004:
		switch ((data & 0xC0) >> 6){
		case 0x00: pulse2_seq.new_sequence = 0b01000000; pulse2_osc.dutycycle = 0.125; break;
		case 0x01: pulse2_seq.new_sequence = 0b01100000; pulse2_osc.dutycycle = 0.250; break;
		case 0x02: pulse2_seq.new_sequence = 0b01111000; pulse2_osc.dutycycle = 0.500; break;
		case 0x03: pulse2_seq.new_sequence = 0b10011111; pulse2_osc.dutycycle = 0.750; break;
		}
		pulse2_seq.sequence = pulse2_seq.new_sequence;
		pulse2_halt = (data & 0x20);
		pulse2_env.volume = (data & 0x0F);
		pulse2_env.disable = (data & 0x10);
		break;

	case 0x4005:
		pulse2_sweep.enabled = data & 0x80;
		pulse2_sweep.period = (data & 0x70) >> 4;
		pulse2_sweep.down = data & 0x08;
		pulse2_sweep.shift = data & 0x07;
		pulse2_sweep.reload = true;
		break;

	case 0x4006:
		pulse2_seq.reload = (pulse2_seq.reload & 0xFF00) | data;
		break;

	case 0x4007:
		pulse2_seq.reload = (uint16_t)((data & 0x07)) << 8 | (pulse2_seq.reload & 0x00FF);
		pulse2_seq.timer = pulse2_seq.reload;
		pulse2_seq.sequence = pulse2_seq.new_sequence;
		pulse2_lc.counter = length_table[(data & 0xF8) >> 3];
		pulse2_env.start = true;
		break;

	case 0x4008:
		break;

	case 0x400C:
		noise_env.volume = (data & 0x0F);
		noise_env.disable = (data & 0x10);
		noise_halt = (data & 0x20);
		break;

	case 0x400E:
		switch (data & 0x0F){
		case 0x00: noise_seq.reload = 0; break;
		case 0x01: noise_seq.reload = 4; break;
		case 0x02: noise_seq.reload = 8; break;
		case 0x03: noise_seq.reload = 16; break;
		case 0x04: noise_seq.reload = 32; break;
		case 0x05: noise_seq.reload = 64; break;
		case 0x06: noise_seq.reload = 96; break;
		case 0x07: noise_seq.reload = 128; break;
		case 0x08: noise_seq.reload = 160; break;
		case 0x09: noise_seq.reload = 202; break;
		case 0x0A: noise_seq.reload = 254; break;
		case 0x0B: noise_seq.reload = 380; break;
		case 0x0C: noise_seq.reload = 508; break;
		case 0x0D: noise_seq.reload = 1016; break;
		case 0x0E: noise_seq.reload = 2034; break;
		case 0x0F: noise_seq.reload = 4068; break;
		}
		break;

	case 0x4015: // APU STATUS
		pulse1_enable = data & 0x01;
		pulse2_enable = data & 0x01;
		noise_enable = data & 0x04;
		break;

	case 0x400F:
		pulse1_env.start = true;
		pulse2_env.start = true;
		noise_env.start = true;
		noise_lc.counter = length_table[(data & 0xF8) >> 3];
		break;
	}
}

uint8_t APU2A03::cpuRead(uint16_t addr){
    return 0x00;
}

#include <iostream>

void APU2A03::clock(){
	bool bQuarterFrameClock = false;
	bool bHalfFrameClock = false;

	dGlobalTime += (0.3333333333333 / 1789773);

    if(clock_counter % 6 == 0){
		frame_clock_counter++;

		
		// 4-Step Sequence Mode
		if (frame_clock_counter == 3729)
		{
			bQuarterFrameClock = true;
		}

		if (frame_clock_counter == 7457)
		{
			bQuarterFrameClock = true;
			bHalfFrameClock = true;
		}

		if (frame_clock_counter == 11186)
		{
			bQuarterFrameClock = true;
		}

		if (frame_clock_counter == 14916)
		{
			bQuarterFrameClock = true;
			bHalfFrameClock = true;
			frame_clock_counter = 0;
		}

		// Update functional units

		*HalfFrame = bHalfFrameClock;
		*QuarterFrame = bQuarterFrameClock;

		// Quater frame "beats" adjust the volume envelope
		/*if (bQuarterFrameClock)
		{
			pulse1_env.clock(pulse1_halt);
			pulse2_env.clock(pulse2_halt);
			noise_env.clock(noise_halt);
		}

		// hald frame "beats" adjust the volume envelope
		if (bHalfFrameClock)
		{
			pulse1_lc.clock(pulse1_enable, pulse1_halt);
			pulse2_lc.clock(pulse2_enable, pulse2_halt);
			noise_lc.clock(noise_enable, noise_halt);
			pulse1_sweep.clock(pulse1_seq.reload, 0);
			pulse2_sweep.clock(pulse2_seq.reload, 1);
		}*/

		if(*FirstRun){
			MainGenThread = new std::thread(GenerateOutputsThread);

			P1 = new AudioPointers{&pulse1_enable, &pulse1_halt, &pulse1_sample, &pulse1_output, &pulse1_seq, &pulse1_osc, &pulse1_env, &pulse1_lc, &pulse1_sweep};
			P2 = new AudioPointers{&pulse2_enable, &pulse2_halt, &pulse2_sample, &pulse2_output, &pulse2_seq, &pulse2_osc, &pulse2_env, &pulse2_lc, &pulse2_sweep};
			N1 = new AudioPointers{&noise_enable, &noise_halt, &noise_sample, &noise_output, &noise_seq, nullptr, &noise_env, &noise_lc, nullptr};
			
			GTime = &dGlobalTime;
			MainGenThread->detach();
		}

		////Note Half Speed
		if(clock_counter % 12 == 0){
			*P1ThreadFin = false;
			*P2ThreadFin = false;
			*N1ThreadFin = false;
		}
		
		while (!*P1ThreadFin || !*P2ThreadFin || !*N1ThreadFin) {}

		//Pulse 1
		/*pulse1_osc.frequency = 1789773.0 / (16.0 * (double)(pulse1_seq.reload + 1));
		pulse1_osc.amplitude = (double)(pulse1_env.output -1) / 16.0;
		pulse1_sample = pulse1_osc.sample(dGlobalTime);

		if (pulse1_lc.counter > 0 && pulse1_seq.timer >= 8 && !pulse1_sweep.mute && pulse1_env.output > 2)
			pulse1_output += (pulse1_sample - pulse1_output) * 0.5;
		else
			pulse1_output = 0;*/


		//Pulse 2
		/*pulse2_osc.frequency = 1789773.0 / (16.0 * (double)(pulse2_seq.reload + 1));
		pulse2_osc.amplitude = (double)(pulse2_env.output-1) / 16.0;
		pulse2_sample = pulse2_osc.sample(dGlobalTime);

		if (pulse2_lc.counter > 0 && pulse2_seq.timer >= 8 && !pulse2_sweep.mute && pulse2_env.output > 2)
			pulse2_output += (pulse2_sample - pulse2_output) * 0.5;
		else
			pulse2_output = 0;*/
			

		//Noise
		/*noise_seq.clock(noise_enable, [](uint32_t &s)
			{
				s = (((s & 0x0001) ^ ((s & 0x0002) >> 1)) << 14) | ((s & 0x7FFF) >> 1);
			});

		if (noise_lc.counter > 0 && noise_seq.timer >= 8)
		{
			noise_output = (double)noise_seq.output * ((double)(noise_env.output-1) / 16.0);
		}*/

		if (!pulse1_enable) pulse1_output = 0;
		if (!pulse2_enable) pulse2_output = 0;
		if (!noise_enable) noise_output = 0;

	}

	// Frequency sweepers change at high frequency
	pulse1_sweep.track(pulse1_seq.reload);
	pulse2_sweep.track(pulse2_seq.reload);

	pulse1_visual = (pulse1_enable && pulse1_env.output > 1 && !pulse1_sweep.mute) ? pulse1_seq.reload : 2047;
	pulse2_visual = (pulse2_enable && pulse2_env.output > 1 && !pulse2_sweep.mute) ? pulse2_seq.reload : 2047;
	noise_visual = (noise_enable && noise_env.output > 1) ? noise_seq.reload : 2047;

	clock_counter++;
}

void APU2A03::reset(){
    
}

double APU2A03::GetOutputSample(){
	return ((1.0 * pulse1_output) - 0.8) * 0.1 + 
		((1.0 * pulse2_output) - 0.8) * 0.1 +
		((2.0 * (noise_output - 0.5))) * 0.1;
}