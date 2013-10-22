#ifndef LCM100_H
#define LCM100_H

#define LCD_CMD                 254
#define LCD_CLEAR               1
#define LCD_HOME_CURSOR		2
#define LCD_BLANK_SCREEN	8
#define LCD_HIDE_CURSOR      	12
#define LCD_ENABLE_BLOCK_CURSOR	13
#define LCD_ENABLE_UNDER_CURSOR	14
#define LCD_CURSOR_LEFT		16
#define LCD_CURSOR_RIGHT	20
#define LCD_SCROLL_LEFT		24
#define LCD_SCROLL_RIGHT	28
/*
 * 64~127 character-generator address
 * 128~144 Line 1 address
 * 192~208 Line 2 address
 */

#define LCM100_CELLHGT		8
#define LCM100_CELLWID		5

#define LCM100_UP_KEY 'A'
#define LCM100_DOWN_KEY 'B'
#define LCM100_LEFT_KEY 'D'
#define LCM100_RIGHT_KEY 'H'
#define LCM100_ENTER_KEY 'P'

#endif
