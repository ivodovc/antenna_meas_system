/*
 * measurements.c
 *
 *  Created on: Apr 25, 2023
 *      Author: IvkoPivko
 */
#include "pll_maxim.h"
#include "gpio.h"
#include "stdlib.h"
#include "adc_control.h";

void freq_char(uint32_t from, uint32_t to, uint32_t step){
	char* buffer = malloc(3000);
	uint32_t freq = 0;
	buffer[0] = 0;
	for (int i = from; i<to; i+=step){
		  	  freq = i;
		  	  set_requested_frequency(freq);

		  	  GPIO_PinState lock_detect = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
		  	  while(lock_detect!=GPIO_PIN_SET){
		  		  lock_detect = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
		  	  }
		  	  float voltage = read_voltage();
		  	  uint16_t raw = read_raw();
		  	  sprintf(buffer + strlen(buffer), "%d, %d, %.5f;", freq, raw, voltage);
		  	  if (i%100 == 0){
		  		  printf(buffer);
		  		  printf("\n");
		  		  buffer[0] = 0;
		  	  }
		  	  // log values
		  	  //printf( "%d, %d, %.5f \n", freq, raw, voltage);
	 }
	 printf(buffer);
}
