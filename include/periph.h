#ifndef _PERIPH_H_
#define _PERIPH_H_

#include <stdint.h>

#define ADC_NCHAN 1
uint16_t channel_values[ADC_NCHAN];

void gpio_setup(void);
void adc_setup(void);
void dma_setup(void);
void tim3_setup(void);

#endif
