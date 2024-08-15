#ifndef _RETRIEVAL_H_
#define _RETRIEVAL_H_

#include "photo.h"

/*******字库 *******/
#define CHINESE_FONT_FILE_PATH "DZK24.Dzk"     // 汉字
#define LETTER_FONT_FILE_PATH "dzk_ascill.Dzk" // 字母数字

DI *font_init();

int Display_Ascii(PI *pro_inf, int where_x, int where_y, char *string);
int Display_Chinese(PI *pro_inf, int where_x, int where_y, char *string);

#endif