/*
  PS2Keyboard.cpp - PS2Keyboard library
  Copyright (c) 2007 Free Software Foundation.  All right reserved.
  Written by Christian Weichel <info@32leaves.net>

  ** Mostly rewritten Paul Stoffregen <paul@pjrc.com> 2010, 2011
  ** Modified for use beginning with Arduino 13 by L. Abraham Smith, <n3bah@microcompdesign.com> * 
  ** Modified for easy interrup pin assignement on method begin(datapin,irq_pin). Cuningan <cuninganreset@gmail.com> **

  for more information you can read the original wiki in arduino.cc
  at http://www.arduino.cc/playground/Main/PS2Keyboard
  or http://www.pjrc.com/teensy/td_libs_PS2Keyboard.html

  Version 2.4 (March 2013)
  - Support Teensy 3.0, Arduino Due, Arduino Leonardo & other boards
  - French keyboard layout, David Chochoi, tchoyyfr at yahoo dot fr

  Version 2.3 (October 2011)
  - Minor bugs fixed

  Version 2.2 (August 2011)
  - Support non-US keyboards - thanks to Rainer Bruch for a German keyboard :)

  Version 2.1 (May 2011)
  - timeout to recover from misaligned input
  - compatibility with Arduino "new-extension" branch
  - TODO: send function, proposed by Scott Penrose, scooterda at me dot com

  Version 2.0 (June 2010)
  - Buffering added, many scan codes can be captured without data loss
    if your sketch is busy doing other work
  - Shift keys supported, completely rewritten scan code to ascii
  - Slow linear search replaced with fast indexed table lookups
  - Support for Teensy, Arduino Mega, and Sanguino added

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

#include <stdio.h>
#include <stdbool.h>
#include "PS2Keyboard.h"
#include "debug.h"

//#define BUFFER_SIZE 45
#define BUFFER_SIZE 16
static volatile uint8_t buffer[BUFFER_SIZE];
static volatile uint8_t head, tail;
//static uint8_t DataPin;
static uint8_t CharBuffer=0;
static uint8_t UTF8next=0;
static const PS2Keymap_t *keymap=NULL;
static bool CapsLock=false;

// The ISR for the external interrupt

void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void EXTI7_0_IRQHandler(void)
{
    static uint8_t bitcount=0;
    static uint8_t incoming=0;
    static uint32_t prev_ms=0;
    uint32_t now_ms;
    uint8_t n, val;

    if (EXTI_GetITStatus(EXTI_Line1) != RESET) {

        //  val = digitalRead(DataPin);
        val=GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2);
        //  now_ms = millis();
        now_ms=(SysTick->CNT)/6;       // 48MHz * 6 = 1us
        if (now_ms - prev_ms > 250) {
            bitcount = 0;
            incoming = 0;
        }
        prev_ms = now_ms;
        n = bitcount - 1;
        if (n <= 7)
            incoming |= (val << n);
        bitcount++;
        if (bitcount == 11) {
            uint8_t i = head + 1;
            if (i >= BUFFER_SIZE)
                i = 0;
            if (i != tail) {
                buffer[i] = incoming;
                head = i;
            }
            bitcount = 0;
            incoming = 0;
        }

        EXTI_ClearITPendingBit(EXTI_Line1); /* Clear Flag */
    }
}

static inline uint8_t get_scan_code(void)
{
  uint8_t c, i;

  i = tail;
  if (i == head) 
    return 0;
  i++;
  if (i >= BUFFER_SIZE) 
    i = 0;
  c = buffer[i];
  tail = i;
  return c;
}

// http://www.quadibloc.com/comp/scan.htm
// http://www.computer-engineering.org/ps2keyboard/scancodes2.html

// These arrays provide a simple key map, to turn scan codes into ISO8859
// output.  If a non-US keyboard is used, these may need to be modified
// for the desired output.
//

