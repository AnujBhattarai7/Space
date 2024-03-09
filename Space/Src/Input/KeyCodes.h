#pragma once

#include "RepeatCodes.h"

/* Printable keys */
#define SP_KEY_SPACE 32
#define SP_KEY_APOSTROPHE 39 /* ' */
#define SP_KEY_COMMA 44      /* , */
#define SP_KEY_MINUS 45      /* - */
#define SP_KEY_PERIOD 46     /* . */
#define SP_KEY_SLASH 47      /* / */
#define SP_KEY_0 48
#define SP_KEY_1 49
#define SP_KEY_2 50
#define SP_KEY_3 51
#define SP_KEY_4 52
#define SP_KEY_5 53
#define SP_KEY_6 54
#define SP_KEY_7 55
#define SP_KEY_8 56
#define SP_KEY_9 57
#define SP_KEY_SEMICOLON 59 /* ; */
#define SP_KEY_EQUAL 61     /* = */
#define SP_KEY_A 65
#define SP_KEY_B 66
#define SP_KEY_C 67
#define SP_KEY_D 68
#define SP_KEY_E 69
#define SP_KEY_F 70
#define SP_KEY_G 71
#define SP_KEY_H 72
#define SP_KEY_I 73
#define SP_KEY_J 74
#define SP_KEY_K 75
#define SP_KEY_L 76
#define SP_KEY_M 77
#define SP_KEY_N 78
#define SP_KEY_O 79
#define SP_KEY_P 80
#define SP_KEY_Q 81
#define SP_KEY_R 82
#define SP_KEY_S 83
#define SP_KEY_T 84
#define SP_KEY_U 85
#define SP_KEY_V 86
#define SP_KEY_W 87
#define SP_KEY_X 88
#define SP_KEY_Y 89
#define SP_KEY_Z 90
#define SP_KEY_LEFT_BRACKET 91  /* [ */
#define SP_KEY_BACKSLASH 92     /* \ */
#define SP_KEY_RIGHT_BRACKET 93 /* ] */
#define SP_KEY_GRAVE_ACCENT 96  /* ` */
#define SP_KEY_WORLD_1 161      /* non-US #1 */
#define SP_KEY_WORLD_2 162      /* non-US #2 */

/* Function keys */
#define SP_KEY_ESCAPE 256
#define SP_KEY_ENTER 257
#define SP_KEY_TAB 258
#define SP_KEY_BACKSPACE 259
#define SP_KEY_INSERT 260
#define SP_KEY_DELETE 261
#define SP_KEY_RIGHT 262
#define SP_KEY_LEFT 263
#define SP_KEY_DOWN 264
#define SP_KEY_UP 265
#define SP_KEY_PAGE_UP 266
#define SP_KEY_PAGE_DOWN 267
#define SP_KEY_HOME 268
#define SP_KEY_END 269
#define SP_KEY_CAPS_LOCK 280
#define SP_KEY_SCROLL_LOCK 281
#define SP_KEY_NUM_LOCK 282
#define SP_KEY_PRINT_SCREEN 283
#define SP_KEY_PAUSE 284
#define SP_KEY_F1 290
#define SP_KEY_F2 291
#define SP_KEY_F3 292
#define SP_KEY_F4 293
#define SP_KEY_F5 294
#define SP_KEY_F6 295
#define SP_KEY_F7 296
#define SP_KEY_F8 297
#define SP_KEY_F9 298
#define SP_KEY_F10 299
#define SP_KEY_F11 300
#define SP_KEY_F12 301
#define SP_KEY_F13 302
#define SP_KEY_F14 303
#define SP_KEY_F15 304
#define SP_KEY_F16 305
#define SP_KEY_F17 306
#define SP_KEY_F18 307
#define SP_KEY_F19 308
#define SP_KEY_F20 309
#define SP_KEY_F21 310
#define SP_KEY_F22 311
#define SP_KEY_F23 312
#define SP_KEY_F24 313
#define SP_KEY_F25 314
#define SP_KEY_KP_0 320
#define SP_KEY_KP_1 321
#define SP_KEY_KP_2 322
#define SP_KEY_KP_3 323
#define SP_KEY_KP_4 324
#define SP_KEY_KP_5 325
#define SP_KEY_KP_6 326
#define SP_KEY_KP_7 327
#define SP_KEY_KP_8 328
#define SP_KEY_KP_9 329
#define SP_KEY_KP_DECIMAL 330
#define SP_KEY_KP_DIVIDE 331
#define SP_KEY_KP_MULTIPLY 332
#define SP_KEY_KP_SUBTRACT 333
#define SP_KEY_KP_ADD 334
#define SP_KEY_KP_ENTER 335
#define SP_KEY_KP_EQUAL 336
#define SP_KEY_LEFT_SHIFT 340
#define SP_KEY_LEFT_CONTROL 341
#define SP_KEY_LEFT_ALT 342
#define SP_KEY_LEFT_SUPER 343
#define SP_KEY_RIGHT_SHIFT 344
#define SP_KEY_RIGHT_CONTROL 345
#define SP_KEY_RIGHT_ALT 346
#define SP_KEY_RIGHT_SUPER 347
#define SP_KEY_MENU 348

