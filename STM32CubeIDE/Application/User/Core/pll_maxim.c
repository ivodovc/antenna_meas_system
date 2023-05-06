/*
 * pll_maxim.c
 *
 * Used to control MAX2870 PLL through GPIO pins and SPI interface
 *
 *  Created on: Mar 26, 2023
 *      Author: IvkoPivko
 */
#include "gpio.h"
#include "spi.h"
#include "pll_maxim.h"
#include <stdio.h>
#include <math.h>

// default register values
const uint32_t default_reg0 = 0x007D0000;
const uint32_t default_reg1 = 0x2000FFF9;
const uint32_t default_reg2 = 0x00004042;
const uint32_t default_reg3 = 0x0000000B;
const uint32_t default_reg4 = 0x6180B23C;
const uint32_t default_reg5 = 0x00400005;
const uint32_t readback_reg6 = 0x00000006;

uint32_t reg0 = 0;
uint32_t reg1 = 0;
uint32_t reg2 = 0;
uint32_t reg3 = 0;
uint32_t reg4 = 0;
uint32_t reg5 = 0;
uint32_t reg6 = 0;

/**
  * @brief read Lock Detect from MAX
  * @retval GPIO_PinState Lock Detect
  */
GPIO_PinState max_read_LD(){
	//HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
	return 0;
}

/**
  * @brief Turn MAX2870 PLL on
  * By setting CE pin high
  * @retval
  */
void EnableChip(){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
}

void DisableChip(){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
}

void EnableRFOutput(){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
}

void DisableRFOutput(){
 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
}

void print_registers(){
	printf("reg0: %x\n", reg0);
	printf("reg1: %x\n", reg1);
	printf("reg2: %x\n", reg2);
	printf("reg3: %x\n", reg3);
	printf("reg4: %x\n", reg4);
	printf("reg5: %x\n", reg5);
}

void setBandSelect(uint16_t bandSelect){
	//first separated 2 MSB
	uint32_t bandSelect_2MSB_MASK = (0b11 << 24);
	uint32_t bandSelect_7LSB_MASK = (0b1111111 << 12);
	uint32_t bandSelect_2MSB = (bandSelect & (0b11 << 8)) >> 8;
	uint32_t bandSelect_7LSB = bandSelect & (0b11111111);
	// write 2MSB
	uint32_t reg4_1 = reg4 & ~(bandSelect_2MSB_MASK);
	uint32_t reg4_2 = reg4_1 | (bandSelect_2MSB<<24);
	// write 7LSB
	uint32_t reg4_3 = reg4_2 & ~(bandSelect_7LSB_MASK);
	uint32_t reg4_4 = reg4_3 | (bandSelect_7LSB<<12);
	write_reg(reg4_4);
}

void Register_Shutdown(){
	uint32_t SHDN_BIT = 1 << 5;
	write_reg(SHDN_BIT | reg2);
}

void Register_Startup(){
	uint32_t SHDN_BIT = 1 << 5;
	write_reg(reg2 & ~(SHDN_BIT));
}

// permitted n values: 1, 2, 4, 8, 16, 32, 64, 128
void setDIVA(uint8_t n){
	uint32_t diva_value = 0;
	//sets DIVA to 111
	switch (n){
		case 128:
			diva_value = 0b111 << 20;
			break;
		case 64:
			diva_value = 0b110 << 20;
			break;
		case 32:
			diva_value = 0b101 << 20;
			break;
		case 16:
			diva_value = 0b100 << 20;
			break;
		case 8:
			diva_value = 0b011 << 20;
			break;
		case 4:
			diva_value = 0b010 << 20;
			break;
		case 2:
			diva_value = 0b001 << 20;
			break;
		case 1:
			diva_value = 0b000 << 20;
			break;
		default:
			// do not set diva mask
			return;
			break;
	}
	uint32_t DIVA_MASK = 0b111 << 20;
	uint32_t new_reg4 = reg4 & ~(DIVA_MASK);
	uint32_t newnewreg4 = new_reg4 | ((diva_value)&DIVA_MASK);
	write_reg(newnewreg4);
}

