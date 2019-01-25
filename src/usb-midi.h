#ifndef _USB_MIDI_H_
#define _USB_MIDI_H_

#include <libopencm3/usb/usbd.h>

uint8_t usbd_control_buffer[128];

void usb_midi_set_config(usbd_device *usbd_dev, uint16_t wValue);
void usb_midi_data_rx_cb(usbd_device *usbd_dev, uint8_t ep);

const struct usb_device_descriptor dev;
const struct usb_config_descriptor config;
#endif
