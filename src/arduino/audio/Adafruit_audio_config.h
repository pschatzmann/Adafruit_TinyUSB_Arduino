#pragma once

// Look at /proc/asound/<device>/stream0 to verify the USB settings

#define CFG_TUD_AUDIO 1  // number of audio streaming interfaces
#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_USB_CHANNELS 2
#define AUDIO_USB_CHANNELS_MIC AUDIO_USB_CHANNELS
#define AUDIO_USB_CHANNELS_SPK AUDIO_USB_CHANNELS
#define AUDIO_USB_TRANSFER_BYTES 2
#define AUDIO_USB_RESOLUTION_BITS 16
#define AUDIO_USB_FUNC AUDIO_FUNC_HOME_THEATER
#define AUDIO_USB_CHANNEL_ASSIGN_MIC  (AUDIO_CHANNEL_CONFIG_FRONT_LEFT | AUDIO_CHANNEL_CONFIG_FRONT_RIGHT)
#define AUDIO_USB_CHANNEL_ASSIGN_SPK  (AUDIO_CHANNEL_CONFIG_FRONT_LEFT | AUDIO_CHANNEL_CONFIG_FRONT_RIGHT)
#define AUDIO_FREQ_MIN 8000
#define AUDIO_FREQ_MAX AUDIO_SAMPLE_RATE
#define CFG_TUD_AUDIO_ENABLE_EP_IN 1
#define CFG_TUD_AUDIO_ENABLE_EP_OUT 1
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP 1

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

// Unit numbers are arbitrary selected
#define UAC2_ENTITY_CLOCK 0x07
// Speaker path
#define UAC2_ENTITY_SPK_INPUT_TERMINAL 0x08
#define UAC2_ENTITY_SPK_FEATURE_UNIT 0x09
#define UAC2_ENTITY_SPK_OUTPUT_TERMINAL 0x10
// Microphone path
#define UAC2_ENTITY_MIC_INPUT_TERMINAL 0x11
#define UAC2_ENTITY_MIC_OUTPUT_TERMINAL 0x13

#define EPNUM_AUDIO_IN 0x22
#define EPNUM_AUDIO_OUT 0x21

#define ITF_NUM_AUDIO_CONTROL 0
#define ITF_NUM_AUDIO_STREAMING_SPK 1
#define ITF_NUM_AUDIO_STREAMING_MIC 2
#define ITF_NUM_TOTAL 3


#define TUD_AUDIO_DESC_CTRL_LEN                                     \
  (TUD_AUDIO_DESC_CLK_SRC_LEN + TUD_AUDIO_DESC_INPUT_TERM_LEN +     \
   TUD_AUDIO_DESC_OUTPUT_TERM_LEN + TUD_AUDIO_DESC_INPUT_TERM_LEN + \
   TUD_AUDIO_DESC_OUTPUT_TERM_LEN)

#define TUD_AUDIO_DESC_LEN              \
  (                                     \
      TUD_AUDIO_DESC_IAD_LEN            \
    + TUD_AUDIO_DESC_STD_AC_LEN         \
    + TUD_AUDIO_DESC_CS_AC_LEN          \
    + TUD_AUDIO_DESC_CTRL_LEN           \
    /* Interface 1, Alternate 0 */      \
    + TUD_AUDIO_DESC_STD_AS_INT_LEN     \
    /* Interface 1, Alternate 1 */      \
    + TUD_AUDIO_DESC_STD_AS_INT_LEN     \
    + TUD_AUDIO_DESC_CS_AS_INT_LEN      \
    + TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN  \
    + TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN  \
    + TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN   \
    /* Interface 2, Alternate 0 */      \
    + TUD_AUDIO_DESC_STD_AS_INT_LEN     \
    /* Interface 2, Alternate 1 */      \
    + TUD_AUDIO_DESC_STD_AS_INT_LEN     \
    + TUD_AUDIO_DESC_CS_AS_INT_LEN      \
    + TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN  \
    + TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN  \
    + TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN   \
  )

