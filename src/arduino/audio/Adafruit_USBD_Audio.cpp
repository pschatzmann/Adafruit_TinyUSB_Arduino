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

#include "Adafruit_USBD_Audio.h"

#if CFG_TUD_ENABLED && CFG_TUD_AUDIO

Adafruit_USBD_Audio *self_Adafruit_USBD_Audio = nullptr;

/*------------- MAIN -------------*/
bool Adafruit_USBD_Audio::begin(unsigned long rate, int channels,
                                int bitsPerSample) {
  _itf_number_total = 0;

  if (!_cdc_active) {
    TinyUSBDevice.clearConfiguration();
  }

  // check data
  if (rate > AUDIO_FREQ_MAX || rate < AUDIO_FREQ_MIN) {
    LOG_AUDIO_DEBUG("Invalid sample rate %d: must be in range %d - %d",rate, AUDIO_FREQ_MIN, AUDIO_FREQ_MAX);
    setStatus(AudioProcessingStatus::ERROR);
    return false;
  }
  if (channels > AUDIO_USB_MAX_CHANNELS || channels < 1) {
    LOG_AUDIO_DEBUG("Invalid channels %d: must be in range %d - %d",channels, 1, AUDIO_USB_MAX_CHANNELS);
    setStatus(AudioProcessingStatus::ERROR);
    return false;
  }
  if (bitsPerSample > MAX_BITS_PER_SAMPLE) {
    LOG_AUDIO_ERROR("Invalid bits %d: must be <= %d",bitsPerSample, MAX_BITS_PER_SAMPLE);
    setStatus(AudioProcessingStatus::ERROR);
    return false;
  }
  if (!isMicrophone() && !isSpeaker()){
    LOG_AUDIO_ERROR("No callback has been defined");
    setStatus(AudioProcessingStatus::ERROR);
    return false;
  }

  // headset & cdc requires a bigger config buffer
  if (isHeadset()){
    _config_buffer_size = 512;
  }

  // setup config buffer
  if (_config_buffer_size > 256){
    _config_buffer.resize(_config_buffer_size);
    TinyUSBDevice.setConfigurationBuffer(&(_config_buffer[0]), _config_buffer_size);
  }

  // init device stack on configured roothub port
  // Init values
  this->_sample_rate = rate;
  this->_bits_per_sample = bitsPerSample;
  this->_channels = channels;

  if (_stridx == 0) {
  _stridx = TinyUSBDevice.addStringDescriptor("TinyUSB Audio");
  }

  // calculate descriptor length;
  if (interfaceDescriptor(nullptr, 1024) == 0){
    setStatus(AudioProcessingStatus::ERROR);
    LOG_AUDIO_ERROR("Interface Descriptor length was 0");
    return false;
  }

  // add the interface
  if (!TinyUSBDevice.addInterface(*this)){
    setStatus(AudioProcessingStatus::ERROR);
    LOG_AUDIO_ERROR("addInterface failed");
    return false;
  }

  _clk_is_valid = 1;
  setStatus(AudioProcessingStatus::ACTIVE);

  return true;
}

void Adafruit_USBD_Audio::end() {
  tud_deinit(_rh_port);
  setStatus(AudioProcessingStatus::INACTIVE);
  if (_out_buffer.size() > 0) _out_buffer.resize(0);
  if (_in_buffer.size() > 0) _out_buffer.resize(0);
}

/// Call from loop to blink led
bool Adafruit_USBD_Audio::updateLED(int pin) {
  if (_is_led_setup) {
    pinMode(pin, OUTPUT);
    _is_led_setup = false;
  }

  // led must be active
  if (_processing_status != AudioProcessingStatus::INACTIVE && millis() > _led_timeout) {
    _led_timeout = millis() + (uint16_t)_processing_status;
    _led_active = !_led_active;
    digitalWrite(pin, _led_active);
    return true;
  }

  // led is inactive
  if (_processing_status == AudioProcessingStatus::INACTIVE) {
    if (_led_active) {
      _led_active = false;
      digitalWrite(pin, _led_active);
    }
  }
  return false;
}

//--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when set interface is called, typically on start/stop streaming or
// format change
bool Adafruit_USBD_Audio::set_itf_cb(uint8_t rhport,
                                     tusb_control_request_t const *p_request) {
  uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
  uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));

  if (alt != 0) setStatus(AudioProcessingStatus::PLAYING);

  return true;
}

// Invoked when audio class specific set request received for an EP
bool Adafruit_USBD_Audio::set_req_ep_cb(uint8_t rhport,
                                        tusb_control_request_t const *p_request,
                                        uint8_t *pBuff) {
  (void)rhport;
  (void)pBuff;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t ep = TU_U16_LOW(p_request->wIndex);

  (void)channelNum;
  (void)ctrlSel;
  (void)ep;

  return false;  // Yet not implemented
}

// Invoked when audio class specific set request received for an interface
bool Adafruit_USBD_Audio::set_req_itf_cb(
    uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *pBuff) {
  (void)rhport;
  (void)pBuff;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);

  (void)channelNum;
  (void)ctrlSel;
  (void)itf;

  return false;  // Yet not implemented
}

// Invoked when audio class specific set request received for an entity
bool Adafruit_USBD_Audio::set_req_entity_cb(
    uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *buf) {
  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);
  audio_control_request_t const *request =
      (audio_control_request_t const *)p_request;

  debugWrite(5, HIGH);

  // for speaker
  if (request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT){
    bool rc = feature_unit_set_request(rhport, p_request, buf);
    debugWrite(5, LOW);
    return rc;
  }
  if (request->bEntityID == UAC2_ENTITY_CLOCK){
    bool rc = clock_set_request(rhport, p_request, buf);
    debugWrite(5, LOW);
    return rc;
  }

  debugWrite(5, HIGH);
  LOG_AUDIO_DEBUG(
      "Set request not handled, entity = %d, selector = %d, request = %d",
      request->bEntityID, request->bControlSelector, request->bRequest);
  return false;  // Yet not implemented
}

// Invoked when audio class specific get request received for an EP
bool Adafruit_USBD_Audio::get_req_ep_cb(
    uint8_t rhport, tusb_control_request_t const *p_request) {
  (void)rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t ep = TU_U16_LOW(p_request->wIndex);

  (void)channelNum;
  (void)ctrlSel;
  (void)ep;

  //	return tud_control_xfer(rhport, p_request, &tmp, 1);

  return false;  // Yet not implemented
}

