

#include "Adafruit_USBD_Audio.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "arduino/Adafruit_USBD_Device.h"

#if CFG_TUD_AUDIO

Adafruit_USBD_Audio *self_Adafruit_USBD_Audio = nullptr;

/*------------- MAIN -------------*/
bool Adafruit_USBD_Audio::begin(unsigned long rate, int bytesPerSample,
                                int channeld) {
  // init device stack on configured roothub port
  // Init values
  this->_sample_rate = rate;
  this->_bits_per_sample = bytesPerSample;
  this->_channels = channeld;
  _clk_is_valid = 1;
  _is_active = true;

  //tud_init(rh_port);
  bool rc = TinyUSBDevice.addInterface(*this);

  return rc;
}

void Adafruit_USBD_Audio::end() {
  tud_deinit(rh_port);
  _is_active = false;
  if (_out_buffer.size() > 0) _out_buffer.resize(0);
  if (_in_buffer.size() > 0) _out_buffer.resize(0);
}

//--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when set interface is called, typically on start/stop streaming or
// format change
bool Adafruit_USBD_Audio::set_itf_cb(uint8_t rhport,
                                     tusb_control_request_t const *p_request) {
  // uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
  uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));

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
    uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *pBuff) {
  (void)rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

  (void)itf;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // If request is for our feature unit
  if (entityID == UAC2_ENTITY_SPK_FEATURE_UNIT) {
    switch (ctrlSel) {
      case AUDIO_FU_CTRL_MUTE:
        // Request uses format layout 1
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_1_t));

        _mute[channelNum] = ((audio_control_cur_2_t *)pBuff)->bCur;

        TU_LOG2("    Set Mute: %d of channel: %u\r\n", _mute[channelNum],
                channelNum);
        return true;

      case AUDIO_FU_CTRL_VOLUME:
        // Request uses format layout 2
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));

        _volume[channelNum] = (uint16_t)((audio_control_cur_2_t *)pBuff)->bCur;

        TU_LOG2("    Set Volume: %d dB of channel: %u\r\n", _volume[channelNum],
                channelNum);
        return true;

        // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  // Clock Source unit
  if (entityID == UAC2_ENTITY_CLOCK) {
    switch (ctrlSel) {
      case AUDIO_CS_CTRL_SAM_FREQ:
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));

        _sample_rate = (uint32_t)((audio_control_cur_2_t *)pBuff)->bCur;

        TU_LOG2("Clock set current freq: %" PRIu32 "\r\n", sampFreq);

        return true;
        break;

      // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

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
  (void)rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  // uint8_t itf = TU_U16_LOW(p_request->wIndex); 			// Since
  // we have only one audio function implemented, we do not need the itf value
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

  // Input terminal (Microphone input)
  if (entityID == UAC2_ENTITY_SPK_INPUT_TERMINAL) {
    switch (ctrlSel) {
      case AUDIO_TE_CTRL_CONNECTOR: {
        // The terminal connector control only has a get request with only the
        // CUR attribute.
        audio_desc_channel_cluster_t ret;

        // Those are dummy values for now
        ret.bNrChannels = _channels;
        ret.bmChannelConfig = (audio_channel_config_t)0;
        ret.iChannelNames = 0;

        TU_LOG2("    Get terminal connector\r\n");

        return tud_audio_buffer_and_schedule_control_xfer(
            rhport, p_request, (void *)&ret, sizeof(ret));
      } break;

        // Unknown/Unsupported control selector
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  // Feature unit
  if (entityID == UAC2_ENTITY_SPK_FEATURE_UNIT) {
    switch (ctrlSel) {
      case AUDIO_FU_CTRL_MUTE:
        // Audio control mute cur parameter block consists of only one byte - we
        // thus can send it right away There does not exist a range parameter
        // block for mute
        TU_LOG2("    Get Mute of channel: %u\r\n", channelNum);
        return tud_control_xfer(rhport, p_request, &_mute[channelNum], 1);

      case AUDIO_FU_CTRL_VOLUME:
        switch (p_request->bRequest) {
          case AUDIO_CS_REQ_CUR:
            TU_LOG2("    Get Volume of channel: %u\r\n", channelNum);
            return tud_control_xfer(rhport, p_request, &_volume[channelNum],
                                    sizeof(_volume[channelNum]));

          case AUDIO_CS_REQ_RANGE:
            TU_LOG2("    Get Volume range of channel: %u\r\n", channelNum);

            // Copy values - only for testing - better is version below
            audio_control_range_2_n_t(1) ret;

            ret.wNumSubRanges = 1;
            ret.subrange[0].bMin = -90;  // -90 dB
            ret.subrange[0].bMax = 30;   // +30 dB
            ret.subrange[0].bRes = 1;    // 1 dB steps

            return tud_audio_buffer_and_schedule_control_xfer(
                rhport, p_request, (void *)&ret, sizeof(ret));

            // Unknown/Unsupported control
          default:
            TU_BREAKPOINT();
            return false;
        }
        break;

        // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  // Clock Source unit
  if (entityID == UAC2_ENTITY_CLOCK) {
    switch (ctrlSel) {
      case AUDIO_CS_CTRL_SAM_FREQ:
        // channelNum is always zero in this case
        switch (p_request->bRequest) {
          case AUDIO_CS_REQ_CUR:
            TU_LOG2("    Get Sample Freq.\r\n");
            return tud_control_xfer(rhport, p_request, &_sample_rate,
                                    sizeof(_sample_rate));

          case AUDIO_CS_REQ_RANGE: {
            TU_LOG2("    Get Sample Freq. range\r\n");
            audio_control_range_2_n_t(1)
                rangef = {.wNumSubRanges = tu_htole16(1)};
            TU_LOG1("Clock get %d freq ranges\r\n", 1);
            rangef.subrange[0].bMin = (int16_t)AUDIO_FREQ_MIN;
            rangef.subrange[0].bMax = (int16_t)AUDIO_FREQ_MAX;
            rangef.subrange[0].bRes = 0;
            TU_LOG1("Range %d (%d, %d, %d)\r\n", 0,
                    (int)rangef.subrange[0].bMin, (int)rangef.subrange[0].bMax,
                    (int)rangef.subrange[0].bRes);
            return tud_audio_buffer_and_schedule_control_xfer(
                rhport, p_request, &rangef, sizeof(rangef));
          }
            // Unknown/Unsupported control
          default:
            TU_BREAKPOINT();
            return false;
        }
        break;

      case AUDIO_CS_CTRL_CLK_VALID:
        // Only cur attribute exists for this request
        TU_LOG2("    Get Sample Freq. valid\r\n");
        return tud_control_xfer(rhport, p_request, &_clk_is_valid, sizeof(_clk_is_valid));

      // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  TU_LOG2("  Unsupported entity: %d\r\n", entityID);
  return false;  // Yet not implemented
}

bool Adafruit_USBD_Audio::tx_done_pre_load_cb(uint8_t rhport, uint8_t itf,
                                              uint8_t ep_in,
                                              uint8_t cur_alt_setting) {
  (void)rhport;
  (void)itf;
  (void)ep_in;
  (void)cur_alt_setting;

  tud_audio_write((uint8_t *)&_out_buffer[0], get_io_size());

  return true;
}

bool Adafruit_USBD_Audio::tx_done_post_load_cb(uint8_t rhport,
                                               uint16_t n_bytes_copied,
                                               uint8_t itf, uint8_t ep_in,
                                               uint8_t cur_alt_setting) {
  (void)rhport;
  (void)n_bytes_copied;
  (void)itf;
  (void)ep_in;
  (void)cur_alt_setting;

  uint16_t len = get_io_size();
  size_t rc = 0;

  // output audio to usb
  if (isReadDefined()) {
    if (_out_buffer.size() < len) _out_buffer.resize(len);
    uint8_t *adr = &_out_buffer[0];
    memset(adr, len, 0);
    rc = p_read_callback(adr, len, *this);
  }

  return true;
}

bool Adafruit_USBD_Audio::rx_done_pre_read_cb(uint8_t rhport,
                                              uint16_t n_bytes_received,
                                              uint8_t func_id, uint8_t ep_out,
                                              uint8_t cur_alt_setting) {
  // read audio from usb
  if (isWriteDefined()) {
    uint16_t len = get_io_size();
    if (_in_buffer.size() < len) _in_buffer.resize(len);
    uint8_t *adr = &_in_buffer[0];
    uint16_t len_read = tud_audio_read(adr, len);
    return true;
  }
  return false;
}

bool Adafruit_USBD_Audio::rx_done_post_read_cb(uint8_t rhport,
                                               uint16_t n_bytes_received,
                                               uint8_t func_id, uint8_t ep_out,
                                               uint8_t cur_alt_setting) {
  // read audio from usb
  if (isWriteDefined()) {
    uint16_t len = get_io_size();
    uint8_t *adr = &_in_buffer[0];
    size_t rc = p_write_callback(adr, len, *this);
    // we assume a blocking write
    //assert(rc == len);
    //return true;
  }
  return true;
}

bool Adafruit_USBD_Audio::set_itf_close_EP_cb(
    uint8_t rhport, tusb_control_request_t const *p_request) {
  (void)rhport;
  (void)p_request;

  return true;
}

uint16_t Adafruit_USBD_Audio::getDescrCtlLen(){
  return TUD_AUDIO_DESC_CLK_SRC_LEN + TUD_AUDIO_DESC_INPUT_TERM_LEN +     
    TUD_AUDIO_DESC_OUTPUT_TERM_LEN + TUD_AUDIO_DESC_INPUT_TERM_LEN + 
    TUD_AUDIO_DESC_OUTPUT_TERM_LEN;
}

uint16_t Adafruit_USBD_Audio::getMaxEPSize(){
  return TUD_AUDIO_EP_SIZE(_sample_rate, _bits_per_sample / 8, _channels);
}

uint16_t Adafruit_USBD_Audio::getInterfaceDescriptor(uint8_t itfnum_deprecated,
                                                     uint8_t *buf,
                                                     uint16_t bufsize) {
  (void)itfnum_deprecated;

  // if no source or sink then audio is not active
  if (!isReadDefined() && !isWriteDefined()) 
    return 0;

  if (buf == nullptr && desc_len > 0){
    return desc_len;
  }

  uint8_t _itfnum_ctl = TinyUSBDevice.allocInterface();
  uint8_t _itfnum_spk = TinyUSBDevice.allocInterface();
  uint8_t _itfnum_mic = TinyUSBDevice.allocInterface();
  uint8_t itf_number_total = 3;

  uint8_t ep_in = TinyUSBDevice.allocEndpoint(_itfnum_spk);
  uint8_t ep_out = TinyUSBDevice.allocEndpoint(_itfnum_mic);
  uint8_t ep_fb = TinyUSBDevice.allocEndpoint(_itfnum_ctl);

  uint8_t _stridx = 0; //2;
  uint8_t _strid_rx = 0; // 4;
  uint8_t _strid_tx = 0; //5;
  // uint8_t audio_desc[] = {TUD_AUDIO_DESCRIPTOR(2, 4, 5, EPNUM_AUDIO_OUT, EPNUM_AUDIO_IN | 0x80)};
  // memcpy(buf + len, audio_desc, sizeof(audio_desc));
  // len += sizeof(audio_desc);

  append_pos = 0;
  /* Standard Interface Association Descriptor (IAD) */
  uint8_t d1[] = {TUD_AUDIO_DESC_IAD(/*_firstitfs*/ _itfnum_ctl, /*_nitfs*/ itf_number_total, /*_stridx*/ 0)};
  append(buf, d1, sizeof(d1));

  /* Standard AC Interface Descriptor(4.7.1) */\
  uint8_t d2[] = {TUD_AUDIO_DESC_STD_AC(/*_itfnum*/ _itfnum_ctl, /*_nEPs*/ 0x00, /*_stridx*/ _stridx)};
  append(buf, d2, sizeof(d2));

  /* Class-Specific AC Interface Header Descriptor(4.7.2) */
  uint8_t d3[] = {TUD_AUDIO_DESC_CS_AC(/*_bcdADC*/ 0x0200, /*_category*/ AUDIO_USB_FUNC, /*_totallen*/  getDescrCtlLen(), /*_ctrl*/ AUDIO_CTRL_NONE << AUDIO_CS_AS_INTERFACE_CTRL_LATENCY_POS)};
  append(buf, d3, sizeof(d3));

  /* Clock Source Descriptor(4.7.2.1) */
  uint8_t d4[] = {TUD_AUDIO_DESC_CLK_SRC(/*_clkid*/ UAC2_ENTITY_CLOCK, /*_attr*/ AUDIO_CLOCK_SOURCE_ATT_EXT_CLK, /*_ctrl*/ (AUDIO_CTRL_NONE << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS)|(AUDIO_CTRL_NONE << AUDIO_CLOCK_SOURCE_CTRL_CLK_VAL_POS), /*_assocTerm*/ 0x00,  /*_stridx*/ 0x00)};
  append(buf, d4, sizeof(d4));
  /* Input Terminal Descriptor(4.7.2.4) */
  uint8_t d5[] = {TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x00, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_nchannelslogical*/ _channels, /*_channelcfg*/ AUDIO_USB_CHANNEL_ASSIGN, /*_idxchannelnames*/ 0x00, /*_ctrl*/ AUDIO_CTRL_NONE << AUDIO_IN_TERM_CTRL_CONNECTOR_POS, /*_stridx*/ 0x00)};
  append(buf, d5, sizeof(d5));
  /* Output Terminal Descriptor(4.7.2.5) */
  uint8_t d6[] = {TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ UAC2_ENTITY_SPK_OUTPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_OUT_GENERIC_SPEAKER, /*_assocTerm*/ 0x00, /*_srcid*/ UAC2_ENTITY_SPK_FEATURE_UNIT, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00)};
  append(buf, d6, sizeof(d6));
  /* Input Terminal Descriptor(4.7.2.4) */
  uint8_t d7[] = {TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_IN_GENERIC_MIC, /*_assocTerm*/ 0x00, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_nchannelslogical*/ _channels, /*_channelcfg*/ AUDIO_USB_CHANNEL_ASSIGN, /*_idxchannelnames*/ 0x00, /*_ctrl*/ AUDIO_CTRL_NONE << AUDIO_IN_TERM_CTRL_CONNECTOR_POS, /*_stridx*/ 0x00)};
  append(buf, d7, sizeof(d7));
  /* Output Terminal Descriptor(4.7.2.5) */
  uint8_t d8[] = {TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x00, /*_srcid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00)};
  append(buf, d8, sizeof(d8));
  /* Standard AS Interface Descriptor(4.9.1) */
  /* Interface 1, Alternate 0 - default alternate setting with 0 bandwidth */
  uint8_t d9[] = {TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(_itfnum_spk), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ _strid_rx)};
  append(buf, d9, sizeof(d9));
  /* Standard AS Interface Descriptor(4.9.1) */
  /* Interface 1, Alternate 1 - alternate interface for data streaming */
  uint8_t d10[] = {TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(_itfnum_spk), /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ _strid_rx)};
  append(buf, d10, sizeof(d10));
  /* Class-Specific AS Interface Descriptor(4.9.2) */
  uint8_t d11[] = {TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ _channels, /*_channelcfg*/ AUDIO_USB_CHANNEL_ASSIGN, /*_stridx*/ 0x00)};
  append(buf, d11, sizeof(d11));
  /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */
  uint8_t d12[] = {TUD_AUDIO_DESC_TYPE_I_FORMAT((uint8_t)(_bits_per_sample/8), _bits_per_sample)};
  append(buf, d12, sizeof(d12));
  /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */
  uint8_t d13[] = {TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ ep_out, /*_attr*/ (TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ADAPTIVE | TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ getMaxEPSize(), /*_interval*/ 0x01)};
  append(buf, d13, sizeof(d13));
  /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */
  uint8_t d14[] = {TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_MILLISEC, /*_lockdelay*/ 0x0001)};
  append(buf, d14, sizeof(d14));
  /* Standard AS Interface Descriptor(4.9.1) */\
  /* Interface 2, Alternate 0 - default alternate setting with 0 bandwidth */
  uint8_t d15[] = {TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(_itfnum_mic), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ _strid_tx)};
  append(buf, d15, sizeof(d15));
  /* Standard AS Interface Descriptor(4.9.1) */
  /* Interface 2, Alternate 1 - alternate interface for data streaming */
  uint8_t d16[] = {TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(_itfnum_mic), /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ _strid_tx)};
  append(buf, d16, sizeof(d16));
  /* Class-Specific AS Interface Descriptor(4.9.2) */
  uint8_t d17[] = {TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ _channels, /*_channelcfg*/ AUDIO_USB_CHANNEL_ASSIGN, /*_stridx*/ 0x00)};
  append(buf, d17, sizeof(d17));
  /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */
  uint8_t d18[] = {TUD_AUDIO_DESC_TYPE_I_FORMAT((uint8_t)(_bits_per_sample/8), (uint8_t)_bits_per_sample)};
  append(buf, d18, sizeof(d18));
  /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */
  uint8_t d19[] = {TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ ep_in, /*_attr*/ (TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ASYNCHRONOUS | TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ getMaxEPSize(), /*_interval*/ 0x01)};
  append(buf, d19, sizeof(d19));
  /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */
  uint8_t d20[] = {TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, /*_lockdelay*/ 0x0000)};
  append(buf, d20, sizeof(d20));

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  if (desc_len==0){
    desc_len = append_pos;
  }

  return desc_len;
}

