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

const char *usb_strings[] = {
	"WASV",
	"MIDI Demo",
	"DEMO",
};

int main(void)
{
	usbd_device *usbd_dev;

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
	}

	return 0;
}
