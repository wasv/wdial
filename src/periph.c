#include "periph.h"

#include <stdint.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

void gpio_setup(void)
{
    /* Enable GPIOC clock. */
    rcc_periph_clock_enable(RCC_GPIOC);

    /* Set GPIO13 (in GPIOC) to 'output push-pull'. */
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}

void adc_setup(void)
{
    /* Enable ADC1 clock. */
    rcc_periph_clock_enable(RCC_ADC1);

    /* Start ADC1 Powered Off */
    adc_power_off(ADC1);

    /* Set scan mode to support multiple analog inputs */
    adc_enable_scan_mode(ADC1);
    /* Set continuous mode */
    adc_set_continuous_conversion_mode(ADC1);
    /* Right align ADC reading */
    adc_set_left_aligned(ADC1);
    /* DMA request on ADC reading */
    adc_enable_dma(ADC1);

    uint8_t channel_array[ADC_NCHAN] = {0};
    /* Set channel sequence */
    adc_set_regular_sequence(ADC1, sizeof(channel_array), channel_array);
    //adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_239DOT5CYC);
    
    /* Enable GPIOA clock. */
    rcc_periph_clock_enable(RCC_GPIOA);

    /* Configure Output Pin A0 as ADC0 */
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                  GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, GPIO0);

    /* Power on ADC1 */
    adc_power_on(ADC1);

    /* 0.1s delay for ADC stabilization. */
    for(int i = 0; i < 800000; i++)
        __asm__("nop");
    
    /* Calibrate ADC */
    adc_reset_calibration(ADC1);
    adc_calibrate(ADC1);

    adc_start_conversion_direct(ADC1);
}

void tim3_setup(void)
{
    /* Enable Timer 3. */
    rcc_periph_clock_enable(RCC_TIM3);

    /* Configure Timer 3 w/ No clock divider, edge triggered, upcounting */
    timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    /* Prescale CLOCK_FREQ to (SAMPL_FREQ*SAMPL_RESN) */
    timer_set_prescaler(TIM3, 15);

    /* Configure Timer 3 Output 3 Active when counter < compare value */
    timer_set_oc_mode(TIM3, TIM_OC3, TIM_OCM_PWM1);

    /* Configure Timer 3 Output 3 to be active high */
    timer_set_oc_polarity_high(TIM3, TIM_OC3);

    /* Set 127 tick period. */
    timer_set_period(TIM3, 0x7F);

    /* Start at 10% duty cycle. */
    timer_set_oc_value(TIM3, TIM_OC3, 0);

    /* Enable DMA request on update */
    timer_update_on_overflow(TIM3);
    timer_enable_update_event(TIM3);

    /* Enable Timer */
    timer_enable_counter(TIM3);

    /* Enable GPIOB clock. */
    rcc_periph_clock_enable(RCC_GPIOB);

    /* Configure Output Pin B0 as TIM3C3 */
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO0);

    /* Enable Output compare mode */
    timer_enable_oc_output(TIM3, TIM_OC3);
}

void dma_setup(void) {
    /* Enable DMA 1. */
    rcc_periph_clock_enable(RCC_DMA1);


    /* Reset DMA 1 Channel 1 (ADC1) */
    dma_channel_reset(DMA1, DMA_CHANNEL1);
    
    /* Reading from memory to perihperal */
    dma_set_read_from_peripheral(DMA1, DMA_CHANNEL1);

    /* Writing to same 16 bit address each time */
    dma_disable_peripheral_increment_mode(DMA1, DMA_CHANNEL1);
    dma_set_peripheral_size(DMA1, DMA_CHANNEL1, DMA_CCR_PSIZE_16BIT);
    /* Specify peripheral register */
    dma_set_peripheral_address(DMA1, DMA_CHANNEL1, (uint32_t)(&ADC1_DR));

    /* Reading from same 16 bit address each time */
    dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL1);
    dma_set_memory_size(DMA1, DMA_CHANNEL1, DMA_CCR_MSIZE_16BIT);
    /* Specify buffer location and length */
    dma_set_memory_address(DMA1, DMA_CHANNEL1, (uint32_t)(&channel_values));
    dma_set_number_of_data(DMA1, DMA_CHANNEL1,
                           sizeof(channel_values)/sizeof(uint16_t));

    /* Enable circular mode, loops to start when done. */
    dma_enable_circular_mode(DMA1, DMA_CHANNEL1);

    /* Set DMA Priority to high */
    dma_set_priority(DMA1, DMA_CHANNEL1, DMA_CCR_PL_HIGH);


    /* Enable xfer complete ISR */
    dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL1);

    nvic_set_priority(NVIC_DMA1_CHANNEL1_IRQ, 0);
    nvic_enable_irq(NVIC_DMA1_CHANNEL1_IRQ);

    /* Enable DMA Channel 1 */
    dma_enable_channel(DMA1, DMA_CHANNEL1);
}
