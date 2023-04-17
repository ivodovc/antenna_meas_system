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
	write_to_PLL(reg4_4);
}

void Register_Shutdown(){
	uint32_t SHDN_BIT = 1 << 5;
	write_to_PLL(SHDN_BIT | reg2);
}

void Register_Startup(){
	uint32_t SHDN_BIT = 1 << 5;
	write_to_PLL(reg2 & ~(SHDN_BIT));
}

void setDIVA(uint8_t n){
	//sets DIVA to 111
	uint32_t DIVA_MASK = 0b111 << 20;
	uint32_t new_reg4 = reg4 & ~(DIVA_MASK);
	uint32_t newnewreg4 = new_reg4 | (n<<20);
	write_to_PLL(newnewreg4);
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
	write_to_PLL(newnewreg4);
}

void setN(uint16_t N){
	// bits 4 and 3
	uint32_t N_MASK = 0xFFFF << 15;
	// set bits to zero
	uint32_t newreg0 = reg0 & ~(N_MASK);
	uint32_t newnewreg0 = newreg0 | ((N << 15)&N_MASK);
	write_to_PLL(newnewreg0);
}

void setR(uint16_t R){
	// bits 4 and 3
	uint32_t R_MASK = 0xFFC << 14;
	// set bits to zero
	uint32_t newreg2 = reg2 & ~(R_MASK);
	uint32_t newnewreg2 = newreg2 | ((R_MASK << 15)&R_MASK);
	write_to_PLL(newnewreg2);
}

void setFB_MUX(uint8_t FB_MUX){
	uint8_t FB_MUX_MASK = 0b1 << 23;
	uint32_t newreg4 = reg4 & ~(FB_MUX_MASK);
	uint32_t newnewreg4 = newreg4 | ((FB_MUX << 23)&FB_MUX_MASK);
	write_to_PLL(newnewreg4);
}

void enableRFA(){
	write_to_PLL(reg4 | (1<<5));
}

void setIntegerMode(){
	write_to_PLL(reg0 | (1<<31));
}

/**
  * @brief Initialize chip as specified in datasheet
  * @retval GPIO_PinState Lock Detect
  */
void init_PLL(){
	// Zapis registrov default hodnotami
	write_to_PLL(default_reg5);
	HAL_Delay(30);
	// disable RF outputs
	write_to_PLL(default_reg4 & ~((1 << 5) | (1<<8)));
	write_to_PLL(default_reg3);
	write_to_PLL(default_reg2);
	write_to_PLL(default_reg1);
	write_to_PLL(default_reg0);
	// 20 ms between writes as specified in datasheet
	HAL_Delay(30);
	write_to_PLL(default_reg5);
	write_to_PLL(default_reg4 & ~((1 << 5) | (1<<8)));
	write_to_PLL(default_reg3);
	write_to_PLL(default_reg2);
	write_to_PLL(default_reg1);
	write_to_PLL(default_reg0);
	HAL_Delay(30);
}

void write_regs_SOFT(){
	//write registers as specified in MAX emulator
	uint32_t reg0 = 0x80320000;
	uint32_t reg1 = 0x8000FFF9;
	uint32_t reg2 = 0x0C004042;
	uint32_t reg3 = 0x0000000B;
	uint32_t reg4 = 0x62A8023C;
	uint32_t reg5 = 0x00400005;
	write_to_PLL(reg5);
	write_to_PLL(reg4);
	write_to_PLL(reg3);
	write_to_PLL(reg2);
	write_to_PLL(reg1);
	write_to_PLL(reg0);

	/*write_to_PLL(default_reg0);
	write_to_PLL(default_reg1);
	write_to_PLL(0x0C010042); // reg 2 is modified for R value
	write_to_PLL(default_reg3);
	write_to_PLL(default_reg4);
	write_to_PLL(default_reg5);*/
}

// big endian to little endian for 32bits
uint32_t swap_words(uint32_t rozumne){
	uint32_t swapped = (rozumne >> 16) | (rozumne << 16);
	return swapped;
}

uint8_t write_to_PLL(uint32_t data){
	// endianness swap
	uint32_t swapped_data = swap_words(data);
	uint16_t* pData = (uint16_t*)&swapped_data;
	// determine register address:
	uint32_t reg_addr = data & 0b111;
	// save transmitted data to variable
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

	//printf("pData %x\n", *(uint32_t*)pData);
	//printf("data: %x\n", data);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_Delay(1);
	//HAL_StatusTypeDef result =  HAL_SPI_Transmit(&hspi1, (uint8_t*)pData, 2, 1000);
	HAL_SPI_Transmit_DMA(&hspi1, (uint8_t*)pData, 2);
	//printf("SPI Transmit result: %x\n", result);
	HAL_Delay(1);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	/*for (int i=0; i<2; i++){
		printf("writing to SPI: ");
		printf("%x\n", *(pData+i));
		printf("\n");
	}*/
	return 0;
}

void enable_MUXOUT(){
	uint32_t MUX_enabled_mask = 1 << 28 | 1 << 27;
	uint32_t MUX_enabled_reg2 =  reg2 | MUX_enabled_mask;
	write_to_PLL(MUX_enabled_reg2);
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