// #define TUD_AUDIO_DESCRIPTOR(_stridx, _strid_rx, _strid_tx, _epout, _epin) \
//     /* Standard Interface Association Descriptor (IAD) */\
//     TUD_AUDIO_DESC_IAD(/*_firstitfs*/ ITF_NUM_AUDIO_CONTROL, /*_nitfs*/ ITF_NUM_TOTAL, /*_stridx*/ 0x00),\
//     /* Standard AC Interface Descriptor(4.7.1) */\
//     TUD_AUDIO_DESC_STD_AC(/*_itfnum*/ ITF_NUM_AUDIO_CONTROL, /*_nEPs*/ 0x00, /*_stridx*/ _stridx),\
//     /* Class-Specific AC Interface Header Descriptor(4.7.2) */\
//     TUD_AUDIO_DESC_CS_AC(/*_bcdADC*/ 0x0200, /*_category*/ AUDIO_USB_FUNC, /*_totallen*/ TUD_AUDIO_DESC_CTRL_LEN, /*_ctrl*/ AUDIO_CTRL_NONE << AUDIO_CS_AS_INTERFACE_CTRL_LATENCY_POS),\
//     /* Clock Source Descriptor(4.7.2.1) */\
//     TUD_AUDIO_DESC_CLK_SRC(/*_clkid*/ UAC2_ENTITY_CLOCK, /*_attr*/ AUDIO_CLOCK_SOURCE_ATT_EXT_CLK, /*_ctrl*/ (AUDIO_CTRL_NONE << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS)|(AUDIO_CTRL_NONE << AUDIO_CLOCK_SOURCE_CTRL_CLK_VAL_POS), /*_assocTerm*/ 0x00,  /*_stridx*/ 0x00),    \
//     /* Input Terminal Descriptor(4.7.2.4) */\
//     TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x00, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_nchannelslogical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_SPK, /*_channelcfg*/ CFG_TUD_AUDIO_FUNC_1_ASSIGN_SPK, /*_idxchannelnames*/ 0x00, /*_ctrl*/ AUDIO_CTRL_NONE << AUDIO_IN_TERM_CTRL_CONNECTOR_POS, /*_stridx*/ 0x00),\
//     /* Output Terminal Descriptor(4.7.2.5) */\
//     TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ UAC2_ENTITY_SPK_OUTPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_OUT_GENERIC_SPEAKER, /*_assocTerm*/ 0x00, /*_srcid*/ UAC2_ENTITY_SPK_FEATURE_UNIT, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00),\
//     /* Input Terminal Descriptor(4.7.2.4) */\
//     TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_IN_GENERIC_MIC, /*_assocTerm*/ 0x00, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_nchannelslogical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_MIC, /*_channelcfg*/ CFG_TUD_AUDIO_FUNC_1_ASSIGN_MIC, /*_idxchannelnames*/ 0x00, /*_ctrl*/ AUDIO_CTRL_NONE << AUDIO_IN_TERM_CTRL_CONNECTOR_POS, /*_stridx*/ 0x00),\
//     /* Output Terminal Descriptor(4.7.2.5) */\
//     TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x00, /*_srcid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_ctrl*/ 0x0000, /*_stridx*/ 0x00),\
//     /* Standard AS Interface Descriptor(4.9.1) */\
//     /* Interface 1, Alternate 0 - default alternate setting with 0 bandwidth */\
//     TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(ITF_NUM_AUDIO_STREAMING_SPK), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ _strid_rx),\
//     /* Standard AS Interface Descriptor(4.9.1) */\
//     /* Interface 1, Alternate 1 - alternate interface for data streaming */\
//     TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(ITF_NUM_AUDIO_STREAMING_SPK), /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ _strid_rx),\
//     /* Class-Specific AS Interface Descriptor(4.9.2) */\
//     TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_SPK, /*_channelcfg*/ CFG_TUD_AUDIO_FUNC_1_ASSIGN_SPK, /*_stridx*/ 0x00),\
//     /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */\
//     TUD_AUDIO_DESC_TYPE_I_FORMAT(CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_SPK, CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_SPK),\
//     /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */\
//     TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ _epout, /*_attr*/ (TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ADAPTIVE | TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_OUT, /*_interval*/ 0x01),\
//     /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */\
//     TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_MILLISEC, /*_lockdelay*/ 0x0001),\
//     /* Standard AS Interface Descriptor(4.9.1) */\
//     /* Interface 2, Alternate 0 - default alternate setting with 0 bandwidth */\
//     TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(ITF_NUM_AUDIO_STREAMING_MIC), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ _strid_tx),\
//     /* Standard AS Interface Descriptor(4.9.1) */\
//     /* Interface 2, Alternate 1 - alternate interface for data streaming */\
//     TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)(ITF_NUM_AUDIO_STREAMING_MIC), /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ _strid_tx),\
//     /* Class-Specific AS Interface Descriptor(4.9.2) */\
//     TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_MIC, /*_channelcfg*/ CFG_TUD_AUDIO_FUNC_1_ASSIGN_MIC, /*_stridx*/ 0x00),\
//     /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */\
//     TUD_AUDIO_DESC_TYPE_I_FORMAT(CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_MIC, CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_MIC),\
//     /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */\
//     TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ _epin, /*_attr*/ (TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ASYNCHRONOUS | TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN, /*_interval*/ 0x01),\
//     /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */\
//     TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, /*_lockdelay*/ 0x0000)

