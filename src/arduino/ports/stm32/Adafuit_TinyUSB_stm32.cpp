#include "tusb_option.h"
#include "Arduino.h"

// Porting API
//--------------------------------------------------------------------+
void TinyUSB_Port_InitDevice(uint8_t rhport) {
  (void)rhport;
}

void TinyUSB_Port_EnterDFU(void) {
}


uint8_t TinyUSB_Port_GetSerialNumber(uint8_t serial_id[16]) {
  uint32_t ch32_uuid = HAL_GetDEVID ();
  memcpy(&serial_id, &ch32_uuid, 4);
  return 4;
}