#ifdef USE_US_KEYBOARD
const PROGMEM PS2Keymap_t PS2Keymap_US = {
  // without shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '`', 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'q', '1', 0,
  0, 0, 'z', 's', 'a', 'w', '2', 0,
  0, 'c', 'x', 'd', 'e', '4', '3', 0,
  0, ' ', 'v', 'f', 't', 'r', '5', 0,
  0, 'n', 'b', 'h', 'g', 'y', '6', 0,
  0, 0, 'm', 'j', 'u', '7', '8', 0,
  0, ',', 'k', 'i', 'o', '0', '9', 0,
  0, '.', '/', 'l', ';', 'p', '-', 0,
  0, 0, '\'', 0, '[', '=', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, ']', 0, '\\', 0, 0,
  0, 0, 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  // with shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '~', 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'Q', '!', 0,
  0, 0, 'Z', 'S', 'A', 'W', '@', 0,
  0, 'C', 'X', 'D', 'E', '$', '#', 0,
  0, ' ', 'V', 'F', 'T', 'R', '%', 0,
  0, 'N', 'B', 'H', 'G', 'Y', '^', 0,
  0, 0, 'M', 'J', 'U', '&', '*', 0,
  0, '<', 'K', 'I', 'O', ')', '(', 0,
  0, '>', '?', 'L', ':', 'P', '_', 0,
  0, 0, '"', 0, '{', '+', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '}', 0, '|', 0, 0,
  0, 0, 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  0
};
#endif


#ifdef USE_GR_KEYBOARD
const PROGMEM PS2Keymap_t PS2Keymap_German = {
  // without shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '^', 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'q', '1', 0,
  0, 0, 'y', 's', 'a', 'w', '2', 0,
  0, 'c', 'x', 'd', 'e', '4', '3', 0,
  0, ' ', 'v', 'f', 't', 'r', '5', 0,
  0, 'n', 'b', 'h', 'g', 'z', '6', 0,
  0, 0, 'm', 'j', 'u', '7', '8', 0,
  0, ',', 'k', 'i', 'o', '0', '9', 0,
  0, '.', '-', 'l', PS2_o_DIAERESIS, 'p', PS2_SHARP_S, 0,
  0, 0, PS2_a_DIAERESIS, 0, PS2_u_DIAERESIS, '\'', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '+', 0, '#', 0, 0,
  0, '<', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  // with shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, PS2_DEGREE_SIGN, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'Q', '!', 0,
  0, 0, 'Y', 'S', 'A', 'W', '"', 0,
  0, 'C', 'X', 'D', 'E', '$', PS2_SECTION_SIGN, 0,
  0, ' ', 'V', 'F', 'T', 'R', '%', 0,
  0, 'N', 'B', 'H', 'G', 'Z', '&', 0,
  0, 0, 'M', 'J', 'U', '/', '(', 0,
  0, ';', 'K', 'I', 'O', '=', ')', 0,
  0, ':', '_', 'L', PS2_O_DIAERESIS, 'P', '?', 0,
  0, 0, PS2_A_DIAERESIS, 0, PS2_U_DIAERESIS, '`', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '*', 0, '\'', 0, 0,
  0, '>', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  1,
  // with altgr
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, 0, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, '@', 0, 0,
  0, 0, 0, 0, 0, 0, PS2_SUPERSCRIPT_TWO, 0,
  0, 0, 0, 0, PS2_CURRENCY_SIGN, 0, PS2_SUPERSCRIPT_THREE, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, PS2_MICRO_SIGN, 0, 0, '{', '[', 0,
  0, 0, 0, 0, 0, '}', ']', 0,
  0, 0, 0, 0, 0, 0, '\\', 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '~', 0, '#', 0, 0,
  0, '|', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 }
};
#endif


