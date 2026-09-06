/****************************************************************************
 * (c)LGPL2+
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtCore module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/

#ifndef KEYSYM_TABLE_H
#define KEYSYM_TABLE_H

// (davidsansome) Nicked from qkeymapper_x11.cpp

#include <Qt>

// X11 keysym values and libxkbcommon keysym values are the same numbers
// (xkbcommon's keysyms header is generated from the same upstream
// keysymdef.h X11 uses) - pulling the table's XKB_KEY_* values from here instead
// keeps this file (and the rest of hotkeys/, which is otherwise X11-free
// since the portal-based rewrite) off libX11 entirely. Unlike X11's own
// keysymdef.h, xkbcommon's header has no MISCELLANY/LATIN1/KOREAN/XKB_KEYS
// feature gates - everything below is always available unconditionally.
#include <xkbcommon/xkbcommon-keysyms.h>

//
// Keyboard event translation
//

// the next lines are taken from XFree > 4.0 (X11/XF86keysyms.h), defining some special
// multimedia keys. They are included here as not every system has them.
#define XF86XK_Standby          0x1008FF10
#define XF86XK_AudioLowerVolume 0x1008FF11
#define XF86XK_AudioMute        0x1008FF12
#define XF86XK_AudioRaiseVolume 0x1008FF13
#define XF86XK_AudioPlay        0x1008FF14
#define XF86XK_AudioStop        0x1008FF15
#define XF86XK_AudioPrev        0x1008FF16
#define XF86XK_AudioNext        0x1008FF17
#define XF86XK_HomePage         0x1008FF18
#define XF86XK_Calculator       0x1008FF1D
#define XF86XK_Mail             0x1008FF19
#define XF86XK_Start            0x1008FF1A
#define XF86XK_Search           0x1008FF1B
#define XF86XK_AudioRecord      0x1008FF1C
#define XF86XK_Back             0x1008FF26
#define XF86XK_Forward          0x1008FF27
#define XF86XK_Stop             0x1008FF28
#define XF86XK_Refresh          0x1008FF29
#define XF86XK_Favorites        0x1008FF30
#define XF86XK_AudioPause       0x1008FF31
#define XF86XK_AudioMedia       0x1008FF32
#define XF86XK_MyComputer       0x1008FF33
#define XF86XK_OpenURL          0x1008FF38
#define XF86XK_Launch0          0x1008FF40
#define XF86XK_Launch1          0x1008FF41
#define XF86XK_Launch2          0x1008FF42
#define XF86XK_Launch3          0x1008FF43
#define XF86XK_Launch4          0x1008FF44
#define XF86XK_Launch5          0x1008FF45
#define XF86XK_Launch6          0x1008FF46
#define XF86XK_Launch7          0x1008FF47
#define XF86XK_Launch8          0x1008FF48
#define XF86XK_Launch9          0x1008FF49
#define XF86XK_LaunchA          0x1008FF4A
#define XF86XK_LaunchB          0x1008FF4B
#define XF86XK_LaunchC          0x1008FF4C
#define XF86XK_LaunchD          0x1008FF4D
#define XF86XK_LaunchE          0x1008FF4E
#define XF86XK_LaunchF          0x1008FF4F


/* Backlight controls. */
#ifndef XF86XK_MonBrightnessUp
#define XF86XK_MonBrightnessUp   0x1008FF02  /* Monitor/panel brightness */
#endif

#ifndef XF86XK_MonBrightnessDown
#define XF86XK_MonBrightnessDown 0x1008FF03  /* Monitor/panel brightness */
#endif

#ifndef XF86XK_KbdLightOnOff
#define XF86XK_KbdLightOnOff     0x1008FF04  /* Keyboards may be lit     */
#endif

#ifndef XF86XK_KbdBrightnessUp
#define XF86XK_KbdBrightnessUp   0x1008FF05  /* Keyboards may be lit     */
#endif

#ifndef XF86XK_KbdBrightnessDown
#define XF86XK_KbdBrightnessDown 0x1008FF06  /* Keyboards may be lit     */
#endif

// end of XF86keysyms.h

