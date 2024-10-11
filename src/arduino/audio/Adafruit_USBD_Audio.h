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

#include "Arduino.h"
#include "Adafruit_TinyUSB.h"
#include "Adafruit_audio_config.h"
#include "common/tusb_types.h"
#include <vector>

// output using 5 debug pins
#if AUDIO_DEBUG
# define debugWrite(pin, active) digitalWrite(pin, active);
#else
# define debugWrite(pin, active)
#endif

// Reference to audio: TODO multiple instances
class Adafruit_USBD_Audio;
extern Adafruit_USBD_Audio *self_Adafruit_USBD_Audio;

// LED delays 
enum class LEDDelay { INACTIVE=0, ERROR=500, PLAYING=1000,ACTIVE=2000 };


/***
 * USB Audio Device: 
 * - provide data access via callbacks
 * - configure audio info via begin method
 * - provide all potential methods so that we can easily overwrite them
 * - implement audio sink
 * - implement audio source
 */

class Adafruit_USBD_Audio : public Adafruit_USBD_Interface {
 public:

  Adafruit_USBD_Audio(void) { 
    self_Adafruit_USBD_Audio = this; 
    setupDebugPins();
  }
  
  /// callback for audio sink (speaker): we write out the received data e.g. to a dac
  void setWriteCallback(size_t (*write_cb)(const uint8_t* data, size_t len, Adafruit_USBD_Audio& ref)) {
    p_write_callback = write_cb;
  }

  /// callback for audio source (microphone): we provide the audio data e.g. from the adc
  void setReadCallback(size_t (*read_cb)(uint8_t* data,size_t len, Adafruit_USBD_Audio& ref)) {
    p_read_callback = read_cb;
  }

  /// Alternative to setWriteCallback
  void setOutput(Print &out) { 
    p_print = &out; 
    setWriteCallback(defaultWriteCB);
  }

  /// Alternaive to setReadCallback
  void setInput(Stream &in) { 
    p_stream = &in;
    setReadCallback(defaultReadCB); 
  }

  /// start the processing
  virtual bool begin(unsigned long rate = 44100, int channels = 2,
             int bitsPerSample = 16);

  // end audio
  virtual void end(void);

  // If is mounted
  bool started(void) { return _is_active; }

  operator bool() { return started(); }

  // if cdc's DTR is asserted
  bool connected(void);

  // get sample rate
  uint32_t rate() { return _sample_rate; }

  // get number of channels
  int channels() { return _channels; }

  uint16_t volume(int channel) { return _volume[channel]; }

  bool isMute(int channel) { return _mute[channel]; }

  // from Adafruit_USBD_Interface
  virtual uint16_t getInterfaceDescriptor(uint8_t itfnum_deprecated,
                                          uint8_t *buf, uint16_t bufsize) override;
  size_t getInterfaceDescriptorLength() {
    return getInterfaceDescriptor(0, nullptr, 0);
  }

  /// Call from loop to blink led
  bool updateLED(int pin=LED_BUILTIN);

  /// Define the led delay
  void setLEDDelay(LEDDelay delayMs) {
    _led_delay_ms = delayMs;
  }

  /// allow/disalow Serial output
  void setCDCActive(bool flag){
    _cdc_active = flag;
  }

  uint16_t getMaxEPSize();


  //--------------------------------------------------------------------+
  // Application Callback API Implementations
  //--------------------------------------------------------------------+

  uint16_t get_io_size() {
    return TUD_AUDIO_EP_SIZE(_sample_rate, _bits_per_sample/8, _channels);
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

// for speaker
#if CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP
  virtual void feedback_params_cb(uint8_t func_id, uint8_t alt_itf, audio_feedback_params_t* feedback_param);
#endif


 protected:
  int _rh_port = 0;
  uint8_t _channels = 0;
  bool _is_active = false;
  bool _is_led_setup = true;
  LEDDelay _led_delay_ms = LEDDelay::INACTIVE;

  // Audio controls
  bool _mute[AUDIO_USB_MAX_CHANNELS+1] = {false};    // +1 for master channel 0
  uint16_t _volume[AUDIO_USB_MAX_CHANNELS+1] = {100};  // +1 for master channel 0
  // Current states
  uint32_t _sample_rate;
  uint8_t _bits_per_sample;
  uint8_t _clk_is_valid = true;

  // Audio test data
  std::vector<uint8_t> _in_buffer;
  std::vector<uint8_t> _out_buffer;
  uint16_t _out_buffer_available = 0;
  uint16_t _in_buffer_available = 0;

  bool _led_active = false;
  uint64_t _led_timeout = 0;

  // persisted descriptor data
  uint8_t _itfnum_spk = 0, ep_mic = 0;
  uint8_t _itfnum_mic = 0, ep_spk = 0;
  uint8_t _itf_number_total = 0;
  uint8_t _itfnum_ctl = 0;
  uint8_t _ep_ctl = 0;
  uint8_t _ep_mic = 0;
  uint8_t _ep_spk = 0;
  uint8_t _ep_fb = 0;
  bool _cdc_active = CDC_DEFAULT_ACTIVE;

  // input/output callbacks
  size_t (*p_write_callback)(const uint8_t* data,size_t len, Adafruit_USBD_Audio& ref);
  size_t (*p_read_callback)(uint8_t* data, size_t len, Adafruit_USBD_Audio& ref);
  Stream *p_stream = nullptr;
  Print *p_print = nullptr;
  int _append_pos = 0;
  int _desc_len = 0;

  /// We can use 8 debug pins with a logic analyser
  void setupDebugPins();

  void append(uint8_t *to, uint8_t *str, int len){
    if (to != nullptr) memcpy(to + _append_pos, str, len);
    _append_pos += len;
  }

  bool isMicrophone() {
    return p_read_callback!=nullptr && p_read_callback!=defaultReadCB
    || p_read_callback==defaultReadCB && p_stream!=nullptr;
  }

  bool isSpeaker() {
    return p_write_callback!=nullptr && p_write_callback!=defaultWriteCB
    || p_write_callback==defaultWriteCB && p_print!=nullptr;
  }

  static size_t defaultWriteCB(const uint8_t* data, size_t len, Adafruit_USBD_Audio& ref){
    Print* p_print = ref.p_print;
    if (p_print) return p_print->write((const uint8_t*)data, len);
    return 0;
  }

  static size_t defaultReadCB(uint8_t* data, size_t len, Adafruit_USBD_Audio& ref){
    Stream* p_stream = ref.p_stream;
    if (p_stream) return p_stream->readBytes((uint8_t*)data, len);
    return 0;
  }

  // for speaker
  virtual bool speaker_feature_unit_get_request(uint8_t rhport, audio_control_request_t const *request);
  virtual bool speaker_feature_unit_set_request(uint8_t rhport, audio_control_request_t const *request, uint8_t const *buf);
  virtual bool speaker_clock_get_request(uint8_t rhport, audio_control_request_t const *request);
  virtual bool speaker_clock_set_request(uint8_t rhport, audio_control_request_t const *request, uint8_t const *buf);

  virtual bool microphone_feature_unit_get_request(uint8_t rhport, tusb_control_request_t const *request);
//virtual bool microphon_feature_unit_set_request(uint8_t rhport, audio_control_request_t const *request, uint8_t const *buf);
  virtual bool microphone_clock_get_request(uint8_t rhport, tusb_control_request_t const *request);
//virtual bool microphone_clock_set_request(uint8_t rhport, audio_control_request_t const *request, uint8_t const *buf);

};

#endif
