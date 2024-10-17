/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2024 Phl Schatzmann

 A combination of a microphone and headset at the same time.
 Unfortunately it will only work when Serial output is not active

*********************************************************************/

#include "Adafruit_TinyUSB.h"

Adafruit_USBD_Audio usb;
size_t sample_count_mic = 0;
size_t sample_count_spk = 0;

// Microphone: generate data for USB
size_t readCB(uint8_t* data, size_t len, Adafruit_USBD_Audio& ref) {
  int16_t* data16 = (int16_t*)data;
  size_t samples = len / sizeof(int16_t);
  size_t result = 0;
  // generate random stereo data
  for (int j = 0; j < samples; j+=2) {
    data16[j] = random(-32000, 32000);
    data16[j+1] = random(-32000, 32000);;
    result += sizeof(int16_t)*2;
    sample_count_mic += 2;
  }
  return result;
}

// Speaker: receive data from USB and write them to the final destination
size_t writeCB(const uint8_t* data, size_t len, Adafruit_USBD_Audio& ref) {
  int16_t* data16 = (int16_t*)data;
  size_t samples = len / sizeof(int16_t);
  sample_count_spk += samples;
  return len;
}

void setup() {
    // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  Serial.begin(115200);

  // Start USB device as both Audio Source and Sink
  usb.setReadCallback(readCB);
  usb.setWriteCallback(writeCB);
  usb.begin(44100, 2, 16);

  // If already enumerated, additional class driver begin() e.g msc, hid, midi won't take effect until re-enumeration
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
  if (usb.updateLED()){
    Serial.print("Total Microphone samples: ");
    Serial.print(sample_count_mic);
    Serial.print(" / Speaker samples: ");
    Serial.print(sample_count_spk);
    Serial.print(" / Sample rate: ");
    Serial.println(usb.rate());
  }
}