// Invoked when audio class specific get request received for an interface
bool Adafruit_USBD_Audio::get_req_itf_cb(
    uint8_t rhport, tusb_control_request_t const *p_request) {
  (void)rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);

  (void)channelNum;
  (void)ctrlSel;
  (void)itf;

  return false;  // Yet not implemented
}

// Invoked when audio class specific get request received for an entity
bool Adafruit_USBD_Audio::get_req_entity_cb(
    uint8_t rhport, tusb_control_request_t const *p_request) {
  _rh_port = rhport;
  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  // uint8_t itf = TU_U16_LOW(p_request->wIndex); 			// Since
  // we have only one audio function implemented, we do not need the itf value
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);
  audio_control_request_t const *request =
      (audio_control_request_t const *)p_request;

  debugWrite(6, HIGH);

  // Clock Source 
  if (request->bEntityID == UAC2_ENTITY_CLOCK){
    bool rc = clock_get_request(rhport, p_request);
    if (rc) debugWrite(6, LOW);
    return rc;
  }

  // Fueature unit speaker
  if (request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT){
    bool rc = feature_unit_get_request(rhport, p_request);  
    if (rc) debugWrite(6, LOW);
    return rc;
  }

  // Feature unit mic
  if (entityID == UAC2_ENTITY_MIC_FEATURE_UNIT){
    bool rc = feature_unit_get_request(rhport, p_request);
    if (rc) debugWrite(6, LOW);
    return rc;
  }

    // Input terminal (Microphone input)
  if (entityID == UAC2_ENTITY_MIC_INPUT_TERMINAL
  || entityID == UAC2_ENTITY_SPK_INPUT_TERMINAL) {
    switch (ctrlSel) {
      case AUDIO_TE_CTRL_CONNECTOR: {
        // The terminal connector control only has a get request with only the
        // CUR attribute.
        audio_desc_channel_cluster_t ret;
        ret.bNrChannels = _channels;
        ret.bmChannelConfig = (audio_channel_config_t)0;
        ret.iChannelNames = 0;

        LOG_AUDIO_DEBUG("    Get terminal connector");

        bool rc = tud_audio_buffer_and_schedule_control_xfer(
            rhport, p_request, (void *)&ret, sizeof(ret));
        if (rc) debugWrite(6, LOW);
        return rc;
      } break;

        // Unknown/Unsupported control selector
      default:
        LOG_AUDIO_DEBUG("  Unsupported selector: %d", entityID);
        debugWrite(6, HIGH);
        return false;
    }
  }

  LOG_AUDIO_DEBUG("  Unsupported entity: %d", entityID);
  debugWrite(6, HIGH);
  return false;  // Yet not implemented
}

bool Adafruit_USBD_Audio::tx_done_pre_load_cb(uint8_t rhport, uint8_t itf,
                                              uint8_t ep_in,
                                              uint8_t cur_alt_setting) {
  // (void)rhport;
  // (void)itf;
  // (void)ep_in;
  // (void)cur_alt_setting;

  // fill buffer from "microphone" input
  if (isMicrophone()) {
    debugWrite(1, HIGH);
    // manage buffer size
    uint16_t len = getIOSize() - 2;  // CFG_TUD_AUDIO_EP_SZ_IN - 2;
    if (_out_buffer.size() < len) _out_buffer.resize(len);

    // return if the buffer is already filled
    if (_out_buffer_available != 0) {
      return true;
    }

    // fill the buffer with data
    uint8_t *adr = &_out_buffer[0];
    memset(adr, 0, len);
    _out_buffer_available = (*p_read_callback)(adr, len, *this);
    debugWrite(1, LOW);
  }

  return true;
}

bool Adafruit_USBD_Audio::tx_done_post_load_cb(uint8_t rhport,
                                               uint16_t n_bytes_copied,
                                               uint8_t itf, uint8_t ep_in,
                                               uint8_t cur_alt_setting) {
  (void)rhport;
  (void)itf;
  (void)ep_in;
  (void)cur_alt_setting;

  // output audio from "microphone" buffer to usb
  if (isMicrophone()) {
    debugWrite(2, HIGH);
    uint8_t *adr = &_out_buffer[0];
    tud_audio_write(adr, _out_buffer_available);
    _out_buffer_available = 0;
    debugWrite(2, LOW);
  }

  return true;
}

bool Adafruit_USBD_Audio::rx_done_pre_read_cb(uint8_t rhport,
                                              uint16_t n_bytes_received,
                                              uint8_t func_id, uint8_t ep_out,
                                              uint8_t cur_alt_setting) {
  //read audio from usb
  if (isSpeaker() && _in_buffer_available == 0) {
    debugWrite(3, HIGH);
    uint16_t len = tud_audio_available();
    if (len > 0) {
      if (_in_buffer.size() < len) _in_buffer.resize(len);
      uint8_t *adr = &_in_buffer[0];
      _in_buffer_available = tud_audio_read(adr, len);
    }
    debugWrite(3, LOW);
    return true;
  }
  return true;
}

bool Adafruit_USBD_Audio::rx_done_post_read_cb(uint8_t rhport,
                                               uint16_t n_bytes_received,
                                               uint8_t func_id, uint8_t ep_out,
                                               uint8_t cur_alt_setting) {
  // read audio from usb
  if (isSpeaker() && _in_buffer_available > 0) {
    debugWrite(4, HIGH);
    uint8_t *adr = &_in_buffer[0];
    size_t rc = p_write_callback(adr, _in_buffer_available, *this);
    _in_buffer_available -= rc;
    if (_in_buffer_available > 0){
      memmove(adr, adr+rc, _in_buffer_available);
    }
    debugWrite(4, LOW);
  }
  return true;
}

bool Adafruit_USBD_Audio::set_itf_close_EP_cb(
    uint8_t rhport, tusb_control_request_t const *p_request) {
  (void)rhport;
  uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
  uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));
  if (alt == 0) setStatus(AudioProcessingStatus::ACTIVE);

  return true;
}