#ifdef USE_FR_KEYBOARD
const PROGMEM PS2Keymap_t PS2Keymap_French = {
  // without shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, PS2_SUPERSCRIPT_TWO, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'a', '&', 0,
  0, 0, 'w', 's', 'q', 'z', PS2_e_ACUTE, 0,
  0, 'c', 'x', 'd', 'e', '\'', '"', 0,
  0, ' ', 'v', 'f', 't', 'r', '(', 0,
  0, 'n', 'b', 'h', 'g', 'y', '-', 0,
  0, 0, ',', 'j', 'u', PS2_e_GRAVE, '_', 0,
  0, ';', 'k', 'i', 'o', PS2_a_GRAVE, PS2_c_CEDILLA, 0,
  0, ':', '!', 'l', 'm', 'p', ')', 0,
  0, 0, PS2_u_GRAVE, 0, '^', '=', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '$', 0, '*', 0, 0,
  0, '<', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  // with shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, 0, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'A', '1', 0,
  0, 0, 'W', 'S', 'Q', 'Z', '2', 0,
  0, 'C', 'X', 'D', 'E', '4', '3', 0,
  0, ' ', 'V', 'F', 'T', 'R', '5', 0,
  0, 'N', 'B', 'H', 'G', 'Y', '6', 0,
  0, 0, '?', 'J', 'U', '7', '8', 0,
  0, '.', 'K', 'I', 'O', '0', '9', 0,
  0, '/', PS2_SECTION_SIGN, 'L', 'M', 'P', PS2_DEGREE_SIGN, 0,
  0, 0, '%', 0, PS2_DIAERESIS, '+', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, PS2_POUND_SIGN, 0, PS2_MICRO_SIGN, 0, 0,
  0, '>', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  1,
  // with altgr
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, 0, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, '@', 0, 0,
  0, 0, 0, 0, 0, 0, '~', 0,
  0, 0, 0, 0, 0 /*PS2_EURO_SIGN*/, '{', '#', 0,
  0, 0, 0, 0, 0, 0, '[', 0,
  0, 0, 0, 0, 0, 0, '|', 0,
  0, 0, 0, 0, 0, '`', '\\', 0,
  0, 0, 0, 0, 0, '@', '^', 0,
  0, 0, 0, 0, 0, 0, ']', 0,
  0, 0, 0, 0, 0, 0, '}', 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '?', 0, '#', 0, 0,
  0, '|', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 }
};
#endif


#ifdef USE_JP_KEYBOARD
// http://blog.livedoor.jp/hardyboy/archives/5393891.html
// https://gist.github.com/houmei/2356812
const PROGMEM PS2Keymap_t PS2Keymap_Japanese = {
  // without shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, 0, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'q', '1', 0,
  0, 0, 'z', 's', 'a', 'w', '2', 0,
  0, 'c', 'x', 'd', 'e', '4', '3', 0,
  0, ' ', 'v', 'f', 't', 'r', '5', 0,
  0, 'n', 'b', 'h', 'g', 'y', '6', 0,
  0, 0, 'm', 'j', 'u', '7', '8', 0,
  0, ',', 'k', 'i', 'o', '0', '9', 0,
  0, '.', '/', 'l', ';', 'p', '-', 0,
  0, '\\', ':', 0, '@', '^', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '[', 0, ']', 0, 0,
  0, 0, 0, 0, 0 /*Henkan*/, 0, PS2_BACKSPACE, 0 /*MuHenkan*/,
  0, '1', '\\', '4', '7', 'I', 'J', 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  // with shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, 0, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'Q', '!', 0,
  0, 0, 'Z', 'S', 'A', 'W', '"', 0,
  0, 'C', 'X', 'D', 'E', '$', '#', 0,
  0, ' ', 'V', 'F', 'T', 'R', '%', 0,
  0, 'N', 'B', 'H', 'G', 'Y', '&', 0,
  0, 0, 'M', 'J', 'U', '\'', '(', 0,
  0, '<', 'K', 'I', 'O', 0, ')', 0,
  0, '>', '?', 'L', '+', 'P', '=', 0,
  0, '_', '*', 0, '`', '~', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '{', 0, '}', 0, 0,
  0, 0, 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', '|', '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  0
};
#endif

#define BREAK     0x01
#define MODIFIER  0x02
#define SHIFT_L   0x04
#define SHIFT_R   0x08
#define ALTGR     0x10

