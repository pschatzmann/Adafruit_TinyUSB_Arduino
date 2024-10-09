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


#pragma once

// //--------------------------------------------------------------------
// // DEVICE CONFIGURATION
// //--------------------------------------------------------------------
// #define AUDIO_USB_MAX_CHANNELS 2
// #define AUDIO_USB_FUNC AUDIO_FUNC_HOME_THEATER
// #define AUDIO_USB_CHANNEL_ASSIGN  (AUDIO_CHANNEL_CONFIG_FRONT_LEFT | AUDIO_CHANNEL_CONFIG_FRONT_RIGHT)
// #define AUDIO_FREQ_MIN 8000
// #define AUDIO_FREQ_MAX 48000
// #define MAX_BITS_PER_SAMPLE 24

// // Unit numbers are arbitrary selected
// #define UAC2_ENTITY_CLOCK 0x07
// // Speaker path
// #define UAC2_ENTITY_SPK_INPUT_TERMINAL 0x08
// #define UAC2_ENTITY_SPK_FEATURE_UNIT 0x09
// #define UAC2_ENTITY_SPK_OUTPUT_TERMINAL 0x10
// // Microphone path
// #define UAC2_ENTITY_MIC_INPUT_TERMINAL 0x11
// #define UAC2_ENTITY_MIC_OUTPUT_TERMINAL 0x13

// //--------------------------------------------------------------------
// // AUDIO CLASS DRIVER CONFIGURATION
// //--------------------------------------------------------------------
// #define CFG_TUD_AUDIO 1  // number of audio streaming interfaces

// #define CFG_TUD_AUDIO_ENABLE_ENCODING 0
// #define CFG_TUD_AUDIO_ENABLE_TYPE_I_ENCODING 0
// #define CFG_TUD_AUDIO_EP_IN_FLOW_CONTROL 1
// #define CFG_TUD_AUDIO_ENABLE_EP_IN 1
// //#define CFG_TUD_AUDIO_ENABLE_EP_OUT true
// //#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP true

// #define CFG_TUD_AUDIO_FUNC_1_DESC_LEN getUSBDAudioInterfaceDescriptorLength() 
// #define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN
// #define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_OUT

// // Number of Standard AS Interface Descriptors (4.9.1) defined per audio
// // function - this is required to be able to remember the current alternate
// // settings of these interfaces - We restrict us here to have a constant number
// // for all audio functions (which means this has to be the maximum number of AS
// // interfaces an audio function has and a second audio function with less AS
// // interfaces just wastes a few bytes)
// #define CFG_TUD_AUDIO_FUNC_1_N_AS_INT 2

// // Size of control request buffer
// #define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ 64

// // TODO to be relplaced!
// #define CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE AUDIO_FREQ_MAX 
// #define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX    2         // This value is not required by the driver, it parses this information from the descriptor once the alternate interface is set by the host - we use it for the setup
// #define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX            2         // This value is not required by the driver, it parses this information from the descriptor once the alternate interface is set by the host - we use it for the setup
// #define CFG_TUD_AUDIO_EP_SZ_IN                        TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE, CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)
// #define CFG_TUD_AUDIO_FUNC_1_CHANNEL_PER_FIFO_TX      2         // One I2S stream contains two channels, each stream is saved within one support FIFO - this value is currently fixed, the driver does not support a changing value

// //--------------------------------------------------------------------
// // AUDIO CLASS DESCRIPTOR
// //--------------------------------------------------------------------
// #define TUD_AUDIO_MIC_TWO_CH_DESC_LEN (TUD_AUDIO_DESC_IAD_LEN\
//   + TUD_AUDIO_DESC_STD_AC_LEN\
//   + TUD_AUDIO_DESC_CS_AC_LEN\
//   + TUD_AUDIO_DESC_CLK_SRC_LEN\
//   + TUD_AUDIO_DESC_INPUT_TERM_LEN\
//   + TUD_AUDIO_DESC_OUTPUT_TERM_LEN\
//   + TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL_LEN\
//   + TUD_AUDIO_DESC_STD_AS_INT_LEN\
//   + TUD_AUDIO_DESC_STD_AS_INT_LEN\
//   + TUD_AUDIO_DESC_CS_AS_INT_LEN\
//   + TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN\
//   + TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN\
//   + TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN)

// #define TUD_AUDIO_MIC_TWO_CH_DESC_N_AS_INT 1   // Number of AS interfaces