//--------------------------------------------------------------------
// AUDIO CLASS DRIVER CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN TUD_AUDIO_DESC_LEN

// How many formats are used, need to adjust USB descriptor if changed
//#define CFG_TUD_AUDIO_FUNC_1_N_FORMATS 1

// Audio format type I specifications
#define CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE AUDIO_SAMPLE_RATE

#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_MIC AUDIO_USB_CHANNELS_MIC
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_SPK AUDIO_USB_CHANNELS_SPK
#define CFG_TUD_AUDIO_FUNC_1_ASSIGN_MIC AUDIO_USB_CHANNEL_ASSIGN_MIC
#define CFG_TUD_AUDIO_FUNC_1_ASSIGN_SPK AUDIO_USB_CHANNEL_ASSIGN_SPK

#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_MIC  AUDIO_USB_TRANSFER_BYTES
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_MIC AUDIO_USB_RESOLUTION_BITS
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_SPK  AUDIO_USB_TRANSFER_BYTES
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_SPK AUDIO_USB_RESOLUTION_BITS



#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN                            \
  TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                 \
                    CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_MIC, \
                    CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_MIC)
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_OUT                           \
  TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                 \
                    CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_SPK, \
                    CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_SPK)

#if CFG_TUD_AUDIO_FUNC_1_N_FORMATS <= 1
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ \
  CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN * 2
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN

#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ \
  CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_OUT * 2
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX \
  CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_OUT
#else
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_EP_SZ_IN                            \
  TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                 \
                    CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_MIC, \
                    CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_MIC)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ       \
  TU_MAX(CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN,   \
         CFG_TUD_AUDIO_FUNC_1_FORMAT_2_EP_SZ_IN) * \
      2
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX     \
  TU_MAX(                                     \
      CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN, \
      CFG_TUD_AUDIO_FUNC_1_FORMAT_2_EP_SZ_IN)  // Maximum EP IN size for all AS
                                               // alternate settings used

#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_EP_SZ_OUT                           \
  TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                 \
                    CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_SPK, \
                    CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_SPK)
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ      \
  TU_MAX(CFG_TUD_AUDIO_UNC_1_FORMAT_1_EP_SZ_OUT,   \
         CFG_TUD_AUDIO_UNC_1_FORMAT_2_EP_SZ_OUT) * \
      2
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX    \
  TU_MAX(                                     \
      CFG_TUD_AUDIO_UNC_1_FORMAT_1_EP_SZ_OUT, \
      CFG_TUD_AUDIO_UNC_1_FORMAT_2_EP_SZ_OUT)  // Maximum EP OUT size for all AS
                                               // alternate settings used
#endif

// Number of Standard AS Interface Descriptors (4.9.1) defined per audio
// function - this is required to be able to remember the current alternate
// settings of these interfaces - We restrict us here to have a constant number
// for all audio functions (which means this has to be the maximum number of AS
// interfaces an audio function has and a second audio function with less AS
// interfaces just wastes a few bytes)
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT 2

// Size of control request buffer
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ 64


#define CFG_FUNC_1_N_CHANNELS_TX AUDIO_USB_CHANNELS