uint16_t Adafruit_USBD_Audio::getInterfaceDescriptor(uint8_t itfnum_deprecated,
                                                     uint8_t *buf,
                                                     uint16_t bufsize) {

  (void)itfnum_deprecated;
  // if no source or sink then audio is not active
  if (!isMicrophone() && !isSpeaker()) {
    return 0;
  }

  // if we know the len, we can return it
  if (buf == nullptr && _desc_len > 0){
    return _desc_len;
  }

  // the buffer was too small
  if (_desc_len > 0 && bufsize <_desc_len){
    return 0;
  }

  return interfaceDescriptor(buf, bufsize);
}

uint16_t Adafruit_USBD_Audio::interfaceDescriptor(uint8_t *buf, uint16_t bufsize) {

  // Generate the descriptor and calculate it's length
  uint8_t feature_unit_len = getFeatureUnitLength();
  _append_pos = 0;

  // setup control interface and endpoint
  if (_desc_len == 0){
   _itf_number_total = 1;
   _itfnum_ctl = TinyUSBDevice.allocInterface();
   _ep_ctl = TinyUSBDevice.allocEndpoint(TUSB_DIR_IN);
  }

  // Setup endpints and interfaces
  if (isHeadset() && _desc_len==0) {
    _itfnum_mic = TinyUSBDevice.allocInterface(); // input interface
    _itfnum_spk = TinyUSBDevice.allocInterface(); // output interface
    _ep_mic = TinyUSBDevice.allocEndpoint(TUSB_DIR_IN); // intput
    _ep_int = TinyUSBDevice.allocEndpoint(TUSB_DIR_IN); // input 
    _ep_spk = TinyUSBDevice.allocEndpoint(TUSB_DIR_OUT); // output
    _itf_number_total += 2;
  } else  if (isMicrophone() && _desc_len==0) {
    _itfnum_mic = TinyUSBDevice.allocInterface();
    _ep_mic = TinyUSBDevice.allocEndpoint(TUSB_DIR_IN);
    _itf_number_total++;
  } else if (isSpeaker() && _desc_len==0) {
    _itfnum_spk = TinyUSBDevice.allocInterface(); // output interface
    _ep_spk = TinyUSBDevice.allocEndpoint(TUSB_DIR_OUT);
    _ep_fb = TinyUSBDevice.allocEndpoint(TUSB_DIR_IN);
    _itf_number_total++;
  }

  // generate descriptor
  if (isHeadset()){
    uint8_t total_len = TUD_AUDIO_DESC_CLK_SRC_LEN + feature_unit_len + (2 * (TUD_AUDIO_DESC_INPUT_TERM_LEN+TUD_AUDIO_DESC_OUTPUT_TERM_LEN));
    interfaceDescriptorHeadset(buf, total_len);
  } else if (isMicrophone()){
    uint8_t total_len = TUD_AUDIO_DESC_CLK_SRC_LEN + feature_unit_len+TUD_AUDIO_DESC_INPUT_TERM_LEN+TUD_AUDIO_DESC_OUTPUT_TERM_LEN;
    interfaceDescriptorMicrophone(buf, total_len);
  } else if (isSpeaker()){
    uint8_t total_len = TUD_AUDIO_DESC_CLK_SRC_LEN + feature_unit_len+TUD_AUDIO_DESC_INPUT_TERM_LEN+TUD_AUDIO_DESC_OUTPUT_TERM_LEN;
    interfaceDescriptorSpeaker(buf, total_len);
  }

  // record descriptor length
  if (_desc_len==0){
    _desc_len = _append_pos;
  }

  return _desc_len;
}