//--------------------------------------------------------------------+
// Global Callback API Diapatch
//--------------------------------------------------------------------+

bool tud_audio_set_itf_cb(uint8_t rhport,
                          tusb_control_request_t const *p_request) {
  return self_Adafruit_USBD_Audio->set_itf_cb(rhport, p_request);
}

// Invoked when audio class specific set request received for an EP
bool tud_audio_set_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request,
                             uint8_t *pBuff) {
  return self_Adafruit_USBD_Audio->set_req_ep_cb(rhport, p_request, pBuff);
}

// Invoked when audio class specific set request received for an interface
bool tud_audio_set_req_itf_cb(uint8_t rhport,
                              tusb_control_request_t const *p_request,
                              uint8_t *pBuff) {
  return self_Adafruit_USBD_Audio->set_req_itf_cb(rhport, p_request, pBuff);
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const *p_request,
                                 uint8_t *pBuff) {
  return self_Adafruit_USBD_Audio->set_req_entity_cb(rhport, p_request, pBuff);
}
// Invoked when audio class specific get request received for an EP
bool tud_audio_get_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request) {
  return self_Adafruit_USBD_Audio->get_req_ep_cb(rhport, p_request);
}

// Invoked when audio class specific get request received for an interface
bool tud_audio_get_req_itf_cb(uint8_t rhport,
                              tusb_control_request_t const *p_request) {
  return self_Adafruit_USBD_Audio->get_req_itf_cb(rhport, p_request);
}

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const *p_request) {
  return self_Adafruit_USBD_Audio->get_req_entity_cb(rhport, p_request);
}

bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in,
                                   uint8_t cur_alt_setting) {
  return self_Adafruit_USBD_Audio->tx_done_pre_load_cb(rhport, itf, ep_in,
                                                       cur_alt_setting);
}

bool tud_audio_tx_done_post_load_cb(uint8_t rhport, uint16_t n_bytes_copied,
                                    uint8_t itf, uint8_t ep_in,
                                    uint8_t cur_alt_setting) {
  return self_Adafruit_USBD_Audio->tx_done_post_load_cb(
      rhport, n_bytes_copied, itf, ep_in, cur_alt_setting);
}

bool tud_audio_set_itf_close_EP_cb(uint8_t rhport,
                                   tusb_control_request_t const *p_request) {
  return self_Adafruit_USBD_Audio->set_itf_close_EP_cb(rhport, p_request);
}

bool tud_audio_rx_done_pre_read_cb(uint8_t rhport, uint16_t n_bytes_received,
                                   uint8_t func_id, uint8_t ep_out,
                                   uint8_t cur_alt_setting) {
  return self_Adafruit_USBD_Audio->rx_done_pre_read_cb(
      rhport, n_bytes_received, func_id, ep_out, cur_alt_setting);
}

bool tud_audio_rx_done_post_read_cb(uint8_t rhport, uint16_t n_bytes_received,
                                    uint8_t func_id, uint8_t ep_out,
                                    uint8_t cur_alt_setting) {
  return self_Adafruit_USBD_Audio->rx_done_post_read_cb(
      rhport, n_bytes_received, func_id, ep_out, cur_alt_setting);
}

int getUSBDAudioInterfaceDescriptorLength() {return self_Adafruit_USBD_Audio->getInterfaceDescriptorLength();}

#endif