//00, 01, 02, 03
void setRFA_PWR(uint8_t rfa_pwr){
	if (rfa_pwr > 4 || rfa_pwr < 0){
		return;//invalid value
	}
	// bits 4 and 3
	uint32_t RFA_PWR_MASK = 0b11 << 3;
	// set bits to zero
	uint32_t newreg4 = reg4 & ~(RFA_PWR_MASK);
	uint32_t newnewreg4 = newreg4 | ((rfa_pwr << 3)&RFA_PWR_MASK);
	write_reg(newnewreg4);
}

void setN(uint16_t N){
	// bits 4 and 3
	uint32_t N_MASK = 0xFFFF << 15;
	// set bits to zero
	uint32_t newreg0 = reg0 & ~(N_MASK);
	uint32_t newnewreg0 = newreg0 | ((N << 15)&N_MASK);
	write_reg(newnewreg0);
}

void setR(uint16_t R){
	// bits 4 and 3
	uint32_t R_MASK = 0xFFC << 12;
	// set bits to zero
	uint32_t newreg2 = reg2 & ~(R_MASK);
	uint32_t newnewreg2 = newreg2 | ((R << 14)&R_MASK);
	write_reg(newnewreg2);
}

void setFB_MUX(uint8_t FB_MUX){
	uint32_t FB_MUX_MASK = 0b1 << 23;
	uint32_t newreg4 = reg4 & ~(FB_MUX_MASK);
	uint32_t newnewreg4 = newreg4 | ((FB_MUX << 23)&FB_MUX_MASK);
	write_reg(newnewreg4);
}

void enableRFA(){
	write_reg(reg4 | (1<<5));
}

void setIntegerMode(){
	write_reg(reg0 | (1<<31));
}

uint32_t R, DIVA;
// all units in MHz
void sweep(uint32_t from, uint32_t to, uint32_t step){
	//set Divider to 128
	setDIVA(128);
	setR(1);
	R=1;
	DIVA=128;
	sweep_N(94, 186, 4);

	setDIVA(64);
	DIVA/=2;
	setR(1);
	sweep_N(94, 187, 2);

	setDIVA(32);
	DIVA/=2;
	setR(1);
	sweep_N(94, 187, 1);

	setDIVA(16);
	DIVA/=2;
	setR(2);
	R=2;
	sweep_N(188, 379, 1);

	setDIVA(8);
	setR(4);
	R=4;
	DIVA/=2;
	sweep_N(375, 750, 1);

	setDIVA(4);
	setR(8);
	R=8;
	DIVA/=2;
	sweep_N(750, 1500, 1);

	setDIVA(2);
	setR(16);
	R=16;
	DIVA/=2;
	sweep_N(1500, 3000, 1);
}

// freq is in MHz
void set_requested_frequency(uint32_t freq){
	// first determine required DIVA value
	float divided_by_235 = freq/23.5;
	float band = log2(divided_by_235);
	uint32_t band_i = band;
	uint32_t DIVA_caluclated = 128>>band_i;

	uint32_t R_value = 1;
	uint32_t N_1MHZ_step = 1;
	if (DIVA_caluclated<32){
		R_value = 32/DIVA_caluclated;
	}
	if (DIVA_caluclated>32){
		N_1MHZ_step = DIVA_caluclated/32;
	}
	uint32_t N_value = N_1MHZ_step * freq;
	//printf("%d\n", DIVA_caluclated);
	//printf("R: %d\n", R_value);
	//printf("N: %d\n", N_value);
	setN(N_value);
	setDIVA(DIVA_caluclated);
	setR(R_value);
	program_PLL();
	// give time to allow PLL to lock into frequency
	HAL_Delay(1);
}