void Adafruit_USBD_Audio::interfaceDescriptorHeader(uint8_t *buf,uint8_t total_len, uint8_t category){

  /* Standard Interface Association Descriptor (IAD) */
  uint8_t d1[] = {TUD_AUDIO_DESC_IAD(/*_firstitfs*/ _itfnum_ctl, /*_nitfs*/ _itf_number_total, /*_stridx*/ 0)};
  append(buf, d1, sizeof(d1));

  /* Standard AC Interface Descriptor(4.7.1) */\
  uint8_t d2[] = {TUD_AUDIO_DESC_STD_AC(/*_itfnum*/ _itfnum_ctl, /*_nEPs*/ 0x00, /*_stridx*/ _stridx)};
  append(buf, d2, sizeof(d2));

  /* Class-Specific AC Interface Header Descriptor(4.7.2)  AUDIO_FUNC_OTHER */
  uint8_t d3[] = {TUD_AUDIO_DESC_CS_AC(/*_bcdADC*/ 0x0200, /*_category*/ category, /*_totallen*/  total_len, /*_ctrl*/ AUDIO_CTRL_NONE << AUDIO_CS_AS_INTERFACE_CTRL_LATENCY_POS)};
  append(buf, d3, sizeof(d3));

}
void Adafruit_USBD_Audio::interfaceDescriptorMicrophone(uint8_t *buf, uint8_t total_len){
    interfaceDescriptorHeader(buf, total_len, AUDIO_FUNC_MICROPHONE);
    /* Clock Source Descriptor(4.7.2.1) */
    //   TUD_AUDIO_DESC_CLK_SRC(/*_clkid*/ 0x04, /*_attr*/ AUDIO_CLOCK_SOURCE_ATT_INT_FIX_CLK, /*_ctrl*/ (AUDIO_CTRL_R << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS), /*_assocTerm*/ 0x01,  /*_stridx*/ 0x00),
    uint8_t d4[] = {TUD_AUDIO_DESC_CLK_SRC(/*_clkid*/ UAC2_ENTITY_CLOCK, /*_attr*/ AUDIO_CLOCK_SOURCE_ATT_INT_FIX_CLK, /*_ctrl*/ (AUDIO_CTRL_R << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS), /*_assocTerm*/ UAC2_ENTITY_MIC_INPUT_TERMINAL,  /*_stridx*/ 0x00)};
    append(buf, d4, sizeof(d4));

    /* Input Terminal Descriptor(4.7.2.4) */
    // TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ 0x01, /*_termtype*/ AUDIO_TERM_TYPE_IN_GENERIC_MIC, /*_assocTerm*/ 0x03, /*_clkid*/ 0x04, /*_nchannelslogical*/ 0x02, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ 0x00, /*_ctrl*/ AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS, /*_stridx*/ 0x00),
    uint8_t d7[] = {TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL, /*_termtype*/ UAC2_ENTITY_SPK_OUTPUT_TERMINAL, /*_assocTerm*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_nchannelslogical*/ _channels, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ 0x00, /*_ctrl*/ AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS, /*_stridx*/ 0x00)};
    append(buf, d7, sizeof(d7));
    /* Output Terminal Descriptor(4.7.2.5) */
    //TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ 0x03, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x01, /*_srcid*/ 0x02, /*_clkid*/ 0x04, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00),
    uint8_t d8[] = {TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_srcid*/ UAC2_ENTITY_MIC_FEATURE_UNIT, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00)};
    append(buf, d8, sizeof(d8));

  /* Feature Unit Descriptor(4.7.2.8) */
  // #define TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL(_unitid, _srcid, _ctrlch0master, _ctrlch1, _ctrlch2, _stridx) 	TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL_LEN, TUSB_DESC_CS_INTERFACE, AUDIO_CS_AC_INTERFACE_FEATURE_UNIT, _unitid, _srcid, U32_TO_U8S_LE(_ctrlch0master), U32_TO_U8S_LE(_ctrlch1), U32_TO_U8S_LE(_ctrlch2), _stridx
  //TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL(/*_unitid*/ 0x02, /*_srcid*/ 0x01, /*_ctrlch0master*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_ctrlch1*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_ctrlch2*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS,  /*_stridx*/ 0x00),\

    uint8_t feature_unit_len = getFeatureUnitLength();
    uint8_t df1[] = {	feature_unit_len, TUSB_DESC_CS_INTERFACE, AUDIO_CS_AC_INTERFACE_FEATURE_UNIT, /*_unitid*/UAC2_ENTITY_MIC_FEATURE_UNIT, /*_srcid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL};
    append(buf, df1, sizeof(df1));
    for (int j=0;j <_channels+1;j++) {
        uint8_t df2[] = { U32_TO_U8S_LE(AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS) };
        append(buf, df2, sizeof(df2));
    }
    // _stridx
    uint8_t df3[1] = { 0x00 };
    append(buf, df3, sizeof(df3));

    /* Standard AS Interface Descriptor(4.9.1) */\
    /* Interface 2, Alternate 0 - default alternate setting with 0 bandwidth */
    //               TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)((_itfnum)+1), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x00),
    uint8_t d15[] = {TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(_itfnum_mic), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x00)};
    append(buf, d15, sizeof(d15));
    /* Standard AS Interface Descriptor(4.9.1) */
    /* Interface 2, Alternate 1 - alternate interface for data streaming */
    //               TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)((_itfnum)+1), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x00),
    uint8_t d16[] = {TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(_itfnum_mic), /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ 0x00)};
    append(buf, d16, sizeof(d16));
    /* Class-Specific AS Interface Descriptor(4.9.2) */
    //               TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ 0x03, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ 0x02, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00),
    uint8_t d17[] = {TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ _channels, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00)};
    append(buf, d17, sizeof(d17));
    /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */
    //               TUD_AUDIO_DESC_TYPE_I_FORMAT(_nBytesPerSample, _nBitsUsedPerSample),
    uint8_t d18[] = {TUD_AUDIO_DESC_TYPE_I_FORMAT((uint8_t)(_bits_per_sample/8), (uint8_t)_bits_per_sample)};
    append(buf, d18, sizeof(d18));
    /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */
    //               TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ _epin, /*_attr*/ (uint8_t) ((uint8_t)TUSB_XFER_ISOCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_ASYNCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ _epsize, /*_interval*/ 0x01),
    uint8_t d19[] = {TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ _ep_mic, /*_attr*/ (TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ASYNCHRONOUS | TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ getMaxEPSize(), /*_interval*/ 0x01)};
    append(buf, d19, sizeof(d19));
    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */
    //               TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, /*_lockdelay*/ 0x0000)
    uint8_t d20[] = {TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, /*_lockdelay*/ 0x0000)};
    append(buf, d20, sizeof(d20));

}

