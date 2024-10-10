/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2024 Phl Schatzmann

 This example reads audio data from an I2S ADC which you can output 
 on your PC

*********************************************************************/
#ifndef ARDUINO_ARCH_RP2040
#  error This example is using the RP2040 I2S API: Adapt it for your platform
#endif

#include "Adafruit_TinyUSB.h"
#include "I2S.h"

I2S i2s(INPUT);
Adafruit_USBD_Audio usb;
const int sample_rate = 44100;
const int bits = 16;
const int channels = 2;

void setup() {
  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  Serial.begin(115200);
  
  // setup i2s
  i2s.setDATA(0);
  i2s.setBCLK(1); // Note: LRCLK = BCLK + 1
  i2s.setBitsPerSample(bits);
  i2s.setFrequency(sample_rate);
  i2s.begin();

  // Start USB device as Audio Source
  usb.setInput(i2s);
  usb.begin(sample_rate, channels, bits);

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }
}

void loop() {
  // use LED do display status
  usb.updateLED();
}