/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2024 Phil Schatzmann

 This example receives Audio from your PC and you can visualize the
 audio data in the Arduino Serial Plotter

*********************************************************************/

#include "Adafruit_TinyUSB.h"

Adafruit_USBD_Audio usb;

size_t writeCB(const uint8_t* data, size_t len, Adafruit_USBD_Audio& ref) {
  int16_t* data16 = (int16_t*)data;
  size_t samples = len / sizeof(int16_t);
  size_t result = 0;
  // print received data to serial
  for (int j = 0; j < samples; j+=2) {
    Serial.print(data16[j]);
    Serial.print(",");
    Serial.println(data16[j+1]);
    result += sizeof(int16_t)*2;
  }
  return result;
}

void setup() {
  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }
  Serial.begin(115200);

  // Start USB device as Audio Sink
  usb.setWriteCallback(writeCB);
  usb.begin(44100, 2, 16);

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

}

void loop() {
  #ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
  #endif

  // use LED do display status
  usb.updateLED();
}