void Adafruit_USBD_Audio::interfaceDescriptorSpeaker(uint8_t *buf, uint8_t total_len){
    interfaceDescriptorHeader(buf, total_len, AUDIO_FUNC_DESKTOP_SPEAKER);
    /* Clock Source Descriptor(4.7.2.1) */
    //   TUD_AUDIO_DESC_CLK_SRC(/*_clkid*/ 0x04, /*_attr*/ AUDIO_CLOCK_SOURCE_ATT_INT_FIX_CLK, /*_ctrl*/ (AUDIO_CTRL_R << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS), /*_assocTerm*/ 0x01,  /*_stridx*/ 0x00),
    uint8_t d4[] = {TUD_AUDIO_DESC_CLK_SRC(/*_clkid*/ UAC2_ENTITY_CLOCK, /*_attr*/ AUDIO_CLOCK_SOURCE_ATT_INT_PRO_CLK, /*_ctrl*/ (AUDIO_CTRL_RW << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS), /*_assocTerm*/ UAC2_ENTITY_SPK_INPUT_TERMINAL,  /*_stridx*/ 0x00)};
    append(buf, d4, sizeof(d4));
    /* Input Terminal Descriptor(4.7.2.4) */
    //              TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ 0x01, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x00, /*_clkid*/ 0x04, /*_nchannelslogical*/ 0x02, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ 0x00, /*_ctrl*/ 0 * (AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS), /*_stridx*/ 0x00),
    uint8_t d7[] = {TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x00, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_nchannelslogical*/ _channels, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ 0x00, /*_ctrl*/ 0 * (AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS), /*_stridx*/ 0x00)};
    append(buf, d7, sizeof(d7));
    /* Output Terminal Descriptor(4.7.2.5) */
    //              TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ 0x03, /*_termtype*/ AUDIO_TERM_TYPE_OUT_DESKTOP_SPEAKER, /*_assocTerm*/ 0x01, /*_srcid*/ 0x02, /*_clkid*/ 0x04, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00),
    uint8_t d8[] = {TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ UAC2_ENTITY_SPK_OUTPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_OUT_DESKTOP_SPEAKER, /*_assocTerm*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_srcid*/ UAC2_ENTITY_SPK_FEATURE_UNIT, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00)};
    append(buf, d8, sizeof(d8));

  /* Feature Unit Descriptor(4.7.2.8) */
  // #define TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL(_unitid, _srcid, _ctrlch0master, _ctrlch1, _ctrlch2, _stridx) 	TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL_LEN, TUSB_DESC_CS_INTERFACE, AUDIO_CS_AC_INTERFACE_FEATURE_UNIT, _unitid, _srcid, U32_TO_U8S_LE(_ctrlch0master), U32_TO_U8S_LE(_ctrlch1), U32_TO_U8S_LE(_ctrlch2), _stridx
  // TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL(/*_unitid*/ 0x02, /*_srcid*/ 0x01, /*_ctrlch0master*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_ctrlch1*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_ctrlch2*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS,/*_stridx*/ 0x00),\
  // uint8_t dfu[] = {TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL(/*_unitid*/ UAC2_ENTITY_SPK_FEATURE_UNIT, /*_srcid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_ctrlch0master*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_ctrlch1*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_ctrlch2*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS,  /*_stridx*/ 0x00)};
  // append(buf, dfu, sizeof(dfu));

    uint8_t feature_unit_len = getFeatureUnitLength();
    uint8_t df1[] = {	feature_unit_len, TUSB_DESC_CS_INTERFACE, AUDIO_CS_AC_INTERFACE_FEATURE_UNIT, /*_unitid*/UAC2_ENTITY_SPK_FEATURE_UNIT, /*_srcid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL};
    append(buf, df1, sizeof(df1));
    for (int j=0;j<_channels+1;j++) {
        uint8_t df2[] = { U32_TO_U8S_LE(AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS) };
        append(buf, df2, sizeof(df2));
    }
    /*_stridx 0x00*/ 
    uint8_t df3[1] = { 0x00 };
    append(buf, df3, sizeof(df3));

    /* Standard AS Interface Descriptor(4.9.1) */\
    /* Interface 2, Alternate 0 - default alternate setting with 0 bandwidth */
    //                TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)((_itfnum) + 1), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x00),
    uint8_t d15[] = { TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(_itfnum_spk), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x00)};
    append(buf, d15, sizeof(d15));
    /* Standard AS Interface Descriptor(4.9.1) */
    /* Interface 2, Alternate 1 - alternate interface for data streaming */
    //                TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)((_itfnum) + 1), /*_altset*/ 0x01, /*_nEPs*/ 0x02, /*_stridx*/ 0x00),
    uint8_t d16[] = { TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(_itfnum_spk), /*_altset*/ 0x01, /*_nEPs*/ 0x02, /*_stridx*/ 0x00)};
    append(buf, d16, sizeof(d16));
    // //               TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ 0x03, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ 0x02, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00),
    uint8_t d17[] = {TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ _channels, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00)};
    append(buf, d17, sizeof(d17));

    /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */
    //               TUD_AUDIO_DESC_TYPE_I_FORMAT(_nBytesPerSample, _nBitsUsedPerSample),
    uint8_t d18[] = {TUD_AUDIO_DESC_TYPE_I_FORMAT((uint8_t)(_bits_per_sample/8), (uint8_t)_bits_per_sample)};
    append(buf, d18, sizeof(d18));
    // /* Class-Specific AS Interface Descriptor(4.9.2) */
    /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */
    //               TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ _epout, /*_attr*/ (uint8_t) ((uint8_t)TUSB_XFER_ISOCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_ASYNCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ _epoutsize, /*_interval*/ 0x01),
    uint8_t d19[] = {TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ _ep_spk, /*_attr*/ (uint8_t) ((uint8_t)TUSB_XFER_ISOCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_ASYNCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ getMaxEPSize(), /*_interval*/ 0x01)};
    append(buf, d19, sizeof(d19));
    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */
    //               TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_MILLISEC, /*_lockdelay*/ 0x0001)
    uint8_t d20[] = {TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_MILLISEC, /*_lockdelay*/ 0x0001)};
    append(buf, d20, sizeof(d20));

    // /* Standard AS Isochronous Feedback Endpoint Descriptor(4.10.2.1) */\
    // TUD_AUDIO_DESC_STD_AS_ISO_FB_EP(/*_ep*/ _epfb, /*_epsize*/ _epfbsize, /*_interval*/ TUD_OPT_HIGH_SPEED ? 4 : 1)
    uint8_t d21[] = {TUD_AUDIO_DESC_STD_AS_ISO_FB_EP(/*_ep*/ _ep_fb, /*_epsize*/ 0X04, /*_interval*/ TUD_OPT_HIGH_SPEED ? 4 : 1)};
    append(buf, d21, sizeof(d21));

}