#define SP_KEY_LAST SP_KEY_MENU

#define SP_MOD_SHIFT 0x0001
#define SP_MOD_CONTROL 0x0002
#define SP_MOD_ALT 0x0004
#define SP_MOD_SUPER 0x0008
#define SP_MOD_CAPS_LOCK 0x0010
#define SP_MOD_NUM_LOCK 0x0020

namespace Space
{
    static const char *GetKeyName(int key)
    {
        switch (key)
        {
            // Printable keys
        case SP_KEY_A:
            return "A";
        case SP_KEY_B:
            return "B";
        case SP_KEY_C:
            return "C";
        case SP_KEY_D:
            return "D";
        case SP_KEY_E:
            return "E";
        case SP_KEY_F:
            return "F";
        case SP_KEY_G:
            return "G";
        case SP_KEY_H:
            return "H";
        case SP_KEY_I:
            return "I";
        case SP_KEY_J:
            return "J";
        case SP_KEY_K:
            return "K";
        case SP_KEY_L:
            return "L";
        case SP_KEY_M:
            return "M";
        case SP_KEY_N:
            return "N";
        case SP_KEY_O:
            return "O";
        case SP_KEY_P:
            return "P";
        case SP_KEY_Q:
            return "Q";
        case SP_KEY_R:
            return "R";
        case SP_KEY_S:
            return "S";
        case SP_KEY_T:
            return "T";
        case SP_KEY_U:
            return "U";
        case SP_KEY_V:
            return "V";
        case SP_KEY_W:
            return "W";
        case SP_KEY_X:
            return "X";
        case SP_KEY_Y:
            return "Y";
        case SP_KEY_Z:
            return "Z";
        case SP_KEY_1:
            return "1";
        case SP_KEY_2:
            return "2";
        case SP_KEY_3:
            return "3";
        case SP_KEY_4:
            return "4";
        case SP_KEY_5:
            return "5";
        case SP_KEY_6:
            return "6";
        case SP_KEY_7:
            return "7";
        case SP_KEY_8:
            return "8";
        case SP_KEY_9:
            return "9";
        case SP_KEY_0:
            return "0";
        case SP_KEY_SPACE:
            return "SPACE";
        case SP_KEY_MINUS:
            return "MINUS";
        case SP_KEY_EQUAL:
            return "EQUAL";
        case SP_KEY_LEFT_BRACKET:
            return "LEFT BRACKET";
        case SP_KEY_RIGHT_BRACKET:
            return "RIGHT BRACKET";
        case SP_KEY_BACKSLASH:
            return "BACKSLASH";
        case SP_KEY_SEMICOLON:
            return "SEMICOLON";
        case SP_KEY_APOSTROPHE:
            return "APOSTROPHE";
        case SP_KEY_GRAVE_ACCENT:
            return "GRAVE ACCENT";
        case SP_KEY_COMMA:
            return "COMMA";
        case SP_KEY_PERIOD:
            return "PERIOD";
        case SP_KEY_SLASH:
            return "SLASH";
        case SP_KEY_WORLD_1:
            return "WORLD 1";
        case SP_KEY_WORLD_2:
            return "WORLD 2";

            // Function keys
        case SP_KEY_ESCAPE:
            return "ESCAPE";
        case SP_KEY_F1:
            return "F1";
        case SP_KEY_F2:
            return "F2";
        case SP_KEY_F3:
            return "F3";
        case SP_KEY_F4:
            return "F4";
        case SP_KEY_F5:
            return "F5";
        case SP_KEY_F6:
            return "F6";
        case SP_KEY_F7:
            return "F7";
        case SP_KEY_F8:
            return "F8";
        case SP_KEY_F9:
            return "F9";
        case SP_KEY_F10:
            return "F10";
        case SP_KEY_F11:
            return "F11";
        case SP_KEY_F12:
            return "F12";
        case SP_KEY_F13:
            return "F13";
        case SP_KEY_F14:
            return "F14";
        case SP_KEY_F15:
            return "F15";
        case SP_KEY_F16:
            return "F16";
        case SP_KEY_F17:
            return "F17";
        case SP_KEY_F18:
            return "F18";
        case SP_KEY_F19:
            return "F19";
        case SP_KEY_F20:
            return "F20";
        case SP_KEY_F21:
            return "F21";
        case SP_KEY_F22:
            return "F22";
        case SP_KEY_F23:
            return "F23";
        case SP_KEY_F24:
            return "F24";
        case SP_KEY_F25:
            return "F25";
        case SP_KEY_UP:
            return "UP";
        case SP_KEY_DOWN:
            return "DOWN";
        case SP_KEY_LEFT:
            return "LEFT";
        case SP_KEY_RIGHT:
            return "RIGHT";
        case SP_KEY_LEFT_SHIFT:
            return "LEFT SHIFT";
        case SP_KEY_RIGHT_SHIFT:
            return "RIGHT SHIFT";
        case SP_KEY_LEFT_CONTROL:
            return "LEFT CONTROL";
        case SP_KEY_RIGHT_CONTROL:
            return "RIGHT CONTROL";
        case SP_KEY_LEFT_ALT:
            return "LEFT ALT";
        case SP_KEY_RIGHT_ALT:
            return "RIGHT ALT";
        case SP_KEY_TAB:
            return "TAB";
        case SP_KEY_ENTER:
            return "ENTER";
        case SP_KEY_BACKSPACE:
            return "BACKSPACE";
        case SP_KEY_INSERT:
            return "INSERT";
        case SP_KEY_DELETE:
            return "DELETE";
        case SP_KEY_PAGE_UP:
            return "PAGE UP";
        case SP_KEY_PAGE_DOWN:
            return "PAGE DOWN";
        case SP_KEY_HOME:
            return "HOME";
        case SP_KEY_END:
            return "END";
        case SP_KEY_KP_0:
            return "KEYPAD 0";
        case SP_KEY_KP_1:
            return "KEYPAD 1";
        case SP_KEY_KP_2:
            return "KEYPAD 2";
        case SP_KEY_KP_3:
            return "KEYPAD 3";
        case SP_KEY_KP_4:
            return "KEYPAD 4";
        case SP_KEY_KP_5:
            return "KEYPAD 5";
        case SP_KEY_KP_6:
            return "KEYPAD 6";
        case SP_KEY_KP_7:
            return "KEYPAD 7";
        case SP_KEY_KP_8:
            return "KEYPAD 8";
        case SP_KEY_KP_9:
            return "KEYPAD 9";
        case SP_KEY_KP_DIVIDE:
            return "KEYPAD DIVIDE";
        case SP_KEY_KP_MULTIPLY:
            return "KEYPAD MULTIPLY";
        case SP_KEY_KP_SUBTRACT:
            return "KEYPAD SUBTRACT";
        case SP_KEY_KP_ADD:
            return "KEYPAD ADD";
        case SP_KEY_KP_DECIMAL:
            return "KEYPAD DECIMAL";
        case SP_KEY_KP_EQUAL:
            return "KEYPAD EQUAL";
        case SP_KEY_KP_ENTER:
            return "KEYPAD ENTER";
        case SP_KEY_PRINT_SCREEN:
            return "PRINT SCREEN";
        case SP_KEY_NUM_LOCK:
            return "NUM LOCK";
        case SP_KEY_CAPS_LOCK:
            return "CAPS LOCK";
        case SP_KEY_SCROLL_LOCK:
            return "SCROLL LOCK";
        case SP_KEY_PAUSE:
            return "PAUSE";
        case SP_KEY_LEFT_SUPER:
            return "LEFT SUPER";
        case SP_KEY_RIGHT_SUPER:
            return "RIGHT SUPER";
        case SP_KEY_MENU:
            return "MENU";
        default:
            return "UNKNOWN";
        }
    }

    static const char *GetModName(int Mod)
    {
        switch (Mod)
        {
        case SP_MOD_SHIFT:
            return "SHIFT";
        case SP_MOD_ALT:
            return "ALT";
        case SP_MOD_CAPS_LOCK:
            return "CAPS_LOCK";
        case SP_MOD_CONTROL:
            return "CONTROL";
        case SP_MOD_NUM_LOCK:
            return "NUM_LOCK";
        case SP_MOD_SUPER:
            return "SUPER";
        }
    }

    static const char *GetModName(KeyMods Mod)
    {
        switch (Mod)
        {
        case KeyMods::None:
            return " ";
        case KeyMods::SHIFT:
            return "SHIFT ";
        case KeyMods::ALT:
            return "ALT ";
        case KeyMods::CAPS_LOCK:
            return "CAPS_LOCK ";
        case KeyMods::CONTROL:
            return "CONTROL ";
        case KeyMods::NUM_LOCK:
            return "NUM_LOCK ";
        case KeyMods::SUPER:
            return "SUPER ";
        default:
            return " ";
        }
    }
}
