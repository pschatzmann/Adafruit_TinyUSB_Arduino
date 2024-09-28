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

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------
#define AUDIO_USB_MAX_CHANNELS 2
#define AUDIO_USB_FUNC AUDIO_FUNC_HOME_THEATER
#define AUDIO_USB_CHANNEL_ASSIGN  (AUDIO_CHANNEL_CONFIG_FRONT_LEFT | AUDIO_CHANNEL_CONFIG_FRONT_RIGHT)
#define AUDIO_FREQ_MIN 8000
#define AUDIO_FREQ_MAX 48000
#define MAX_BITS_PER_SAMPLE 32

// Unit numbers are arbitrary selected
#define UAC2_ENTITY_CLOCK 0x07
// Speaker path
#define UAC2_ENTITY_SPK_INPUT_TERMINAL 0x08
#define UAC2_ENTITY_SPK_FEATURE_UNIT 0x09
#define UAC2_ENTITY_SPK_OUTPUT_TERMINAL 0x10
// Microphone path
#define UAC2_ENTITY_MIC_INPUT_TERMINAL 0x11
#define UAC2_ENTITY_MIC_OUTPUT_TERMINAL 0x13

//--------------------------------------------------------------------
// AUDIO CLASS DRIVER CONFIGURATION
//--------------------------------------------------------------------
#define CFG_TUD_AUDIO 1  // number of audio streaming interfaces

#define CFG_TUD_AUDIO_ENABLE_ENCODING 0
#define CFG_TUD_AUDIO_ENABLE_EP_IN true
#define CFG_TUD_AUDIO_ENABLE_EP_OUT true
//#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP true

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN getUSBDAudioInterfaceDescriptorLength() 
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_OUT

// Number of Standard AS Interface Descriptors (4.9.1) defined per audio
// function - this is required to be able to remember the current alternate
// settings of these interfaces - We restrict us here to have a constant number
// for all audio functions (which means this has to be the maximum number of AS
// interfaces an audio function has and a second audio function with less AS
// interfaces just wastes a few bytes)
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT 2

// Size of control request buffer
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ 64


//--------------------------------------------------------------------
// Global discovery functions used by driver
//--------------------------------------------------------------------

// Look at /proc/asound/<device>/stream0 to verify the USB settings
extern int getUSBDAudioInterfaceDescriptorLength();

