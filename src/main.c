/* Sets the brightness an LED using PWM on Timer 3 on an stm32f103
 * 'Blue Pill' dev board.  Brightness given using analog value on PA0
 * read using DMA on ADC1.  Uses PC13 for status LED, PB0 as PWM LED,
 * PA0 as brightness control (such as var. resistor)
 *
 * - William A Stevens V (wasv)
 */
#include "periph.h"
#include "usb-midi.h"

#include <stdint.h>

#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

usbd_device *usbd_dev;
volatile uint8_t adc0_update = 0;

const char *usb_strings[] = {
    "WASV",
    "MIDI Dial",
    "DEMO",
};

void usb_midi_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
    (void)ep;

    uint8_t buf[8];
    usbd_ep_read_packet(usbd_dev, 0x01, buf, 8);

    if(buf[1] == 0x90) {
        timer_set_oc_value(TIM3, TIM_OC3, buf[3] & 0x7F);
    }

    if(buf[1] == 0x80) {
        timer_set_oc_value(TIM3, TIM_OC3, 0);
    }
}

void dma1_channel1_isr(void)
{
    if ((DMA1_ISR &DMA_ISR_TCIF1) != 0) {
        DMA1_IFCR |= DMA_IFCR_CTCIF1;
    }
    adc0_update = channel_values[0] >> 8 & 0x7F;
}

int main(void)
{
    uint8_t old_val = 0;
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    dma_setup();
    tim3_setup();
    gpio_setup();
    adc_setup();

    gpio_set(GPIOC, GPIO13);

    usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev, &config,
                         usb_strings, 3,
                         usbd_control_buffer, sizeof(usbd_control_buffer));

    usbd_register_set_config_callback(usbd_dev, usb_midi_set_config);

    gpio_clear(GPIOC, GPIO13);

    while (1) {
        usbd_poll(usbd_dev);

        if ( adc0_update != old_val ) {
            old_val = adc0_update;

            uint8_t buf[4] = { 0x03, 0xB0, 0x03, adc0_update };
            usbd_ep_write_packet(usbd_dev, 0x81, buf, 4);
        }
    }

    return 0;
}
