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

// for a start we support audio only on the Rasperry Pico
#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_SAMD)

#define CFG_TUD_AUDIO             1

#ifndef CDC_DEFAULT_ACTIVE
#  define CDC_DEFAULT_ACTIVE        true
#endif
#ifndef AUDIO_USB_MAX_CHANNELS
#  define AUDIO_USB_MAX_CHANNELS    2
#endif
#ifndef AUDIO_FREQ_MIN
#  define AUDIO_FREQ_MIN            8000
#endif
#ifndef AUDIO_FREQ_MAX
#  define AUDIO_FREQ_MAX            48000
#endif
#ifndef MAX_BITS_PER_SAMPLE
#  define MAX_BITS_PER_SAMPLE       32
#endif

//--------------------------------------------------------------------
// Debugging Logging and Testing
//--------------------------------------------------------------------
#define AUDIO_LOG(...)              {char msg[160]; snprintf(msg, 160, __VA_ARGS__); LOG_AUDIO_OUTPUT.println(msg); LOG_AUDIO_OUTPUT.flush();}
#define AUDIO_NO_LOG(...)

#define LOG_AUDIO_OUTPUT            Serial
#define AUDIO_DEBUG                 false
#define LOG_AUDIO_ERROR             AUDIO_LOG
#define LOG_AUDIO_DEBUG             AUDIO_LOG      


//--------------------------------------------------------------------
// Unit numbers are arbitrary selected
//--------------------------------------------------------------------

// Speaker path
#define UAC2_ENTITY_SPK_INPUT_TERMINAL 0x15
#define UAC2_ENTITY_SPK_FEATURE_UNIT 0x16
#define UAC2_ENTITY_SPK_OUTPUT_TERMINAL 0x17
#define UAC2_ENTITY_SPK_CLOCK 0x18

// Microphone path
#define UAC2_ENTITY_MIC_INPUT_TERMINAL 0x11
#define UAC2_ENTITY_MIC_FEATURE_UNIT 0x12
#define UAC2_ENTITY_MIC_OUTPUT_TERMINAL 0x13
#define UAC2_ENTITY_MIC_CLOCK 0x14

//--------------------------------------------------------------------
// AUDIO CLASS DRIVER CONFIGURATION
//--------------------------------------------------------------------

// Have a look into audio_device.h for all configurations
#define CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE              (AUDIO_FREQ_MAX*2)

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN                 getUSBDAudioInterfaceDescriptorLength() 

#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT                 1
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ              64

// Microphone
#define CFG_TUD_AUDIO_ENABLE_EP_IN                    1
#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX    2         // This value is not required by the driver, it parses this information from the descriptor once the alternate interface is set by the host - we use it for the setup
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX            2         // This value is not required by the driver, it parses this information from the descriptor once the alternate interface is set by the host - we use it for the setup
#define CFG_TUD_AUDIO_EP_SZ_IN                        TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE, CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)
#define CFG_TUD_AUDIO_EP_IN_FLOW_CONTROL              1
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX             CFG_TUD_AUDIO_EP_SZ_IN
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ          (TUD_OPT_HIGH_SPEED ? 32 : 4) * CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX // Example write FIFO every 1ms, so it should be 8 times larger for HS device

// Speaker
#define CFG_TUD_AUDIO_ENABLE_EP_OUT                   1
#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX    2         // This value is not required by the driver, it parses this information from the descriptor once the alternate interface is set by the host - we use it for the setup
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX            2         // This value is not required by the driver, it parses this information from the descriptor once the alternate interface is set by the host - we use it for the setup
#define CFG_TUD_AUDIO_EP_SZ_OUT                       TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE, CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX)
#define CFG_TUD_AUDIO_EP_OUT_FLOW_CONTROL             1
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX            CFG_TUD_AUDIO_EP_SZ_OUT
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ         (TUD_OPT_HIGH_SPEED ? 32 : 4) * CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX // Example read FIFO every 1ms, so it should be 8 times larger for HS device

// Enable if Full-Speed on OSX, also set feedback EP size to 3
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_FORMAT_CORRECTION              0
// Enable feedback EP
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP                             1


//--------------------------------------------------------------------
// Definitions
//--------------------------------------------------------------------
// hack to make CFG_TUD_AUDIO_FUNC_1_DESC_LEN dynamic
extern int getUSBDAudioInterfaceDescriptorLength();

#endif // ARDUINO_ARCH_*