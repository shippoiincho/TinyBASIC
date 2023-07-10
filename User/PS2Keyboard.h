/*
  PS2Keyboard.h - PS2Keyboard library
  Copyright (c) 2007 Free Software Foundation.  All right reserved.
  Written by Christian Weichel <info@32leaves.net>

  ** Mostly rewritten Paul Stoffregen <paul@pjrc.com>, June 2010
  ** Modified for use with Arduino 13 by L. Abraham Smith, <n3bah@microcompdesign.com> * 
  ** Modified for easy interrup pin assignement on method begin(datapin,irq_pin). Cuningan <cuninganreset@gmail.com> **

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef PS2Keyboard_h
#define PS2Keyboard_h

/*
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h" // for attachInterrupt, FALLING
#else
#include "WProgram.h"
#endif

#include "utility/int_pins.h"
*/
#define PROGMEM

// Every call to read() returns a single byte for each
// keystroke.  These configure what byte will be returned
// for each "special" key.  To ignore a key, use zero.
#define PS2_TAB                      9
#define PS2_ENTER                    13
#define PS2_BACKSPACE                127
#define PS2_ESC                      27
#define PS2_INSERT                   0
#define PS2_DELETE                   127
#define PS2_HOME                     0
#define PS2_END                      0
#define PS2_PAGEUP                   25
#define PS2_PAGEDOWN                 26
#define PS2_UPARROW                  11
#define PS2_LEFTARROW                8
#define PS2_DOWNARROW                10
#define PS2_RIGHTARROW               21
#define PS2_F1                       0
#define PS2_F2                       0
#define PS2_F3                       0
#define PS2_F4                       0
#define PS2_F5                       0
#define PS2_F6                       0
#define PS2_F7                       0
#define PS2_F8                       0
#define PS2_F9                       0
#define PS2_F10                      0
#define PS2_F11                      0
#define PS2_F12                      0
#define PS2_SCROLL                   0

#define PS2_INVERTED_EXCLAMATION     161
#define PS2_CENT_SIGN                162
#define PS2_POUND_SIGN               163
#define PS2_CURRENCY_SIGN            164
#define PS2_YEN_SIGN                 165
#define PS2_BROKEN_BAR               166
#define PS2_SECTION_SIGN             167
#define PS2_DIAERESIS                168
#define PS2_COPYRIGHT_SIGN           169
#define PS2_FEMININE_ORDINAL         170
#define PS2_LEFT_DOUBLE_ANGLE_QUOTE  171
#define PS2_NOT_SIGN                 172
#define PS2_HYPHEN                   173
#define PS2_REGISTERED_SIGN          174
#define PS2_MACRON                   175
#define PS2_DEGREE_SIGN              176
#define PS2_PLUS_MINUS_SIGN          177
#define PS2_SUPERSCRIPT_TWO          178
#define PS2_SUPERSCRIPT_THREE        179
#define PS2_ACUTE_ACCENT             180
#define PS2_MICRO_SIGN               181
#define PS2_PILCROW_SIGN             182
#define PS2_MIDDLE_DOT               183
#define PS2_CEDILLA                  184
#define PS2_SUPERSCRIPT_ONE          185
#define PS2_MASCULINE_ORDINAL        186
#define PS2_RIGHT_DOUBLE_ANGLE_QUOTE 187
#define PS2_FRACTION_ONE_QUARTER     188
#define PS2_FRACTION_ONE_HALF        189
#define PS2_FRACTION_THREE_QUARTERS  190
#define PS2_INVERTED_QUESTION MARK   191
#define PS2_A_GRAVE                  192
#define PS2_A_ACUTE                  193
#define PS2_A_CIRCUMFLEX             194
#define PS2_A_TILDE                  195
#define PS2_A_DIAERESIS              196
#define PS2_A_RING_ABOVE             197
#define PS2_AE                       198
#define PS2_C_CEDILLA                199
#define PS2_E_GRAVE                  200
#define PS2_E_ACUTE                  201
#define PS2_E_CIRCUMFLEX             202
#define PS2_E_DIAERESIS              203
#define PS2_I_GRAVE                  204
#define PS2_I_ACUTE                  205
#define PS2_I_CIRCUMFLEX             206
#define PS2_I_DIAERESIS              207
#define PS2_ETH                      208
#define PS2_N_TILDE                  209
#define PS2_O_GRAVE                  210
#define PS2_O_ACUTE                  211
#define PS2_O_CIRCUMFLEX             212
#define PS2_O_TILDE                  213
#define PS2_O_DIAERESIS              214
#define PS2_MULTIPLICATION           215
#define PS2_O_STROKE                 216
#define PS2_U_GRAVE                  217
#define PS2_U_ACUTE                  218
#define PS2_U_CIRCUMFLEX             219
#define PS2_U_DIAERESIS              220
#define PS2_Y_ACUTE                  221
#define PS2_THORN                    222
#define PS2_SHARP_S                  223
#define PS2_a_GRAVE                  224
#define PS2_a_ACUTE                  225
#define PS2_a_CIRCUMFLEX             226
#define PS2_a_TILDE                  227
#define PS2_a_DIAERESIS              228
#define PS2_a_RING_ABOVE             229
#define PS2_ae                       230
#define PS2_c_CEDILLA                231
#define PS2_e_GRAVE                  232
#define PS2_e_ACUTE                  233
#define PS2_e_CIRCUMFLEX             234
#define PS2_e_DIAERESIS              235
#define PS2_i_GRAVE                  236
#define PS2_i_ACUTE                  237
#define PS2_i_CIRCUMFLEX             238
#define PS2_i_DIAERESIS              239
#define PS2_eth                      240
#define PS2_n_TILDE                  241
#define PS2_o_GRAVE                  242
#define PS2_o_ACUTE                  243
#define PS2_o_CIRCUMFLEX             244
#define PS2_o_TILDE                  245
#define PS2_o_DIAERESIS              246
#define PS2_DIVISION                 247
#define PS2_o_STROKE                 248
#define PS2_u_GRAVE                  249
#define PS2_u_ACUTE                  250
#define PS2_u_CIRCUMFLEX             251
#define PS2_u_DIAERESIS              252
#define PS2_y_ACUTE                  253
#define PS2_thorn                    254
#define PS2_y_DIAERESIS              255


#define PS2_KEYMAP_SIZE 136

//#define USE_US_KEYBOARD
//#define USE_GR_KEYBOARD
//#define USE_FR_KEYBOARD
#define USE_JP_KEYBOARD


typedef struct {
  uint8_t noshift[PS2_KEYMAP_SIZE];
  uint8_t shift[PS2_KEYMAP_SIZE];
  uint8_t uses_altgr;
  uint8_t altgr[PS2_KEYMAP_SIZE];
} PS2Keymap_t;


#ifdef USE_US_KEYBOARD
extern const PROGMEM PS2Keymap_t PS2Keymap_US;
#endif
#ifdef USE_GR_KEYBOARD
extern const PROGMEM PS2Keymap_t PS2Keymap_German;
#endif
#ifdef USE_FR_KEYBOARD
extern const PROGMEM PS2Keymap_t PS2Keymap_French;
#endif
#ifdef USE_JP_KEYBOARD
extern const PROGMEM PS2Keymap_t PS2Keymap_Japanese;
#endif

void kbd_init( void);
bool kbd_available();
int kbd_read();

#endif