void Adafruit_USBD_Audio::interfaceDescriptorHeadset(uint8_t *buf, uint8_t total_len) {

  interfaceDescriptorHeader(buf, total_len, AUDIO_FUNC_HEADSET);

    /* Clock Source Descriptor(4.7.2.1) */
    // TUD_AUDIO_DESC_CLK_SRC(/*_clkid*/ UAC2_ENTITY_CLOCK, /*_attr*/ 3, /*_ctrl*/ 7, /*_assocTerm*/ 0x00,  /*_stridx*/ 0x00),    
  uint8_t d1[] = {TUD_AUDIO_DESC_CLK_SRC(/*_clkid*/ UAC2_ENTITY_CLOCK, /*_attr*/ AUDIO_CLOCK_SOURCE_ATT_INT_PRO_CLK, /*_ctrl*/ 7, /*_assocTerm*/ 00,  /*_stridx*/ 0x00)};
  append(buf, d1, sizeof(d1));

    /* Input Terminal Descriptor(4.7.2.4) */
    // TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x00, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_nchannelslogical*/ 0x02, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ 0x00, /*_ctrl*/ 0 * (AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS), /*_stridx*/ 0x00),
  uint8_t d2[] = {TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x00, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_nchannelslogical*/ _channels, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ 0x00, /*_ctrl*/ 0 * (AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS), /*_stridx*/ 0x00)};
  append(buf, d2, sizeof(d2));


    /* Feature Unit Descriptor(4.7.2.8) */
    // TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL(/*_unitid*/ UAC2_ENTITY_SPK_FEATURE_UNIT, /*_srcid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_ctrlch0master*/ (AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS), /*_ctrlch1*/ (AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS), /*_ctrlch2*/ (AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS), /*_stridx*/ 0x00),
  uint8_t feature_unit_len = getFeatureUnitLength();
  uint8_t df1[] = {	feature_unit_len, TUSB_DESC_CS_INTERFACE, AUDIO_CS_AC_INTERFACE_FEATURE_UNIT, /*_unitid*/UAC2_ENTITY_SPK_FEATURE_UNIT, /*_srcid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL};
  append(buf, df1, sizeof(df1));
  // first is master channel
  for (int j=0; j<_channels+1; j++) {
      uint8_t df2[] = { U32_TO_U8S_LE((AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS)) };
      append(buf, df2, sizeof(df2));
  }
  /*_stridx 0x00*/ 
  uint8_t df3[1] = { 0x00 };
  append(buf, df3, sizeof(df3));

//-- out ---

    /* Output Terminal Descriptor(4.7.2.5) */
    // TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ UAC2_ENTITY_SPK_OUTPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_OUT_HEADPHONES, /*_assocTerm*/ 0x00, /*_srcid*/ UAC2_ENTITY_SPK_FEATURE_UNIT, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00),
  uint8_t d3[] = {TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ UAC2_ENTITY_SPK_OUTPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_OUT_HEADPHONES, /*_assocTerm*/ 00, /*_srcid*/ UAC2_ENTITY_SPK_FEATURE_UNIT, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00)};
  append(buf, d3, sizeof(d3));

    /* Input Terminal Descriptor(4.7.2.4) */
    // TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_IN_GENERIC_MIC, /*_assocTerm*/ 0x00, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_nchannelslogical*/ 0x01, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ 0x00, /*_ctrl*/ 0 * (AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS), /*_stridx*/ 0x00),
  uint8_t d4[] = {TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_IN_GENERIC_MIC, /*_assocTerm*/ 00, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_nchannelslogical*/ _channels, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ 0x00, /*_ctrl*/0 * (AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS), /*_stridx*/ 0x00)};
  append(buf, d4, sizeof(d4));

    /* Output Terminal Descriptor(4.7.2.5) */
    // TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x00, /*_srcid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00),
  uint8_t d5[] = {TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 00, /*_srcid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00)};
  append(buf, d5, sizeof(d5));

    /* Standard AC Interrupt Endpoint Descriptor(4.8.2.1) */
    // TUD_AUDIO_DESC_STD_AC_INT_EP(/*_ep*/ _epint, /*_interval*/ 0x01), 
  uint8_t d6[] = {TUD_AUDIO_DESC_STD_AC_INT_EP(/*_ep*/ _ep_int, /*_interval*/ 0x01)};
  append(buf, d6, sizeof(d6));

// -- SPK ---

    /* Standard AS Interface Descriptor(4.9.1) */
    /* Interface 1, Alternate 0 - default alternate setting with 0 bandwidth */
    // TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(ITF_NUM_AUDIO_STREAMING_SPK), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x05),
  uint8_t d7[] = { TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(_itfnum_spk), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x05)};
  append(buf, d7, sizeof(d7));

    /* Standard AS Interface Descriptor(4.9.1) */
    /* Interface 1, Alternate 1 - alternate interface for data streaming */
    // TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(ITF_NUM_AUDIO_STREAMING_SPK), /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ 0x05),
  uint8_t d8[] = { TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(_itfnum_spk), /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ 0x05)};
  append(buf, d8, sizeof(d8));


    /* Class-Specific AS Interface Descriptor(4.9.2) */
    // TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00),
  uint8_t d9[] = {TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ _channels, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00)};
  append(buf, d9, sizeof(d9));


    /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */
    // TUD_AUDIO_DESC_TYPE_I_FORMAT(CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX, CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX),
  uint8_t d10[] = {TUD_AUDIO_DESC_TYPE_I_FORMAT((uint8_t)(_bits_per_sample/8), (uint8_t)_bits_per_sample)};
  append(buf, d10, sizeof(d10));


    /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */
    // TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ _epout, /*_attr*/ (uint8_t) ((uint8_t)TUSB_XFER_ISOCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_ADAPTIVE | (uint8_t)TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE, CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX), /*_interval*/ 0x01),
  uint8_t d11[] = {TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ _ep_spk, /*_attr*/ (uint8_t) ((uint8_t)TUSB_XFER_ISOCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_ADAPTIVE | (uint8_t)TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ getMaxEPSize(), /*_interval*/ 0x01)};
  append(buf, d11, sizeof(d11));

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */
    // TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_MILLISEC, /*_lockdelay*/ 0x0001),
  uint8_t d12[] = {TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_MILLISEC, /*_lockdelay*/ 0x0001)};
  append(buf, d12, sizeof(d12));

// -- MIC ---
    /* Standard AS Interface Descriptor(4.9.1) */
    /* Interface 2, Alternate 0 - default alternate setting with 0 bandwidth */
    // TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(ITF_NUM_AUDIO_STREAMING_MIC), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x04),
  uint8_t d13[] = {TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(_itfnum_mic), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x04)};
  append(buf, d13, sizeof(d13));

    /* Standard AS Interface Descriptor(4.9.1) */
    /* Interface 2, Alternate 1 - alternate interface for data streaming */
    // TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(ITF_NUM_AUDIO_STREAMING_MIC), /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ 0x04),
  uint8_t d14[] = {TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(_itfnum_mic), /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ 0x04)};
  append(buf, d14, sizeof(d14));

    /* Class-Specific AS Interface Descriptor(4.9.2) */
    // TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00),
  uint8_t d15[] = {TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ _channels, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00)};
  append(buf, d15, sizeof(d15));

    /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */
    // TUD_AUDIO_DESC_TYPE_I_FORMAT(CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX, CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_TX),
  uint8_t d16[] = {TUD_AUDIO_DESC_TYPE_I_FORMAT((uint8_t)(_bits_per_sample/8), (uint8_t)_bits_per_sample)};
  append(buf, d16, sizeof(d16));

    /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */
    // TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ _epin, /*_attr*/ (uint8_t) ((uint8_t)TUSB_XFER_ISOCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_ASYNCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE, CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX), /*_interval*/ 0x01),
  uint8_t d17[] = {TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ _ep_mic, /*_attr*/ (uint8_t) ((uint8_t)TUSB_XFER_ISOCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_ASYNCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ getMaxEPSize(), /*_interval*/ 0x01)};
  append(buf, d17, sizeof(d17));

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */
    // TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, /*_lockdelay*/ 0x0000),
  uint8_t d18[] = {TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, /*_lockdelay*/ 0x0000)};
  append(buf, d18, sizeof(d18));

}