// #define TUD_AUDIO_MIC_TWO_CH_DESCRIPTOR(_itfnum, _stridx, _nBytesPerSample, _nBitsUsedPerSample, _epin, _epsize) \
//   /* Standard Interface Association Descriptor (IAD) */\
//   TUD_AUDIO_DESC_IAD(/*_firstitf*/ _itfnum, /*_nitfs*/ 0x02, /*_stridx*/ 0x00),\
//   /* Standard AC Interface Descriptor(4.7.1) */\
//   TUD_AUDIO_DESC_STD_AC(/*_itfnum*/ _itfnum, /*_nEPs*/ 0x00, /*_stridx*/ _stridx),\
//   /* Class-Specific AC Interface Header Descriptor(4.7.2) */\
//   TUD_AUDIO_DESC_CS_AC(/*_bcdADC*/ 0x0200, /*_category*/ AUDIO_FUNC_MICROPHONE, /*_totallen*/ TUD_AUDIO_DESC_CLK_SRC_LEN+TUD_AUDIO_DESC_INPUT_TERM_LEN+TUD_AUDIO_DESC_OUTPUT_TERM_LEN+TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL_LEN, /*_ctrl*/ AUDIO_CS_AS_INTERFACE_CTRL_LATENCY_POS),\
//   /* Clock Source Descriptor(4.7.2.1) */\
//   TUD_AUDIO_DESC_CLK_SRC(/*_clkid*/ 0x04, /*_attr*/ AUDIO_CLOCK_SOURCE_ATT_INT_FIX_CLK, /*_ctrl*/ (AUDIO_CTRL_R << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS), /*_assocTerm*/ 0x01,  /*_stridx*/ 0x00),\
//   /* Input Terminal Descriptor(4.7.2.4) */\
//   TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ 0x01, /*_termtype*/ AUDIO_TERM_TYPE_IN_GENERIC_MIC, /*_assocTerm*/ 0x03, /*_clkid*/ 0x04, /*_nchannelslogical*/ 0x02, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ 0x00, /*_ctrl*/ AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS, /*_stridx*/ 0x00),\
//   /* Output Terminal Descriptor(4.7.2.5) */\
//   TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ 0x03, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x01, /*_srcid*/ 0x02, /*_clkid*/ 0x04, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00),\
//   /* Feature Unit Descriptor(4.7.2.8) */\
//   TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL(/*_unitid*/ 0x02, /*_srcid*/ 0x01, /*_ctrlch0master*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_ctrlch1*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_ctrlch2*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS,  /*_stridx*/ 0x00),\
//   /* Standard AS Interface Descriptor(4.9.1) */\
//   /* Interface 1, Alternate 0 - default alternate setting with 0 bandwidth */\
//   TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)((_itfnum)+1), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x00),\
//   /* Standard AS Interface Descriptor(4.9.1) */\
//   /* Interface 1, Alternate 1 - alternate interface for data streaming */\
//   TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)((_itfnum)+1), /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ 0x00),\
//   /* Class-Specific AS Interface Descriptor(4.9.2) */\
//   TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ 0x03, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ 0x02, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00),\
//   /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */\
//   TUD_AUDIO_DESC_TYPE_I_FORMAT(_nBytesPerSample, _nBitsUsedPerSample),\
//   /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */\
//   TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ _epin, /*_attr*/ (uint8_t) ((uint8_t)TUSB_XFER_ISOCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_ASYNCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ _epsize, /*_interval*/ 0x01),\
//   /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */\
//   TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, /*_lockdelay*/ 0x0000)

#define CFG_TUD_AUDIO             1
#define AUDIO_USB_MAX_CHANNELS    2
#define AUDIO_FREQ_MIN            8000
#define AUDIO_FREQ_MAX            48000
#define MAX_BITS_PER_SAMPLE       24


// //--------------------------------------------------------------------
// // Global discovery functions used by driver
// //--------------------------------------------------------------------

