

#include "Adafruit_USBD_Audio.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "arduino/Adafruit_USBD_Device.h"

#if CFG_TUD_AUDIO

const uint8_t desc_configuration[] = {
    // Interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // string index overall, string index spk, string index mic, EP Out & EP
    // In
    // address
    TUD_AUDIO_DESCRIPTOR(2, 4, 5, EPNUM_AUDIO_OUT, EPNUM_AUDIO_IN | 0x80)

    // Interface number, string index, protocol, report descriptor len, EP OUT
    // &
    // IN address, size & polling interval
    //    TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID_INOUT, 6, 0, ???,
    //    EPNUM_HID_OUT,
    //    EPNUM_HID_IN | 0x80, 64, 1),
};

Adafruit_USBD_Audio *self_Adafruit_USBD_Audio = nullptr;

/*------------- MAIN -------------*/
bool Adafruit_USBD_Audio::begin(unsigned long rate, int bytesPerSample,
                                int channeld) {
  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  // Init values
  this->sampFreq = rate;
  this->bytesPerSample = bytesPerSample;
  this->_channels = channeld;
  clkValid = 1;
  _is_active = true;

  return 0;
}

void Adafruit_USBD_Audio::end() {
  tud_deinit(BOARD_TUD_RHPORT);
  _is_active = false;
  if (out_buffer.size() > 0) out_buffer.resize(0);
  if (in_buffer.size() > 0) out_buffer.resize(0);
}

//--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when set interface is called, typically on start/stop streaming or
// format change
bool Adafruit_USBD_Audio::set_itf_cb(uint8_t rhport,
                                     tusb_control_request_t const *p_request) {
  (void)rhport;
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

        volume[channelNum] = (uint16_t)((audio_control_cur_2_t *)pBuff)->bCur;

        TU_LOG2("    Set Volume: %d dB of channel: %u\r\n", volume[channelNum],
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

        sampFreq = (uint32_t)((audio_control_cur_2_t *)pBuff)->bCur;

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
        ret.bNrChannels = 1;
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
            return tud_control_xfer(rhport, p_request, &volume[channelNum],
                                    sizeof(volume[channelNum]));

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
            return tud_control_xfer(rhport, p_request, &sampFreq,
                                    sizeof(sampFreq));

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
        return tud_control_xfer(rhport, p_request, &clkValid, sizeof(clkValid));

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

  tud_audio_write((uint8_t *)&out_buffer[0], get_io_size());

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
    if (out_buffer.size() < len) out_buffer.resize(len);
    uint8_t *adr = &out_buffer[0];
    memset(adr, len, 0);
    rc = p_read_callback(adr, len, this);
  }

  return rc > 0;
}

bool Adafruit_USBD_Audio::rx_done_pre_read_cb(uint8_t rhport,
                                              uint16_t n_bytes_received,
                                              uint8_t func_id, uint8_t ep_out,
                                              uint8_t cur_alt_setting) {
  // read audio from usb
  if (isWriteDefined()) {
    uint16_t len = get_io_size();
    if (in_buffer.size() < len) in_buffer.resize(len);
    uint8_t *adr = &in_buffer[0];
    uint16_t len_read = tud_audio_read(adr, len);
    return len_read > 0;
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
    uint8_t *adr = &in_buffer[0];
    size_t rc = p_write_callback(adr, len, this);
    // we assume a blocking write
    assert(rc == len);
    return rc > 0;
  }
  return false;
}

bool Adafruit_USBD_Audio::set_itf_close_EP_cb(
    uint8_t rhport, tusb_control_request_t const *p_request) {
  (void)rhport;
  (void)p_request;

  return true;
}

uint16_t Adafruit_USBD_Audio::getInterfaceDescriptor(uint8_t itfnum_deprecated,
                                                     uint8_t *buf,
                                                     uint16_t bufsize) {
  (void)itfnum_deprecated;

  uint16_t const desc_len = sizeof(desc_configuration);

  // null buffer is used to get the length of descriptor only
  if (!buf) {
    return desc_len;
  }

  if (bufsize < desc_len) {
    return 0;
  }

  uint8_t _itfnum = TinyUSBDevice.allocInterface(2);
  uint8_t ep_in = TinyUSBDevice.allocEndpoint(TUSB_DIR_IN);
  uint8_t ep_out = TinyUSBDevice.allocEndpoint(TUSB_DIR_OUT);

  // const uint8_t *tmp = TUD_AUDIO_MIC_ONE_CH_2_FORMAT_DESCRIPTOR(itfnum, 0,
  // ep_in);
  memcpy(buf, desc_configuration, desc_len);

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

#endif