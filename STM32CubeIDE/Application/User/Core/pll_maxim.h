/*
 * pll_maxim.h
 *
 *  Created on: Mar 28, 2023
 *      Author: IvkoPivko
 */

#ifndef APPLICATION_USER_CORE_PLL_MAXIM_H_
#define APPLICATION_USER_CORE_PLL_MAXIM_H_

 #include <stdint.h>
void EnableChip();
void DisableChip();

void EnableRFOutput();
void DisableRFOutput();

void setRFA_PWR(uint8_t rfa_pwr);
void set_DIVA(uint8_t);

void init_PLL();
void read_from_PLL();

void Register_Shutdown();

uint8_t write_to_PLL(uint32_t data);
void write_reg(uint32_t data);

void set_requested_frequency(uint32_t);
void set_requested_frequency_NDIV(float freq, uint32_t NDIV);

void program_PLL();

#endif /* APPLICATION_USER_CORE_PLL_MAXIM_H_ */
