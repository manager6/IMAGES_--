#ifndef _PHOTO_H_
#define _PHOTO_H_

#include "list.h"

/*设备地址*/
#define LCD_DEV_PATH "/dev/fb0"
#define TOUCH_LCD_DEV_PATH "/dev/input/event0"

/*界面选择模式 mode*/
#define MAIN_MODE 1           // 主界面
#define IMAGES_MODE 2         // 图片界面
#define CATALOG_SEARCH_MODE 3 // 检索界面
#define PREVIOUS_IMAGE_MODE 4 // 上一张图片
#define NEXT_IMAGE_MODE 5     // 下一张图片
#define THUMBNAIL_MODE 6      // 缩略图模式
#define EXIT_MODE 7           // 退出当前 返回主界面
#define EXIT_SYSTEM_MODE 8    // 退出系统

/*屏幕像素字节大小*/
#define BMP_PIXEL_BYTES 3
#define LCD_PIXEL_BYTES 4

/************函数声明********/
PI *pro_init();                                                                           // 初始化结构体
IMAGE *link_init();                                                                       // 初始化链表
int select_interface(PI *pro_inf, IMAGE *image_inf);                                      // 界面切换
int display_image(PI *pro_inf, IMAGE *obj_image, char *file_path);                        // 显示图片
int touch_screen(PI *pro_inf);                                                            // 检测屏幕触摸
int thumbnail_display(IMAGE *image_inf, PI *pro_inf, const char *dir_path, int max_cols); // 缩小图片
int display_thumbnail(PI *pro_inf, const char *bmp_file_path, int x_offset, int y_offset, int thumb_width, int thumb_height);
int insert_address(IMAGE *obj_image, const char *file_path); // 节点插入图片
char *traverse_linked_list(IMAGE *obj_image);                // 遍历链表
int is_bmp_file(const char *file_path);                      // 判断是否是BMP图片
void cleanup(PI *pro_inf, IMAGE *img_inf);                   // 清除空间
int loading_interface(PI *pro_inf, int progress);            // 加载界面
#endif