// Special keys used by Qtopia, mapped into the X11 private keypad range.
#define QTOPIAXK_Select         0x11000601
#define QTOPIAXK_Yes            0x11000602
#define QTOPIAXK_No             0x11000603
#define QTOPIAXK_Cancel         0x11000604
#define QTOPIAXK_Printer        0x11000605
#define QTOPIAXK_Execute        0x11000606
#define QTOPIAXK_Sleep          0x11000607
#define QTOPIAXK_Play           0x11000608
#define QTOPIAXK_Zoom           0x11000609
#define QTOPIAXK_Context1       0x1100060A
#define QTOPIAXK_Context2       0x1100060B
#define QTOPIAXK_Context3       0x1100060C
#define QTOPIAXK_Context4       0x1100060D
#define QTOPIAXK_Call           0x1100060E
#define QTOPIAXK_Hangup         0x1100060F
#define QTOPIAXK_Flip           0x11000610

// keyboard mapping table
static const unsigned int KeyTbl[] = {

    // misc keys
    XKB_KEY_comma,                   Qt::Key_Comma,
    XKB_KEY_period,                  Qt::Key_Period,
    XKB_KEY_space,                   Qt::Key_Space,
    XKB_KEY_Escape,                  Qt::Key_Escape,
    XKB_KEY_Tab,                     Qt::Key_Tab,
    XKB_KEY_ISO_Left_Tab,            Qt::Key_Backtab,
    XKB_KEY_BackSpace,               Qt::Key_Backspace,
    XKB_KEY_Return,                  Qt::Key_Return,
    XKB_KEY_Insert,                  Qt::Key_Insert,
    XKB_KEY_Delete,                  Qt::Key_Delete,
    XKB_KEY_Clear,                   Qt::Key_Delete,
    XKB_KEY_Pause,                   Qt::Key_Pause,
    XKB_KEY_Print,                   Qt::Key_Print,
    0x1005FF60,                 Qt::Key_SysReq,         // hardcoded Sun SysReq
    0x1007ff00,                 Qt::Key_SysReq,         // hardcoded X386 SysReq

    // cursor movement

    XKB_KEY_Home,                    Qt::Key_Home,
    XKB_KEY_End,                     Qt::Key_End,
    XKB_KEY_Left,                    Qt::Key_Left,
    XKB_KEY_Up,                      Qt::Key_Up,
    XKB_KEY_Right,                   Qt::Key_Right,
    XKB_KEY_Down,                    Qt::Key_Down,
    XKB_KEY_Prior,                   Qt::Key_PageUp,
    XKB_KEY_Next,                    Qt::Key_PageDown,

    // modifiers

    XKB_KEY_Shift_L,                 Qt::Key_Shift,
    XKB_KEY_Shift_R,                 Qt::Key_Shift,
    XKB_KEY_Shift_Lock,              Qt::Key_Shift,
    XKB_KEY_Control_L,               Qt::Key_Control,
    XKB_KEY_Control_R,               Qt::Key_Control,
    XKB_KEY_Meta_L,                  Qt::Key_Meta,
    XKB_KEY_Meta_R,                  Qt::Key_Meta,
    XKB_KEY_Alt_L,                   Qt::Key_Alt,
    XKB_KEY_Alt_R,                   Qt::Key_Alt,
    XKB_KEY_Caps_Lock,               Qt::Key_CapsLock,
    XKB_KEY_Num_Lock,                Qt::Key_NumLock,
    XKB_KEY_Scroll_Lock,             Qt::Key_ScrollLock,
    XKB_KEY_Super_L,                 Qt::Key_Super_L,
    XKB_KEY_Super_R,                 Qt::Key_Super_R,
    XKB_KEY_Menu,                    Qt::Key_Menu,
    XKB_KEY_Hyper_L,                 Qt::Key_Hyper_L,
    XKB_KEY_Hyper_R,                 Qt::Key_Hyper_R,
    XKB_KEY_Help,                    Qt::Key_Help,
    0x1000FF74,                 Qt::Key_Backtab,        // hardcoded HP backtab
    0x1005FF10,                 Qt::Key_F11,            // hardcoded Sun F36 (labeled F11)
    0x1005FF11,                 Qt::Key_F12,            // hardcoded Sun F37 (labeled F12)

    // numeric and function keypad keys
    XKB_KEY_KP_Tab,                  Qt::Key_Tab,
    XKB_KEY_KP_Enter,                Qt::Key_Enter,
    //XKB_KEY_KP_F1,                 Qt::Key_F1,
    //XKB_KEY_KP_F2,                 Qt::Key_F2,
    //XKB_KEY_KP_F3,                 Qt::Key_F3,
    //XKB_KEY_KP_F4,                 Qt::Key_F4,
    XKB_KEY_KP_Home,                 Qt::Key_Home,
    XKB_KEY_KP_Left,                 Qt::Key_Left,
    XKB_KEY_KP_Up,                   Qt::Key_Up,
    XKB_KEY_KP_Right,                Qt::Key_Right,
    XKB_KEY_KP_Down,                 Qt::Key_Down,
    XKB_KEY_KP_Prior,                Qt::Key_PageUp,
    XKB_KEY_KP_Next,                 Qt::Key_PageDown,
    XKB_KEY_KP_End,                  Qt::Key_End,
    XKB_KEY_KP_Begin,                Qt::Key_Clear,
    XKB_KEY_KP_Insert,               Qt::Key_Insert,
    XKB_KEY_KP_Delete,               Qt::Key_Delete,
    XKB_KEY_KP_Equal,                Qt::Key_Equal,
    XKB_KEY_KP_Multiply,             Qt::Key_Asterisk,
    XKB_KEY_KP_Add,                  Qt::Key_Plus,
    XKB_KEY_KP_Subtract,             Qt::Key_Minus,
    XKB_KEY_KP_Divide,               Qt::Key_Slash,

    // International input method support keys

    // International & multi-key character composition
    XKB_KEY_ISO_Level3_Shift,        Qt::Key_AltGr,
    XKB_KEY_Multi_key,		Qt::Key_Multi_key,
    XKB_KEY_Codeinput,		Qt::Key_Codeinput,
    XKB_KEY_SingleCandidate,		Qt::Key_SingleCandidate,
    XKB_KEY_MultipleCandidate,	Qt::Key_MultipleCandidate,
    XKB_KEY_PreviousCandidate,	Qt::Key_PreviousCandidate,

    // Misc Functions
    XKB_KEY_Mode_switch,		Qt::Key_Mode_switch,
    XKB_KEY_script_switch,		Qt::Key_Mode_switch,

    // Japanese keyboard support
    XKB_KEY_Kanji,			Qt::Key_Kanji,
    XKB_KEY_Muhenkan,		Qt::Key_Muhenkan,
    //XKB_KEY_Henkan_Mode,		Qt::Key_Henkan_Mode,
    XKB_KEY_Henkan_Mode,		Qt::Key_Henkan,
    XKB_KEY_Henkan,			Qt::Key_Henkan,
    XKB_KEY_Romaji,			Qt::Key_Romaji,
    XKB_KEY_Hiragana,		Qt::Key_Hiragana,
    XKB_KEY_Katakana,		Qt::Key_Katakana,
    XKB_KEY_Hiragana_Katakana,	Qt::Key_Hiragana_Katakana,
    XKB_KEY_Zenkaku,			Qt::Key_Zenkaku,
    XKB_KEY_Hankaku,			Qt::Key_Hankaku,
    XKB_KEY_Zenkaku_Hankaku,		Qt::Key_Zenkaku_Hankaku,
    XKB_KEY_Touroku,			Qt::Key_Touroku,
    XKB_KEY_Massyo,			Qt::Key_Massyo,
    XKB_KEY_Kana_Lock,		Qt::Key_Kana_Lock,
    XKB_KEY_Kana_Shift,		Qt::Key_Kana_Shift,
    XKB_KEY_Eisu_Shift,		Qt::Key_Eisu_Shift,
    XKB_KEY_Eisu_toggle,		Qt::Key_Eisu_toggle,
    //XKB_KEY_Kanji_Bangou,		Qt::Key_Kanji_Bangou,
    //XKB_KEY_Zen_Koho,		Qt::Key_Zen_Koho,
    //XKB_KEY_Mae_Koho,		Qt::Key_Mae_Koho,
    XKB_KEY_Kanji_Bangou,		Qt::Key_Codeinput,
    XKB_KEY_Zen_Koho,		Qt::Key_MultipleCandidate,
    XKB_KEY_Mae_Koho,		Qt::Key_PreviousCandidate,

    // Korean keyboard support
    XKB_KEY_Hangul,			Qt::Key_Hangul,
    XKB_KEY_Hangul_Start,		Qt::Key_Hangul_Start,
    XKB_KEY_Hangul_End,		Qt::Key_Hangul_End,
    XKB_KEY_Hangul_Hanja,		Qt::Key_Hangul_Hanja,
    XKB_KEY_Hangul_Jamo,		Qt::Key_Hangul_Jamo,
    XKB_KEY_Hangul_Romaja,		Qt::Key_Hangul_Romaja,
    //XKB_KEY_Hangul_Codeinput,	Qt::Key_Hangul_Codeinput,
    XKB_KEY_Hangul_Codeinput,	Qt::Key_Codeinput,
    XKB_KEY_Hangul_Jeonja,		Qt::Key_Hangul_Jeonja,
    XKB_KEY_Hangul_Banja,		Qt::Key_Hangul_Banja,
    XKB_KEY_Hangul_PreHanja,		Qt::Key_Hangul_PreHanja,
    XKB_KEY_Hangul_PostHanja,	Qt::Key_Hangul_PostHanja,
    //XKB_KEY_Hangul_SingleCandidate,Qt::Key_Hangul_SingleCandidate,
    //XKB_KEY_Hangul_MultipleCandidate,Qt::Key_Hangul_MultipleCandidate,
    //XKB_KEY_Hangul_PreviousCandidate,Qt::Key_Hangul_PreviousCandidate,
    XKB_KEY_Hangul_SingleCandidate,	Qt::Key_SingleCandidate,
    XKB_KEY_Hangul_MultipleCandidate,Qt::Key_MultipleCandidate,
    XKB_KEY_Hangul_PreviousCandidate,Qt::Key_PreviousCandidate,
    XKB_KEY_Hangul_Special,		Qt::Key_Hangul_Special,
    //XKB_KEY_Hangul_switch,		Qt::Key_Hangul_switch,
    XKB_KEY_Hangul_switch,		Qt::Key_Mode_switch,

    // dead keys
    XKB_KEY_dead_grave,              Qt::Key_Dead_Grave,
    XKB_KEY_dead_acute,              Qt::Key_Dead_Acute,
    XKB_KEY_dead_circumflex,         Qt::Key_Dead_Circumflex,
    XKB_KEY_dead_tilde,              Qt::Key_Dead_Tilde,
    XKB_KEY_dead_macron,             Qt::Key_Dead_Macron,
    XKB_KEY_dead_breve,              Qt::Key_Dead_Breve,
    XKB_KEY_dead_abovedot,           Qt::Key_Dead_Abovedot,
    XKB_KEY_dead_diaeresis,          Qt::Key_Dead_Diaeresis,
    XKB_KEY_dead_abovering,          Qt::Key_Dead_Abovering,
    XKB_KEY_dead_doubleacute,        Qt::Key_Dead_Doubleacute,
    XKB_KEY_dead_caron,              Qt::Key_Dead_Caron,
    XKB_KEY_dead_cedilla,            Qt::Key_Dead_Cedilla,
    XKB_KEY_dead_ogonek,             Qt::Key_Dead_Ogonek,
    XKB_KEY_dead_iota,               Qt::Key_Dead_Iota,
    XKB_KEY_dead_voiced_sound,       Qt::Key_Dead_Voiced_Sound,
    XKB_KEY_dead_semivoiced_sound,   Qt::Key_Dead_Semivoiced_Sound,
    XKB_KEY_dead_belowdot,           Qt::Key_Dead_Belowdot,
    XKB_KEY_dead_hook,               Qt::Key_Dead_Hook,
    XKB_KEY_dead_horn,               Qt::Key_Dead_Horn,

    // Special multimedia keys
    // currently only tested with MS internet keyboard

    // browsing keys
    XF86XK_Back,                Qt::Key_Back,
    XF86XK_Forward,             Qt::Key_Forward,
    XF86XK_Stop,                Qt::Key_Stop,
    XF86XK_Refresh,             Qt::Key_Refresh,
    XF86XK_Favorites,           Qt::Key_Favorites,
    XF86XK_AudioMedia,          Qt::Key_LaunchMedia,
    XF86XK_OpenURL,             Qt::Key_OpenUrl,
    XF86XK_HomePage,            Qt::Key_HomePage,
    XF86XK_Search,              Qt::Key_Search,

    // media keys
    XF86XK_AudioLowerVolume,    Qt::Key_VolumeDown,
    XF86XK_AudioMute,           Qt::Key_VolumeMute,
    XF86XK_AudioRaiseVolume,    Qt::Key_VolumeUp,
    XF86XK_AudioPlay,           Qt::Key_MediaPlay,
    XF86XK_AudioStop,           Qt::Key_MediaStop,
    XF86XK_AudioPrev,           Qt::Key_MediaPrevious,
    XF86XK_AudioNext,           Qt::Key_MediaNext,
    XF86XK_AudioRecord,         Qt::Key_MediaRecord,

    XF86XK_MonBrightnessUp,     Qt::Key_MonBrightnessUp,
    XF86XK_MonBrightnessDown,   Qt::Key_MonBrightnessDown,
    XF86XK_KbdBrightnessUp,     Qt::Key_KeyboardBrightnessUp,
    XF86XK_KbdBrightnessDown,   Qt::Key_KeyboardBrightnessDown,

    // launch keys
    XF86XK_Mail,                Qt::Key_LaunchMail,
    XF86XK_MyComputer,          Qt::Key_Launch0,
    XF86XK_Calculator,          Qt::Key_Launch1,
    XF86XK_Standby,             Qt::Key_Standby,

    XF86XK_Launch0,             Qt::Key_Launch2,
    XF86XK_Launch1,             Qt::Key_Launch3,
    XF86XK_Launch2,             Qt::Key_Launch4,
    XF86XK_Launch3,             Qt::Key_Launch5,
    XF86XK_Launch4,             Qt::Key_Launch6,
    XF86XK_Launch5,             Qt::Key_Launch7,
    XF86XK_Launch6,             Qt::Key_Launch8,
    XF86XK_Launch7,             Qt::Key_Launch9,
    XF86XK_Launch8,             Qt::Key_LaunchA,
    XF86XK_Launch9,             Qt::Key_LaunchB,
    XF86XK_LaunchA,             Qt::Key_LaunchC,
    XF86XK_LaunchB,             Qt::Key_LaunchD,
    XF86XK_LaunchC,             Qt::Key_LaunchE,
    XF86XK_LaunchD,             Qt::Key_LaunchF,

    // Qtopia keys
    QTOPIAXK_Select,            Qt::Key_Select,
    QTOPIAXK_Yes,               Qt::Key_Yes,
    QTOPIAXK_No,                Qt::Key_No,
    QTOPIAXK_Cancel,            Qt::Key_Cancel,
    QTOPIAXK_Printer,           Qt::Key_Printer,
    QTOPIAXK_Execute,           Qt::Key_Execute,
    QTOPIAXK_Sleep,             Qt::Key_Sleep,
    QTOPIAXK_Play,              Qt::Key_Play,
    QTOPIAXK_Zoom,              Qt::Key_Zoom,
    QTOPIAXK_Context1,          Qt::Key_Context1,
    QTOPIAXK_Context2,          Qt::Key_Context2,
    QTOPIAXK_Context3,          Qt::Key_Context3,
    QTOPIAXK_Context4,          Qt::Key_Context4,
    QTOPIAXK_Call,              Qt::Key_Call,
    QTOPIAXK_Hangup,            Qt::Key_Hangup,
    QTOPIAXK_Flip,              Qt::Key_Flip,

    0,                          0
};

#endif // KEYSYM_TABLE_H
