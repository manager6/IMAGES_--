#include "photo.h"
#include "retrieval.h"

int main()
{
    // 初始化硬件和系统相关结构体
    PI *pro_inf = pro_init();
    IMAGE *img_inf = link_init();

    // 检查初始化是否成功
    if (pro_inf == (PI *)-1 || img_inf == (IMAGE *)-1)
    {
        fprintf(stderr, "Initialization failed!\n");
        cleanup(pro_inf, img_inf);
        return -1;
    }

    // 插入图片地址
    if (insert_address(img_inf, "images") == -1)
    {
        fprintf(stderr, "Failed to insert image addresses!\n");
        cleanup(pro_inf, img_inf);
        return -1;
    }

    // 显示加载图片
    display_image(pro_inf, img_inf, "load.bmp");

    // 显示加载进度
    for (int i = 0; i <= 100; i++)
    {
        usleep(10000);                 // 延时 10 毫秒
        loading_interface(pro_inf, i); // 更新加载进度界面
    }
    sleep(1);

    // 进入选择界面，等待用户操作
    select_interface(pro_inf, img_inf);

    // 释放资源
    cleanup(pro_inf, img_inf);

    return 0;
}
