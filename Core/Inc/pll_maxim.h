/*
 * pll_maxim.h
 *
 *  Created on: Mar 28, 2023
 *      Author: IvkoPivko
 */

#ifndef APPLICATION_USER_CORE_PLL_MAXIM_H_
#define APPLICATION_USER_CORE_PLL_MAXIM_H_

void EnableChip();
void DisableChip();

void EnableRFOutput();
void DisableRFOutput();

void init_PLL();
void write_to_PLL(uint32_t);
void read_from_PLL();
void enable_MUXOUT();



#endif /* APPLICATION_USER_CORE_PLL_MAXIM_H_ */