static char get_iso8859_code(void)
{
  static uint8_t state=0;
  uint8_t s;
  char c;

  while (1) {
    s = get_scan_code();
    if (!s) 
      return 0;
    if (s == 0xF0) {
      state |= BREAK;
    } else if (s == 0xE0) {
      state |= MODIFIER;
    } else {
      if (state & BREAK) {
        if (s == 0x12) {
          state &= ~SHIFT_L;
        } else if (s == 0x59) {
          state &= ~SHIFT_R;
        } else if (s == 0x11 && (state & MODIFIER)) {
          state &= ~ALTGR;
        }
        // CTRL, ALT & WIN keys could be added
        // but is that really worth the overhead?
        state &= ~(BREAK | MODIFIER);
        continue;
      }
      if (s == 0x12) {
        state |= SHIFT_L;
        continue;
      } else if (s == 0x59) {
        state |= SHIFT_R;
        continue;
      } else if (s == 0x11 && (state & MODIFIER)) {
        state |= ALTGR;
      } else if (s == 0x58 && (state & SHIFT_L)) {
        CapsLock = !CapsLock;
      }
      c = 0;
      if (state & MODIFIER) {
        switch (s) {
          case 0x70: 
            c = PS2_INSERT;
            break;
          case 0x6C: 
            c = PS2_HOME;
            break;
          case 0x7D: 
            c = PS2_PAGEUP;      
            break;
          case 0x71: 
            c = PS2_DELETE;      
            break;
          case 0x69: 
            c = PS2_END;         
            break;
          case 0x7A: 
            c = PS2_PAGEDOWN;    
            break;
          case 0x75: 
            c = PS2_UPARROW;     
            break;
          case 0x6B: 
            c = PS2_LEFTARROW;   
            break;
          case 0x72: 
            c = PS2_DOWNARROW;   
            break;
          case 0x74: 
            c = PS2_RIGHTARROW;  
            break;
          case 0x4A: 
            c = '/';             
            break;
          case 0x5A: 
            c = PS2_ENTER;        
            break;
          default: 
            break;
        }
      } else {
        bool IsAlpha = false;
        switch (s) {
          case 0x15: case 0x1A: case 0x1B: case 0x1C:
          case 0x1D: case 0x21: case 0x22: case 0x23:
          case 0x24: case 0x2A: case 0x2B: case 0x2C:
          case 0x2D: case 0x31: case 0x32: case 0x33:
          case 0x34: case 0x35: case 0x3A: case 0x3B:
          case 0x3C: case 0x42: case 0x43: case 0x44:
          case 0x4B: case 0x4D:
            IsAlpha = true;
            break;
        }                                                                    
        if (s < PS2_KEYMAP_SIZE) {                                           
          uint8_t addr;
          if ((state & ALTGR) && keymap->uses_altgr) 
            addr = 2;
          else if (state & (SHIFT_L | SHIFT_R))
            addr = (CapsLock && IsAlpha) ? 0 : 1;
          else
            addr = (CapsLock && IsAlpha) ? 1 : 0;
          switch (addr) {
            case 1: 
              c = keymap->shift[s];
              break;
            case 2:
              c = keymap->altgr[s];
              break;
            default: 
              c = keymap->noshift[s];
              break;
          }
        }
      }
      state &= ~(BREAK | MODIFIER);
      if (c) 
        return c;
    }
  }
}

bool kbd_available() {
  if (CharBuffer || UTF8next) 
    return true;
  CharBuffer = get_iso8859_code();
  if (CharBuffer) 
    return true;
  return false;
}

int kbd_read() {
  uint8_t result;

  result = UTF8next;
  if (result) {
    UTF8next = 0;
  } else {
    result = CharBuffer;
    if (result) {
      CharBuffer = 0;
    } else {
      result = get_iso8859_code();
    }
    if (result >= 128) {
      UTF8next = (result & 0x3F) | 0x80;
      result = ((result >> 6) & 0x1F) | 0xC0;
    }
  }
  if (!result) 
    return -1;
  return result;
}

void kbd_init(void) {

  keymap = &PS2Keymap_Japanese;

  GPIO_InitTypeDef GPIO_InitStructure = { 0 };
  EXTI_InitTypeDef EXTI_InitStructure = { 0 };
  NVIC_InitTypeDef NVIC_InitStructure = { 0 };

  RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC, ENABLE);

  // PC1:Clock

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init( GPIOC, &GPIO_InitStructure);

  // PC2: Data

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init( GPIOC, &GPIO_InitStructure);

  // Set EXTI

  /* GPIOA ----> EXTI_Line0 */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource1);
  EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  head = 0;
  tail = 0;

}


