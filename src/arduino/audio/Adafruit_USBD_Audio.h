/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Phil Schatzmann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ADAFRUIT_USBD_AUDIO_H_
#define ADAFRUIT_USBD_AUDIO_H_

#include <vector>

#include "Adafruit_TinyUSB.h"
#include "Stream.h"
#include "Adafruit_audio_config.h"
#include "common/tusb_types.h"

// /// 5.2.3.2 2-byte Control RANGE Parameter Block
// #define audio_control_range_2_n_t(numSubRanges) \
//   struct TU_ATTR_PACKED { \
//     uint16_t wNumSubRanges; \
//     struct TU_ATTR_PACKED { \
//       int16_t bMin; /*The setting for the MIN attribute of the nth subrange
//       of \
//                        the addressed Control*/ \
//       int16_t bMax; /*The setting for the MAX attribute of the nth subrange
//       of \
//                        the addressed Control*/ \
//       uint16_t bRes; /*The setting for the RES attribute of the nth subrange
//       \
//                         of the addressed Control*/ \
//     } subrange[numSubRanges]; \
//   }

/***
 * USB Audio Device
 * 
 */
class Adafruit_USBD_Audio;
extern Adafruit_USBD_Audio *self_Adafruit_USBD_Audio;

class Adafruit_USBD_Audio : public Adafruit_USBD_Interface {
 public:
  Adafruit_USBD_Audio(void) { self_Adafruit_USBD_Audio = this; }
  void setWriteCallback(size_t (*write_cb)(const void* data,size_t len, Adafruit_USBD_Audio* ref)) {
    p_write_callback = write_cb;
  }
  void setReadCallback(size_t (*read_cb)(void* data,size_t len, Adafruit_USBD_Audio* ref)) {
    p_read_callback = read_cb;
  }
  void setOutput(Print &out) { 
    p_print = &out; 
    setWriteCallback(defaultWriteCB);
  }
  void setInput(Stream &in) { 
    p_stream = &in;
    setReadCallback(defaultReadCB); 
  }

  bool begin(unsigned long rate = 44100, int channels = 2,
             int bytesPerSample = 16);

  // end audio
  void end(void);

  // If is mounted
  bool started(void) { return _is_active; }
  operator bool() { return started(); }

  // if cdc's DTR is asserted
  bool connected(void);

  // get sample rate
  uint32_t rate() { return sampFreq; }

  // get number of channels
  int channels() { return _channels; }

  uint16_t getVolume(int channel) { return volume[channel]; }

  bool isMute(int channel) { return _mute[channel]; }

  // from Adafruit_USBD_Interface
  virtual uint16_t getInterfaceDescriptor(uint8_t itfnum_deprecated,
                                          uint8_t *buf, uint16_t bufsize);

  //--------------------------------------------------------------------+
  // Application Callback API Implementations
  //--------------------------------------------------------------------+

  uint16_t get_io_size() {
    return sampFreq / (TUD_OPT_HIGH_SPEED ? 8000 : 1000) * bytesPerSample;
  }

  // Invoked when set interface is called, typically on start/stop streaming or
  // format change
  virtual bool set_itf_cb(uint8_t rhport,
                          tusb_control_request_t const *p_request);

  // Invoked when audio class specific set request received for an EP
  virtual bool set_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request,
                             uint8_t *pBuff);

  // Invoked when audio class specific set request received for an interface
  virtual bool set_req_itf_cb(uint8_t rhport,
                              tusb_control_request_t const *p_request,
                              uint8_t *pBuff);

  // Invoked when audio class specific set request received for an entity
  virtual bool set_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const *p_request,
                                 uint8_t *pBuff);
  // Invoked when audio class specific get request received for an EP
  virtual bool get_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request);

  // Invoked when audio class specific get request received for an interface
  virtual bool get_req_itf_cb(uint8_t rhport,
                              tusb_control_request_t const *p_request);

  // Invoked when audio class specific get request received for an entity
  virtual bool get_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const *p_request);
  virtual bool tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in,
                                   uint8_t cur_alt_setting);

  virtual bool tx_done_post_load_cb(uint8_t rhport, uint16_t n_bytes_copied,
                                    uint8_t itf, uint8_t ep_in,
                                    uint8_t cur_alt_setting);

  virtual bool rx_done_pre_read_cb(uint8_t rhport, uint16_t n_bytes_received,
                                   uint8_t func_id, uint8_t ep_out,
                                   uint8_t cur_alt_setting);
  virtual bool rx_done_post_read_cb(uint8_t rhport, uint16_t n_bytes_received,
                                    uint8_t func_id, uint8_t ep_out,
                                    uint8_t cur_alt_setting);

  virtual bool set_itf_close_EP_cb(uint8_t rhport,
                                   tusb_control_request_t const *p_request);

 protected:
  int _channels = 2;
  bool _is_active = false;

  // Audio controls
  bool _mute[CFG_FUNC_1_N_CHANNELS_TX + 1];       // +1 for master channel 0
  uint16_t volume[CFG_FUNC_1_N_CHANNELS_TX + 1];  // +1 for master channel 0
  audio_control_range_2_n_t(
      1) volumeRng[CFG_FUNC_1_N_CHANNELS_TX + 1];  // Volume range state

  // Current states
  uint32_t sampFreq;
  uint8_t bytesPerSample;
  uint8_t clkValid;

  // Audio test data
  std::vector<uint8_t> in_buffer;
  std::vector<uint8_t> out_buffer;

  // input/output callbacks
  size_t (*p_write_callback)(const uint8_t* data,size_t len, Adafruit_USBD_Audio* ref);
  size_t (*p_read_callback)(uint8_t* data,size_t len, Adafruit_USBD_Audio* ref);
  Stream *p_stream = nullptr;
  Print *p_print = nullptr;

  bool isReadDefined() {
    return p_read_callback!=nullptr && p_read_callback!=defaultReadCB
    || p_read_callback==defaultReadCB && p_stream!=nullptr;
  }

  bool isWriteDefined() {
    return p_write_callback!=nullptr && p_write_callback!=defaultWriteCB
    || p_read_callback==defaultWriteCB && p_print!=nullptr;
  }

  static size_t defaultWriteCB(const uint8_t* data,size_t len, Adafruit_USBD_Audio* ref){
    Print p_print = ref.p_print;
    if (p_print) return p_print->write((const uint8_t*)data, len);
    return 0;
  }

  static size_t defaultReadCB(uint8_t* data,size_t len, Adafruit_USBD_Audio* ref){
    Stream p_stream = ref.p_stream;
    if (p_stream) return p_stream->readBytes((uint8_t*)data, len);
    return 0;
  }


};

#endif