bool Adafruit_USBD_Audio::clock_get_request(
  uint8_t rhport, tusb_control_request_t const *p_request) {
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  // uint8_t itf = TU_U16_LOW(p_request->wIndex); 			// Since
  // we have only one audio function implemented, we do not need the itf value
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);
  switch (ctrlSel) {
    case AUDIO_CS_CTRL_SAM_FREQ:
      // channelNum is always zero in this case
      switch (p_request->bRequest) {
        case AUDIO_CS_REQ_CUR:
          LOG_AUDIO_DEBUG("    Get Sample Freq. -> %d", _sample_rate);
          // Buffered control transfer is needed for IN flow control to work
          return tud_audio_buffer_and_schedule_control_xfer(
              rhport, p_request, &_sample_rate, sizeof(_sample_rate));

        case AUDIO_CS_REQ_RANGE:
          LOG_AUDIO_DEBUG("    Get Sample Freq. range -> %d - %d",AUDIO_FREQ_MIN,AUDIO_FREQ_MAX);
          audio_control_range_4_n_t(2) sampleFreqRng;
          sampleFreqRng.wNumSubRanges = 1;
          sampleFreqRng.subrange[0].bMin = _sample_rate;
          sampleFreqRng.subrange[0].bMax = _sample_rate;
          sampleFreqRng.subrange[0].bRes = 0;
          sampleFreqRng.subrange[1].bMin = AUDIO_FREQ_MIN;
          sampleFreqRng.subrange[1].bMax = AUDIO_FREQ_MAX;
          sampleFreqRng.subrange[1].bRes = 0;
          return tud_control_xfer(rhport, p_request, &sampleFreqRng,
                                  sizeof(sampleFreqRng));

        // Unknown/Unsupported control
        default:
          //TU_BREAKPOINT();
          return false;
      }
      break;

    case AUDIO_CS_CTRL_CLK_VALID:
      // Only cur attribute exists for this request
      LOG_AUDIO_DEBUG("    Get Sample Freq. valid");
      return tud_control_xfer(rhport, p_request, &_clk_is_valid,
                              sizeof(_clk_is_valid));

    // Unknown/Unsupported control
    default:
      //TU_BREAKPOINT();
      return false;
  }
}

// for speaker:
bool Adafruit_USBD_Audio::clock_set_request(
    uint8_t rhport, tusb_control_request_t const *p_request,
    uint8_t const *buf) {
  (void)rhport;
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  // uint8_t itf = TU_U16_LOW(p_request->wIndex); 			// Since
  // we have only one audio function implemented, we do not need the itf value
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

  // TU_ASSERT(request->bEntityID == UAC2_ENTITY_CLOCK);
  //TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  if (ctrlSel == AUDIO_CS_CTRL_SAM_FREQ) {
    TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_4_t));
    _sample_rate = (uint32_t)((audio_control_cur_4_t const *)buf)->bCur;
    LOG_AUDIO_DEBUG("Clock set current freq: %ld", _sample_rate);
    return true;
  } else {
    LOG_AUDIO_DEBUG(
        "Clock set request not supported, entity = %u, selector = %u, request "
        "= %u",
        entityID, ctrlSel, p_request->bRequest);
    return false;
  }
}

#if CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP
void Adafruit_USBD_Audio::feedback_params_cb(
    uint8_t func_id, uint8_t alt_itf, audio_feedback_params_t *feedback_param) {
  (void)func_id;
  (void)alt_itf;
  // Set feedback method to fifo counting
  feedback_param->method = AUDIO_FEEDBACK_METHOD_FIFO_COUNT;
  feedback_param->sample_freq = _sample_rate;
}
#endif

// Helper for feature unit get requests
bool Adafruit_USBD_Audio::feature_unit_get_request(
    uint8_t rhport, tusb_control_request_t const *p_request) {
  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  // uint8_t itf = TU_U16_LOW(p_request->wIndex); 			// Since
  // we have only one audio function implemented, we do not need the itf value
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);
  switch (ctrlSel) {
    case AUDIO_FU_CTRL_MUTE:
      // Audio control mute cur parameter block consists of only one byte - we
      // thus can send it right away There does not exist a range parameter
      // block for mute
      LOG_AUDIO_DEBUG("    Get Mute of channel: %u", channelNum);
      return tud_control_xfer(rhport, p_request, &_mute[channelNum], 1);

    case AUDIO_FU_CTRL_VOLUME:
      switch (p_request->bRequest) {
        case AUDIO_CS_REQ_CUR:
          LOG_AUDIO_DEBUG("    Get Volume of channel: %u", channelNum);
          return tud_control_xfer(rhport, p_request, &_volume[channelNum],
                                  sizeof(_volume[channelNum]));

        case AUDIO_CS_REQ_RANGE:
          LOG_AUDIO_DEBUG("    Get Volume range of channel: %u", channelNum);

          // Copy values - only for testing - better is version below
          audio_control_range_2_n_t(1) ret;

          ret.wNumSubRanges = 1;
          ret.subrange[0].bMin = 0;  // -90 dB
          ret.subrange[0].bMax = 100;   // +90 dB
          ret.subrange[0].bRes = 1;    // 1 dB steps

          return tud_audio_buffer_and_schedule_control_xfer(
              rhport, p_request, (void *)&ret, sizeof(ret));

          // Unknown/Unsupported control
        default:
          //TU_BREAKPOINT();
          return false;
      }
      break;

      // Unknown/Unsupported control
    default:
      //TU_BREAKPOINT();
      return false;
  }
  return false;
}