void sweep_N(uint32_t from, uint32_t to, uint32_t step){

	step = 20*step;
	for (int i=from; i<to; i+=step){
		setN(i);
		printf("frequency: %d MHz\n", 32*i/R/DIVA);
		program_PLL();
		HAL_Delay(20);
	}
}
/**
  * @brief Initialize chip as specified in datasheet
  * @retval GPIO_PinState Lock Detect
  */
void init_PLL(){
	// Zapis registrov default hodnotami
	write_reg(default_reg5);
	// disable RF outputs
	write_reg(default_reg4 & ~((1 << 5) | (1<<8)));
	write_reg(default_reg3);
	write_reg(default_reg2);
	write_reg(default_reg1);
	write_reg(default_reg0);
	program_PLL();
	// 20 ms between writes as specified in datasheet
	HAL_Delay(20);
	write_reg(default_reg5);
	write_reg(default_reg4 & ~((1 << 5) | (1<<8)));
	write_reg(default_reg3);
	write_reg(default_reg2);
	write_reg(default_reg1);
	write_reg(default_reg0);
	program_PLL();
	HAL_Delay(20);
}

void write_regs_SOFT(){
	//write registers as specified in MAX emulator
	uint32_t reg0 = 0x80320000;
	uint32_t reg1 = 0x80033E81;
	uint32_t reg2 = 0x0C004042;
	uint32_t reg3 = 0x00000133;
	uint32_t reg4 = 0x629802FC;
	uint32_t reg5 = 0x00400005;
	write_reg(reg5);
	write_reg(reg4);
	write_reg(reg3);
	write_reg(reg2);
	write_reg(reg1);
	write_reg(reg0);
	program_PLL();
}

// big endian to little endian for 32bits
uint32_t swap_words(uint32_t rozumne){
	uint32_t swapped = (rozumne >> 16) | (rozumne << 16);
	return swapped;
}

// preforms write to register
uint8_t write_reg(uint32_t data){
	// determine register address
	uint32_t reg_addr = data & 0b111;

	switch (reg_addr){
			case 0:
				reg0 = data;
				break;
			case 1:
				reg1 = data;
				break;
			case 2:
				reg2 = data;
				break;
			case 3:
				reg3 = data;
				break;
			case 4:
				reg4 = data;
				break;
			case 5:
				reg5 = data;
				break;
			default:
				// invalid address
				return 1;
				break;
	}
}

// sends all registers to PLL
void program_PLL(){
	write_to_PLL(reg5);
	write_to_PLL(reg4);
	write_to_PLL(reg3);
	write_to_PLL(reg2);
	write_to_PLL(reg1);
	write_to_PLL(reg0);
}

uint8_t write_to_PLL(uint32_t data){
	// endianness swap
	uint32_t swapped_data = swap_words(data);
	uint16_t* pData = (uint16_t*)&swapped_data;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_Delay(1);
	//HAL_StatusTypeDef result =  HAL_SPI_Transmit(&hspi1, (uint8_t*)pData, 2, 1000);
	HAL_SPI_Transmit_DMA(&hspi1, (uint8_t*)pData, 2);
	//printf("SPI Transmit result: %x\n", result);
	HAL_Delay(1);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

	return 0;
}

void enable_MUXOUT(){
	uint32_t MUX_enabled_mask = 1 << 28 | 1 << 27;
	uint32_t MUX_enabled_reg2 =  reg2 | MUX_enabled_mask;
	write_reg(MUX_enabled_reg2);
}

void read_from_PLL() {
	uint16_t readback_send[5] = {0, 6, 0, 0, 0};
	uint32_t buffer = 0;
	// initiate readback by sending addres of register 6
	uint32_t readBackWrite = 6; // zeroes followed by 6
	uint32_t swapped = swap_words(readBackWrite);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&swapped, 2, 1000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_SPI_Receive(&hspi1, (uint8_t*)&buffer, 2, 1000);
	uint32_t swapped_buffer = swap_words(buffer);
	printf("Readback1: %x ", ((uint16_t*)buffer)[0]);
	printf("%x\n",  ((uint16_t*)buffer)[1]);

}
