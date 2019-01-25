/* Sets a MIDI control value over USB using a knob.  Sets an LEDs
 * brightness based on note velocity.  Uses PC13 for status LED, PB0
 * as PWM LED, PA0 as MIDI control (such as var. resistor)
 *
 * - William A Stevens V (wasv)
 */
#include "periph.h"
#include "usb-midi.h"
#include "cbuf.h"

#include <stdint.h>

#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define NSAMP 4

usbd_device *usbd_dev;
volatile uint8_t adc0_update = false;

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
    adc0_update = true;
}

uint8_t calc_avg(uint8_t *data, size_t count)
{
    uint8_t avg = 0;
    for(size_t i = 0; i < count; i++) {
        avg += data[i] / count;
    }
    return avg;
}

int main(void)
{
    uint8_t old_avg = 0;
    circ_buff *adc0_cbuf = circ_buff_new(sizeof(uint8_t), NSAMP);
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

        if ( adc0_update ) {
            uint8_t new_val = channel_values[0] >> 9 & 0x7F;

            circ_buff_write(adc0_cbuf, &new_val);
            uint8_t new_avg = calc_avg(adc0_cbuf->buffer, NSAMP);

            if(new_avg != old_avg) {
                uint8_t buf[4] = { 0x03, 0xB0, 0x03, new_val };
                usbd_ep_write_packet(usbd_dev, 0x81, buf, 4);
            }
            old_avg = new_avg;
        }
    }

    return 0;
}
