#pragma once

#include <cstdint>
#include <random>
#include <string.h>


const unsigned int KEY_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVELS = 16;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;


class Chip8 {
	std::default_random_engine randGen;
	std::uniform_int_distribution<uint8_t> randByte;
public:
	uint8_t registers[16]{};
	uint8_t memory[4096]{};
	uint16_t index{};
	uint16_t pc{};
	uint16_t stack[16]{};
	uint8_t sp{};
	uint8_t delayTimer{};
	uint8_t soundTimer{};
	uint8_t keypad[16]{};
	uint32_t video[64 * 32]{};
	uint16_t opcode;

	// ???


	typedef void (Chip8::*Chip8Func)();
	Chip8Func table[0xF + 1];
	Chip8Func table0[0xE + 1];
	Chip8Func table8[0xE + 1];
	Chip8Func tableE[0xE + 1];
	Chip8Func tableF[0x65 + 1];	


	Chip8();

	void LoadROM(const char* filename);

	void OP_00E0() { //CLS
		memset(video, 0, sizeof(video));
	}

	void OP_00EE() { // RET
		--sp;
		pc = stack[sp];
	}

	void OP_1nnn() { // JMP
		uint16_t address = opcode & 0x0FFFu;
		pc = address;
	}

	void OP_2nnn() { // CALL
		uint16_t address = opcode & 0x0FFFu;
		stack[sp] = address;
		sp++;
		pc = address;
	}


	void OP_3xkk() { // skip if Vx = kk
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t byte = opcode * 0x00FFu;

		if (registers[Vx] == byte) {
			pc += 2;
		}
	}

	void OP_4xkk() { // skip if Vx != kk
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t byte = opcode * 0x00FFu;

		if (registers[Vx] != byte) {
			pc += 2;
		}
	}

	void OP_5xy0() { // skip if Vx = Vy
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x0F00u) >> 4u;

		if (registers[Vx] == registers[Vy]) {
			pc += 2;
		}
	}

	void OP_6xkk() { // set Vx to kk
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t byte = opcode * 0x00FFu;

		registers[Vx] = byte;
	}

	void OP_7xkk() { // Vx = Vx + kk;
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t byte = opcode * 0x00FFu;

		registers[Vx] += byte;
	}

	void OP_8xy0() { // set Vx = Vy
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x0F00u) >> 4u;

		registers[Vx] = registers[Vy];
	}

	void OP_8xy1() { // set Vx = Vx | Vy
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x0F00u) >> 4u;

		registers[Vx] |= registers[Vy];
	}

	void OP_8xy2() { // set Vx = Vx & Vy
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x0F00u) >> 4u;

		registers[Vx] &= registers[Vy];
	}

	void OP_8xy3() { // set Vx = Vx ^ Vy
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x0F00u) >> 4u;

		registers[Vx] ^= registers[Vy];
	}

	void OP_8xy4() { // set Vx = Vx + Vy, VF = carry
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x0F00u) >> 4u;

		uint16_t sum = registers[Vx] + registers[Vy];

		if (sum > 255U) {
			registers[0xF] = 1;
		} else {
			registers[0xF] = 0;
		}

		registers[Vx] = sum & 0xFFu;
	}	

	void OP_8xy5() { // set Vx = Vx - Vy, VF = NOT borrw
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;

		if (registers[Vx] > registers[Vy]) {
			registers[0xF] = 1;
		}
		else {
			registers[0xF] = 0;
		}

		registers[Vx] -= registers[Vy];
	}

	void OP_8xy6() { // Vx = Vx >> 1 (shift right). If LSB of Vx = 1, VF = 1, else 0. Vx /= 2
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		// Save LSB in VF
		registers[0xF] = (registers[Vx] & 0x1u);

		registers[Vx] >>= 1;
	}

	void OP_8xy7() { // set Vx = Vy - Vx, VF = NOT carry
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;

		if (registers[Vy] > registers[Vx]) {
			registers[0xF] = 1;
		}
		else {
			registers[0xF] = 0;
		}

		registers[Vx] = registers[Vy] - registers[Vx];
	}	

	void OP_8xyE() { // Vx = Vx << 1, VF = 1 if MSB of Vx = 1, else 0. Vx *= 2
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		// Save MSB in VF
		registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

		registers[Vx] <<= 1;
	}

	void OP_9xy0() { // Skip instruction if Vx != Vy
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;

		if (registers[Vx] != registers[Vy]) {
			pc += 2;
		}
	}

	void OP_Annn() { // set I = nnn
		uint16_t address = opcode & 0x0FFFu;
		index = address;
	}

	void OP_Bnnn() { // Jump to location nnn + V0
		uint16_t address = opcode & 0x0FFFu;

		pc = registers[0] + address;
	}

	void OP_Cxkk() { // set Vx = randomByte & kk
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t byte = opcode & 0x00FFu;

		registers[Vx] = randByte(randGen) & byte;
	}

	void OP_Dxyn(); // draw sprite starting at I (index) at (Vx, Vy), set VF = collision

	void OP_Ex9E() { // Skip if key with the value of Vx is pressd
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		uint8_t key = registers[Vx];

		if (keypad[key])
		{
			pc += 2;
		}
	}

	void OP_ExA1() { // Skip if key with the value of Vx is NOT pressd
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		uint8_t key = registers[Vx];

		if (!keypad[key])
		{
			pc += 2;
		}
	}	

	void OP_Fx07() { // set Vx to delay timer
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		registers[Vx] = delayTimer;
	}

	void OP_Fx0A(); // wait for a key press, store the value of key into Vx

	void OP_Fx15() { // set delayTimer = Vx
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		delayTimer = registers[Vx];
	}	

	void OP_Fx18() { // set soundTimer = Vx
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		soundTimer = registers[Vx];
	}

	void OP_Fx1E() { // set I (index) = I + Vx
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		index += registers[Vx];
	}

	void OP_Fx29() { // set I = location of sprite for digit Vx
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t digit = registers[Vx];

		index = FONTSET_START_ADDRESS + (5 * digit);
	}

	void OP_Fx33() { // Store BCD representation of Vx in memory locations I, I+1, and I+2.
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t value = registers[Vx];

		// Ones-place
		memory[index + 2] = value % 10;
		value /= 10;

		// Tens-place
		memory[index + 1] = value % 10;
		value /= 10;

		// Hundreds-place
		memory[index] = value % 10;
	}

	void OP_Fx55() { // Store registers V0 through Vx in memory starting at location I.
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		for (uint8_t i = 0; i <= Vx; ++i) {
			memory[index + i] = registers[i];
		}
	}

	void OP_Fx65() { // Read registers V0 through Vx from memory starting at location I.
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		for (uint8_t i = 0; i <= Vx; ++i)
		{
			registers[i] = memory[index + i];
		}
	}

	void Table0() {
		((*this).*(table0[opcode & 0x000Fu]))();
	}

	void Table8() {
		((*this).*(table8[opcode & 0x000Fu]))();
	}

	void TableE() {
		((*this).*(tableE[opcode & 0x000Fu]))();
	}

	void TableF() {
		((*this).*(tableF[opcode & 0x00FFu]))();
	}

	void OP_NULL() {}

	void Cycle();

};