// // Look at /proc/asound/<device>/stream0 to verify the USB settings
// extern int getUSBDAudioInterfaceDescriptorLength();

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN        __attribute__ ((aligned(4)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE    64
#endif

//--------------------------------------------------------------------
// AUDIO CLASS DRIVER CONFIGURATION
//--------------------------------------------------------------------

// Have a look into audio_device.h for all configurations
#define CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE              44100

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN                 TUD_AUDIO_MIC_TWO_CH_DESC_LEN

#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT                 1
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ              64

#define CFG_TUD_AUDIO_ENABLE_EP_IN                    1
#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX    2         // This value is not required by the driver, it parses this information from the descriptor once the alternate interface is set by the host - we use it for the setup
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX            2         // This value is not required by the driver, it parses this information from the descriptor once the alternate interface is set by the host - we use it for the setup
#define CFG_TUD_AUDIO_EP_SZ_IN                        TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE, CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)

#define CFG_TUD_AUDIO_ENABLE_ENCODING                 0
#define CFG_TUD_AUDIO_EP_IN_FLOW_CONTROL              1

#if CFG_TUD_AUDIO_ENABLE_ENCODING

#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX             CFG_TUD_AUDIO_EP_SZ_IN
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ          CFG_TUD_AUDIO_EP_SZ_IN

#define CFG_TUD_AUDIO_ENABLE_TYPE_I_ENCODING          1
#define CFG_TUD_AUDIO_FUNC_1_CHANNEL_PER_FIFO_TX      2         // One I2S stream contains two channels, each stream is saved within one support FIFO - this value is currently fixed, the driver does not support a changing value
#define CFG_TUD_AUDIO_FUNC_1_N_TX_SUPP_SW_FIFO        (CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX / CFG_TUD_AUDIO_FUNC_1_CHANNEL_PER_FIFO_TX)
#define CFG_TUD_AUDIO_FUNC_1_TX_SUPP_SW_FIFO_SZ       (TUD_OPT_HIGH_SPEED ? 32 : 4) * (CFG_TUD_AUDIO_EP_SZ_IN / CFG_TUD_AUDIO_FUNC_1_N_TX_SUPP_SW_FIFO) // Example write FIFO every 1ms, so it should be 8 times larger for HS device

#else

#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX             CFG_TUD_AUDIO_EP_SZ_IN
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ          (TUD_OPT_HIGH_SPEED ? 32 : 4) * CFG_TUD_AUDIO_EP_SZ_IN // Example write FIFO every 1ms, so it should be 8 times larger for HS device

#endif

//--------------------------------------------------------------------
// AUDIO CLASS DESCRIPTOR
//--------------------------------------------------------------------
#define TUD_AUDIO_MIC_TWO_CH_DESC_LEN (TUD_AUDIO_DESC_IAD_LEN\
  + TUD_AUDIO_DESC_STD_AC_LEN\
  + TUD_AUDIO_DESC_CS_AC_LEN\
  + TUD_AUDIO_DESC_CLK_SRC_LEN\
  + TUD_AUDIO_DESC_INPUT_TERM_LEN\
  + TUD_AUDIO_DESC_OUTPUT_TERM_LEN\
  + TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL_LEN\
  + TUD_AUDIO_DESC_STD_AS_INT_LEN\
  + TUD_AUDIO_DESC_STD_AS_INT_LEN\
  + TUD_AUDIO_DESC_CS_AS_INT_LEN\
  + TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN\
  + TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN\
  + TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN)

#define TUD_AUDIO_MIC_TWO_CH_DESC_N_AS_INT 1   // Number of AS interfaces

#define TUD_AUDIO_MIC_TWO_CH_DESCRIPTOR(_itfnum, _stridx, _nBytesPerSample, _nBitsUsedPerSample, _epin, _epsize) \
  /* Standard Interface Association Descriptor (IAD) */\
  TUD_AUDIO_DESC_IAD(/*_firstitf*/ _itfnum, /*_nitfs*/ 0x02, /*_stridx*/ 0x00),\
  /* Standard AC Interface Descriptor(4.7.1) */\
  TUD_AUDIO_DESC_STD_AC(/*_itfnum*/ _itfnum, /*_nEPs*/ 0x00, /*_stridx*/ _stridx),\
  /* Class-Specific AC Interface Header Descriptor(4.7.2) */\
  TUD_AUDIO_DESC_CS_AC(/*_bcdADC*/ 0x0200, /*_category*/ AUDIO_FUNC_MICROPHONE, /*_totallen*/ TUD_AUDIO_DESC_CLK_SRC_LEN+TUD_AUDIO_DESC_INPUT_TERM_LEN+TUD_AUDIO_DESC_OUTPUT_TERM_LEN+TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL_LEN, /*_ctrl*/ AUDIO_CS_AS_INTERFACE_CTRL_LATENCY_POS),\
  /* Clock Source Descriptor(4.7.2.1) */\
  TUD_AUDIO_DESC_CLK_SRC(/*_clkid*/ 0x04, /*_attr*/ AUDIO_CLOCK_SOURCE_ATT_INT_FIX_CLK, /*_ctrl*/ (AUDIO_CTRL_R << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS), /*_assocTerm*/ 0x01,  /*_stridx*/ 0x00),\
  /* Input Terminal Descriptor(4.7.2.4) */\
  TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ 0x01, /*_termtype*/ AUDIO_TERM_TYPE_IN_GENERIC_MIC, /*_assocTerm*/ 0x03, /*_clkid*/ 0x04, /*_nchannelslogical*/ 0x02, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ 0x00, /*_ctrl*/ AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS, /*_stridx*/ 0x00),\
  /* Output Terminal Descriptor(4.7.2.5) */\
  TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ 0x03, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x01, /*_srcid*/ 0x02, /*_clkid*/ 0x04, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00),\
  /* Feature Unit Descriptor(4.7.2.8) */\
  TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL(/*_unitid*/ 0x02, /*_srcid*/ 0x01, /*_ctrlch0master*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_ctrlch1*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_ctrlch2*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS,  /*_stridx*/ 0x00),\
  /* Standard AS Interface Descriptor(4.9.1) */\
  /* Interface 1, Alternate 0 - default alternate setting with 0 bandwidth */\
  TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)((_itfnum)+1), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x00),\
  /* Standard AS Interface Descriptor(4.9.1) */\
  /* Interface 1, Alternate 1 - alternate interface for data streaming */\
  TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)((_itfnum)+1), /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ 0x00),\
  /* Class-Specific AS Interface Descriptor(4.9.2) */\
  TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ 0x03, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ 0x02, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00),\
  /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */\
  TUD_AUDIO_DESC_TYPE_I_FORMAT(_nBytesPerSample, _nBitsUsedPerSample),\
  /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */\
  TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ _epin, /*_attr*/ (uint8_t) ((uint8_t)TUSB_XFER_ISOCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_ASYNCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ _epsize, /*_interval*/ 0x01),\
  /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */\
  TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, /*_lockdelay*/ 0x0000)