// Helper for feature unit set requests for speaker
bool Adafruit_USBD_Audio::feature_unit_set_request(
    uint8_t rhport, tusb_control_request_t const *p_request,
    uint8_t const *buf) {
  (void)rhport;
  audio_control_request_t const *request =(audio_control_request_t const *)p_request;

  // TU_ASSERT(request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT);
  TU_VERIFY(request->bRequest == AUDIO_CS_REQ_CUR);

  if (request->bControlSelector == AUDIO_FU_CTRL_MUTE) {
    TU_VERIFY(request->wLength == sizeof(audio_control_cur_1_t));

    _mute[request->bChannelNumber] = ((audio_control_cur_1_t const *)buf)->bCur;

    LOG_AUDIO_DEBUG("Set channel %d Mute: %d", request->bChannelNumber,
            _mute[request->bChannelNumber]);

    return true;
  } else if (request->bControlSelector == AUDIO_FU_CTRL_VOLUME) {
    TU_VERIFY(request->wLength == sizeof(audio_control_cur_2_t));

    _volume[request->bChannelNumber] =
        ((audio_control_cur_2_t const *)buf)->bCur;

    LOG_AUDIO_DEBUG("Set channel %d volume: %d dB", request->bChannelNumber,
            _volume[request->bChannelNumber] / 256);

    return true;
  } else {
    LOG_AUDIO_DEBUG(
        "Feature unit set request not supported, entity = %u, selector = %u, "
        "request = %u",
        request->bEntityID, request->bControlSelector, request->bRequest);
    return false;
  }
}

/// We can use 8 debug pins with a logic analyser
void Adafruit_USBD_Audio::setupDebugPins() {
#if AUDIO_DEBUG
  for (int j = 0; j < 8; j++) {
    pinMode(j, OUTPUT);
  }
#endif
}

//--------------------------------------------------------------------+
// Global Callback API Dispatch
//--------------------------------------------------------------------+

bool tud_audio_set_itf_cb(uint8_t rhport,
                          tusb_control_request_t const *p_request) {
  if (self_Adafruit_USBD_Audio==nullptr) return false;
  return self_Adafruit_USBD_Audio->set_itf_cb(rhport, p_request);
}

// Invoked when audio class specific set request received for an EP
bool tud_audio_set_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request,
                             uint8_t *pBuff) {
  if (self_Adafruit_USBD_Audio==nullptr) return false;
  return self_Adafruit_USBD_Audio->set_req_ep_cb(rhport, p_request, pBuff);
}

// Invoked when audio class specific set request received for an interface
bool tud_audio_set_req_itf_cb(uint8_t rhport,
                              tusb_control_request_t const *p_request,
                              uint8_t *pBuff) {
  if (self_Adafruit_USBD_Audio==nullptr) return false;
  return self_Adafruit_USBD_Audio->set_req_itf_cb(rhport, p_request, pBuff);
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const *p_request,
                                 uint8_t *pBuff) {
  if (self_Adafruit_USBD_Audio==nullptr) return false;
  return self_Adafruit_USBD_Audio->set_req_entity_cb(rhport, p_request, pBuff);
}
// Invoked when audio class specific get request received for an EP
bool tud_audio_get_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request) {
  if (self_Adafruit_USBD_Audio==nullptr) return false;
  return self_Adafruit_USBD_Audio->get_req_ep_cb(rhport, p_request);
}

// Invoked when audio class specific get request received for an interface
bool tud_audio_get_req_itf_cb(uint8_t rhport,
                              tusb_control_request_t const *p_request) {
  if (self_Adafruit_USBD_Audio==nullptr) return false;
  return self_Adafruit_USBD_Audio->get_req_itf_cb(rhport, p_request);
}

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const *p_request) {
  if (self_Adafruit_USBD_Audio==nullptr) return false;
  return self_Adafruit_USBD_Audio->get_req_entity_cb(rhport, p_request);
}

bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in,
                                   uint8_t cur_alt_setting) {
  if (self_Adafruit_USBD_Audio==nullptr) return false;
  return self_Adafruit_USBD_Audio->tx_done_pre_load_cb(rhport, itf, ep_in,
                                                       cur_alt_setting);
}

bool tud_audio_tx_done_post_load_cb(uint8_t rhport, uint16_t n_bytes_copied,
                                    uint8_t itf, uint8_t ep_in,
                                    uint8_t cur_alt_setting) {
  if (self_Adafruit_USBD_Audio==nullptr) return false;
  return self_Adafruit_USBD_Audio->tx_done_post_load_cb(
      rhport, n_bytes_copied, itf, ep_in, cur_alt_setting);
}

bool tud_audio_rx_done_pre_read_cb(uint8_t rhport, uint16_t n_bytes_received,
                                   uint8_t func_id, uint8_t ep_out,
                                   uint8_t cur_alt_setting) {
  if (self_Adafruit_USBD_Audio==nullptr) return false;
  return self_Adafruit_USBD_Audio->rx_done_pre_read_cb(
      rhport, n_bytes_received, func_id, ep_out, cur_alt_setting);
}

bool tud_audio_rx_done_post_read_cb(uint8_t rhport, uint16_t n_bytes_received,
                                    uint8_t func_id, uint8_t ep_out,
                                    uint8_t cur_alt_setting) {
  if (self_Adafruit_USBD_Audio==nullptr) return false;
  return self_Adafruit_USBD_Audio->rx_done_post_read_cb(
      rhport, n_bytes_received, func_id, ep_out, cur_alt_setting);
}

bool tud_audio_set_itf_close_EP_cb(uint8_t rhport,
                                   tusb_control_request_t const *p_request) {
  if (self_Adafruit_USBD_Audio==nullptr) return false;
  return self_Adafruit_USBD_Audio->set_itf_close_EP_cb(rhport, p_request);
}

uint16_t tinyusb_audio_descriptor_length(uint8_t func) {
  if (self_Adafruit_USBD_Audio==nullptr) return 0;
  return self_Adafruit_USBD_Audio->getInterfaceDescriptorLength();
}

#if CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP

void tud_audio_feedback_params_cb(uint8_t func_id, uint8_t alt_itf, audio_feedback_params_t *feedback_param) {
  if (self_Adafruit_USBD_Audio != nullptr){
      self_Adafruit_USBD_Audio->feedback_params_cb(func_id, alt_itf, feedback_param);
  } 
}

#endif

#endif