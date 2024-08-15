#include "retrieval.h"

DI *font_init()
{
    DI *dis_font = (DI *)malloc(sizeof(DI));
    if (dis_font == NULL)
    {
        perror("malloc dis font ...");
        return (DI *)-1;
    }

    dis_font->font_lib_c = open(CHINESE_FONT_FILE_PATH, O_RDONLY);
    dis_font->font_lib_l = open(LETTER_FONT_FILE_PATH, O_RDONLY);

    if (dis_font->font_lib_c == -1 || dis_font->font_lib_l == -1)
    {
        perror("open font_lib fail ...");
        free(dis_font);
        return (DI *)-1;
    }

    dis_font->font_lib_size_c = lseek(dis_font->font_lib_c, 0, SEEK_END);
    dis_font->font_lib_size_l = lseek(dis_font->font_lib_l, 0, SEEK_END);

    lseek(dis_font->font_lib_c, 0, SEEK_SET);
    lseek(dis_font->font_lib_l, 0, SEEK_SET);

    dis_font->font_mmap_start_c = (char *)mmap(NULL, dis_font->font_lib_size_c, PROT_READ, MAP_SHARED, dis_font->font_lib_c, 0);
    dis_font->font_mmap_start_l = (char *)mmap(NULL, dis_font->font_lib_size_l, PROT_READ, MAP_SHARED, dis_font->font_lib_l, 0);

    if (dis_font->font_mmap_start_c == MAP_FAILED || dis_font->font_mmap_start_l == MAP_FAILED)
    {
        perror("mmap fail ...");
        close(dis_font->font_lib_c);
        close(dis_font->font_lib_l);
        free(dis_font);
        return (DI *)-1;
    }

    return dis_font;
}

int Display_Ascii(PI *pro_inf, int where_x, int where_y, char *string)
{
    DI *obj_font = font_init();
    int len = strlen(string);
    char character;
    for (int lp = 0; lp < len; lp++)
    {
        character = string[lp];
        int offset = character * 72;
        char *new_dzk_mmap = obj_font->font_mmap_start_l + offset;
        int *new_lcd_mmap = pro_inf->mmap_init_point + (pro_inf->LCD_W) * where_y + where_x;
        char datatype;
        for (int y = 0; y < 24; y++)
        {
            for (int x = 0; x < 24 / 8; x++)
            {
                datatype = *(new_dzk_mmap + (24 / 8) * y + x);
                for (int z = 0; z < 8; z++)
                {
                    if (datatype & 0x80 >> z)
                    {
                        *(new_lcd_mmap + (pro_inf->LCD_W * y + 8 * x + z)) = 0xffffff;
                    }
                }
            }
        }
        where_x += 24;
    }
    if (close(obj_font->font_lib_l) == -1)
    {
        perror("close failed ");
        return -1;
    }
    if (munmap(obj_font->font_mmap_start_l, obj_font->font_lib_size_l) == -1)
    {
        perror("mumap failed");
        return -1;
    }
    free(obj_font);
    return 0;
}

int Display_Chinese(PI *pro_inf, int where_x, int where_y, char *string)
{
    DI *obj_font = font_init();
    int len = strlen(string);
    char *fontl;
    for (int lp = 0; lp < len; lp += 2)
    {
        fontl = string + lp;
        int offset = (94 * (fontl[0] - 1 - 0xa0) + (fontl[1] - 1 - 0xa0)) * 72;
        char *new_dzk_mmap = obj_font->font_mmap_start_c + offset;
        int *new_lcd_mmap = pro_inf->mmap_init_point + (pro_inf->LCD_W) * where_y + where_x;
        char datatype;
        for (int y = 0; y < 24; y++)
        {
            for (int x = 0; x < 24 / 8; x++)
            {
                datatype = *(new_dzk_mmap + (24 / 8) * y + x);
                for (int z = 0; z < 8; z++)
                {
                    if (datatype & 0x80 >> z)
                    {
                        *(new_lcd_mmap + (pro_inf->LCD_W * y + 8 * x + z)) = 0xffffff;
                    }
                }
            }
        }
        where_x += 24;
    }
    if (close(obj_font->font_lib_c) == -1)
    {
        perror("close failed ");
        return -1;
    }
    if (munmap(obj_font->font_mmap_start_c, obj_font->font_lib_size_c) == -1)
    {
        perror("mumap failed");
        return -1;
    }
    free(obj_font);

    return 0;
}
