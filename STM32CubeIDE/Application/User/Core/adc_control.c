/*
 * adc_control.c
 *
 *  Created on: Apr 17, 2023
 *      Author: IvkoPivko
 */

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "gpio.h"

float VDDA = 3.2;


float read_voltage(){
	 float voltage;
	 HAL_ADC_Start(&hadc1);
	 HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	 uint16_t raw = HAL_ADC_GetValue(&hadc1);
	 HAL_ADC_Stop(&hadc1);
	 voltage =raw*VDDA/4095;
	 //printf("voltage in adc_control %.5f\n", voltage);
	 return voltage;
}

uint16_t read_raw(){
		 HAL_ADC_Start(&hadc1);
		 HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
		 uint16_t raw = HAL_ADC_GetValue(&hadc1);
		 HAL_ADC_Stop(&hadc1);
		 //printf("voltage in adc_control %.5f\n", voltage);
		 return raw;
}

uint32_t calibrate(){
	uint16_t* VREFINT_CAL_p =0x1FFF75AA;
	uint16_t VREFINT_CAL = *VREFINT_CAL_p;
	printf("VREFINT_CAL: %d\n", VREFINT_CAL);
	setVREFINTChannel();
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	uint16_t VREFINT_DATA = HAL_ADC_GetValue(&hadc1);
	printf("VREFINT_DATA: %d\n", VREFINT_DATA);
	HAL_ADC_Stop(&hadc1);

	set5Channel();

	VDDA = 3.6 * ((float)VREFINT_CAL/(float)VREFINT_DATA);
	printf("%.5f VDDA \n", VDDA);
}


